/*
  DOS support for Crystal Space VESA library
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
#include "vesa.h"

/*---------------------- Macros and type definitions ----------------------*/
#pragma pack(1)

/* SuperVGA information block */
typedef struct {
    char    VESASignature[4];       /* 'VESA' 4 byte signature          */
unsigned char VESAVersionMinor;
unsigned char VESAVersion;          /* VBE version number               */
    long    OemStringPtr;           /* Pointer to OEM string            */
    long    Capabilities;           /* Capabilities of video card       */
    long    VideoModePtr;           /* Pointer to supported modes       */
    short   TotalMemory;            /* Number of 64kb memory blocks     */

    /* VBE 2.0 extensions */
    short   OemSoftwareRev;         /* OEM Software revision number     */
    long    OemVendorNamePtr;       /* Pointer to Vendor Name string    */
    long    OemProductNamePtr;      /* Pointer to Product Name string   */
    long    OemProductRevPtr;       /* Pointer to Product Revision str  */
    char    reserved[222];          /* Pad to 256 byte block size       */
    char    OemDATA[256];           /* Scratch pad for OEM data         */
    } VBE_vgaInfo;

/* SuperVGA mode information block */
typedef struct {
    short   ModeAttributes;         /* Mode attributes                  */
    char    WinAAttributes;         /* Window A attributes              */
    char    WinBAttributes;         /* Window B attributes              */
    short   WinGranularity;         /* Window granularity in k          */
    short   WinSize;                /* Window size in k                 */
    short   WinASegment;            /* Window A segment                 */
    short   WinBSegment;            /* Window B segment                 */
    long    WinFuncPtr;             /* Pointer to window function       */
    short   BytesPerScanLine;       /* Bytes per scanline               */
    short   XResolution;            /* Horizontal resolution            */
    short   YResolution;            /* Vertical resolution              */
    char    XCharSize;              /* Character cell width             */
    char    YCharSize;              /* Character cell height            */
    char    NumberOfPlanes;         /* Number of memory planes          */
    char    BitsPerPixel;           /* Bits per pixel                   */
    char    NumberOfBanks;          /* Number of CGA style banks        */
    char    MemoryModel;            /* Memory model type                */
    char    BankSize;               /* Size of CGA style banks          */
    char    NumberOfImagePages;     /* Number of images pages           */
    char    res1;                   /* Reserved                         */

    char    RedMaskSize;            /* Size of direct color red mask    */
    char    RedFieldPosition;       /* Bit posn of lsb of red mask      */

    char    GreenMaskSize;          /* Size of direct color green mask  */
    char    GreenFieldPosition;     /* Bit posn of lsb of green mask    */

    char    BlueMaskSize;           /* Size of direct color blue mask   */
    char    BlueFieldPosition;      /* Bit posn of lsb of blue mask     */

    char    RsvdMaskSize;           /* Size of direct color res mask    */
    char    RsvdFieldPosition;      /* Bit posn of lsb of res mask      */

    char    DirectColorModeInfo;    /* Direct color mode attributes     */

    /* VBE 2.0 extensions */
    long    PhysBasePtr;            /* Physical address for linear buf  */
    long    OffScreenMemOffset;     /* Pointer to start of offscreen mem*/
    short   OffScreenMemSize;       /* Amount of offscreen mem in 1K's  */
    char    res2[206];              /* Pad to 256 byte block size       */
    } VBE_modeInfo;

#define vbeMemPK        4           /* Packed Pixel memory model        */
#define vbeMemDirCol    6           /* Direct color (HiColor, 24-bit color) */
#define vbeUseLFB       0x4000      /* Enable linear framebuffer mode   */

/* Flags for the mode attributes returned by VBE_getModeInfo. If
 * vbeMdNonBanked is set to 1 and vbeMdLinear is also set to 1, then only
 * the linear framebuffer mode is available. */

#define vbeMdAvailable  0x0001      /* Video mode is available          */
#define vbeMdColorMode  0x0008      /* Mode is a color video mode       */
#define vbeMdGraphMode  0x0010      /* Mode is a graphics mode          */
#define vbeVGAcompatib  0x0020
#define vbeMdNonBanked  0x0040      /* Banked mode                      */
#define vbeMdLinear     0x0080      /* Linear mode                      */

/* Inline assembler block fill/move routines */
void LfbMemset(int sel,int off,int c,int n);
#pragma aux LfbMemset =             \
    "push   es"                     \
    "mov    es,ax"                  \
    "shr    ecx,2"                  \
    "xor    eax,eax"                \
    "mov    al,bl"                  \
    "shl    ebx,8"                  \
    "or     ax,bx"                  \
    "mov    ebx,eax"                \
    "shl    ebx,16"                 \
    "or     eax,ebx"                \
    "rep    stosd"                  \
    "pop    es"                     \
    parm [eax] [edi] [ebx] [ecx];

void LfbMemcpy(int sel,int off,void *src,int n);
#pragma aux LfbMemcpy =             \
    "push   es"                     \
    "mov    es,ax"                  \
    "shr    ecx,2"                  \
    "rep    movsd"                  \
    "pop    es"                     \
    parm [eax] [edi] [esi] [ecx];

/* Map a real mode pointer into address space */
#define LfbMapRealPointer(p)    (void*)(((unsigned)(p) >> 12) + ((p) & 0xFFFF))

/* Get the current timer tick count */
#define LfbGetTicks()       *((long*)0x46C)

#pragma pack()

/*---------------------------- Global Variables ---------------------------*/
int     VESABuf_len = 1024;         /* Length of VESABuf                */
int     VESABuf_sel = 0;            /* Selector for VESABuf             */
int     VESABuf_rseg;               /* Real mode segment of VESABuf     */
short   VESA_modeList[50];          /* List of available VBE modes      */
int     VESA_xres,VESA_yres;        /* Video mode resolution            */
int     VESA_BPP;                   /* bytes per pixel                  */
int     VESA_bytesperline;          /* Bytes per scanline for mode      */
long    VESA_imageSize;             /* Length of the video image        */
short   VESA_ver;                   /* VESA version: 1|2                */
char    far * VESA_LFBPtr;          /* Pointer to linear framebuffer    */
unsigned  short VESA_Granularity;   /* Uses for banked mode             */

/*-------------------------- VBE Interface routines -----------------------*/

static void ExitVBEBuf(void)
{
  DPMI_freeRealSeg(VESABuf_sel);
}

void VBE_initRMBuf(void)
/****************************************************************************
* Function:     VBE_initRMBuf
* Description:  Initialises the VBE transfer buffer in real mode memory.
*               This routine is called by the VESAVBE module every time
*               it needs to use the transfer buffer, so we simply allocate
*               it once and then return.
****************************************************************************/
{
  if(!VESABuf_sel)
   {
    DPMI_allocRealSeg(VESABuf_len, &VESABuf_sel, &VESABuf_rseg);
    atexit(ExitVBEBuf);
   }
}

short VBE_callESDI(RMREGS *regs, void *buffer, int size)
/****************************************************************************
* Function:     VBE_callESDI
* Parameters:   regs    - Registers to load when calling VBE
*               buffer  - Buffer to copy VBE info block to
*               size    - Size of buffer to fill
* Description:  Calls the VESA VBE and passes in a buffer for the VBE to
*               store information in, which is then copied into the users
*               buffer space. This works in protected mode as the buffer
*               passed to the VESA VBE is allocated in conventional
*               memory, and is then copied into the users memory block.
****************************************************************************/
{
  RMSREGS sregs;

  VBE_initRMBuf();
  if(!VESABuf_rseg)
  {
    printf("Real segment not allocated!!!\n");
    return 1;
  }
  sregs.es = VESABuf_rseg;
  regs->x.di = 0;
  _fmemcpy(MK_FP(VESABuf_sel,0),buffer,size);
  DPMI_int86x(0x10, regs, regs, &sregs);
  _fmemcpy(buffer,MK_FP(VESABuf_sel,0),size);

  return 0;
}

int VBE_Detect(char *ver_str)
/****************************************************************************
 * Function:     VBE_Detect
 * Parameters:   vgaInfo - Place to store the VGA information block
 * Returns:      VBE version number, or 0 if not detected.
 * Description:  Detects if a VESA VBE is out there and functioning
 *               correctly. If we detect a VBE interface we return the
 *               VGAInfoBlock returned by the VBE and the VBE version number.
 ****************************************************************************/
{
  RMREGS      regs;
  short       *p1,*p2;
  VBE_vgaInfo vgaInfo;

  /* Put 'VBE2' into the signature area so that the VBE 2.0 BIOS knows
   * that we have passed a 512 byte extended block to it, and wish
   * the extended information to be filled in.
   */
  memset(&vgaInfo, 0, sizeof(VBE_vgaInfo));
  strncpy(vgaInfo.VESASignature,ver_str,4);

  // Get the SuperVGA Information block
  regs.x.ax = 0x4F00;
  VBE_callESDI(&regs, &vgaInfo, sizeof(VBE_vgaInfo));

  if(regs.x.ax != 0x004F) return 0;
  if(vgaInfo.VESAVersion<1) {memset(&vgaInfo, 0, sizeof(VBE_vgaInfo)); return 0;}

  /* Now that we have detected a VBE interface, copy the list of available
   * video modes into our local buffer. We *must* copy this mode list, since
   * the VBE will build the mode list in the VBE_vgaInfo buffer that we have
   * passed, so the next call to the VBE will trash the list of modes.
   */
  p1 = (short *)LfbMapRealPointer(vgaInfo.VideoModePtr);
  p2 = VESA_modeList;

  while(*p1 != -1) *p2++ = *p1++;
  *p2 = -1;

  return vgaInfo.VESAVersion;
}

int VBE_getModeInfo(int mode,VBE_modeInfo *modeInfo)
/****************************************************************************
 * Function:     VBE_getModeInfo
 * Parameters:   mode        - VBE mode to get information for
 *               modeInfo    - Place to store VBE mode information
 * Returns:      1 on success, 0 if function failed.
 * Description:  Obtains information about a specific video mode from the
 *               VBE. You should use this function to find the video mode
 *               you wish to set, as the new VBE 2.0 mode numbers may be
 *               completely arbitrary.
 ****************************************************************************/
{
  RMREGS  regs;

  regs.x.ax = 0x4F01;     // Get mode information
  regs.x.cx = mode;
  memset(modeInfo, 0, sizeof(VBE_modeInfo));
  VBE_callESDI(&regs, modeInfo, sizeof(VBE_modeInfo));

  if(regs.h.al != 0x4F) return 0;
  if((modeInfo->ModeAttributes & vbeMdAvailable) == 0) return 0;
  return 1;
}

void VBE_setVideoMode(int mode)
/****************************************************************************
 * Function:     VBE_setVideoMode
 * Parameters:   mode    - VBE mode number to initialise
 ****************************************************************************/
{
  RMREGS  regs;
  regs.x.ax = 0x4F02;
  regs.x.bx = mode;
  DPMI_int86(0x10,&regs,&regs);
}

/*-------------------- Application specific routine -----------------------*/
void far *GetPtrToLFB(long physAddr)
/****************************************************************************
 * Function:     GetPtrToLFB
 * Parameters:   physAddr    - Physical memory address of linear framebuffer
 * Returns:      Far pointer to the linear framebuffer memory
 ****************************************************************************/
{
  int     sel;
  long    linAddr,limit = (4096 * 1024) - 1;

  sel = DPMI_allocSelector();
  linAddr = DPMI_mapPhysicalToLinear(physAddr,limit);
  DPMI_setSelectorBase(sel,linAddr);
  DPMI_setSelectorLimit(sel,limit);
  return MK_FP(sel,0);
}

void AvailableModes(void)
/****************************************************************************
 * Function:     AvailableModes
 * Description:  Display a list of available LFB mode resolutions.
 ****************************************************************************/
{
  short           *p;
  VBE_modeInfo    modeInfo;
  int             debug_handle;

  printf("Available 256 color video modes:\n");

  // Available LFB modes
  for(p = VESA_modeList; *p != -1; p++)
   {
    if(VBE_getModeInfo(*p, &modeInfo))
     {// Filter out only 8 bit linear framebuffer modes
      if((modeInfo.ModeAttributes & vbeMdLinear) == 0) continue;
      if(modeInfo.MemoryModel != vbeMemPK
         || modeInfo.BitsPerPixel != 8
         || modeInfo.NumberOfPlanes != 1) continue;
      printf("LFB:    %4d x %4d %d bits per pixel\n",
          modeInfo.XResolution, modeInfo.YResolution,
          modeInfo.BitsPerPixel);
     }
   }

  // Available banked modes
  for(p = VESA_modeList; *p != -1; p++)
   {
    if(VBE_getModeInfo(*p, &modeInfo))
     {// Filter out only 8 bit banked modes
      if((modeInfo.ModeAttributes & vbeMdLinear) == 1) continue;
//      if(modeInfo.MemoryModel != vbeMemPK
//         || modeInfo.BitsPerPixel != 8
//         || modeInfo.NumberOfPlanes != 1) continue;
      if(modeInfo.BitsPerPixel != 8) continue;
      printf("Banked: %4d x %4d %d bits per pixel\n",
          modeInfo.XResolution, modeInfo.YResolution,
          modeInfo.BitsPerPixel);
     }
   }
}

typedef signed    long  int SLONG;    //32 bit signed.
typedef unsigned  long  int ULONG;    //32 bit unsigned.
typedef signed    short int SWORD;    //16 bit signed.
typedef unsigned  short int UWORD;    //16 bit unsigned.
typedef signed    char    SBYTE;    //8 bit signed.
typedef unsigned  char    UBYTE;    //8 bit unsigned.

void VBE_setBank(SLONG vpage)
/****************************************************************************
 * Function:    VBE_setBank
 * Description: Set bank of video card memory in banked mode
 ****************************************************************************/
{
  RMREGS  regs;

  //VESA_Page = vpage;
  regs.x.ax = 0x4f05;
  regs.x.bx = 0;
  regs.x.cx = 0;
  regs.x.dx = (UWORD)(vpage == 0 ? 0 : (UWORD)((vpage << 6) / VESA_Granularity));
  DPMI_int86(0x10,&regs,&regs);
  regs.x.ax = 0x4f05;
  regs.x.bx = 1;
  regs.x.cx = 0;
  regs.x.dx = (UWORD)(vpage == 0 ? 0 : (UWORD)((vpage << 6) / VESA_Granularity));
  DPMI_int86(0x10,&regs,&regs);
}

short VESA_SetScanLineLength(ULONG newcx,UWORD VBE_par)
{
  RMREGS  regs;

  regs.x.ax = 0x4F06;
  regs.x.bx = VBE_par; //0 - VBE1, 2 - VBE2
  regs.x.cx = newcx;
  DPMI_int86(0x10,&regs,&regs);

  if(regs.x.ax!=0x004F)
   {
    regs.x.ax = 0x4F06;
    regs.x.bx = 0x0001;
    DPMI_int86(0x10,&regs,&regs);

    if(regs.x.ax!=0x004F)
     {
      VESA_bytesperline = regs.x.bx;
      return 1;
     }
    }

  return 0;
}

void VESA_SetStartOffset(ULONG x, ULONG y)
{
  RMREGS  regs;

  regs.x.ax = 0x4F07;
  regs.x.bx = 0x0000;
  regs.x.cx = x;
  regs.x.dx = y;
  DPMI_int86(0x10,&regs,&regs);
}

short VESA_ScanMode(int x,int y,int Depth,HiColor *Color)
/****************************************************************************
 * Function:     ScanMode
 * Parameters:   x,y - Requested video mode resolution
 * Description:  Initialise the specified video mode. We search through
 *               the list of available video modes for one that matches
 *               the resolution and color depth are are looking for.
 ****************************************************************************/
{
  short           *p;
  int             bpp =Depth>>3; // div 8
  VBE_modeInfo    modeInfo;

  //Search LFB mode
  for(p = VESA_modeList; *p != -1; p++)
   {
    if(VBE_getModeInfo(*p, &modeInfo))
     {
      // Filter out only 8 bit linear framebuffer modes
      //printf("Cur Mode X Res: %d Y Res: %d\n", modeInfo.XResolution, modeInfo.YResolution);

      if((modeInfo.ModeAttributes & vbeMdLinear) == 0) continue; //If not LFB -> next mode
      if(modeInfo.BitsPerPixel != Depth || modeInfo.NumberOfPlanes != 1) continue;
      if(modeInfo.XResolution != x || modeInfo.YResolution != y) continue;

//      if((modeInfo.RedMaskSize+modeInfo.GreenMaskSize+modeInfo.BlueMaskSize)==16) continue;
//      if((modeInfo.RedMaskSize+modeInfo.GreenMaskSize+modeInfo.BlueMaskSize)==15) continue;

      if(bpp==16&&modeInfo.MemoryModel!=vbeMemDirCol) continue;
       else
        {
         Color->RedMaskSize =modeInfo.RedMaskSize;
         Color->RedFieldPosition =modeInfo.RedFieldPosition;
         Color->GreenMaskSize =modeInfo.GreenMaskSize;
         Color->GreenFieldPosition =modeInfo.GreenFieldPosition;
         Color->BlueMaskSize =modeInfo.BlueMaskSize;
         Color->BlueFieldPosition =modeInfo.BlueFieldPosition;
        }

      if(bpp==8&&modeInfo.MemoryModel!=vbeMemPK) continue;

      return 2;
     }
   }

  //Search Bank mode
  for(p = VESA_modeList; *p != -1; p++)
   {
    if(VBE_getModeInfo(*p, &modeInfo))
     {
      // Filter out only 8 bit banked modes
      //printf("Cur Mode X Res: %d Y Res: %d\n", modeInfo.XResolution, modeInfo.YResolution);

      if((modeInfo.ModeAttributes & vbeMdNonBanked) == 1 ) continue; //if non banked mode
      if(modeInfo.BitsPerPixel != Depth) continue;
      if(modeInfo.XResolution != x || modeInfo.YResolution != y) continue;

      if(bpp==16&&modeInfo.MemoryModel!=vbeMemDirCol) continue;
       else
        {
         Color->RedMaskSize =modeInfo.RedMaskSize;
         Color->RedFieldPosition =modeInfo.RedFieldPosition;
         Color->GreenMaskSize =modeInfo.GreenMaskSize;
         Color->GreenFieldPosition =modeInfo.GreenFieldPosition;
         Color->BlueMaskSize =modeInfo.BlueMaskSize;
         Color->BlueFieldPosition =modeInfo.BlueFieldPosition;
        }

      if(bpp==8&&modeInfo.MemoryModel!=vbeMemPK) continue;

      return 1;
     }
   }

  return 0;
}

short VESA_InitGraphics(int x,int y,int Depth)
/****************************************************************************
 * Function:     InitGraphics
 * Parameters:   x,y - Requested video mode resolution
 * Description:  Initialise the specified video mode. We search through
 *               the list of available video modes for one that matches
 *               the resolution and color depth are are looking for.
 ****************************************************************************/
{
  short           *p;
  int             bpp =Depth>>3; // div 8
  VBE_modeInfo    modeInfo;

  //Search & set LFB mode
  for(p = VESA_modeList; *p != -1; p++)
   {
    if(VBE_getModeInfo(*p, &modeInfo))
     {
      // Filter out only 8 bit linear framebuffer modes
      //printf("Cur Mode X Res: %d Y Res: %d\n", modeInfo.XResolution, modeInfo.YResolution);

      if((modeInfo.ModeAttributes & vbeMdLinear) == 0) continue; //If not LFB -> next mode
      if(modeInfo.BitsPerPixel != Depth || modeInfo.NumberOfPlanes != 1) continue;
      if(modeInfo.XResolution != x || modeInfo.YResolution != y) continue;

//      if((modeInfo.RedMaskSize+modeInfo.GreenMaskSize+modeInfo.BlueMaskSize)==16) continue;
//      if((modeInfo.RedMaskSize+modeInfo.GreenMaskSize+modeInfo.BlueMaskSize)==15) continue;

      if(bpp==16&&modeInfo.MemoryModel!=vbeMemDirCol) continue;
      if(bpp==8&&modeInfo.MemoryModel!=vbeMemPK) continue;

      VBE_setVideoMode(*p | vbeUseLFB);
      VESA_ver =2;
      VESA_BPP =bpp;
      VESA_xres = x;
      VESA_yres = y;
      VESA_SetStartOffset(0,0);
      VESA_bytesperline = modeInfo.BytesPerScanLine;
      if(VESA_bytesperline!=VESA_xres*bpp) VESA_SetScanLineLength(VESA_xres * bpp,2);
      VESA_imageSize = VESA_bytesperline * VESA_yres;
      VESA_LFBPtr = (char __far *)GetPtrToLFB(modeInfo.PhysBasePtr);

      return VESA_ver;
     }
   }

  //Search & set Banked mode
  for(p = VESA_modeList; *p != -1; p++)
   {
    if(VBE_getModeInfo(*p, &modeInfo))
     {
      // Filter out only 8 bit banked modes
      //printf("Cur Mode X Res: %d Y Res: %d\n", modeInfo.XResolution, modeInfo.YResolution);

      if((modeInfo.ModeAttributes & vbeMdNonBanked) == 1 ) continue; //if non banked mode
      if(modeInfo.BitsPerPixel != Depth) continue;
      if(modeInfo.XResolution != x || modeInfo.YResolution != y) continue;

      if(bpp==16&&modeInfo.MemoryModel!=vbeMemDirCol) continue;
      if(bpp==8&&modeInfo.MemoryModel!=vbeMemPK) continue;

      VBE_setVideoMode(*p);
      VESA_ver =1;
      VESA_BPP =bpp;
      VESA_xres = x;
      VESA_yres = y;
      VESA_SetStartOffset(0,0);
      VESA_bytesperline = modeInfo.BytesPerScanLine;
      if(VESA_bytesperline!=VESA_xres*bpp) VESA_SetScanLineLength(VESA_xres*bpp,0);
      VESA_imageSize = VESA_bytesperline * VESA_yres;
      VESA_Granularity =modeInfo.WinGranularity;
      VBE_setBank(0);

      return VESA_ver;
     }
   }

  return 0;
}

void VESA_EndGraphics(void)
/****************************************************************************
 * Function:     EndGraphics
 * Description:  Restores text mode.
 ****************************************************************************/
{
  RMREGS  regs;
  regs.x.ax = 0x3;
  DPMI_int86(0x10, &regs, &regs);
}

UBYTE   *VESA_VideoMemory=(UBYTE *)(0xa000<<4);

void BankedCopy2Screen(unsigned char *src, int ncopy)
/****************************************************************************
 * Function:    BankedCopy2Screen
 * Description: Copies a bloks of memory on the screen in banked mode
 ****************************************************************************/
{
  SLONG page =0;

  while(ncopy>0)
   {
    VBE_setBank(page);
    memcpy((void *)(VESA_VideoMemory), src, (ncopy>65536?65536:ncopy));
    src =(unsigned char *)((int)src + 65536);
    ncopy -= 65536;
    page++;
   }
}

void VESA_ScreenCopy(void * src)
/****************************************************************************
 * Function:     VESA_ScreenCopy
 * Description:  Copies a block of memory to a location on the screen
 ****************************************************************************/
{
  int ncopy =VESA_xres * VESA_yres * VESA_BPP;

  if(VESA_ver==2) {LfbMemcpy(FP_SEG(VESA_LFBPtr), 0, src, ncopy); return;}
  if(VESA_ver==1) {BankedCopy2Screen(src, ncopy); return;}
}

void CopyBankedLine(UWORD x, UWORD y, ULONG offset, void *src, long ncopy)
/****************************************************************************
 * Function:     CopyBankedLine
 * Description:  Copies a line of memory to a location on the screen in banked
 ****************************************************************************/
{
  ULONG page =offset/65536;
  ULONG last_line, piece;

  VBE_setBank(page);
  offset=offset%65536;

  if(offset+ncopy<=65536) memcpy((void *)VESA_VideoMemory[offset],src,ncopy);
   else
    {
     //Setup on limit page
     piece =65536 -ncopy;
     last_line =ncopy -piece;
     memcpy((void *)VESA_VideoMemory[offset],src,piece);
     VBE_setBank(++page);
     src =(void*)((int)src+piece);

     while(last_line)
      {
       memcpy((void *)VESA_VideoMemory[0],src,(last_line > 65536 ? 65536 : last_line));
       VBE_setBank(++page);
       src =(void*)((int)src +65536);
       last_line -=(last_line > 65536 ? 65536 : last_line);
      }
    }
}

void VESA_UpdateScreen(long X1, long Y1, long X2, long Y2, void * src)
/****************************************************************************
 * Function:     VESA_UpdateScreen
 * Description:  Copies a block of memory to a location on the screen
 ****************************************************************************/
{
  long ncopy =X2 -X1;
  long offset;

  for(Y1;Y1<=Y2;Y1++)
   {
    offset =Y1 * VESA_bytesperline + X1;
    if(VESA_ver==2) LfbMemcpy(FP_SEG(VESA_LFBPtr), offset, src, ncopy * VESA_BPP);
    if(VESA_ver==1) CopyBankedLine(X1, Y1, offset, src, ncopy * VESA_BPP);
    src =(void*)((int)src +VESA_bytesperline); //Set on next line
   }
}

