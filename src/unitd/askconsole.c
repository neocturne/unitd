/*
 * Copyright (C) 2015 Matthias Schiffer <mschiffer@universe-factory.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "unitd.h"
#include "unit/unit.h"

#include <sys/types.h>

#include <fcntl.h>
#include <unistd.h>


static unitd_service_t service_askconsole = {
	.unit = {
		.type = UNIT_TYPE_SERVICE,
		.loaded = LOAD_STATE_LOADED,
		.name = "askconsole.service",

		.state = UNIT_STATE_INACTIVE,
	},

	.type = SERVICE_TYPE_SIMPLE,
	.ExecStart = (char *[]){
		"/lib/unitd/askfirst",
		"/bin/ash",
		"--login",
		NULL,
	},

	.proc = {},
};


static void init_unit(unitd_unit_t *unit) {
	INIT_LIST_HEAD(&unit->requires);
	INIT_LIST_HEAD(&unit->required_by);
	INIT_LIST_HEAD(&unit->wants);
	INIT_LIST_HEAD(&unit->wanted_by);
	INIT_LIST_HEAD(&unit->conflicts);
	INIT_LIST_HEAD(&unit->conflicted_by);
	INIT_LIST_HEAD(&unit->after);
	INIT_LIST_HEAD(&unit->before);
}


void unitd_askconsole(void) {
	init_unit(&service_askconsole.unit);

	unitd_unit_activate(&service_askconsole.unit);
}
