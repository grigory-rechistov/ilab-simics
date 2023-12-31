/*
  chip16-queue.h - sample risc queue implementation

  This Software is part of Wind River Simics. The rights to copy, distribute,
  modify, or otherwise make use of this Software may be licensed only
  pursuant to the terms of an applicable Wind River license agreement.
  
  Copyright 2010-2014 Intel Corporation

*/

#ifndef CHIP16_QUEUE_H
#define CHIP16_QUEUE_H

#include "chip16.h"

void handle_events(chip16_t *sr, event_queue_t *queue);

#endif
