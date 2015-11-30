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

#pragma once

#include <libubox/avl.h>
#include <libubox/vlist.h>
#include <libubox/list.h>

extern struct avl_tree services;

struct vrule {
	struct avl_node avl;
	char *option;
	char *rule;
};

struct service {
	struct avl_node avl;
	const char *name;

	struct blob_attr *trigger;
	struct vlist_tree instances;
};

int service_start_early(char *name, char *cmdline);
void service_init(void);
void service_event(const char *type, const char *service, const char *instance);
