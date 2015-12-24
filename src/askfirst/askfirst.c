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

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
	int c;

	ioctl(STDIN_FILENO, TIOCSCTTY, 1);
	tcsetpgrp(STDIN_FILENO, getpid());

	printf("Please press Enter to activate this console.\n");
	do {
		c = getchar();
		if (c == EOF)
			return -1;
	} while (c != 0x0a);

	if (argc < 2) {
		printf("%s needs to be called with at least 1 parameter\n", argv[0]);
		return -1;
	}

	execvp(argv[1], &argv[1]);
	printf("Failed to execute %s\n", argv[1]);

	return -1;
}
