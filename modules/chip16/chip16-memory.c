/*
  chip16-memory.c - sample code for memory access and page buffers

  This Software is part of Wind River Simics. The rights to copy, distribute,
  modify, or otherwise make use of this Software may be licensed only
  pursuant to the terms of an applicable Wind River license agreement.
  
  Copyright 2010-2014 Intel Corporation

*/

#include "chip16-memory.h"
#include <simics/processor-api.h>
#include <simics/util/vect.h>

#include <simics/model-iface/image.h>
#include <simics/model-iface/simulator-cache.h>

#ifndef CHIP16_HEADER
#define CHIP16_HEADER "chip16.h"
#endif
#include CHIP16_HEADER

static attr_value_t
attr_bp(breakpoint_info_t bp)
{
        return SIM_make_attr_list(
                2,
                SIM_make_attr_uint64(bp.handle),
                SIM_make_attr_list(
                        2,
                        SIM_make_attr_uint64(bp.start),
                        SIM_make_attr_uint64(bp.end)));
}

static attr_value_t
format_page_for_output(mem_page_t *p)
{
        int num_r = 0, num_w = 0, num_e = 0;
        for (int i = 0; i < p->breakpoints.num_breakpoints; i++) {
                breakpoint_info_t *bi = &p->breakpoints.breakpoints[i];
                if (bi->read_write_execute & Sim_Access_Read)
                        num_r++;
                if (bi->read_write_execute & Sim_Access_Write)
                        num_w++;
                if (bi->read_write_execute & Sim_Access_Execute)
                        num_e++;
        }

        attr_value_t read_bps = SIM_alloc_attr_list(num_r);
        attr_value_t write_bps = SIM_alloc_attr_list(num_w);
        attr_value_t exec_bps = SIM_alloc_attr_list(num_e);
        int nr = 0, nw = 0, ne = 0;
        for (int i = 0; i < p->breakpoints.num_breakpoints; i++) {
                breakpoint_info_t *bi = &p->breakpoints.breakpoints[i];
                if (bi->read_write_execute & Sim_Access_Read)
                        SIM_attr_list_set_item(&read_bps, nr++, attr_bp(*bi));
                if (bi->read_write_execute & Sim_Access_Write)
                        SIM_attr_list_set_item(&write_bps, nw++, attr_bp(*bi));
                if (bi->read_write_execute & Sim_Access_Execute)
                        SIM_attr_list_set_item(&exec_bps, ne++, attr_bp(*bi));
        }
        return SIM_make_attr_list(
                8,
                SIM_make_attr_object(p->mem_space),
                SIM_make_attr_uint64(p->paddr),
                SIM_make_attr_uint64(1ull << p->size_log2),
                SIM_make_attr_uint64(p->group_id),
                SIM_make_attr_uint64(p->access),
                read_bps, write_bps, exec_bps);
}

/* The actual page cache */
void
init_page_cache(chip16_t *sr)
{
        VINIT(sr->cached_pages);
        sr->number_of_cached_pages = 0;
}

static void
clear_page_cache(chip16_t *sr)
{
        VFORI(sr->cached_pages, i) {
                mem_page_t *p = &VGET(sr->cached_pages, i);
                const memory_page_interface_t *mi =
                        SIM_c_get_interface(p->mem_space,
                                            MEMORY_PAGE_INTERFACE);
                mi->free_mem_page(p->mem_space, p);
        }
        VFREE(sr->cached_pages);
        sr->number_of_cached_pages = 0;
}

static mem_page_t *
search_page_cache(chip16_t *sr, conf_object_t *phys_mem_obj,
                  physical_address_t address, access_t access)
{
        VFORI(sr->cached_pages, i) {
                mem_page_t *ret = &VGET(sr->cached_pages, i);
                if (ret->mem_space == phys_mem_obj) {
                        /* TODO check page crossing */
                        physical_address_t mask =
                                ~((1ull << ret->size_log2) - 1);
                        if (ret->paddr == (address & mask))
                                return ret;
                }
        }
        return NULL;
}

static mem_page_t *
add_to_page_cache(chip16_t *sr, conf_object_t *phys_mem_obj,
                  mem_page_t *p)
{
        VADD(sr->cached_pages, *p);
        sr->number_of_cached_pages += 1;
        return &VLAST(sr->cached_pages);
}

static attr_value_t
get_page_cache(void *arg, conf_object_t *obj, attr_value_t *idx)
{
        chip16_t *sr = conf_to_chip16(obj);
        attr_value_t ret = SIM_alloc_attr_list(VLEN(sr->cached_pages));
        VFORI(sr->cached_pages, p_idx) {
                mem_page_t *p = &VGET(sr->cached_pages, p_idx);
                SIM_attr_list_set_item(&ret, p_idx, format_page_for_output(p));
        }
        return ret;
}

/* The main function that allows an outside program to get a page.
   We search the page cache and if we have it, we return it.
   If we don't have it, we ask for it.  If we are allowed to,
   we will save the page in the page cache.  If the page is
   not cachable, we just return NULL. */
mem_page_t *
get_page(chip16_t *sr, conf_object_t *phys_mem_obj,
         const memory_page_interface_t *memp_iface,
         physical_address_t address, access_t access)
{
        bool update_access_bits = false;

        /* check if we have page cached */
        mem_page_t *ret = search_page_cache(sr, phys_mem_obj, address, access);
        if (ret != NULL) {
                /* page match */
                if (access & ret->access)
                        return ret;

                /* missing access permissions */
                access = (access_t)((int)access | (int)ret->access);
                update_access_bits = true;
        }

        mem_page_t p =
                memp_iface->request_page(phys_mem_obj,
                                         chip16_to_conf(sr),
                                         address,
                                         access);

        if (p.size_log2 == 0) {
                /* No page was found */
                SIM_LOG_INFO(3, chip16_to_conf(sr), 0,
                             "failed to cache page at 0x%llx",
                             address);
                return NULL;
        }

        ASSERT((p.access & access) == access);
        // TODO make sure all fields are correct

        if (update_access_bits) {
                SIM_LOG_INFO(3, chip16_to_conf(sr), 0,
                             "%d: updated access bits for page at 0x%llx",
                             sr->number_of_cached_pages, p.paddr);
                /* free the data structures from our last call to get_page */
                memp_iface->free_mem_page(phys_mem_obj, ret);
                /* remember the new data structures */
                *ret = p;
        } else {
                SIM_LOG_INFO(3, chip16_to_conf(sr), 0,
                             "%d: fetched new page at 0x%016llx",
                             sr->number_of_cached_pages, p.paddr);
                ret = add_to_page_cache(sr, phys_mem_obj, &p);
        }

        return ret;
}

void
check_virtual_breakpoints(chip16_t *sr, chip16_t *core,
                          access_t access, logical_address_t virt_start,
                          generic_address_t len, uint8 *data)
{
        logical_address_t virt_end = virt_start + len - 1;
        breakpoint_set_t bp_set =
                 core->context_bp_query_iface->get_breakpoints(
                         core->current_context, access, virt_start, virt_end);
        for (int i = 0; i < bp_set.num_breakpoints; i++) {
                core->context_bp_trig_iface->trigger_breakpoint(
                        core->current_context,
                        chip16_to_conf(core),
                        bp_set.breakpoints[i].handle,
                        virt_start,
                        len,
                        access,
                        data);
        }
        core->context_bp_query_iface->free_breakpoint_set(
                core->current_context, &bp_set);
}

static bool
data_bp_match(breakpoint_info_t bpi,
              physical_address_t start, physical_address_t end)
{
        return (bpi.start <= start && bpi.end >= start)
               || (bpi.start <= end && bpi.end >= end);
}

static void
check_page_breakpoints(chip16_t *sr, chip16_t *core,
                       mem_page_t *page, access_t access,
                       physical_address_t phys_address, generic_address_t len,
                       uint8 *data)
{
        for (int i = 0; i < page->breakpoints.num_breakpoints; i++) {
                breakpoint_info_t *bi = &page->breakpoints.breakpoints[i];
                if ((access & bi->read_write_execute) == 0)
                        continue;

                if (data_bp_match(*bi, phys_address, phys_address + len - 1))
                        core->phys_mem_bp_trig_iface->trigger_breakpoint(
                                core->phys_mem_obj,
                                chip16_to_conf(core),
                                bi->handle, phys_address, len, access, data);
        }
}

static generic_transaction_t
create_generic_transaction(conf_object_t *initiator, mem_op_type_t type,
                           physical_address_t phys_address,
                           physical_address_t len, uint8 *data,
                           endianness_t endian)
{
        generic_transaction_t ret;
        memset(&ret, 0, sizeof(ret));
        ret.logical_address = 0;
        ret.physical_address = phys_address;
        ret.real_address = data;
        ret.size = len;
        ret.ini_type = Sim_Initiator_CPU;
        ret.ini_ptr = initiator;
        ret.use_page_cache = 1;
        ret.inquiry = 0;
        SIM_set_mem_op_type(&ret, type);

#if defined(HOST_LITTLE_ENDIAN)
        if (endian == Sim_Endian_Host_From_BE)
                ret.inverse_endian = 1;
#else
        if (endian == Sim_Endian_Host_From_LE)
                ret.inverse_endian = 1;
#endif

        return ret;
}

/* The program interface allows programs to read, write and fetch
   instructions from memory.  Check if the desired page is in the
   page cache.  If so, do the access from the page cache.  If
   not, call the memory interface function to get the desired
   bytes from the memory.  This is probably a memory mapped I/O
   access if it isn't cachable.
*/
bool
write_memory(chip16_t *sr, chip16_t *core,
             physical_address_t phys_address, physical_address_t len,
             uint8 *data, bool check_bp)
{
        conf_object_t *phys_mem_obj = core->phys_mem_obj;
        mem_page_t *page = get_page(sr, phys_mem_obj,
                                    core->phys_mem_page_iface, phys_address,
                                    Sim_Access_Write);

        if (page) {
                uint8 *ma = page->data
                        + (((1ull << page->size_log2) - 1) & phys_address);
                memcpy(ma, data, len);
                if (check_bp)
                        check_page_breakpoints(sr, core, page,
                                               Sim_Access_Write,
                                               phys_address, len, data);
        } else {
                generic_transaction_t mem_op = create_generic_transaction(
                        chip16_to_conf(core),
                        Sim_Trans_Store,
                        phys_address, len,
                        data,
                        Sim_Endian_Target);
                exception_type_t exc =
                     core->phys_mem_space_iface->access(phys_mem_obj, &mem_op);

                if (exc != Sim_PE_No_Exception) {
                        SIM_LOG_ERROR(chip16_to_conf(sr), 0,
                                      "write error to physical address 0x%llx",
                                      phys_address);
                        return false;
                }
        }

        return true;
}

bool
read_memory(chip16_t *sr, chip16_t *core,
            physical_address_t phys_address, physical_address_t len,
            uint8 *data, bool check_bp)
{
        conf_object_t *phys_mem_obj = core->phys_mem_obj;
        mem_page_t *page = get_page(sr, phys_mem_obj,
                                    core->phys_mem_page_iface, phys_address,
                                    Sim_Access_Read);

        if (page) {
                uint8 *ma = page->data
                        + (((1ull << page->size_log2) - 1) & phys_address);
                memcpy(data, ma, len);
                if (check_bp)
                        check_page_breakpoints(sr, core, page, Sim_Access_Read,
                                               phys_address, len, data);
        } else {
                generic_transaction_t mem_op = create_generic_transaction(
                        chip16_to_conf(core),
                        Sim_Trans_Load,
                        phys_address, len,
                        data,
                        Sim_Endian_Target);
                exception_type_t exc =
                     core->phys_mem_space_iface->access(phys_mem_obj, &mem_op);

                if (exc != Sim_PE_No_Exception) {
                        SIM_LOG_ERROR(chip16_to_conf(sr), 0,
                                     "read error from physical address 0x%llx",
                                      phys_address);
                        return false;
                }
        }

        return true;
}

bool
fetch_instruction(chip16_t *sr, chip16_t *core,
                  physical_address_t phys_address, physical_address_t len,
                  uint8 *data, bool check_bp)
{
        conf_object_t *phys_mem_obj = core->phys_mem_obj;
        mem_page_t *page = get_page(sr, phys_mem_obj,
                                    core->phys_mem_page_iface, phys_address,
                                    Sim_Access_Execute);

        if (page) {
                uint8 *ma = page->data
                            + (((1ull << page->size_log2) - 1) & phys_address);
                if (check_bp)
                        check_page_breakpoints(sr, core, page,
                                               Sim_Access_Execute,
                                               phys_address, len, ma);
                memcpy(data, ma, len);
        } else {
                generic_transaction_t mem_op = create_generic_transaction(
                        chip16_to_conf(core),
                        Sim_Trans_Instr_Fetch,
                        phys_address, len,
                        data,
                        Sim_Endian_Target);
                exception_type_t exc =
                     core->phys_mem_space_iface->access(phys_mem_obj, &mem_op);

                if (exc != Sim_PE_No_Exception) {
                        SIM_LOG_ERROR(chip16_to_conf(sr), 0,
                                    "fetch error from physical address 0x%llx",
                                    phys_address);
                        return false;
                }
        }

        return true;
}

void
release_and_share(chip16_t *sr, chip16_t *core,
                  physical_address_t phys_address)
{
        conf_object_t *phys_mem_obj = core->phys_mem_obj;
        mem_page_t *page =
                search_page_cache(sr, phys_mem_obj, phys_address, (access_t)0);
        if (page)
                core->phys_mem_page_iface->release_page(
                        core->phys_mem_obj, chip16_to_conf(core), page);
}

/*
 * memory page interface functions
 */
static void
release_all_pages(conf_object_t *obj)
{
        chip16_t *sr = conf_to_chip16(obj);
        SIM_LOG_INFO(2, chip16_to_conf(sr), 0, "release_all_pages");
        clear_page_cache(sr);
}

static void
write_protect_all_pages(conf_object_t *obj)
{
        chip16_t *sr = conf_to_chip16(obj);
        SIM_LOG_INFO(2, chip16_to_conf(sr), 0, "write_protect_all_pages");
        clear_page_cache(sr);
}

static void
protect_host_page(conf_object_t *obj, uint8 *host_addr, size_t size,
                  access_t protect)
{
        chip16_t *sr = conf_to_chip16(obj);
        SIM_LOG_INFO(2, chip16_to_conf(sr), 0, "protect_host_page");
        clear_page_cache(sr);
}

/*
 * image snoop interface functions
 */
static void
page_modified(conf_object_t *obj, conf_object_t *img, uint64 offset,
              uint8 *page_data, image_spage_t *spage)
{
        chip16_t *sr = conf_to_chip16(obj);
        SIM_LOG_UNIMPLEMENTED(1, chip16_to_conf(sr), 0, "page_modified");
}

/*
 * simulator cache interface functions
 */
static void
simulator_cache_flush(conf_object_t *obj)
{
        chip16_t *sr = conf_to_chip16(obj);
        SIM_LOG_INFO(2, chip16_to_conf(sr), 0, "simulator_cache_flush");
        clear_page_cache(sr);
}

static void
breakpoint_added(conf_object_t *obj,
                 conf_object_t *bp_obj,
                 breakpoint_handle_t handle)
{
        /* A breakpoint has been added. This may cause some breakpoint
           information in the page cache to become stale. We would request more
           information about the added breakpoint here and do something clever,
           but we instead simply clear the entire page cache. */
        chip16_t *sr = conf_to_chip16(obj);
        SIM_LOG_INFO(2, chip16_to_conf(sr), 0, "breakpoint_added");
        clear_page_cache(sr);
}

static void
breakpoint_removed(conf_object_t *obj,
                   conf_object_t *bp_obj,
                   breakpoint_handle_t handle)
{
        /* A breakpoint has been removed. This may cause some breakpoint
           information in the page cache to become stale. We would request more
           information about the removed breakpoint here and do something 
           clever, but we instead simply clear the entire page cache. */
        chip16_t *sr = conf_to_chip16(obj);
        SIM_LOG_INFO(2, chip16_to_conf(sr), 0, "breakpoint_removed");
        clear_page_cache(sr);
}

/* export the functions to set up the memory interfaces and attributes */
void
register_memory_interfaces(conf_class_t *cls)
{
        static const memory_page_update_interface_t mpui_iface = {
                .release_all_pages = release_all_pages,
                .write_protect_all_pages = write_protect_all_pages,
                .protect_host_page = protect_host_page
        };
        SIM_register_interface(cls, MEMORY_PAGE_UPDATE_INTERFACE, &mpui_iface);

        static const image_snoop_interface_t isi_iface = {
                .page_modified = page_modified
        };
        SIM_register_interface(cls, IMAGE_SNOOP_INTERFACE, &isi_iface);

        static const simulator_cache_interface_t sci_iface = {
                .flush = simulator_cache_flush
        };
        SIM_register_interface(cls, SIMULATOR_CACHE_INTERFACE, &sci_iface);

        static const breakpoint_change_interface_t bp_change_iface = {
                .breakpoint_added = breakpoint_added,
                .breakpoint_removed = breakpoint_removed
        };
        SIM_register_interface(cls, BREAKPOINT_CHANGE_INTERFACE,
                               &bp_change_iface);
}

void
register_memory_attributes(conf_class_t *cls)
{
        SIM_register_typed_attribute(
                cls, "cached_pages",
                get_page_cache, NULL,
                0, NULL,
                Sim_Attr_Pseudo,
                "[[oiiii[[i[ii]]*][[i[ii]]*][[i[ii]]*]]*]", NULL,
                "Cached pages; for debugging/testing purposes only");
}
