/*
  Copyright (c) 2015, Matthias Schiffer <mschiffer@universe-factory.net>
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "../log.h"
#include "unit.h"

#include <errno.h>
#include <string.h>


static int queue_activate(unitd_unit_t *unit, unitd_transaction_t *t, bool warn);
static int queue_deactivate(unitd_unit_t *unit, unitd_transaction_t *t, bool warn);


static int do_queue(unitd_unit_t *unit, unitd_transaction_t *t, unitd_job_type_t type) {
	if (unit->transaction_type) {
		if (unit->transaction_type == type)
			return EALREADY;
		else
			return EBUSY;
	}

	unit->transaction_type = type;
	list_add_tail(&unit->transaction_list, &t->jobs);

	return 0;
}

static int queue_activate_dep(unitd_unit_t *unit, unitd_unit_t *dep, unitd_transaction_t *t,
	                      bool warn, const char *action_desc, const char *dep_desc) {
	LOG("Activating unit %s %s %s\n", dep->name, dep_desc, unit->name);

	int err = queue_activate(dep, t, warn);

	if (err && warn)
		WARN("Unable to %s %s: activating unit %s failed: %s\n",
		     action_desc, unit->name, dep->name, strerror(err));

	return err;
}

static int queue_deactivate_dep(unitd_unit_t *unit, unitd_unit_t *dep, unitd_transaction_t *t,
	                      bool warn, const char *action_desc, const char *dep_desc) {
	LOG("Deactivating unit %s %s %s\n", dep->name, dep_desc, unit->name);

	int err = queue_deactivate(dep, t, warn);

	if (err && warn)
		WARN("Unable to %s %s: deactivating unit %s failed: %s\n",
		     action_desc, unit->name, dep->name, strerror(err));

	return err;
}

static int queue_activate(unitd_unit_t *unit, unitd_transaction_t *t, bool warn) {
	int err;

	if (unit->loaded != LOAD_STATE_LOADED)
		return ENOENT;

	err = do_queue(unit, t, JOB_TYPE_ACTIVATE);
	if (err == EALREADY)
		return 0;
	else if (err)
		return err;

	unitd_dep_t *dep;
	list_for_each_entry(dep, &unit->requires, list_from) {
		err = queue_activate_dep(unit, dep->to, t, warn, "activate", "required by");
		if (err)
			return err;
	}

	list_for_each_entry(dep, &unit->conflicts, list_from) {
		err = queue_deactivate_dep(unit, dep->to, t, warn, "activate", "conflicting with");
		if (err)
			return err;
	}

	list_for_each_entry(dep, &unit->conflicted_by, list_to) {
		err = queue_deactivate_dep(unit, dep->from, t, warn, "activate", "conflicting with");
		if (err)
			return err;
	}

	return 0;
}

static int queue_deactivate(unitd_unit_t *unit, unitd_transaction_t *t, bool warn) {
	int err;

	if (unit->loaded != LOAD_STATE_LOADED)
		return ENOENT;

	err = do_queue(unit, t, JOB_TYPE_DEACTIVATE);
	if (err == EALREADY)
		return 0;
	else if (err)
		return err;

	unitd_dep_t *dep;
	list_for_each_entry(dep, &unit->required_by, list_to) {
		err = queue_deactivate_dep(unit, dep->from, t, warn, "deactivate", "requiring");
		if (err)
			return err;
	}

	return 0;
}

static void clear_transaction(unitd_transaction_t *t) {
	unitd_unit_t *unit;

	list_for_each_entry(unit, &t->jobs, transaction_list)
		unit->transaction_type = JOB_TYPE_NONE;
}

static void handle_unit_wants(unitd_unit_t *unit, unitd_transaction_t *t) {
	unitd_dep_t *dep;
	list_for_each_entry(dep, &unit->wants, list_from) {
		unitd_transaction_t sub;
		INIT_LIST_HEAD(&sub.jobs);

		LOG("Activating unit %s wanted by %s\n",
		    dep->to->name, unit->name);
		int err = queue_activate(dep->to, &sub, false);
		if (!err)
			list_splice_tail(&sub.jobs, &t->jobs);
		else
			clear_transaction(&sub);
	}
}

static void handle_wants(unitd_transaction_t *t) {
	unitd_unit_t *unit;

	list_for_each_entry(unit, &t->jobs, transaction_list) {
		if (unit->transaction_type == JOB_TYPE_ACTIVATE)
			handle_unit_wants(unit, t);
	}
}

static void commit_transaction(unitd_transaction_t *t) {
	unitd_unit_t *unit;

	list_for_each_entry(unit, &t->jobs, transaction_list)
                unitd_unit_add_pending(unit, unit->transaction_type);
}

int unitd_unit_activate(unitd_unit_t *unit) {
	unitd_transaction_t t;
	INIT_LIST_HEAD(&t.jobs);

	int err = queue_activate(unit, &t, true);

	if (!err) {
		handle_wants(&t);
		commit_transaction(&t);
	}

	clear_transaction(&t);

	if (!err)
		unitd_unit_wakeup_pending();

	return err;
}

int unitd_unit_deactivate(unitd_unit_t *unit) {
	unitd_transaction_t t;
	INIT_LIST_HEAD(&t.jobs);

	int err = queue_deactivate(unit, &t, true);

	if (!err)
		commit_transaction(&t);

	clear_transaction(&t);

	if (!err)
		unitd_unit_wakeup_pending();

	return err;
}
