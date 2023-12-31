/*
  chip16-step.h - sample code for the step queue

  This Software is part of Wind River Simics. The rights to copy, distribute,
  modify, or otherwise make use of this Software may be licensed only
  pursuant to the terms of an applicable Wind River license agreement.
  
  Copyright 2010-2014 Intel Corporation

*/

#ifndef CHIP16_STEP_H
#define CHIP16_STEP_H

#include "chip16.h"

void instantiate_step_queue(chip16_t *sr);
void register_step_queue(conf_class_t *cls);

#endif
