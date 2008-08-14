/*
    Colour ASCII art rendering support for Crystal Space 3D library
    Copyright (C) 2005 by dr.W.C.A. Wijngaards
    Based on aalib canvas by Andrew Zabolotny <bit@eltech.ru>

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

#include "cssysdef.h"
#include <stdarg.h>
#include <stdlib.h>
#include <limits.h>
#include "csqint.h"

#include "csgeom/csrect.h"
#include "csgfx/rgbpixel.h"
#include "csutil/cfgacc.h"
#include "csutil/setenv.h"
#include "iutil/cfgfile.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"

#include "cscaca.h"

//-------------------------------------------------------- csGraphics2DCaca ---

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csGraphics2DCaca)

csGraphics2DCaca::csGraphics2DCaca (iBase *iParent) : 
  scfImplementationType (this, iParent), cucul_canvas (0), dither (0),
  caca_display (0)
{
  EventOutlet = 0;
  Memory = 0;
}

csGraphics2DCaca::~csGraphics2DCaca (void)
{
  Close ();
}

bool csGraphics2DCaca::Initialize (iObjectRegistry *object_reg)
{
  if (!csGraphics2D::Initialize (object_reg))
    return false;

  csConfigAccess config;
  config.AddConfig(object_reg, "/config/cacacanvas.cfg");
  config.AddConfig(object_reg, "/config/video.cfg");

  // Load settings from config file and setup the aa_defparams structure
  HardwareCursor = config->GetBool ("Video.SystemMouseCursor", true);

  // set internal render format
  Depth = 32;
  pfmt.RedMask   = 0xff << 16;
  pfmt.GreenMask = 0xff << 8;
  pfmt.BlueMask  = 0xff;
  pfmt.AlphaMask = 0;
  pfmt.PalEntries = 0;
  pfmt.PixelBytes = 4;
  pfmt.complete ();
  _GetPixelAt = GetPixelAt32;

  // create event outlet
  csRef<iEventQueue> q = csQueryRegistry<iEventQueue> (object_reg);
  if (q != 0)
  {
    if (!EventOutlet.IsValid())
      EventOutlet = q->CreateEventOutlet (this);
  }
  return true;
}

bool csGraphics2DCaca::Open ()
{
  if (is_open) return true;

  csConfigAccess config;
  config.AddConfig(object_reg, "/config/cacacanvas.cfg");
  config.AddConfig(object_reg, "/config/video.cfg");

  if(config->KeyExists("Video.ASCII.Console.Size"))
    CS::Utility::setenv ("CACA_GEOMETRY", 
      config->GetStr("Video.ASCII.Console.Size"), false);
  if(config->KeyExists("Video.ASCII.Console.Driver"))
    CS::Utility::setenv ("CACA_DRIVER", 
      config->GetStr("Video.ASCII.Console.Driver", "x11"), false);
  if(config->KeyExists("Video.ASCII.Console.Font"))
    CS::Utility::setenv ("CACA_FONT", 
      config->GetStr("Video.ASCII.Console.Font", "fixed"), false);
  
  fbWidth = config->GetInt("Video.Ascii.Offscreen.Width", 320);
  fbHeight = config->GetInt("Video.Ascii.Offscreen.Height", 240);
  
  cucul_canvas = cucul_create_canvas (0, 0);
  if (cucul_canvas == 0) return false;
  caca_display = caca_create_display (cucul_canvas);
  if (caca_display == 0)
    return false;
  cucul_clear_canvas (cucul_canvas);
  
  dither = cucul_create_dither (Depth, fbWidth, fbHeight, 
    fbWidth*pfmt.PixelBytes, pfmt.RedMask, pfmt.GreenMask, 
    pfmt.BlueMask, pfmt.AlphaMask);
  if (dither == 0)
    return false;
  if(config->KeyExists("Video.ASCII.Console.AntiAlias"))
    cucul_set_dither_antialias (dither, 
      config->GetStr("Video.ASCII.Console.AntiAlias"));
  if(config->KeyExists("Video.ASCII.Console.Dither"))
    cucul_set_dither_mode (dither, 
      config->GetStr("Video.ASCII.Console.Dither"));
  
  caca_set_display_title (caca_display, win_title);
  
  Memory = new unsigned char[ pfmt.PixelBytes*fbWidth*fbHeight ];
  

  return csGraphics2D::Open ();
}

void csGraphics2DCaca::Close ()
{
  if (dither)
  {
    cucul_free_dither (dither);
    dither = 0;
  }
  if(caca_display) 
  {
    caca_free_display(caca_display);
    caca_display = 0;
  }
  if (cucul_canvas)
  {
    cucul_free_canvas (cucul_canvas);
    cucul_canvas = 0;
  }
  if(Memory) delete[] Memory;
  Memory = 0;
  
  if (!is_open) return;
  csGraphics2D::Close ();
}

int csGraphics2DCaca::MapKey(int raw)
{

  switch(raw)
  {
    case CACA_KEY_UNKNOWN: return 0;
    case CACA_KEY_BACKSPACE : return CSKEY_BACKSPACE;
    case CACA_KEY_TAB : return CSKEY_TAB;
    case CACA_KEY_RETURN : return CSKEY_ENTER;
    case CACA_KEY_PAUSE : return CSKEY_PAUSE;
    case CACA_KEY_ESCAPE : return CSKEY_ESC;
    case CACA_KEY_DELETE : return CSKEY_DEL;
    case CACA_KEY_UP : return CSKEY_UP;
    case CACA_KEY_DOWN : return CSKEY_DOWN;
    case CACA_KEY_LEFT : return CSKEY_LEFT;
    case CACA_KEY_RIGHT : return CSKEY_RIGHT;
    case CACA_KEY_INSERT : return CSKEY_INS;
    case CACA_KEY_HOME : return CSKEY_HOME;
    case CACA_KEY_END : return CSKEY_END;
    case CACA_KEY_PAGEUP : return CSKEY_PGUP;
    case CACA_KEY_PAGEDOWN : return CSKEY_PGDN;
    case CACA_KEY_F1 : return CSKEY_F1;
    case CACA_KEY_F2 : return CSKEY_F2;
    case CACA_KEY_F3 : return CSKEY_F3;
    case CACA_KEY_F4 : return CSKEY_F4;
    case CACA_KEY_F5 : return CSKEY_F5;
    case CACA_KEY_F6 : return CSKEY_F6;
    case CACA_KEY_F7 : return CSKEY_F7;
    case CACA_KEY_F8 : return CSKEY_F8;
    case CACA_KEY_F9 : return CSKEY_F9;
    case CACA_KEY_F10 : return CSKEY_F10;
    case CACA_KEY_F11 : return CSKEY_F11;
    case CACA_KEY_F12 : return CSKEY_F12;
    case CACA_KEY_F13 : return 0; // no CSKEY for this
    case CACA_KEY_F14 : return 0; // no CSKEY for this
    case CACA_KEY_F15 : return 0; // no CSKEY for this
    default:
      return raw;
  }
}

void csGraphics2DCaca::Print (csRect const* area)
{
  cucul_dither_bitmap (cucul_canvas, 0, 0,
    cucul_get_canvas_width (cucul_canvas), 
    cucul_get_canvas_height (cucul_canvas), dither, Memory);
  caca_refresh_display (caca_display);

  /* Get all events from keyboard and mouse and put them into system queue */
  caca_event_t event;
  while (caca_get_event (caca_display, CACA_EVENT_ANY, &event, 0))
  {
    switch(event.type)
    {
    case CACA_EVENT_KEY_PRESS:
    case CACA_EVENT_KEY_RELEASE:
      {
        utf32_char raw, cooked;
        if (event.data.key.utf32 != 0)
          raw = cooked = event.data.key.utf32;
        else
          raw = cooked = MapKey (event.data.key.ch);
	EventOutlet->Key (raw, cooked, 
	  (event.type == CACA_EVENT_KEY_PRESS));
      }
      break;
    case CACA_EVENT_MOUSE_PRESS:
      EventOutlet->Mouse (event.data.mouse.button- 1, true,
        caca_get_mouse_x (caca_display), caca_get_mouse_y (caca_display));
      break;
    case CACA_EVENT_MOUSE_RELEASE:
      EventOutlet->Mouse (event.data.mouse.button - 1, false,
        caca_get_mouse_x (caca_display), caca_get_mouse_y (caca_display));
      break;
    case CACA_EVENT_MOUSE_MOTION:
      EventOutlet->Mouse (csmbNone, false, 
        event.data.mouse.x, event.data.mouse.y);
      break;
    default:
      break;
    }
  }
}

bool csGraphics2DCaca::BeginDraw ()
{
  csGraphics2D::BeginDraw ();
  return true;
}

void csGraphics2DCaca::FinishDraw ()
{
  csGraphics2D::FinishDraw ();
}

void csGraphics2DCaca::SetTitle(const char* title)
{
  caca_set_display_title (caca_display, title);
}
