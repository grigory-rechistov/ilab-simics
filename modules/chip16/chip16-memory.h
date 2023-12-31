/*
  chip16-memory.h - sample code for the page cache and memory interface

  This Software is part of Wind River Simics. The rights to copy, distribute,
  modify, or otherwise make use of this Software may be licensed only
  pursuant to the terms of an applicable Wind River license agreement.
  
  Copyright 2010-2014 Intel Corporation

*/

#ifndef CHIP16_MEMORY_H
#define CHIP16_MEMORY_H

#include <simics/model-iface/memory-page.h>
#include "chip16.h"

void register_memory_interfaces(conf_class_t *cls);
void register_memory_attributes(conf_class_t *cls);

void init_page_cache(chip16_t *sr);

void chip16_check_virtual_breakpoints(chip16_t *core,
                                      access_t access,
                                      logical_address_t virt_start,
                                      generic_address_t len,
                                      uint8 *data);

bool chip16_write_memory(chip16_t *core,
                         physical_address_t phys_address,
                         physical_address_t len,
                         uint8 *data, bool check_bp);

bool chip16_read_memory(chip16_t *core,
                        physical_address_t phys_address,
                        physical_address_t len,
                        uint8 *data, bool check_bp);

bool chip16_fetch_instruction(chip16_t *core,
                              physical_address_t phys_address,
                              physical_address_t len,
                              uint8 *data,
                              bool check_bp);

void chip16_write_memory16(chip16_t *core, logical_address_t la,
                           physical_address_t pa, uint16_t value);

uint16_t
chip16_read_memory16(chip16_t *core, logical_address_t la,
                          physical_address_t pa);

void chip16_release_and_share(chip16_t *core, physical_address_t phys_address);

mem_page_t *get_page(chip16_t *sr, conf_object_t *phys_mem_obj,
                     const memory_page_interface_t *memp_iface,
                     physical_address_t address, access_t access);

#endif
