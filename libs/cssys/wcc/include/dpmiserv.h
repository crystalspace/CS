#ifndef _DPMISERV_
#define _DPMISERV_

typedef struct {
  unsigned long handle;                 /* 0, 2 */
  unsigned long size;   /* or count */  /* 4, 6 */
  unsigned long address;                /* 8, 10 */
} __dpmi_meminfo;

/*
 * DPMI routines for gettin' into innards of protected mode. Useful
 * for interfacing to real-mode stuff
 */

void DPMI_allocRealSeg(int size,int *sel,int *r_seg);
void DPMI_freeRealSeg(unsigned sel);
int  DPMI_int86(int intno, RMREGS *in, RMREGS *out);
int  DPMI_int86x(int intno, RMREGS *in, RMREGS *out, RMSREGS *sregs);
int  DPMI_allocSelector(void);
long DPMI_mapPhysicalToLinear(long physAddr,long limit);
void DPMI_setSelectorBase(int sel,long linAddr);
void DPMI_setSelectorLimit(int sel,long limit);
int  DPMI_physical_address_mapping(__dpmi_meminfo *_info);
int  DPMI_free_physical_address_mapping(__dpmi_meminfo *_info);

#endif
