/*
  sample-risc-queue.c - sample risc queue implementation

  This Software is part of Wind River Simics. The rights to copy, distribute,
  modify, or otherwise make use of this Software may be licensed only
  pursuant to the terms of an applicable Wind River license agreement.
  
  Copyright 2010-2014 Intel Corporation

*/

#include "sample-risc-queue.h"
#include "event-queue.h"
#ifndef SAMPLE_RISC_HEADER
 #define SAMPLE_RISC_HEADER "sample-risc.h"
#endif
#include SAMPLE_RISC_HEADER

void
handle_events(sample_risc_t *sr, event_queue_t *queue)
{
        while (!queue_is_empty(queue)
               && get_delta(queue) == 0
               && sr->state != State_Stopped) {
                handle_next_event(queue);
        }
}
