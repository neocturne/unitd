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

#include <libubox/vlist.h>
#include <libubox/uloop.h>
#include <libubox/ustream.h>
#include "../utils/utils.h"

#define RESPAWN_ERROR	(5 * 60)

struct jail {
	bool procfs;
	bool sysfs;
	bool ubus;
	bool log;
	char *name;
	char *root;
	struct blobmsg_list mount;
	int argc;
};

struct service_instance {
	struct vlist_node node;
	struct service *srv;
	const char *name;

	int8_t nice;
	bool valid;

	uid_t uid;
	gid_t gid;

	bool halt;
	bool restart;
	bool respawn;
	int respawn_count;
	struct timespec start;

	uint32_t respawn_timeout;
	uint32_t respawn_threshold;
	uint32_t respawn_retry;

	struct blob_attr *config;
	struct uloop_process proc;
	struct uloop_timeout timeout;
	struct ustream_fd _stdout;
	struct ustream_fd _stderr;

	struct blob_attr *command;
	struct blobmsg_list env;
	struct blobmsg_list data;
	struct blobmsg_list netdev;
	struct blobmsg_list file;
	struct blobmsg_list limits;
	struct blobmsg_list errors;
};

void instance_start(struct service_instance *in);
void instance_stop(struct service_instance *in);
bool instance_update(struct service_instance *in, struct service_instance *in_new);
void instance_init(struct service_instance *in, struct service *s, struct blob_attr *config);
void instance_free(struct service_instance *in);
void instance_dump(struct blob_buf *b, struct service_instance *in, int debug);
