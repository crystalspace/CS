/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#include <stdarg.h>

#include "sysdef.h"
#include "cs2d/svgalib/svga.h"
#include "cssys/unix/iunix.h"
#include "csutil/csrect.h"
#include "csinput/csinput.h"
#include "isystem.h"

static unsigned short ScanCodeToChar[128] =
{
  CSKEY_ESC,27,       '1',      '2',      '3',      '4',      '5',      '6',    // 00..07
  '7',      '8',      '9',      '0',      '-',      '=',      '\b',     '\t',   // 08..0F
  'q',      'w',      'e',      'r',      't',      'y',      'u',      'i',    // 10..17
  'o',      'p',      '[',      ']',      '\n',     CSKEY_CTRL,'a',     's',    // 18..1F
  'd',      'f',      'g',      'h',      'j',      'k',      'l',      ';',    // 20..27
  39,       '`',      CSKEY_SHIFT,'\\',   'z',      'x',      'c',      'v',    // 28..2F
  'b',      'n',      'm',      ',',      '.',      '/',      CSKEY_SHIFT,'*',  // 30..37
  CSKEY_ALT,' ',      0,        CSKEY_F1, CSKEY_F2, CSKEY_F3, CSKEY_F4, CSKEY_F5,// 38..3F
  CSKEY_F6,  CSKEY_F7, CSKEY_F8, CSKEY_F9, CSKEY_F10,0,       0,        CSKEY_HOME,// 40..47
  CSKEY_UP,  CSKEY_PGUP,'-',    CSKEY_LEFT,CSKEY_CENTER,CSKEY_RIGHT,'+',CSKEY_END,// 48..4F
  CSKEY_DOWN,CSKEY_PGDN,CSKEY_INS,CSKEY_DEL,0,      0,        0,        CSKEY_F11,// 50..57
  CSKEY_F12,0,        0,        0,        0,        0,        0,        0,      // 58..5F
  CSKEY_ENTER,CSKEY_CTRL,'/',   0,CSKEY_ALT,        0,CSKEY_HOME,CSKEY_UP,      // 60..67
  CSKEY_PGUP,CSKEY_LEFT,CSKEY_RIGHT,CSKEY_END,CSKEY_DOWN,CSKEY_PGDN,CSKEY_INS,CSKEY_DEL,// 68..6F
  0,        0,        0,        0,        0,        0,        0,        0,      // 70..77
  0,        0,        0,        0,        0,        0,        0,        0       // 78..7F
};

BEGIN_INTERFACE_TABLE(csGraphics2DSVGALib)
  IMPLEMENTS_COMPOSITE_INTERFACE_EX( IGraphics2D, XGraphics2D )
  IMPLEMENTS_COMPOSITE_INTERFACE_EX( IGraphicsInfo, XGraphicsInfo )
END_INTERFACE_TABLE()

IMPLEMENT_UNKNOWN_NODELETE(csGraphics2DSVGALib)

// csGraphics2DSVGALib functions
csGraphics2DSVGALib::csGraphics2DSVGALib(ISystem* piSystem) : csGraphics2D (piSystem)
{
  System = piSystem;
  if (FAILED (System->QueryInterface (IID_IUnixSystemDriver, (void**)&UnixSystem)))
  {
    CsPrintf (MSG_FATAL_ERROR, "FATAL: The system driver does not support "
                               "the IUnixSystemDriver interface\n");
    exit (-1);
  }
}

void csGraphics2DSVGALib::Initialize ()
{
  csGraphics2D::Initialize ();

  Font = 0;
  Memory = NULL;

  // SVGALIB Starts here

  CsPrintf (MSG_INITIALIZATION, "Crystal Space Linux/SVGALIB version.\n");
  CsPrintf (MSG_INITIALIZATION,  "Using %dx%dx%d resolution.\n\n", Width, Height, Depth);

  gl_copyscreen (&physicalscreen);

  Memory = NULL;

  switch (Depth)
  {
    case 8:
      pfmt.RedMask = pfmt.GreenMask = pfmt.BlueMask = 0;
      pfmt.PalEntries = 256; pfmt.PixelBytes = 1;
      break;

    case 15:
      pfmt.RedMask   = 0x1f << 10;
      pfmt.GreenMask = 0x1f << 5;
      pfmt.BlueMask  = 0x1f;
      pfmt.PalEntries = 0;
      pfmt.PixelBytes = 2;

      DrawPixel = DrawPixel16;
      WriteChar = WriteChar16;
      GetPixelAt = GetPixelAt16;
      DrawSprite = DrawSprite16;
      break;

    case 16:
      pfmt.RedMask   = 0x1f << 11;
      pfmt.GreenMask = 0x3f << 5;
      pfmt.BlueMask  = 0x1f;
      pfmt.PalEntries = 0;
      pfmt.PixelBytes = 2;

      DrawPixel = DrawPixel16;
      WriteChar = WriteChar16;
      GetPixelAt = GetPixelAt16;
      DrawSprite = DrawSprite16;
      break;

    case 32:
      pfmt.RedMask   = 0xff << 16;
      pfmt.GreenMask = 0xff << 8;
      pfmt.BlueMask  = 0xff;
      pfmt.PalEntries = 0;
      pfmt.PixelBytes = 4;

      DrawPixel = DrawPixel32;
      WriteChar = WriteChar32;
      GetPixelAt = GetPixelAt32;
      DrawSprite = DrawSprite32;
      break;
  }

  complete_pixel_format();

  mouse_x = mouse_y = -1;
  memset (mouse_button , 0, sizeof (mouse_button));
  memset (keydown, 0, sizeof (keydown));
}

csGraphics2DSVGALib::~csGraphics2DSVGALib(void)
{
  // Destroy your graphic interface
  Close();
}

// Used to printf through system driver
void csGraphics2DSVGALib::CsPrintf (int msgtype, const char *format, ...)
{
  va_list arg;
  char buf[256];

  va_start (arg, format);
  vsprintf (buf, format, arg);
  va_end (arg);

  System->Print (msgtype, buf);
}

bool csGraphics2DSVGALib::Open(const char *Title)
{
  // Open your graphic interface
  if (!csGraphics2D::Open (Title))
    return false;

  // Set loop callback
  UnixSystem->SetLoopCallback (ProcessEvents, this);

  vga_init ();

  // Enable mouse support in SVGALIB
  vga_setmousesupport (1);

  int vgamode;
  char modestr[50],depthstr[10];

  switch (Depth)
  {
    case 8  :
      sprintf (depthstr, "256");
      break;
    case 15 :
      sprintf (depthstr, "32K");
      break;
    case 16 :
      sprintf (depthstr, "64K");
      break;
    case 32 :
      sprintf (depthstr, "16M32");
      break;
    default :
      CsPrintf (MSG_FATAL_ERROR, "Unsupported depth %d\n", Depth);
      return false;
  }

  sprintf (modestr, "G%dx%dx%s", Width, Height, depthstr);
  vgamode = vga_getmodenumber (modestr);

  if ((vgamode==-1) || (vga_setmode (vgamode) == -1))
  {
    CsPrintf (MSG_FATAL_ERROR, "Specified screenmode %s is not available!\n", modestr);
    return false;
  }

  gl_setcontextvga (vgamode);
  gl_getcontext (&physicalscreen);
  gl_setcontextvgavirtual (vgamode);
  Memory = (unsigned char*)VBUF;
  gl_enablepageflipping (&physicalscreen);

  keyboard_init ();
#if !defined (OS_DOS)
//mouse_setxrange (0, Width-1);
//mouse_setyrange (0, Height-1);
//mouse_setwrap (MOUSE_NOWRAP);
#endif // OS_DOS

  Clear (0);
  return true;
}

void csGraphics2DSVGALib::Close(void)
{
  // Close your graphic interface
  keyboard_close ();
  vga_setmode (TEXT);
  csGraphics2D::Close ();
}

void csGraphics2DSVGALib::ProcessEvents (void* Param)
{
  static int mouse_button_mask [3] =
  {MOUSE_LEFTBUTTON, MOUSE_RIGHTBUTTON, MOUSE_MIDDLEBUTTON};
  
  csGraphics2DSVGALib *Self = (csGraphics2DSVGALib *)Param;

  keyboard_update ();
  bool shift = keyboard_keypressed (SCANCODE_LEFTSHIFT)
            || keyboard_keypressed (SCANCODE_RIGHTSHIFT);
  bool alt   = keyboard_keypressed (SCANCODE_LEFTALT)
            || keyboard_keypressed (SCANCODE_RIGHTALT);
  bool ctrl  = keyboard_keypressed (SCANCODE_LEFTCONTROL);
  int state = (shift ? CSMASK_SHIFT : 0) | (alt ? CSMASK_ALT : 0) | (ctrl ? CSMASK_CTRL : 0);

  for (unsigned int scancode = 0; scancode < 128; scancode++)
  {
    int key = ScanCodeToChar [scancode];
    bool down = key ? keyboard_keypressed (scancode) : false;
    if (down != Self->keydown [scancode])
    {
      Self->keydown [scancode] = down;
      Self->UnixSystem->KeyboardEvent (key, down);
    }
  }

  if (mouse_update ())
  {
    int x = mouse_getx ();
    int y = mouse_gety ();
    if ((x != Self->mouse_x) || (y != Self->mouse_y))
    {
      Self->mouse_x = x; Self->mouse_y = y;
      Self->UnixSystem->MouseEvent (0, false, x, y, 0);
    }
    
    int buttons = mouse_getbutton ();
    for (int button = 0; button < 3; button++)
    {
      bool down = (buttons & mouse_button_mask [button]) != 0;
      if (down != Self->mouse_button [button])
      {
        Self->mouse_button [button] = down;
        Self->UnixSystem->MouseEvent (button + 1, down, x, y, state);
      }
    }
  }
}
  
void csGraphics2DSVGALib::Print (csRect *area)
{
  gl_copyscreen (&physicalscreen);
}

void csGraphics2DSVGALib::SetRGB(int i, int r, int g, int b)
{
  if (pfmt.PalEntries)
    vga_setpalette (i, r/4, g/4, b/4);

  csGraphics2D::SetRGB (i, r, g, b);
}
