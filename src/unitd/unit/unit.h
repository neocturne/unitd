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

#pragma once

#include <libubox/list.h>
#include <libubox/uloop.h>

#include <stdbool.h>


typedef struct unitd_unit unitd_unit_t;


typedef enum unitd_job_type {
	JOB_TYPE_NONE = 0,
	JOB_TYPE_ACTIVATE,
	JOB_TYPE_DEACTIVATE,
} unitd_job_type_t;


typedef struct unitd_dep {
	unitd_unit_t *from;		/**< requires/wants/conflicts/after */
	unitd_unit_t *to;		/**< required-by/wanted-by/conflicted-by/before */

	struct list_head list_from;	/**< List head for the "from" list */
	struct list_head list_to;	/**< List head for the "to" list */
} unitd_dep_t;


typedef enum unitd_unit_type {
	UNIT_TYPE_TARGET,
	UNIT_TYPE_SERVICE,
} unitd_unit_type_t;


typedef enum unitd_load_state {
	LOAD_STATE_NOT_FOUND = 0,
	LOAD_STATE_LOADED,
} unitd_load_state_t;


typedef enum unitd_unit_state {
	UNIT_STATE_INACTIVE = 0,
	UNIT_STATE_ACTIVE,
	UNIT_STATE_RELOADING,
	UNIT_STATE_FAILED,
	UNIT_STATE_ACTIVATING,
	UNIT_STATE_DEACTIVATING,
} unitd_unit_state_t;


struct unitd_unit {
	/* Static part of unit */
	unitd_unit_type_t type;
	unitd_load_state_t loaded;

	struct list_head list;
	char *name;

	struct list_head requires;
	struct list_head required_by;
	struct list_head wants;
	struct list_head wanted_by;
	struct list_head conflicts;
	struct list_head conflicted_by;
	struct list_head after;
	struct list_head before;

	/* Dynamic part of unit */
	unitd_unit_state_t state;

	unitd_job_type_t pending_type;
	struct list_head pending_list;

	unitd_job_type_t transaction_type;
	struct list_head transaction_list;
};


typedef enum unitd_service_type {
	SERVICE_TYPE_SIMPLE,
	SERVICE_TYPE_FORKING,
	SERVICE_TYPE_ONESHOT,
	SERVICE_TYPE_NOTIFY,
} unitd_service_type_t;

typedef struct unitd_service {
	unitd_unit_t unit;

	/* Service options */
	unitd_service_type_t type;
	char **ExecStart;

	/* Instance state */
	struct uloop_process proc;
} unitd_service_t;


typedef struct unitd_transaction {
	struct list_head jobs;
} unitd_transaction_t;


extern struct list_head unitd_pending_units;


int unitd_unit_activate(unitd_unit_t *unit);
int unitd_unit_deactivate(unitd_unit_t *unit);

void unitd_unit_wakeup_pending(void);

void unitd_service_start(unitd_service_t *service);
void unitd_service_stop(unitd_service_t *service);


static inline void unitd_unit_add_pending(unitd_unit_t *unit, unitd_job_type_t type) {
	if (unit->pending_type)
		list_del(&unit->pending_list);

	unit->pending_type = type;
	list_add_tail(&unit->pending_list, &unitd_pending_units);
}
