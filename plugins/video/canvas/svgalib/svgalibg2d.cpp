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

#include "cssysdef.h"
#include "video/canvas/svgalib/svga.h"
#include "csgeom/csrect.h"
#include "cssys/csinput.h"
#include "isys/system.h"

CS_IMPLEMENT_PLUGIN

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

SCF_IMPLEMENT_FACTORY (csGraphics2DSVGALib)

SCF_EXPORT_CLASS_TABLE (svga2d)
  SCF_EXPORT_CLASS_DEP (csGraphics2DSVGALib, "crystalspace.graphics2d.svgalib",
    "SVGALib 2D graphics driver for Crystal Space", "crystalspace.font.server.")
SCF_EXPORT_CLASS_TABLE_END

SCF_IMPLEMENT_IBASE (csGraphics2DSVGALib)
  SCF_IMPLEMENTS_INTERFACE (iPlugIn)
  SCF_IMPLEMENTS_INTERFACE (iGraphics2D)
  SCF_IMPLEMENTS_INTERFACE (iEventPlug)
SCF_IMPLEMENT_IBASE_END

// csGraphics2DSVGALib functions
csGraphics2DSVGALib::csGraphics2DSVGALib(iBase *iParent) : csGraphics2D ()
{
  SCF_CONSTRUCT_IBASE (iParent);
  EventOutlet = NULL;
}

bool csGraphics2DSVGALib::Initialize (iSystem *pSystem)
{
  if (!csGraphics2D::Initialize (pSystem))
    return false;

  Memory = NULL;

  // SVGALIB Starts here

  CsPrintf (CS_MSG_INITIALIZATION, "Crystal Space Linux/SVGALIB version.\n");
  CsPrintf (CS_MSG_INITIALIZATION,  "Using %dx%dx%d resolution.\n\n", Width, Height, Depth);

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

      _DrawPixel = DrawPixel16;
      _WriteString = WriteString16;
      _GetPixelAt = GetPixelAt16;
      break;

    case 16:
      pfmt.RedMask   = 0x1f << 11;
      pfmt.GreenMask = 0x3f << 5;
      pfmt.BlueMask  = 0x1f;
      pfmt.PalEntries = 0;
      pfmt.PixelBytes = 2;

      _DrawPixel = DrawPixel16;
      _WriteString = WriteString16;
      _GetPixelAt = GetPixelAt16;
      break;

    case 32:
      pfmt.RedMask   = 0xff << 16;
      pfmt.GreenMask = 0xff << 8;
      pfmt.BlueMask  = 0xff;
      pfmt.PalEntries = 0;
      pfmt.PixelBytes = 4;

      _DrawPixel = DrawPixel32;
      _WriteString = WriteString32;
      _GetPixelAt = GetPixelAt32;
      break;
  }

  pfmt.complete ();

  mouse_x = mouse_y = -1;
  memset (mouse_button , 0, sizeof (mouse_button));
  memset (keydown, 0, sizeof (keydown));

  // Tell system driver to call us on every frame
  System->CallOnEvents (this, CSMASK_Nothing);
  // Create the event outlet
  EventOutlet = System->CreateEventOutlet (this);

  return true;
}

csGraphics2DSVGALib::~csGraphics2DSVGALib(void)
{
  // Destroy your graphic interface
  Close();
  if (EventOutlet)
    EventOutlet->DecRef ();
}

bool csGraphics2DSVGALib::Open(const char *Title)
{
  // Open your graphic interface
  if (!csGraphics2D::Open (Title))
    return false;

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
      CsPrintf (CS_MSG_FATAL_ERROR, "Unsupported depth %d\n", Depth);
      return false;
  }

  sprintf (modestr, "G%dx%dx%s", Width, Height, depthstr);
  vgamode = vga_getmodenumber (modestr);

  if ((vgamode==-1) || (vga_setmode (vgamode) == -1))
  {
    CsPrintf (CS_MSG_FATAL_ERROR, "Specified screenmode %s is not available!\n", modestr);
    return false;
  }

  gl_setcontextvga (vgamode);
  gl_getcontext (&physicalscreen);
  gl_setcontextvgavirtual (vgamode);
  Memory = (unsigned char*)VBUF;
  gl_enablepageflipping (&physicalscreen);

  keyboard_init ();
//#if !defined (OS_DOS)
//mouse_setxrange (0, Width-1);
//mouse_setyrange (0, Height-1);
//mouse_setwrap (MOUSE_NOWRAP);
//#endif // OS_DOS

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

bool csGraphics2DSVGALib::HandleEvent (iEvent &/*Event*/)
{
  static int mouse_button_mask [3] =
  { MOUSE_LEFTBUTTON, MOUSE_RIGHTBUTTON, MOUSE_MIDDLEBUTTON };
  
  keyboard_update ();
/*
  bool shift = keyboard_keypressed (SCANCODE_LEFTSHIFT)
            || keyboard_keypressed (SCANCODE_RIGHTSHIFT);
  bool alt   = keyboard_keypressed (SCANCODE_LEFTALT)
            || keyboard_keypressed (SCANCODE_RIGHTALT);
  bool ctrl  = keyboard_keypressed (SCANCODE_LEFTCONTROL);
  int state = (shift ? CSMASK_SHIFT : 0) | (alt ? CSMASK_ALT : 0) | (ctrl ? CSMASK_CTRL : 0);
*/
  for (unsigned int scancode = 0; scancode < 128; scancode++)
  {
    int key = ScanCodeToChar [scancode];
    bool down = key ? keyboard_keypressed (scancode) : false;
    if (down != keydown [scancode])
    {
      keydown [scancode] = down;
      EventOutlet->Key (key, -1, down);
    }
  }

  if (mouse_update ())
  {
    int x = mouse_getx ();
    int y = mouse_gety ();
    if ((x != mouse_x) || (y != mouse_y))
    {
      mouse_x = x; mouse_y = y;
      EventOutlet->Mouse (0, false, x, y);
    }
    
    int buttons = mouse_getbutton ();
    for (int button = 0; button < 3; button++)
    {
      bool down = (buttons & mouse_button_mask [button]) != 0;
      if (down != mouse_button [button])
      {
        mouse_button [button] = down;
        EventOutlet->Mouse (button + 1, down, x, y);
      }
    }
  }
  return false;
}
  
void csGraphics2DSVGALib::Print (csRect * /*area*/)
{
  gl_copyscreen (&physicalscreen);
}

void csGraphics2DSVGALib::SetRGB(int i, int r, int g, int b)
{
  if (pfmt.PalEntries)
    vga_setpalette (i, r/4, g/4, b/4);

  csGraphics2D::SetRGB (i, r, g, b);
}
