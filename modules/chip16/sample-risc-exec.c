/*
  sample-risc-exec.c - sample code for executing instructions

  This Software is part of Wind River Simics. The rights to copy, distribute,
  modify, or otherwise make use of this Software may be licensed only
  pursuant to the terms of an applicable Wind River license agreement.
  
  Copyright 2010-2014 Intel Corporation

*/

#include "sample-risc-exec.h"
#include "event-queue.h"
#ifndef SAMPLE_RISC_HEADER
#define SAMPLE_RISC_HEADER "sample-risc.h"
#endif
#include SAMPLE_RISC_HEADER
#include "sample-risc-queue.h"
#include <simics/processor-api.h>

static void
check_event_queues(sample_risc_t *sr)
{
        handle_events(sr, &sr->cycle_queue);
        if (sr->current_core->enabled != 0) {
                handle_events(sr, &sr->step_queue);
        }
}

static void
exec_run(conf_object_t *obj)
{
        sample_risc_t *sr = conf_obj_to_sr(obj);

        sr->cell_iface->set_current_processor_obj(
                sr->cell, sr_core_to_conf_obj(sr->current_core));
        sr->cell_iface->set_current_step_obj(sr->cell, obj);

        SIM_LOG_INFO(2, sr_to_conf_obj(sr), 0, "running");
        sr->state = State_Running;

        /* handle all events on the current step and cycle */
        check_event_queues(sr);

        while (sr->state == State_Running) {
                if (sr->current_core->enabled != 0) {
                        sample_core_fetch_and_execute_instruction(
                                sr->current_core);
                } else {
                        simtime_t delta = get_delta(&sr->cycle_queue);
                        if (delta > 0) {
                                sample_core_increment_cycles(sr->current_core,
                                                             delta);
                        }
                }
                if (sr->state == State_Stopped) // breakpoint triggered
                        break;

                /* handle all events on the current step and cycle */
                check_event_queues(sr);
        }

        /* simulator has been requested to stop */
        if (sr->state == State_Stopped)
                SIM_LOG_INFO(2, sr_to_conf_obj(sr), 0, "stop execution");
}

static void
exec_stop(conf_object_t *obj)
{
        sample_risc_t *sr = conf_obj_to_sr(obj);
        SIM_LOG_INFO(2, sr_to_conf_obj(sr), 0, "stop");
        sr->state = State_Stopped;
}

void
register_execute_interface(conf_class_t *cls)
{
        static const execute_interface_t execute_iface = {
                .run = exec_run,
                .stop = exec_stop
        };
        SIM_register_interface(cls, EXECUTE_INTERFACE, &execute_iface);
}
