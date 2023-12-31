/*
  chip16-exec.c - sample code for executing instructions

  This Software is part of Wind River Simics. The rights to copy, distribute,
  modify, or otherwise make use of this Software may be licensed only
  pursuant to the terms of an applicable Wind River license agreement.
  
  Copyright 2010-2014 Intel Corporation

*/

#include "chip16-exec.h"
#include "event-queue.h"
#ifndef CHIP16_HEADER
#define CHIP16_HEADER "chip16.h"
#endif
#include CHIP16_HEADER
#include "chip16-queue.h"
#include <simics/processor-api.h>

static void
check_event_queues(chip16_t *sr)
{
        handle_events(sr, &sr->cycle_queue);
        if (sr->enabled != 0) {
                handle_events(sr, &sr->step_queue);
        }
}

static void
exec_run(conf_object_t *obj)
{
        chip16_t *sr = conf_to_chip16(obj);

        sr->cell_iface->set_current_processor_obj(
                sr->cell, chip16_to_conf(sr));
        sr->cell_iface->set_current_step_obj(sr->cell, obj);

        SIM_LOG_INFO(2, chip16_to_conf(sr), 0, "running");
        sr->state = State_Running;

        /* handle all events on the current step and cycle */
        check_event_queues(sr);

        while (sr->state == State_Running) {
                if (sr->enabled != 0) {
                        chip16_fetch_and_execute_instruction(
                                sr);
                } else {
                        simtime_t delta = get_delta(&sr->cycle_queue);
                        if (delta > 0) {
                                chip16_increment_cycles(sr,
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
                SIM_LOG_INFO(2, chip16_to_conf(sr), 0, "stop execution");
}

static void
exec_stop(conf_object_t *obj)
{
        chip16_t *sr = conf_to_chip16(obj);
        SIM_LOG_INFO(2, chip16_to_conf(sr), 0, "stop");
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
