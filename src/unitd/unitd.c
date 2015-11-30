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
#include <getopt.h>
#include <string.h>
#include <unistd.h>


unsigned int debug = 4;

static int usage(const char *prog)
{
	ERROR("Usage: %s [options]\n"
		"Options:\n"
		"\t-s <path>\tPath to ubus socket\n"
		"\t-d <level>\tEnable debug messages\n"
		"\n", prog);
	return 1;
}

int main(int argc, char **argv)
{
	static char unitd[] = "unitd";
	int ch;

	if (getpid() != 1) {
		fprintf(stderr, "error: must run as PID 1\n");
		return 1;
	}

	program_invocation_short_name = unitd;
	prctl(PR_SET_NAME, unitd);

	ulog_open(ULOG_KMSG, LOG_DAEMON, "unitd");

	while ((ch = getopt(argc, argv, "d:s:")) != -1) {
		switch (ch) {
		case 's':
			ubus_socket = optarg;
			break;
		case 'd':
			debug = atoi(optarg);
			break;
		default:
			return usage(argv[0]);
		}
	}
	setsid();
	uloop_init();
	unitd_signal();
	unitd_state_next();
	uloop_run();
	uloop_done();

	return 0;
}
