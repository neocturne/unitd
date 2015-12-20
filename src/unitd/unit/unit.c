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


LIST_HEAD(unitd_pending_units);


static bool activate_service(unitd_unit_t *unit) {
	switch (unit->state) {
	case UNIT_STATE_ACTIVATING:
	case UNIT_STATE_ACTIVE:
	case UNIT_STATE_RELOADING:
		return true;

	case UNIT_STATE_DEACTIVATING:
		return false;

	case UNIT_STATE_INACTIVE:
	case UNIT_STATE_FAILED:
		unitd_service_start(container_of(unit, unitd_service_t, unit));
		return true;

	default:
		BUG("invalid service state");
	}
}

static bool deactivate_service(unitd_unit_t *unit) {
	switch (unit->state) {
	case UNIT_STATE_DEACTIVATING:
	case UNIT_STATE_INACTIVE:
	case UNIT_STATE_FAILED:
		return true;

	case UNIT_STATE_ACTIVATING:
	case UNIT_STATE_RELOADING:
		return false;

	case UNIT_STATE_ACTIVE:
		unitd_service_stop(container_of(unit, unitd_service_t, unit));
		return true;

	default:
		BUG("invalid service state");
	}
}

static bool do_activate(unitd_unit_t *unit) {
	LOG("Will now start unit %s\n", unit->name);

	switch (unit->type) {
	case UNIT_TYPE_TARGET:
		unit->state = UNIT_STATE_ACTIVE;
		return true;

	case UNIT_TYPE_SERVICE:
		return activate_service(unit);

	default:
		BUG("invalid service type");
	}
}

static bool do_deactivate(unitd_unit_t *unit) {
	LOG("Will now stop unit %s\n", unit->name);

	switch (unit->type) {
	case UNIT_TYPE_TARGET:
		unit->state = UNIT_STATE_INACTIVE;
		return true;

	case UNIT_TYPE_SERVICE:
		return deactivate_service(unit);

	default:
		BUG("invalid service type");
	}
}

static void exec_pending(unitd_unit_t *unit) {
	unitd_dep_t *dep;
	switch (unit->pending_type) {
	case JOB_TYPE_NONE:
		BUG("invalid job type in pending queue");

	case JOB_TYPE_ACTIVATE:
		list_for_each_entry(dep, &unit->after, list_from) {
			unitd_unit_t *other = dep->to;
			/* We wait for both activating and deactivating jobs */
			if (other->pending_type || other->state == UNIT_STATE_ACTIVATING ||
			    other->state == UNIT_STATE_DEACTIVATING)
				return;
		}

		if (!do_activate(unit))
			return;

		break;

	case JOB_TYPE_DEACTIVATE:
		list_for_each_entry(dep, &unit->before, list_to) {
			unitd_unit_t *other = dep->from;
			/* We care only about deactivating jobs here */
			if (other->pending_type == JOB_TYPE_DEACTIVATE ||
			    other->state == UNIT_STATE_DEACTIVATING)
				return;
		}

		if (!do_deactivate(unit))
			return;
	}

	unit->pending_type = JOB_TYPE_NONE;
	list_del(&unit->pending_list);
}

void unitd_unit_wakeup_pending(void) {
	unitd_unit_t *unit, *next;

	list_for_each_entry_safe(unit, next, &unitd_pending_units, pending_list)
		exec_pending(unit);
}
