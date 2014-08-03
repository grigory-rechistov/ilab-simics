/*
  chip16-queue.c - sample risc queue implementation

  This Software is part of Wind River Simics. The rights to copy, distribute,
  modify, or otherwise make use of this Software may be licensed only
  pursuant to the terms of an applicable Wind River license agreement.
  
  Copyright 2010-2014 Intel Corporation

*/

#include "chip16-queue.h"
#include "event-queue.h"
#ifndef CHIP16_HEADER
 #define CHIP16_HEADER "chip16.h"
#endif
#include CHIP16_HEADER

void
handle_events(chip16_t *sr, event_queue_t *queue)
{
        while (!queue_is_empty(queue)
               && get_delta(queue) == 0
               && sr->state != State_Stopped) {
                handle_next_event(queue);
        }
}
