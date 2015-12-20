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

#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>


static void
early_console(const char *dev)
{
	struct stat s;
	int dd;

	if (stat(dev, &s)) {
		ERROR("Failed to stat %s\n", dev);
		return;
	}

	dd = open(dev, O_RDWR);
	if (dd < 0)
		dd = open("/dev/null", O_RDWR);

	dup2(dd, STDIN_FILENO);
	dup2(dd, STDOUT_FILENO);
	dup2(dd, STDERR_FILENO);

	if (dd != STDIN_FILENO &&
	    dd != STDOUT_FILENO &&
	    dd != STDERR_FILENO)
		close(dd);
}

static void
early_mounts(void)
{
	unsigned int oldumask = umask(0);

	mount("proc", "/proc", "proc", MS_NOATIME | MS_NODEV | MS_NOEXEC | MS_NOSUID, NULL);
	mount("sys", "/sys", "sysfs", MS_NOATIME | MS_NODEV | MS_NOEXEC | MS_NOSUID, NULL);
	mount("cgroup", "/sys/fs/cgroup", "cgroup",  MS_NODEV | MS_NOEXEC | MS_NOSUID, NULL);
	mount("dev", "/dev", "devtmpfs", MS_NOATIME | MS_NOSUID, "mode=0755,size=512K");

	mkdir("/dev/pts", 0755);
	mount("devpts", "/dev/pts", "devpts", MS_NOATIME | MS_NOEXEC | MS_NOSUID, "mode=600");

	mount("run", "/run", "tmpfs", MS_NOATIME, "mode=0755");

	mkdir("/run/tmp", 01777);
	mount("/run/tmp", "/tmp", NULL, MS_BIND, NULL);

	mkdir("/run/shm", 01777);
	mkdir("/dev/shm", 0755);
	mount("/run/shm", "/dev/shm", NULL, MS_BIND, NULL);

	umask(oldumask);
}

static void
early_env(void)
{
	setenv("PATH", "/bin:/sbin:/usr/bin:/usr/sbin", 1);
}

void
unitd_early(void)
{
	early_mounts();
	early_console("/dev/console");
	early_env();

	LOG("Console is alive\n");
}
