/*
  DOS support for Crystal Space DPMI library
  Copyright (C) 1998 by Jorrit Tyberghein
  Written by Slavik Levtchenko <Smirnov@bbs.math.spbu.ru>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "global.h"
#include "dpmiserv.h"

void FatalError(char *msg) {fprintf(stderr,"%s\n", msg); exit(1);}
/*------------------------- DPMI interface routines -----------------------*/

void DPMI_allocRealSeg(int size,int *sel,int *r_seg)
/****************************************************************************
 * Function:     DPMI_allocRealSeg
 * Parameters:   size    - Size of memory block to allocate
 *               sel     - Place to return protected mode selector
 *               r_seg   - Place to return real mode segment
 * Description:  Allocates a block of real mode memory using DPMI services.
 *               This routine returns both a protected mode selector and
 *               real mode segment for accessing the memory block.
 ****************************************************************************/
{
  union REGS      r;

  r.w.ax = 0x100;                 /* DPMI allocate DOS memory         */
  r.w.bx = (size + 0xF) >> 4;     /* number of paragraphs             */
  int386(0x31, &r, &r);
  if(r.w.cflag) FatalError("DPMI_allocRealSeg failed!");

  *sel = r.w.dx;                  /* Protected mode selector          */
  *r_seg = r.w.ax;                /* Real mode segment                */
}

void DPMI_freeRealSeg(unsigned sel)
/****************************************************************************
 * Function:     DPMI_allocRealSeg
 * Parameters:   sel - Protected mode selector of block to free
 * Description:  Frees a block of real mode memory.
 ****************************************************************************/
{
    union REGS  r;

    r.w.ax = 0x101;                 /* DPMI free DOS memory             */
    r.w.dx = sel;                   /* DX := selector from 0x100        */
    int386(0x31, &r, &r);
}

typedef struct {
    long    edi;
    long    esi;
    long    ebp;
    long    reserved;
    long    ebx;
    long    edx;
    long    ecx;
    long    eax;
    short   flags;
    short   es,ds,fs,gs,ip,cs,sp,ss;
    } _RMREGS;

#define IN(reg)     rmregs.e##reg = in->x.reg
#define OUT(reg)    out->x.reg = rmregs.e##reg

int DPMI_int86(int intno, RMREGS *in, RMREGS *out)
/****************************************************************************
 * Function:     DPMI_int86
 * Parameters:   intno   - Interrupt number to issue
 *               in      - Pointer to structure for input registers
 *               out     - Pointer to structure for output registers
 * Returns:      Value returned by interrupt in AX
 * Description:  Issues a real mode interrupt using DPMI services.
 ****************************************************************************/
{
  _RMREGS         rmregs;
  union REGS      r;
  struct SREGS    sr;

  memset(&rmregs, 0, sizeof(rmregs));
  IN(ax); IN(bx); IN(cx); IN(dx); IN(si); IN(di);

  segread(&sr);
  r.w.ax = 0x300;                 /* DPMI issue real interrupt        */
  r.h.bl = intno;
  r.h.bh = 0;
  r.w.cx = 0;
  sr.es = sr.ds;
  r.x.edi = (unsigned)&rmregs;
  int386x(0x31, &r, &r, &sr);     /* Issue the interrupt              */

  OUT(ax); OUT(bx); OUT(cx); OUT(dx); OUT(si); OUT(di);
  out->x.cflag = rmregs.flags & 0x1;
  return out->x.ax;
}

int DPMI_int86x(int intno, RMREGS *in, RMREGS *out, RMSREGS *sregs)
/****************************************************************************
 * Function:     DPMI_int86
 * Parameters:   intno   - Interrupt number to issue
 *               in      - Pointer to structure for input registers
 *               out     - Pointer to structure for output registers
 *               sregs   - Values to load into segment registers
 * Returns:      Value returned by interrupt in AX
 * Description:  Issues a real mode interrupt using DPMI services.
 ****************************************************************************/
{
  _RMREGS         rmregs;
  union REGS      r;
  struct SREGS    sr;

  memset(&rmregs, 0, sizeof(rmregs));
  IN(ax); IN(bx); IN(cx); IN(dx); IN(si); IN(di);
  rmregs.es = sregs->es;
  rmregs.ds = sregs->ds;

  segread(&sr);
  r.w.ax = 0x300;                 /* DPMI issue real interrupt        */
  r.h.bl = intno;
  r.h.bh = 0;
  r.w.cx = 0;
  sr.es = sr.ds;
  r.x.edi = (unsigned)&rmregs;
  int386x(0x31, &r, &r, &sr);     /* Issue the interrupt */

  OUT(ax); OUT(bx); OUT(cx); OUT(dx); OUT(si); OUT(di); OUT(bp); //UV changed +bp
  sregs->es = rmregs.es;
  sregs->cs = rmregs.cs;
  sregs->ss = rmregs.ss;
  sregs->ds = rmregs.ds;
  out->x.cflag = rmregs.flags & 0x1;
  return out->x.ax;
}

int DPMI_allocSelector(void)
/****************************************************************************
 * Function:     DPMI_allocSelector
 * Returns:      Newly allocated protected mode selector
 * Description:  Allocates a new protected mode selector using DPMI
 *               services. This selector has a base address and limit of 0.
 ****************************************************************************/
{
  int         sel;
  union REGS  r;

  r.w.ax = 0;                     /* DPMI allocate selector           */
  r.w.cx = 1;                     /* Allocate a single selector       */
  int386(0x31, &r, &r);
  if(r.x.cflag) FatalError("DPMI_allocSelector() failed!");

  sel = r.w.ax;
  r.w.ax = 9;                     /* DPMI set access rights           */
  r.w.bx = sel;
  r.w.cx = 0x8092;                /* 32 bit page granular             */
  int386(0x31, &r, &r);
  return sel;
}

long DPMI_mapPhysicalToLinear(long physAddr,long limit)
/****************************************************************************
 * Function:     DPMI_mapPhysicalToLinear
 * Parameters:   physAddr    - Physical memory address to map
 *               limit       - Length-1 of physical memory region to map
 * Returns:      Starting linear address for mapped memory
 * Description:  Maps a section of physical memory into the linear address
 *               space of a process using DPMI calls. Note that this linear
 *               address cannot be used directly, but must be used as the
 *               base address for a selector.
 ****************************************************************************/
{
  union REGS  r;

  r.w.ax = 0x800;                 /* DPMI map physical to linear      */
  r.w.bx = physAddr >> 16;
  r.w.cx = physAddr & 0xFFFF;
  r.w.si = limit >> 16;
  r.w.di = limit & 0xFFFF;
  int386(0x31, &r, &r);
  if(r.x.cflag) FatalError("DPMI_mapPhysicalToLinear() failed!");

  return ((long)r.w.bx << 16) + r.w.cx;
}

void DPMI_setSelectorBase(int sel,long linAddr)
/****************************************************************************
 * Function:     DPMI_setSelectorBase
 * Parameters:   sel     - Selector to change base address for
 *               linAddr - Linear address used for new base address
 * Description:  Sets the base address for the specified selector.
 ****************************************************************************/
{
  union REGS  r;

  r.w.ax = 7;                     /* DPMI set selector base address   */
  r.w.bx = sel;
  r.w.cx = linAddr >> 16;
  r.w.dx = linAddr & 0xFFFF;
  int386(0x31, &r, &r);
  if (r.x.cflag) FatalError("DPMI_setSelectorBase() failed!");
}

void DPMI_setSelectorLimit(int sel,long limit)
/****************************************************************************
 * Function:     DPMI_setSelectorLimit
 * Parameters:   sel     - Selector to change limit for
 *               limit   - Limit-1 for the selector
 * Description:  Sets the memory limit for the specified selector.
 ****************************************************************************/
{
  union REGS  r;

  r.w.ax = 8;                     /* DPMI set selector limit          */
  r.w.bx = sel;
  r.w.cx = limit >> 16;
  r.w.dx = limit & 0xFFFF;
  int386(0x31, &r, &r);
  if (r.x.cflag) FatalError("DPMI_setSelectorLimit() failed!");
}

int DPMI_physical_address_mapping(__dpmi_meminfo *_info)/* DPMI 0.9 AX=0800 */
{
    union REGS r;
    r.w.ax = 0x800;
    r.w.bx = (short) (_info->address >> 16);
    r.w.cx = (short) (_info->address & 0xFFFF);
    r.w.si = (short) (_info->size  >>16 );
    r.w.di = (short) (_info->size   & 0xFFFF);
    int386(0x31,&r,&r);
    _info->handle = ((unsigned long)r.w.bx << 16) + (unsigned long)r.w.cx;
    return r.x.cflag;
}

int DPMI_free_physical_address_mapping(__dpmi_meminfo *_info)/* DPMI 0.9 AX=0801 */
{
    union REGS r;
    r.w.ax = 0x0801;
    int386(0x31,&r,&r);
    return r.x.cflag;
}
