;/*************************************************************************
; *
; *     File        : WDPMI.C
; *
; *     Description : DPMI Functions
; *
; *     Portability : PC/WATCOM/PMODE-W/DOS4G
; *
; *
; *     Copyright (C) 1995,1996 RealTech
; *
; ************************************************************************/
#include <i86.h>
#include <string.h>
#include <_dpmi.h>

#define IN(reg)  rmregs.e##reg = in->d.e##reg
#define OUT(reg) out->d.e##reg = rmregs.e##reg

#define DPMI_INT        0x31

int __tb;
typedef struct {
    long edi;
    long esi;
    long ebp;
    long reserved;
    long ebx;
    long edx;
    long ecx;
    long eax;
    short flags;
    short es,ds,fs,gs,ip,cs,sp,ss;
}_RMREGS;


void __dpmi_yield(void)
/* INT 0x2F AX=1680 */
{
    union REGS r;
    r.w.ax = 0x1680;
    int386(0x2F,&r,&r);
    return;
}
int __dpmi_allocate_ldt_descriptors(int _count)
/* DPMI 0.9 AX=0000 */
{
    union REGS r;
    r.w.ax = 0;
    r.w.cx = _count;
    int386(DPMI_INT,&r,&r);
    if (r.x.cflag) return(0);
    return r.w.ax;
}
int __dpmi_free_ldt_descriptor(int _descriptor)
/* DPMI 0.9 AX=0001 */
{
    union REGS r;
    r.w.ax = 1;
    r.w.bx = _descriptor;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
/* DPMI 0.9 AX=0002 */
int __dpmi_segment_to_descriptor(int _segm)
{
    union REGS r;
    r.w.ax = 0x0002;
    r.w.bx = _segm;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_get_selector_increment_value(void)
/* DPMI 0.9 AX=0003 */
{
    union REGS r;
    r.w.ax = 0x0003;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_get_segment_base_address(int _selector, unsigned long *_addr)
/* DPMI 0.9 AX=0006 */
{
    union REGS r;
    r.w.ax = 0x0006;
    r.w.bx = _selector;
    int386(DPMI_INT,&r,&r);
    *_addr = ((long)r.w.cx<<16)+r.w.dx;
    return r.x.cflag;
}
int __dpmi_set_segment_base_address(int _selector, unsigned long _address)
/* DPMI 0.9 AX=0007 */
{
    union REGS r;
    r.w.ax = 7;
    r.w.bx = _selector;
    r.w.cx = _address >> 16;
    r.w.dx = _address & 0xFFFF;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
unsigned long __dpmi_get_segment_limit(int _selector)
/* LSL instruction  */
{
    return 0;
}
int __dpmi_set_segment_limit(int _selector, unsigned long _limit)
/* DPMI 0.9 AX=0008 */
{
    union REGS r;
    r.w.ax = 8;
    r.w.bx = _selector;
    r.w.cx = _limit >> 16;
    r.w.dx = _limit & 0xFFFF;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_get_descriptor_access_rights(int _selector)
/* LAR instruction  */
{

    return 0;
}
int __dpmi_set_descriptor_access_rights(int _selector, int _rights)
/* DPMI 0.9 AX=0009 */
{
    union REGS r;
    r.w.ax = 0x0009;
    r.w.bx = _selector;
    r.w.cx = _rights;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_create_alias_descriptor(int _selector)/* DPMI 0.9 AX=000a */
{
    union REGS r;
    r.w.ax = 0x000a;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_get_descriptor(int _selector, void *_buffer)/* DPMI 0.9 AX=000b */
{
    union REGS r;
    r.w.ax = 0x000b;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_set_descriptor(int _selector, void *_buffer)/* DPMI 0.9 AX=000c */
{
    union REGS r;
    r.w.ax = 0x000c;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_allocate_specific_ldt_descriptor(int _selector)/* DPMI 0.9 AX=000d */
{
    union REGS r;
    r.w.ax = 0x000d;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_get_multiple_descriptors(int _count, void *_buffer)/* DPMI 1.0 AX=000e */
{
    union REGS r;
    r.w.ax = 0x000e;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_set_multiple_descriptors(int _count, void *_buffer)/* DPMI 1.0 AX=000f */
{
    union REGS r;
    r.w.ax = 0x000f;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_allocate_dos_memory(int _paragraphs, int *_ret_selector_or_max)/* DPMI 0.9 AX=0100 */
{
    union REGS r;
    r.w.ax = 0x0100;
    r.w.bx =  (short) ((_paragraphs+0xF)>>4);
    int386(DPMI_INT,&r,&r);
    *_ret_selector_or_max = r.w.dx;
    __tb = r.w.ax;
    return r.x.cflag;
}

int __dpmi_free_dos_memory(int _selector)/* DPMI 0.9 AX=0101 */
{
    union REGS r;
    r.w.ax = 0x0101;
    r.w.bx = _selector;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_resize_dos_memory(int _selector, int _newpara, int *_ret_max)/* DPMI 0.9 AX=0102 */
{
    union REGS r;
    r.w.ax = 0x0102;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_get_real_mode_interrupt_vector(int _vector, __dpmi_raddr *_address)/* DPMI 0.9 AX=0200 */
{
    union REGS r;
    r.w.ax = 0x0200;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_set_real_mode_interrupt_vector(int _vector, __dpmi_raddr *_address)/* DPMI 0.9 AX=0201 */
{
    union REGS r;
    r.w.ax = 0x0201;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_get_processor_exception_handler_vector(int _vector, __dpmi_paddr *_address)/* DPMI 0.9 AX=0202 */
{
    union REGS r;
    r.w.ax = 0x0202;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_set_processor_exception_handler_vector(int _vector, __dpmi_paddr *_address)/* DPMI 0.9 AX=0203 */
{
    union REGS r;
    r.w.ax = 0x0203;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_get_protected_mode_interrupt_vector(int _vector, __dpmi_paddr *_address)/* DPMI 0.9 AX=0204 */
{
    union REGS r;
    r.w.ax = 0x0204;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_set_protected_mode_interrupt_vector(int _vector, __dpmi_paddr *_address)/* DPMI 0.9 AX=0205 */
{
    union REGS r;
    r.w.ax = 0x0205;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_get_extended_exception_handler_vector_pm(int _vector, __dpmi_paddr *_address)/* DPMI 1.0 AX=0210 */
{
    union REGS r;
    r.w.ax = 0x0210;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_get_extended_exception_handler_vector_rm(int _vector, __dpmi_paddr *_address)/* DPMI 1.0 AX=0211 */
{
    union REGS r;
    r.w.ax = 0x0211;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_set_extended_exception_handler_vector_pm(int _vector, __dpmi_paddr *_address)/* DPMI 1.0 AX=0212 */
{
    union REGS r;
    r.w.ax = 0x0212;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_set_extended_exception_handler_vector_rm(int _vector, __dpmi_paddr *_address)/* DPMI 1.0 AX=0213 */
{
    union REGS r;
    r.w.ax = 0x0213;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_int(int _vector, __dpmi_regs *in)/* DPMI 0.9 AX=0300 */
{
    __dpmi_regs *out=in;
    _RMREGS rmregs;
    union REGS r;
    struct SREGS sr;
    memset(&rmregs,0,sizeof(rmregs));
    IN(ax);
    IN(bx);
    IN(cx);
    IN(dx);
    IN(si);
    IN(di);
    IN(bp);
    rmregs.es = in->x.es;
    rmregs.cs = in->x.cs;
    rmregs.ds = in->x.ds;
    segread(&sr); sr.es = sr.ds;
    r.w.ax  = 0x300;
    r.h.bl  = _vector;
    r.h.bh  = 0;
    r.w.cx  = 0;
    r.x.edi = (unsigned)&rmregs;
    int386x(0x31,&r,&r,&sr);
    OUT(ax);
    OUT(bx);
    OUT(cx);
    OUT(dx);
    OUT(si);
    OUT(di);
    OUT(bp);
    in->x.es = rmregs.es;
    in->x.cs = rmregs.cs;
    in->x.ds = rmregs.ds;
    out->x.flags = rmregs.flags;
    return out->x.flags;
}
int __dpmi_simulate_real_mode_interrupt(int _vector, __dpmi_regs *_regs) /* like above, but sets ss sp fl *//* DPMI 0.9 AX=0300 */
{
    return  __dpmi_int(_vector,_regs);
}
int __dpmi_simulate_real_mode_procedure_retf(__dpmi_regs *_regs)/* DPMI 0.9 AX=0301 */
{
    union REGS r;
    r.w.ax = 0x0301;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_simulate_real_mode_procedure_retf_stack(__dpmi_regs *_regs, int stack_bytes_to_copy, const void *stack_bytes) /* DPMI 0.9 AX=0301 */
{
    union REGS r;
    r.w.ax = 0x0301;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_simulate_real_mode_procedure_iret(__dpmi_regs *_regs)/* DPMI 0.9 AX=0302 */
{
    union REGS r;
    r.w.ax = 0x0302;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_allocate_real_mode_callback(void (*_handler)(void), __dpmi_regs *_regs, __dpmi_raddr *_ret) /* DPMI 0.9 AX=0303 */
{
    union REGS r;
    r.w.ax = 0x0303;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_free_real_mode_callback(__dpmi_raddr *_addr)/* DPMI 0.9 AX=0304 */
{
    union REGS r;
    r.w.ax = 0x0304;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_get_state_save_restore_addr(__dpmi_raddr *_rm, __dpmi_paddr *_pm)/* DPMI 0.9 AX=0305 */
{
    union REGS r;
    r.w.ax = 0x0305;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_get_raw_mode_switch_addr(__dpmi_raddr *_rm, __dpmi_paddr *_pm)/* DPMI 0.9 AX=0306 */
{
    union REGS r;
    r.w.ax = 0x0306;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_get_version(__dpmi_version_ret *_ret)/* DPMI 0.9 AX=0400 */
{
    union REGS r;
    r.w.ax = 0x0400;
    int386(DPMI_INT,&r,&r);
    _ret->minor    =  r.h.al;
    _ret->major    =  r.h.ah;
    _ret->flags    =  r.w.bx;
    _ret->cpu      =  r.h.cl;
    return r.x.cflag;
}

int __dpmi_get_capabilities(int *_flags, char *vendor_info)/* DPMI 1.0 AX=0401 */
{
    union REGS r;
    r.w.ax = 0x0401;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}

int __dpmi_get_free_memory_information(__dpmi_free_mem_info *_info)/* DPMI 0.9 AX=0500 */
{
    union REGS regs;
    struct SREGS sregs;
    regs.x.eax = 0x0500;
    memset( &sregs, 0, sizeof(sregs) );
    sregs.es = FP_SEG( _info );
    regs.x.edi = FP_OFF( _info );
    int386x( DPMI_INT, &regs, &regs, &sregs );

    return regs.x.cflag;
}
int __dpmi_allocate_memory(__dpmi_meminfo *_info)/* DPMI 0.9 AX=0501 */
{
    union REGS r;
    r.w.ax = 0x0501;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_free_memory(unsigned long _handle)/* DPMI 0.9 AX=0502 */
{
    union REGS r;
    r.w.ax = 0x0502;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_resize_memory(__dpmi_meminfo *_info)/* DPMI 0.9 AX=0503 */
{
    union REGS r;
    r.w.ax = 0x0503;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_allocate_linear_memory(__dpmi_meminfo *_info, int _commit)/* DPMI 1.0 AX=0504 */
{
    union REGS r;
    r.w.ax = 0x0504;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_resize_linear_memory(__dpmi_meminfo *_info, int _commit)/* DPMI 1.0 AX=0505 */
{
    union REGS r;
    r.w.ax = 0x0505;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_get_page_attributes(__dpmi_meminfo *_info, short *_buffer)/* DPMI 1.0 AX=0506 */
{
    union REGS r;
    r.w.ax = 0x0506;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_set_page_attributes(__dpmi_meminfo *_info, short *_buffer)/* DPMI 1.0 AX=0507 */
{
    union REGS r;
    r.w.ax = 0x0507;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_map_device_in_memory_block(__dpmi_meminfo *_info, unsigned long _physaddr)/* DPMI 1.0 AX=0508 */
{
    union REGS r;
    r.w.ax = 0x0508;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_map_conventional_memory_in_memory_block(__dpmi_meminfo *_info, unsigned long _physaddr) /* DPMI 1.0 AX=0509 */
{
    union REGS r;
    r.w.ax = 0x0509;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_get_memory_block_size_and_base(__dpmi_meminfo *_info)/* DPMI 1.0 AX=050a */
{
    union REGS r;
    r.w.ax = 0x050a;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_get_memory_information(__dpmi_memory_info *_buffer)/* DPMI 1.0 AX=050b */
{
    union REGS r;
    r.w.ax = 0x050b;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_lock_linear_region(__dpmi_meminfo *_info)/* DPMI 0.9 AX=0600 */
{
    union REGS r;
    r.w.ax = 0x0600;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_unlock_linear_region(__dpmi_meminfo *_info)/* DPMI 0.9 AX=0601 */
{
    union REGS r;
    r.w.ax = 0x0601;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_mark_real_mode_region_as_pageable(__dpmi_meminfo *_info)/* DPMI 0.9 AX=0602 */
{
    union REGS r;
    r.w.ax = 0x0602;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_relock_real_mode_region(__dpmi_meminfo *_info)/* DPMI 0.9 AX=0603 */
{
    union REGS r;
    r.w.ax = 0x0603;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_get_page_size(unsigned long *_size)/* DPMI 0.9 AX=0604 */
{
    union REGS r;
    r.w.ax = 0x0604;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_mark_page_as_demand_paging_candidate(__dpmi_meminfo *_info)/* DPMI 0.9 AX=0702 */
{
    union REGS r;
    r.w.ax = 0x0702;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_discard_page_contents(__dpmi_meminfo *_info)/* DPMI 0.9 AX=0703 */
{
    union REGS r;
    r.w.ax = 0x0703;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}

int __dpmi_physical_address_mapping(__dpmi_meminfo *_info)/* DPMI 0.9 AX=0800 */
{
    union REGS r;
    r.w.ax = 0x800;
    r.w.bx = (short) (_info->address >> 16);
    r.w.cx = (short) (_info->address & 0xFFFF);
    r.w.si = (short) (_info->size  >>16 );
    r.w.di = (short) (_info->size   & 0xFFFF);
    int386(DPMI_INT,&r,&r);
    _info->handle = ((unsigned long)r.w.bx << 16) + (unsigned long)r.w.cx;
    return r.x.cflag;
}
int __dpmi_free_physical_address_mapping(__dpmi_meminfo *_info)/* DPMI 0.9 AX=0801 */
{
    union REGS r;
    r.w.ax = 0x0801;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
/* These next four functions return the old state */
int __dpmi_get_and_disable_virtual_interrupt_state(void)/* DPMI 0.9 AX=0900 */
{
    union REGS r;
    r.w.ax = 0x0900;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_get_and_enable_virtual_interrupt_state(void)/* DPMI 0.9 AX=0901 */
{
    union REGS r;
    r.w.ax = 0x0901;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_get_and_set_virtual_interrupt_state(int _old_state)/* DPMI 0.9 AH=09   */
{
    union REGS r;
    r.h.ah = 0x09;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_get_virtual_interrupt_state(void)/* DPMI 0.9 AX=0902 */
{
    union REGS r;
    r.w.ax = 0x0902;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_get_vendor_specific_api_entry_point(char *_id, __dpmi_paddr *_api)/* DPMI 0.9 AX=0a00 */
{
    union REGS r;
    r.w.ax = 0x0a00;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_set_debug_watchpoint(__dpmi_meminfo *_info, int _type)/* DPMI 0.9 AX=0b00 */
{
    union REGS r;
    r.w.ax = 0x0b00;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_clear_debug_watchpoint(unsigned long _handle)/* DPMI 0.9 AX=0b01 */
{
    union REGS r;
    r.w.ax = 0x0b01;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_get_state_of_debug_watchpoint(unsigned long _handle, int *_status)/* DPMI 0.9 AX=0b02 */
{
    union REGS r;
    r.w.ax = 0x0b02;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_reset_debug_watchpoint(unsigned long _handle)/* DPMI 0.9 AX=0b03 */
{
    union REGS r;
    r.w.ax = 0x0b03;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_install_resident_service_provider_callback(__dpmi_callback_info *_info)/* DPMI 1.0 AX=0c00 */
{
    union REGS r;
    r.w.ax = 0x0c00;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_terminate_and_stay_resident(int return_code, int paragraphs_to_keep)/* DPMI 1.0 AX=0c01 */
{
    union REGS r;
    r.w.ax = 0x0c01;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_allocate_shared_memory(__dpmi_shminfo *_info)/* DPMI 1.0 AX=0d00 */
{
    union REGS r;
    r.w.ax = 0x0d00;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_free_shared_memory(unsigned long _handle)/* DPMI 1.0 AX=0d01 */
{
    union REGS r;
    r.w.ax = 0x0d01;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_serialize_on_shared_memory(unsigned long _handle, int _flags)/* DPMI 1.0 AX=0d02 */
{
    union REGS r;
    r.w.ax = 0x0d02;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_free_serialization_on_shared_memory(unsigned long _handle, int _flags)/* DPMI 1.0 AX=0d03 */
{
    union REGS r;
    r.w.ax = 0x0d03;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_get_coprocessor_status(void)/* DPMI 1.0 AX=0e00 */
{
    union REGS r;
    r.w.ax = 0x0e00;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_set_coprocessor_emulation(int _flags)/* DPMI 1.0 AX=0e01 */
{
    union REGS r;
    r.w.ax = 0x0e01;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
int __dpmi_get_PMODE_infos(void)/* DPMI 1.0 AX=eeff */
{
    union REGS r;
    r.w.ax = 0xeeff;
    int386(DPMI_INT,&r,&r);
    return r.x.cflag;
}
