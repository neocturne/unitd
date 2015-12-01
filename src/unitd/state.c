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
#include "syslog.h"
#include "utils.h"
#include "service/service.h"

#include <fcntl.h>
#include <sys/reboot.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>


enum {
	STATE_NONE = 0,
	STATE_EARLY,
	STATE_RUNNING,
	STATE_SHUTDOWN,
	__STATE_MAX,
};

static int state = STATE_NONE;
static int reboot_event;

static void set_stdio(const char* tty)
{
	if (chdir("/dev") ||
	    !freopen(tty, "r", stdin) ||
	    !freopen(tty, "w", stdout) ||
	    !freopen(tty, "w", stderr) ||
	    chdir("/"))
		ERROR("failed to set stdio\n");
	else
		fcntl(STDERR_FILENO, F_SETFL, fcntl(STDERR_FILENO, F_GETFL) | O_NONBLOCK);
}

static void set_console(void)
{
	const char* tty;
	char* split;
	char line[ 20 ];
	const char* try[] = { "tty0", "console", NULL }; /* Try the most common outputs */
	int f, i = 0;

	tty = get_cmdline_val("console",line,sizeof(line));
	if (tty != NULL) {
		split = strchr(tty, ',');
		if ( split != NULL )
			*split = '\0';
	} else {
		// Try a default
		tty=try[i];
		i++;
	}

	if (chdir("/dev")) {
		ERROR("failed to change dir to /dev\n");
		return;
	}
	while (tty!=NULL) {
		f = open(tty, O_RDONLY);
		if (f >= 0) {
			close(f);
			break;
		}

		tty=try[i];
		i++;
	}
	if (chdir("/"))
		ERROR("failed to change dir to /\n");

	if (tty != NULL)
		set_stdio(tty);
}

static void state_enter(void)
{
	char ubus_cmd[] = "/sbin/ubusd";

	switch (state) {
	case STATE_EARLY:
		LOG("- early -\n");
		unitd_early();
		unitd_connect_ubus();
		service_init();
		service_start_early("ubus", ubus_cmd);
		break;

	case STATE_RUNNING:
		LOG("- init -\n");
		unitd_askconsole();

		// switch to syslog log channel
		ulog_open(ULOG_SYSLOG, LOG_DAEMON, "unitd");

		LOG("- init complete -\n");
		break;

	case STATE_SHUTDOWN:
		/* Redirect output to the console for the users' benefit */
		set_console();
		LOG("- shutdown -\n");
		sync();
		// To prevent killed processes from interrupting the sleep
		signal(SIGCHLD, SIG_IGN);
		LOG("- SIGTERM processes -\n");
		kill(-1, SIGTERM);
		sync();
		sleep(1);
		LOG("- SIGKILL processes -\n");
		kill(-1, SIGKILL);
		sync();
		sleep(1);
		if (reboot_event == RB_POWER_OFF)
			LOG("- power down -\n");
		else
			LOG("- reboot -\n");

		/* Allow time for last message to reach serial console, etc */
		sleep(1);

		/* We have to fork here, since the kernel calls do_exit(EXIT_SUCCESS)
		 * in linux/kernel/sys.c, which can cause the machine to panic when
		 * the init process exits... */
		if (!vfork( )) { /* child */
			reboot(reboot_event);
			_exit(EXIT_SUCCESS);
		}

		while (1)
			sleep(1);
		break;

	default:
		ERROR("Unhandled state %d\n", state);
		return;
	};
}

void unitd_state_next(void)
{
	DEBUG(4, "Change state %d -> %d\n", state, state + 1);
	state++;
	state_enter();
}

void unitd_state_ubus_connect(void)
{
	if (state == STATE_EARLY)
		unitd_state_next();
}

void unitd_shutdown(int event)
{
	if (state >= STATE_SHUTDOWN)
		return;
	DEBUG(2, "Shutting down system with event %x\n", event);
	reboot_event = event;
	state = STATE_SHUTDOWN;
	state_enter();
}
