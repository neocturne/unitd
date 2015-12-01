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

#include "log.h"

#include <libubox/uloop.h>
#include <libubox/utils.h>
#include <libubus.h>

#include <stdio.h>
#include <syslog.h>


void unitd_early(void);

void unitd_connect_ubus(void);
void unitd_reconnect_ubus(int reconnect);
void ubus_init_service(struct ubus_context *ctx);
void ubus_init_system(struct ubus_context *ctx);

void unitd_state_next(void);
void unitd_state_ubus_connect(void);
void unitd_shutdown(int event);
void unitd_early(void);
void unitd_preinit(void);
void unitd_coldplug(void);
void unitd_signal(void);
void unitd_signal_preinit(void);
void unitd_askconsole(void);
void unitd_bcast_event(char *event, struct blob_attr *msg);
