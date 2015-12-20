/*
  Copyright (c) 2015, Matthias Schiffer <mschiffer@universe-factory.net>
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "../log.h"
#include "unit.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


static void on_service_exit(struct uloop_process *p, int ret) {
        unitd_service_t *service = container_of(p, unitd_service_t, proc);

        LOG("Process %u of service %s exited with status %i\n", (unsigned)p->pid, service->unit.name, ret);

        /* TODO: Set to failed on failure */
        service->unit.state = UNIT_STATE_INACTIVE;

        /* TODO: Make restart conditional */
	unitd_unit_activate(&service->unit);
}

static void service_exec(unitd_service_t *service) {
        if (setsid() < 0) {
                ERROR("Unable to start service %s: setsid: %s\n", service->unit.name, strerror(errno));
                return;
        }

        execv(service->ExecStart[0], service->ExecStart);
}

static bool service_run(unitd_service_t *service) {
        service->proc.cb = on_service_exit;
        service->proc.pid = fork();

        if (service->proc.pid < 0) {
                ERROR("Unable to start service %s: fork: %s\n", service->unit.name, strerror(errno));
                return false;
        }

        if (service->proc.pid == 0) {
                uloop_done();
                service_exec(service);
                _exit(127);
        }

        uloop_process_add(&service->proc);
        return true;
}

void unitd_service_start(unitd_service_t *service) {
        switch (service->type) {
        case SERVICE_TYPE_SIMPLE:
                if (service_run(service))
		        service->unit.state = UNIT_STATE_ACTIVE;
                else
                        service->unit.state = UNIT_STATE_FAILED;

                return;

        case SERVICE_TYPE_FORKING:
        case SERVICE_TYPE_ONESHOT:
        case SERVICE_TYPE_NOTIFY:
		service->unit.state = UNIT_STATE_ACTIVATING;
                return;

        default:
                BUG("invalid service type");
        }
}

void unitd_service_stop(unitd_service_t *service) {
	service->unit.state = UNIT_STATE_DEACTIVATING;
}
