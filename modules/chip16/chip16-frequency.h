/*
  chip16-frequency.h - sample code for frequency

  This Software is part of Wind River Simics. The rights to copy, distribute,
  modify, or otherwise make use of this Software may be licensed only
  pursuant to the terms of an applicable Wind River license agreement.
  
  Copyright 2010-2014 Intel Corporation

*/

#ifndef CHIP16_FREQUENCY_H
#define CHIP16_FREQUENCY_H

#include "chip16.h"

void register_frequency_interfaces(conf_class_t *cls);
void instantiate_frequency(chip16_t *sr);
void finalize_frequency(chip16_t *sr);

#endif
