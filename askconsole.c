/*
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
#include <sys/ioctl.h>

#include <fcntl.h>
#include <unistd.h>


static void askconsole(struct uloop_process *proc) {
	char *const ask[] = {
		"/sbin/askfirst",
		"/bin/ash",
		"--login",
		NULL,
	};

	pid_t p;

	proc->pid = fork();
	if (!proc->pid) {
		p = setsid();

		fcntl(STDERR_FILENO, F_SETFL, fcntl(STDERR_FILENO, F_GETFL) & ~O_NONBLOCK);

		ioctl(STDIN_FILENO, TIOCSCTTY, 1);
		tcsetpgrp(STDIN_FILENO, p);

		execvp(ask[0], ask);
		ERROR("Failed to execute %s\n", ask[0]);
		exit(-1);
	}

	if (proc->pid > 0) {
		DEBUG(4, "Launched askconsole, pid=%d\n",
					(int) proc->pid);
		uloop_process_add(proc);
	}
}

static void child_exit(struct uloop_process *proc, int ret)
{
	DEBUG(4, "pid:%d\n", proc->pid);
	askconsole(proc);
}

void unitd_askconsole(void) {
	struct uloop_process *proc = malloc(sizeof(*proc));
	proc->cb = child_exit;
	askconsole(proc);
}
