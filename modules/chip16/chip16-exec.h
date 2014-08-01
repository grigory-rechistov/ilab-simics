/*
  chip16-exec.h - sample code for executing instructions

  This Software is part of Wind River Simics. The rights to copy, distribute,
  modify, or otherwise make use of this Software may be licensed only
  pursuant to the terms of an applicable Wind River license agreement.
  
  Copyright 2010-2014 Intel Corporation

*/

#ifndef SAMPLE_RISC_EXEC_H
#define SAMPLE_RISC_EXEC_H

#include <simics/base/conf-object.h>

typedef enum {
        State_Idle,
        State_Running,
        State_Stopped
} execute_state_t;

void register_execute_interface(conf_class_t *cls);

#endif
