/*
  sample-risc-cycle.h - Supporting the cycle queue

  This Software is part of Wind River Simics. The rights to copy, distribute,
  modify, or otherwise make use of this Software may be licensed only
  pursuant to the terms of an applicable Wind River license agreement.
  
  Copyright 2010-2014 Intel Corporation

*/

#ifndef SAMPLE_RISC_CYCLE_H
#define SAMPLE_RISC_CYCLE_H

#include "sample-risc.h"

void instantiate_cycle_queue(sample_risc_t *sr);
void register_cycle_queue(conf_class_t *cls);

#endif
