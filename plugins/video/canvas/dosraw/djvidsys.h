/*
    DOS support for Crystal Space 3D library
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by David N. Arnold <derek_arnold@fuse.net>
    Written by Andrew Zabolotny <bit@eltech.ru>

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

/*
 * Written by David N. Arnold. <derek_arnold@fuse.net>
 * Full featured VGA 320x200 support and various VESA
 * resolutions at 256 colors.
 */

#ifndef _VIDSYS_H_
#define _VIDSYS_H_

#include <dpmi.h>
#include <sys/farptr.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <go32.h>
#include <pc.h>

#pragma pack (1)

struct csRGB
{
  unsigned char r, g, b;
};

typedef csRGB csVGApalette[256];

// VESA information structure
struct VESAInformation
{
  char Signature [4];                   // signature ("VESA"); on call,
                                        // VESA 2.0 request signature ("VBE2"),
                                        // required to receive version 2.0 info
  unsigned short Version;               // VESA version number (one-digit minor version)
  void *OEM;                            // pointer to OEM name; "761295520" for ATI
  unsigned int Capabilities;            // Bitfields for VESA capabilities:
                                        // Bit(s)        Description        (Table 0076)
                                        //  0        DAC can be switched into 8-bit mode
                                        //  1        non-VGA controller
                                        //  2        programmed DAC with blank bit
                                        //  3-31        reserved
  void *VideoModeList;                  // pointer to list of supported video modes
  unsigned short VideoRAM;              // total amount of video memory in 64K blocks
//---- only for VESA 2.0
  unsigned short OEMVersion;            // OEM software version
  void *Vendor;                         // pointer to vendor name
  void *Product;                        // pointer to product name
  void *Revision;                       // pointer to product revision string
  char Reserved [222];
  char Scratchpad [256];                // OEM scratchpad
};

/// VESA mode description structure
struct VESAModeInfoBlock
{
  unsigned short ModeAttributes;        // Bitfields for VESA SuperVGA mode attributes:
                                        // Bit(s)        Description        (Table 0078)
                                        //  0        mode supported
                                        //  1        optional information available
                                        //  2        BIOS output supported
                                        //  3        set if color, clear if monochrome
                                        //  4        set if graphics mode, clear if text mode
                                        // ---VBE v2.0 ---
                                        //  5        mode is not VGA-compatible
                                        //  6        bank-switched mode not supported
                                        //  7        linear framebuffer mode supported
  unsigned char WindowA_Attributes;     // Bitfields for VESA SuperVGA window attributes:
  unsigned char WindowB_Attributes;     // Bit(s)        Description        (Table 0079)
                                        //  0        exists
                                        //  1        readable
                                        //  2        writable
                                        //  3-7        reserved
  unsigned short WindowGranularity;
  unsigned short WindowSize;
  unsigned short WindowA_Seg;
  unsigned short WindowB_Seg;
  void *PositioningFunction;
  unsigned short BytesPerScanLine;
  // optional data (ModeAttributes & 0x02) follows
  unsigned short XResolution;
  unsigned short YResolution;
  unsigned char CharacterCellWidth;
  unsigned char CharacterCellHeight;
  unsigned char MemoryPlanes;
  unsigned char BitsPerPixel;
  unsigned char NumberOfBanks;
  unsigned char MemoryModel;            // Values for VESA SuperVGA memory model type:
                                        //  00h        text
                                        //  01h        CGA graphics
                                        //  02h        HGC graphics
                                        //  03h        16-color (EGA) graphics
                                        //  04h        packed pixel graphics
                                        //  05h        "sequ 256" (non-chain 4) graphics
                                        //  06h        direct color (HiColor, 24-bit color)
                                        //  07h        YUV (luminance-chrominance, also called YIQ)
                                        //  08h-0Fh reserved for VESA
                                        //  10h-FFh OEM memory models
  unsigned char BankSize;
  unsigned char VideoPages;
  unsigned char Reserved1;
  unsigned char RedMaskBits;
  unsigned char RedMaskShift;
  unsigned char GreenMaskBits;
  unsigned char GreenMaskShift;
  unsigned char BlueMaskBits;
  unsigned char BlueMaskShift;
  unsigned char AlphaMaskBits;
  unsigned char AlphaMaskShift;
  unsigned char ColorModeInfo;          // bit 0: color ramp is programmable
                                        // bit 1: bytes in reserved field may be used by application
// ---VBE v2.0 ---
  unsigned int PhysicalAddress;
  unsigned int OffscreenMemory;
  unsigned short OffscreenMemorySize;
  unsigned char Reserved [206];
};

#pragma pack ()

/**
 * Auxiliary helper class for SysGraphics2D when not using Allegro<p>
 * This video driver can use back buffers if enough video RAM is available:
 * this allows us to use page flip instead of copy operation which is
 * substantialy faster.
 */
class VideoSystem
{
public:
  /// Video susbsystem constructor: fix up Width and Height to closest possible
  VideoSystem ();
  /// Video susbsystem destructor
  ~VideoSystem ();

  /// Find closest vide mode; return false if failed; maybe set W & H to closest
  bool FindMode (int Width, int Height, int Depth, int &PaletteSize,
    long &RedMask, long &GreenMask, long &BlueMask);
  /// Flushes double buffer out to VRAM
  void Flush (int x, int y, int w, int h);
  /// Puts current pal in Palette
  void GetPalette (csVGApalette &Palette);
  /// Writes out Palette
  void SetPalette (csVGApalette &Palette);
  /// Clear current buffer
  void Clear (int Color);
  /// Open graphics screen; return error code
  int Open ();
  /// Close graphics screen
  void Close ();
  /// Return address of back buffer
  unsigned char *BackBuffer ();
  /// Enable or disable double buffering
  bool DoubleBuffer (bool Enable);
  /// Return double buffering state
  bool DoubleBuffer ();
  /// Return active video page
  inline int GetPage ()
  { return VideoPage; }
  /// Enable/disabling wait for vertical retrace
  inline void WaitVRetrace (bool Enable)
  { WaitVR = Enable; }

private:
  /// A pointer to linearly mapped video RAM
  unsigned char *VRAM;
  /// Second buffer if it is not available in video RAM
  unsigned char *VRAMBuffer;
  /// Selector of video RAM if linear address unavailable
  unsigned short VRAMSelector;
  /// VESA BIOS extensions version number
  int VESAversion;
  /// Selected video mode number
  int VideoMode;
  /// Is selected video mode a X-mode?
  bool IsXmode;
  /// Is selected video mode banked?
  bool IsBanked;
  /// How many linearly addressable video pages fits in video RAM?
  int VideoPages;
  /// Bytes per scan line
  unsigned int BytesPerScanLine;
  /// Bytes per pixel
  unsigned int BytesPerPixel;
  /// VESA SVGA window granularity
  unsigned int WindowGranularity;
  /// VESA SVGA window size
  unsigned int WindowSize;
  /// Scan lines per page
  unsigned int ScanLinesPerPage;
  /// Current video page we are writing to
  unsigned int VideoPage;
  /// Screen image size
  int ScreenSize;
  /// Size of one videopage
  int PageSize;
  /// Total amount of video RAM on board
  unsigned int VideoRAM;
  /// Wait for vertical retrace when switching pages?
  bool WaitVR;
  /// Use two videopages if available?
  bool UseDoubleBuffering;

  bool AllocateBackBuffer ();
  void FreeBackBuffer ();
  void VESAFillModeBlock (VESAModeInfoBlock &mb);
  void VESASetBank(int Bank);
  void VESASetPage (int Page);
  void XSetPage (int Page);
};

#endif // _VIDSYS_H_
