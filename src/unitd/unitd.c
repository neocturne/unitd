/*
 * Copyright (C) 2015 Matthias Schiffer <mschiffer@universe-factory.net>
 *
 * Based on "procd" by:
 * Copyright (C) 2013 Felix Fietkau <nbd@openwrt.org>
 * Copyright (C) 2013 John Crispin <blogic@openwrt.org>
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

#include <sys/types.h>
#include <sys/prctl.h>

#include <errno.h>
#include <unistd.h>


unsigned int debug = 4;


int main()
{
	static char unitd[] = "unitd";

	if (getpid() != 1) {
		fprintf(stderr, "error: must run as PID 1\n");
		return 1;
	}

	program_invocation_short_name = unitd;
	prctl(PR_SET_NAME, unitd);

	ulog_open(ULOG_KMSG, LOG_DAEMON, "unitd");

	setsid();
	uloop_init();
	unitd_signal();
	unitd_state_next();
	uloop_run();
	uloop_done();

	return 0;
}
