/*
  DOS support for Crystal Space 3D library
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

#ifdef COMP_WCC
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <dos.h>
#include <i86.h>
#include <string.h>
#include <time.h>
#include <sys\timeb.h>

#include "system/sysdef.h"
#include "system/system.h"
#include "util/inifile.h"

#include "system/wcc/include/video.h"
#include "keyboard.h"
#include "mouse.h"
#include "watdpmi.h"

int mTaskSystem, mTaskVersion;

//== class SysGraphics2D =======================================================

int ColorMask(int MaskSize)
{
    int mask=1,i;

    for(i=0;i<(MaskSize-1);i++) mask =(mask<<1)|1;
    return mask;
}

int SysGraphics2D::Depth;

// SysGraphics2D functions
SysGraphics2D::SysGraphics2D(int argc, char *argv[]) : csGraphics2D ()
{
  (void)argc; (void)argv;
  HiColor Color;
  int RR,GG,BB;

  Printf (MSG_INITIALIZATION, "Crystal Space DOS version.\n");
  Memory = NULL;

  if(GetGraphMode(&FRAME_WIDTH,&FRAME_HEIGHT,&Depth,&Color))
  {
    Printf (MSG_FATAL_ERROR, "No VGA or VESA Super VGA adapter detected!\n");
    exit(-1);
  }

  switch (Depth)
  {
    case 16:
      RR =ColorMask(Color.RedMaskSize)<<Color.RedFieldPosition;
      GG =ColorMask(Color.GreenMaskSize)<<Color.GreenFieldPosition;
      BB =ColorMask(Color.BlueMaskSize)<<Color.BlueFieldPosition;

      pfmt.PalEntries = 0;
      pfmt.PixelBytes = 2;
      pfmt.RedMask    = RR; //0xf800;
      pfmt.GreenMask  = GG; //0x07e0;
      pfmt.BlueMask   = BB; //0x001f;
      break;

    case 8:
    default:
      RR =GG =BB =0xff;
      pfmt.PalEntries = 256;
      pfmt.PixelBytes = 1;
      pfmt.RedMask    = RR; //0xff
      pfmt.GreenMask  = GG; //0xff
      pfmt.BlueMask   = BB; //0xff
      break;

  } /* endswitch */

  complete_pixel_format ();

  if ((Memory=(unsigned char *)malloc(FRAME_WIDTH*FRAME_HEIGHT*pfmt.PixelBytes))==NULL)
  {
    Printf (MSG_FATAL_ERROR, "Cannot allocate virtual graphics memory!\n");
    exit (-1);
  }
  memset (Memory,0,FRAME_WIDTH*FRAME_HEIGHT*pfmt.PixelBytes); //Fill shadow memory

  if ((WidthAddress=(int *)malloc(sizeof(int)*FRAME_HEIGHT))==NULL)
  {
    Printf (MSG_FATAL_ERROR, "Cannot allocate memory!\n");
    exit(-1);
  }
  for (int i=0; i<FRAME_HEIGHT; i++)
    WidthAddress [i] = i * FRAME_WIDTH * pfmt.PixelBytes;
}

SysGraphics2D::~SysGraphics2D(void)
{
  Close();
}

bool SysGraphics2D::Open(const char *Title)
{
  if (!csGraphics2D::Open (Title))
    return false;

  if(!SetGraphMode(FRAME_WIDTH,FRAME_HEIGHT,Depth,Memory))
  {
    Printf (MSG_FATAL_ERROR, "Cannot find a suitable videomode match\n");
    return false;
  }

  // Update drawing routines on 16 bits per pixel
  if (pfmt.PixelBytes == 2)
  {
    DrawPixel = DrawPixel16;
    WriteChar = WriteChar16;
    GetPixelAt = GetPixelAt16;
    DrawSprite = DrawSprite16;
  } /* endif */

  printf_Enable (false);
  return true;
}

void SysGraphics2D::Close(void)
{
  RestoreVMode ();
  if (Memory)
    free (Memory);
  printf_Enable (true);
}

void SysGraphics2D::Print(csRect *area)
{
  void *src;

  if (Memory)
//   if(area)
//    {
//     src =(void*)((int)Memory + (FRAME_WIDTH * area->ymin) + area->xmin);
//     UpdateScreen(area->xmin, area->ymin, area->xmax, area->ymax, src);
//    } else
    CopyShadowToScreen (Memory);
}

void SysGraphics2D::Clear (int color)
{
  memset (Memory, color, Width * Height * pfmt.PixelBytes);
}

void SysGraphics2D::SetRGB(int i, int r, int g, int b)
{
  // set a rgb color in the palette of your graphic interface
  if (i < 0 && i > 255) return;
  SetPalette(i,r,g,b);
  csGraphics2D::SetRGB (i, r, g, b);
}

bool SysGraphics2D::SetMouseCursor (int iShape, IMipMapContainer *iBitmap)
{
  (void)iShape; (void)iBitmap;
  return false;
}

//== class SysKeyboardDriver ===================================================
SysKeyboardDriver::SysKeyboardDriver () : csKeyboardDriver ()
{
  // Do nothing: keyboard initializes along with graphics
}

SysKeyboardDriver::~SysKeyboardDriver (void)
{
  Close ();
}

bool SysKeyboardDriver::Open (csEventQueue *EvQueue)
{
  csKeyboardDriver::Open (EvQueue);
  InstallKeyboardHandler ();
  return (1);
}

void SysKeyboardDriver::Close (void)
{
  csKeyboardDriver::Close ();
  DeinstallKeyboardHandler ();
}

unsigned short ScancodeToChar[128] =
{
  CSKEY_ESC,27,       '1',      '2',      '3',      '4',      '5',      '6',	// 00..07
  '7',      '8',      '9',      '0',      '-',      '=',      '\b',     '\t',	// 08..0F
  'q',      'w',      'e',      'r',      't',      'y',      'u',      'i',	// 10..17
  'o',      'p',      '[',      ']',      '\n',     CSKEY_CTRL,'a',     's',	// 18..1F
  'd',      'f',      'g',      'h',      'j',      'k',      'l',      ';',	// 20..27
  39,       '`',      CSKEY_SHIFT,'\\',   'z',      'x',      'c',      'v',	// 28..2F
  'b',      'n',      'm',      ',',      '.',      '/',      CSKEY_SHIFT,'*',	// 30..37
  CSKEY_ALT,' ',      0,        CSKEY_F1, CSKEY_F2, CSKEY_F3, CSKEY_F4, CSKEY_F5,// 38..3F
  CSKEY_F6,  CSKEY_F7, CSKEY_F8, CSKEY_F9, CSKEY_F10,0,       0,        CSKEY_HOME,// 40..47
  CSKEY_UP,  CSKEY_PGUP,'-',    CSKEY_LEFT,CSKEY_CENTER,CSKEY_RIGHT,'+',CSKEY_END,// 48..4F
  CSKEY_DOWN,CSKEY_PGDN,CSKEY_INS,CSKEY_DEL,0,      0,        0,        CSKEY_F11,// 50..57
  CSKEY_F12,0,        0,        0,        0,        0,        0,        0,	// 58..5F
  0,        0,        0,        0,        0,        0,        0,        0,	// 60..67
  0,        0,        0,        0,        0,        0,        0,        0,	// 68..6F
  0,        0,        0,        0,        0,        0,        0,        0,	// 70..77
  0,        0,        0,        0,        0,        0,        0,        0	// 78..7F
};

void SysKeyboardDriver::GenerateEvents()
{
  while (keyboard_queue_tail != keyboard_queue_head)
  {
    unsigned char c = keyboard_queue [keyboard_queue_tail];
    if (c < 0x80)
      do_keypress (ScancodeToChar[c]);
    else
      do_keyrelease (ScancodeToChar[c & 0x7f]);
    keyboard_queue_tail = (keyboard_queue_tail + 1) & KEYBOARD_QUEUE_MASK;
  }
}

//== class SysMouseDriver ======================================================

int MouseOpened;
int oldX, oldY, oldB;

SysMouseDriver::SysMouseDriver () : csMouseDriver ()
{
  MouseExists = MousePresent();
  if (MouseExists)
    Printf (MSG_INITIALIZATION, "Mouse detected.\n");
  else
    Printf (MSG_INITIALIZATION, "No mouse detected !\n");
}

SysMouseDriver::~SysMouseDriver (void)
{
  Close();
}

bool SysMouseDriver::Open (csEventQueue *EvQueue)
{
  csMouseDriver::Open (EvQueue);
  // Open mouse system
  if (MouseExists)
    InstallMouseHandler ();
  return true;
}

void SysMouseDriver::Close ()
{
  csMouseDriver::Close ();
  // Close mouse system
  if (MouseExists)
    DeinstallMouseHandler ();
}

void SysMouseDriver::GenerateEvents ()
{
  while (mouse_queue_head != mouse_queue_tail)
  {
    if (mouse_queue [mouse_queue_tail].button == 0)
      do_mousemotion (mouse_queue [mouse_queue_tail].x,
        mouse_queue [mouse_queue_tail].y);
    else if (mouse_queue [mouse_queue_tail].down)
      do_buttonpress (mouse_queue [mouse_queue_tail].button,
        mouse_queue [mouse_queue_tail].x,
        mouse_queue [mouse_queue_tail].y,
        System->Keyboard->Key.shift,
        System->Keyboard->Key.alt,
        System->Keyboard->Key.ctrl);
    else
      do_buttonrelease (mouse_queue [mouse_queue_tail].button,
        mouse_queue [mouse_queue_tail].x,
        mouse_queue [mouse_queue_tail].y);
  }
}

//== class SysSystemDriver =====================================================

SysSystemDriver::SysSystemDriver () : csSystemDriver ()
{
  DetectMultiTask ();
  if (mTaskSystem)
    Printf (MSG_INITIALIZATION, "Running under %s version %d.%d\n",
      mTaskSystem == mtOS2 ? "OS/2" : "Windows", mTaskVersion >> 8,
      mTaskVersion & 0xFF);
}

unsigned short XKeyChar =0xffff;
unsigned short YKeyChar =0xffff;

// System loop !
void SysSystemDriver::Loop(void)
{
  Shutdown = 0;

  while (!Shutdown && !ExitLoop)
  {
    ((SysKeyboardDriver *)Keyboard)->GenerateEvents();
    ((SysMouseDriver *)Mouse)->GenerateEvents();
    NextFrame ();
  } // while (!Shutdown && !ExitLoop)
}

void SysSystemDriver::Sleep (int SleepTime)
{
  GiveUpTimeSlice ();
}

#endif //COMP_WCC
