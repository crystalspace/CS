/*
    Allegro support for Crystal Space 3D library
    Copyright (C) 1999 by Dan Bogdanov <dan@pshg.edu.ee>
    Modified for full Allegro by Burton Radons <loth@pacificcoast.net>

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
#include "csalleg.h"
#include "cssys/csinput.h"
#include "csgeom/csrect.h"
#include "csutil/csstring.h"
#include "isys/system.h"
#include "allegro.h"

static unsigned short ScanCodeToChar [128] =
{
  0, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
  'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2',
  '3', '4', '5', '6', '7', '8', '9', /* pad */ '0', '1', '2', '3', '4',
  CSKEY_CENTER, '6', '7', '8', '9', CSKEY_F1, CSKEY_F2, CSKEY_F3, CSKEY_F4,
  CSKEY_F5, CSKEY_F6, CSKEY_F7, CSKEY_F8, CSKEY_F9, CSKEY_F10, CSKEY_F11,
  CSKEY_F12, CSKEY_ESC, '`', '-', '=', CSKEY_BACKSPACE, CSKEY_TAB, '[', ']',
  CSKEY_ENTER, ';', '\'', '\\', '\\', ',', '.', '/', ' ', CSKEY_INS, CSKEY_DEL,
  CSKEY_HOME, CSKEY_END, CSKEY_PGUP, CSKEY_PGDN, CSKEY_LEFT, CSKEY_RIGHT,
  CSKEY_UP, CSKEY_DOWN, CSKEY_PADDIV, CSKEY_PADMULT, CSKEY_PADMINUS,
  CSKEY_PADPLUS, CSKEY_DEL, CSKEY_ENTER, 0, 0, 0, 0, CSKEY_SHIFT, CSKEY_SHIFT,
  CSKEY_CTRL, CSKEY_CTRL, CSKEY_ALT, CSKEY_ALT, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

//------------------------------------------------------- csGraphics2DAlleg ---

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csGraphics2DAlleg)

SCF_EXPORT_CLASS_TABLE (alleg2d)
  SCF_EXPORT_CLASS_DEP (csGraphics2DAlleg, "crystalspace.graphics2d.allegro",
    "Allegro 2D graphics driver for Crystal Space", "crystalspace.font.server.")
SCF_EXPORT_CLASS_TABLE_END

SCF_IMPLEMENT_IBASE_EXT (csGraphics2DAlleg)
  SCF_IMPLEMENTS_INTERFACE (iEventPlug)
SCF_IMPLEMENT_IBASE_EXT_END

csGraphics2DAlleg::csGraphics2DAlleg (iBase *iParent) : csGraphics2D (iParent)
{
  EventOutlet = NULL;
  hook_kbd = hook_mouse = true;
  kbd_hook_active = mouse_hook_active = false;
}

csGraphics2DAlleg::~csGraphics2DAlleg ()
{
  Close ();
  if (EventOutlet)
    EventOutlet->DecRef ();
}

bool csGraphics2DAlleg::Initialize (iSystem *pSystem)
{
  if (!csGraphics2D::Initialize (pSystem))
    return false;

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
      break;
    case 16:
      pfmt.RedMask   = 0x1f << 11;
      pfmt.GreenMask = 0x3f << 5;
      pfmt.BlueMask  = 0x1f;
      pfmt.PalEntries = 0;
      pfmt.PixelBytes = 2;
      break;
    case 32:
      pfmt.RedMask   = 0xff << 16;
      pfmt.GreenMask = 0xff << 8;
      pfmt.BlueMask  = 0xff;
      pfmt.PalEntries = 0;
      pfmt.PixelBytes = 4;
      break;
  }
  pfmt.complete ();

  Font = 0;
  Memory = NULL;
  System->CallOnEvents (&scfiPlugIn, CSMASK_Nothing);
  EventOutlet = System->CreateEventOutlet (this);
  return true;
}

bool csGraphics2DAlleg::Open (const char* Title)
{
  if (opened)
    return false;
  if (!csGraphics2D::Open (Title))
    return false;
  PaletteChanged = false;

  allegro_init ();
  set_color_depth (Depth);

  if (set_gfx_mode (GFX_AUTODETECT, Width, Height, 0, 0)
   && (Width > 640 || Height > 480 || set_gfx_mode (GFX_AUTODETECT, 640, 480, 0, 0))
   && (Width > 800 || Height > 600 || set_gfx_mode (GFX_AUTODETECT, 800, 600, 0, 0))
   && (Width > 1024|| Height > 768 || set_gfx_mode (GFX_AUTODETECT, 1024,768, 0, 0)))
  {
    CsPrintf (CS_MSG_FATAL_ERROR, "ERROR! Could not set graphics mode.\n");
    exit (-1);
  }

  bitmap = create_bitmap (Width, Height);

  if (bitmap == NULL)
  {
    CsPrintf (CS_MSG_FATAL_ERROR, "Error initializing graphics subsystem: bad "
                               "videomode!\n");
    return false;
  }

  Memory = (unsigned char *) bitmap->dat;

  // Tell printf() to shut up
  System->PerformExtension ("EnablePrintf", false);

  // Update drawing routine addresses
  switch (pfmt.PixelBytes)
  {
    case 1:
      break;
    case 2:
      _DrawPixel = DrawPixel16;
      _WriteString = WriteString16;
      _GetPixelAt = GetPixelAt16;
      break;
    case 4:
      _DrawPixel = DrawPixel32;
      _WriteString = WriteString32;
      _GetPixelAt = GetPixelAt32;
      break;
    default:
      CsPrintf (CS_MSG_WARNING, "WARNING: No 2D routines for selected mode!\n");
      break;
  } /* endif */

  // Hook keyboard, if needed
  if (hook_kbd)
  {
    install_keyboard ();
    kbd_hook_active = true;
  }
  // Hook mouse, if needed
  if (hook_mouse)
  {
    install_mouse ();
    mouse_hook_active = true;
  }

  // Clear all videopages
  ClearAll (0);
  opened = true;

  return true;
}

void csGraphics2DAlleg::Close (void)
{
  if (!opened)
    return;

  if (kbd_hook_active)
  {
    remove_keyboard ();
    kbd_hook_active = false;
  }
  if (mouse_hook_active)
  {
    remove_mouse ();
    mouse_hook_active = false;
  }

  if (bitmap)
    destroy_bitmap (bitmap);
  bitmap = NULL;
  Memory = NULL;
  csGraphics2D::Close ();
  System->PerformExtension ("EnablePrintf", true);
  set_gfx_mode (GFX_TEXT, 0, 0, 0, 0);
  opened = false;
}

void csGraphics2DAlleg::Print (csRect *area)
{
  if (!opened)
    return;
  if (PaletteChanged && pfmt.PalEntries)
  {
    RGB rgb;

    for (int i = 0; i <= 255; i++)
    {
      rgb.r = Palette[i].red >> 2;
      rgb.g = Palette[i].green >> 2;
      rgb.b = Palette[i].blue >> 2;
      set_color (i, &rgb);
    }
    PaletteChanged = false;
  }

//  vsync ();
  
  if (bitmap->w == SCREEN_W && bitmap->h == SCREEN_H)
  {
    if (area)
      blit (bitmap, screen, area->xmin, area->ymin, area->xmin, area->ymin,
            area->Width (), area->Height ());
    else
      blit (bitmap, screen, 0, 0, 0, 0, Width, Height);
  }
  else if (scale)
  {
    if (area)
    {
      int sx = area->xmin * SCREEN_W / bitmap->w;
      int sy = area->ymin * SCREEN_H / bitmap->h;
      stretch_blit (bitmap, screen, area->xmin, area->ymin,
                    area->Width (), area->Height (), sx, sy,
                    (area->Width () + area->xmin) * SCREEN_W / bitmap->w - sx,
                    (area->Height () + area->ymin) * SCREEN_H / bitmap->h - sy);
    }
    else if (bitmap->w * 2 != SCREEN_W || bitmap->h * 2 != SCREEN_H)
      stretch_blit (bitmap, screen, 0, 0, bitmap->w, bitmap->h,
                    0, 0, SCREEN_W, SCREEN_H);
    else
      DoubleBlit (bitmap, screen, bitmap->w, bitmap->h);
  }
  else
  {
    if (area)
      blit (bitmap, screen, area->xmin, area->ymin,
            (SCREEN_W - bitmap->w) / 2 + area->xmin,
            (SCREEN_H - bitmap->h) / 2 + area->ymin,
            area->Width (), area->Height ());
    else
      blit (bitmap, screen, 0, 0, (SCREEN_W - bitmap->w) / 2,
            (SCREEN_H - bitmap->h) / 2, bitmap->w, bitmap->h);
  }
}

void csGraphics2DAlleg::DoubleBlit (BITMAP *src, BITMAP *dst, int sw, int sh)
{
  unsigned long dptr, eptr;
  unsigned short *ptr;
  int y;

  if (sw > src->w)
    sw = src->w;
  if (sh > src->h)
    sh = src->h;
  sw *= 2, sh *= 2;

  acquire_bitmap (dst);
  switch (bitmap_color_depth (dst))
  {
    default:
      stretch_blit (src, dst, 0, 0, sw, sh, 0, 0, sw * 2, sh * 2);
      break;

    case 15:
    case 16:
      for (y = 0; y < sh; y ++)
      {
        ptr = (unsigned short *) src->line [y / 2];
        dptr = bmp_write_line (dst, y);
        eptr = dptr + sw * 2;
        while (dptr < eptr)
        {
          bmp_write32 (dptr, *ptr | (*ptr << 16));
          dptr += 4;
          ptr ++;
        }
      }
      break;
  }
  bmp_unwrite_line (dst);
  release_bitmap (dst);
}

void csGraphics2DAlleg::SetRGB (int i, int r, int g, int b)
{
  csGraphics2D::SetRGB (i, r, g, b);
  PaletteChanged = true;
}

bool csGraphics2DAlleg::HandleEvent (iEvent &/*Event*/)
{
  int scancode, keycode, c;
  
  if (!opened)
    return false;
  if (keyboard_needs_poll ())
    poll_keyboard ();
  if (mouse_needs_poll ())
    poll_mouse ();

  while (keypressed ())
  {
    scancode = readkey () >> 8;
    keydown [scancode] = 1;
    keycode = ScanCodeToChar [scancode];
    if (keycode)
      EventOutlet->Key (keycode, -1, true);
  }

  for (scancode = 127; scancode >= 0; scancode --)
  {
    keycode = ScanCodeToChar [scancode];
    if (keycode)
    {
      bool down = !!key [scancode];
      if (down != keydown [scancode])
      {
        keydown [scancode] = down;
        EventOutlet->Key (keycode, -1, down);
      }
    }
  }

  if (mouse_x != x_mouse || mouse_y != y_mouse)
  {
    x_mouse = mouse_x; y_mouse = mouse_y;
    EventOutlet->Mouse (0, false, x_mouse, y_mouse);
  }

  for (c = 0; c < 3; c ++)
  {
    bool down = (mouse_b & (1 << c)) != 0;
    if ((mouse_b & (1 << c)) != (button & (1 << c)))
      EventOutlet->Mouse (c + 1, down, x_mouse, y_mouse);
  }
  button = mouse_b;
  
  return false;
}

bool csGraphics2DAlleg::PerformExtensionV (char const* command, va_list)
{
  if (!strcasecmp (command, "fullscreen"))
  {
    System->Printf (CS_MSG_INITIALIZATION, "Fullscreen toggle.");
    scale = !scale;
    clear (screen);
    Print (NULL);
  }
  return true;
}

void csGraphics2DAlleg::EnableEvents (unsigned iType, bool iEnable)
{
  // If the system drivers decides to ignore our keyboard and/or mouse events
  // in favour of other driver (e.g. system built-in such as on DOS) we should
  // drop the Allegro handler (since Allegro has a poor keyboard/mouse handler).

  if (iType == CSEVTYPE_Keyboard)
  {
    if (hook_kbd != iEnable)
    {
      hook_kbd = iEnable;
      if (opened && (hook_kbd != kbd_hook_active))
        if (hook_kbd)
        {
          install_keyboard ();
          kbd_hook_active = true;
        }
        else
        {
          remove_keyboard ();
          kbd_hook_active = false;
        }
    }
  }
  else if (iType == CSEVTYPE_Mouse)
  {
    if (hook_mouse != iEnable)
    {
      hook_mouse = iEnable;
      if (opened && (hook_mouse != mouse_hook_active))
        if (hook_mouse)
        {
          install_mouse ();
          mouse_hook_active = true;
        }
        else
        {
          remove_mouse ();
          mouse_hook_active = false;
        }
    }
  }
}
