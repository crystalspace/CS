;/************************************************************************
; *
; *     File        :   DPMI.H
; *
; *     Description :   Header for DPMI functions with Watcom
; *
; *     Copyright (C) 1995 Realtech
; *
; ***********************************************************************/
#ifndef __DPMIHT
#define __DPMIHT


typedef struct {
    unsigned char major;
    unsigned char minor;
    unsigned char realmode,paging;
    unsigned char cpu;
    unsigned char master_pic;
    unsigned char slave_pic;
}PM_DPMIInfo;

typedef struct {
  unsigned short offset16;
  unsigned short segment;
} __dpmi_raddr;

typedef struct {
  unsigned long  offset32;
  unsigned short selector;
} __dpmi_paddr;

typedef struct {
  unsigned long handle;			/* 0, 2 */
  unsigned long size; 	/* or count */	/* 4, 6 */
  unsigned long address;		/* 8, 10 */
} __dpmi_meminfo;

typedef union {
  struct {
    unsigned long edi;
    unsigned long esi;
    unsigned long ebp;
    unsigned long res;
    unsigned long ebx;
    unsigned long edx;
    unsigned long ecx;
    unsigned long eax;
  } d;
  struct {
    unsigned short di, di_hi;
    unsigned short si, si_hi;
    unsigned short bp, bp_hi;
    unsigned short res, res_hi;
    unsigned short bx, bx_hi;
    unsigned short dx, dx_hi;
    unsigned short cx, cx_hi;
    unsigned short ax, ax_hi;
    unsigned short flags;
    unsigned short es;
    unsigned short ds;
    unsigned short fs;
    unsigned short gs;
    unsigned short ip;
    unsigned short cs;
    unsigned short sp;
    unsigned short ss;
  } x;
  struct {
    unsigned char edi[4];
    unsigned char esi[4];
    unsigned char ebp[4];
    unsigned char res[4];
    unsigned char bl, bh, ebx_b2, ebx_b3;
    unsigned char dl, dh, edx_b2, edx_b3;
    unsigned char cl, ch, ecx_b2, ecx_b3;
    unsigned char al, ah, eax_b2, eax_b3;
  } h;
} __dpmi_regs;
  
typedef struct {
  unsigned char  major;
  unsigned char  minor;
  unsigned short flags;
  unsigned char  cpu;
  unsigned char  master_pic;
  unsigned char  slave_pic;
} __dpmi_version_ret;

typedef struct {
  unsigned long largest_available_free_block_in_bytes;
  unsigned long maximum_unlocked_page_allocation_in_pages;
  unsigned long maximum_locked_page_allocation_in_pages;
  unsigned long linear_address_space_size_in_pages;
  unsigned long total_number_of_unlocked_pages;
  unsigned long total_number_of_free_pages;
  unsigned long total_number_of_physical_pages;
  unsigned long free_linear_address_space_in_pages;
  unsigned long size_of_paging_file_partition_in_pages;
  unsigned long reserved[3];
} __dpmi_free_mem_info;

typedef struct {
  unsigned long total_allocated_bytes_of_physical_memory_host;
  unsigned long total_allocated_bytes_of_virtual_memory_host;
  unsigned long total_available_bytes_of_virtual_memory_host;
  unsigned long total_allocated_bytes_of_virtual_memory_vcpu;
  unsigned long total_available_bytes_of_virtual_memory_vcpu;
  unsigned long total_allocated_bytes_of_virtual_memory_client;
  unsigned long total_available_bytes_of_virtual_memory_client;
  unsigned long total_locked_bytes_of_memory_client;
  unsigned long max_locked_bytes_of_memory_client;
  unsigned long highest_linear_address_available_to_client;
  unsigned long size_in_bytes_of_largest_free_memory_block;
  unsigned long size_of_minimum_allocation_unit_in_bytes;
  unsigned long size_of_allocation_alignment_unit_in_bytes;
  unsigned long reserved[19];
} __dpmi_memory_info;

typedef struct {
  unsigned long data16[2];
  unsigned long code16[2];
  unsigned short ip;
  unsigned short reserved;
  unsigned long data32[2];
  unsigned long code32[2];
  unsigned long eip;
} __dpmi_callback_info;

typedef struct {
  unsigned long size_requested;
  unsigned long size;
  unsigned long handle;
  unsigned long address;
  unsigned long name_offset;
  unsigned short name_selector;
  unsigned short reserved1;
  unsigned long reserved2;
} __dpmi_shminfo;


typedef struct {
	unsigned int len,sel,off,rseg,roff;
}PM_Buffer;

extern int __tb;

void PM_memsetf(unsigned long dst_s, unsigned long dst_o, unsigned long color, unsigned long n);
#pragma aux PM_memsetf = \
"push es"\
"mov es,bx"\
"push ecx"\
"shr ecx,2"\
"rep stosd"\
"pop ecx"\
"and ecx,3"\
"rep stosb"\
"pop es"\
parm [ebx] [edi] [eax] [ecx];

void PM_memcpyfn(unsigned long dst_s, unsigned long dst_o, void *src, unsigned long n);
#pragma aux PM_memcpyfn = \
"push es"\
"mov es,ax"\
"mov eax,ecx"\
"shr ecx,2"\
"rep movsd"\
"mov ecx,eax"\
"and ecx,3"\
"rep movsb"\
"pop es"\
parm [eax] [edi] [esi] [ecx];

void PM_memcpynf(void *dst,unsigned long src_s,unsigned long src_o,unsigned long n);
#pragma aux PM_memcpynf = \
"push ds"\
"mov ds,ax"\
"mov eax,ecx"\
"shr ecx,2"\
"rep movsd"\
"mov ecx,eax"\
"and ecx,3"\
"rep movsb"\
"pop ds"\
parm [edi] [eax] [esi] [ecx];

#ifdef __cplusplus
extern "C" {
    #endif
    int  PM_allocSelector(void);
    int  PM_mapPhysicalToLinear(long phyAddr, long limit, unsigned long *newx);
    unsigned long PM_memory_left(void);
    void PM_specification(PM_DPMIInfo *Dps);
    void PM_callES(unsigned char Inte, __dpmi_regs *regs, void *buffer, int size);
    void RM_Free(void);
    void RM_Initialize(void);
    extern PM_DPMIInfo Dps;
    extern PM_Buffer PMB;
    #ifdef __cplusplus
}
#endif
#define PHYSIC_ADRESS(x)   ((((long)x>>16)&(0x0000FFFF))<<4)+((long)x & 0x0000FFFF)
#endif

