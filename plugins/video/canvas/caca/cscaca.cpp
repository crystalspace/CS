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
  scfImplementationType (this, iParent)
{
  EventOutlet = 0;
  Memory = 0;
  caca_context = 0;
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
  csRef<iEventQueue> q = CS_QUERY_REGISTRY(object_reg, iEventQueue);
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

  bool overwrite = false; // overwrite existing env vars.
  if(config->KeyExists("Video.ASCII.Console.Size"))
    setenv ("CACA_GEOMETRY", 
      config->GetStr("Video.ASCII.Console.Size", "80x24"), overwrite);
  if(config->KeyExists("Video.ASCII.Console.Driver"))
    setenv ("CACA_DRIVER", 
      config->GetStr("Video.ASCII.Console.Driver", "x11"), overwrite);
  if(config->KeyExists("Video.ASCII.Console.Font"))
    setenv ("CACA_FONT", 
      config->GetStr("Video.ASCII.Console.Font", "fixed"), overwrite);
  if(config->KeyExists("Video.ASCII.Console.Background"))
    setenv ("CACA_BACKGROUND", 
      config->GetStr("Video.ASCII.Console.Background", "solid"), overwrite);
  if(config->KeyExists("Video.ASCII.Console.AntiAlias"))
    setenv ("CACA_ANTIALIASING", 
      config->GetStr("Video.ASCII.Console.AntiAlias", "prefilter"), overwrite);
  if(config->KeyExists("Video.ASCII.Console.Dither"))
    setenv ("CACA_DITHERING", 
      config->GetStr("Video.ASCII.Console.Dither", "ordered4"), overwrite);

  if (caca_init() != 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.graphics2d.cacacanvas",
      "Cannot initialize libcaca.");
    return false;
  }
  caca_clear();

  Width = config->GetInt("Video.Ascii.Offscreen.Width", 320);
  Height = config->GetInt("Video.Ascii.Offscreen.Height", 240);
  Memory = new unsigned char[ pfmt.PixelBytes*Width*Height ];

  caca_context = caca_create_bitmap(Depth, Width, Height, 
    Width*pfmt.PixelBytes, pfmt.RedMask, pfmt.GreenMask, 
    pfmt.BlueMask, pfmt.AlphaMask);

  return csGraphics2D::Open ();
}

void csGraphics2DCaca::Close ()
{
  if (!is_open) return;
  if(caca_context) 
  {
    caca_free_bitmap(caca_context);
    caca_context = 0;
    caca_end();
  }
  if(Memory) delete[] Memory;
  Memory = 0;
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
  int sx1, sx2, sy1, sy2;
  if (area)
  {
    sx1 = area->xmin;
    sx2 = area->xmax;
    sy1 = area->ymin;
    sy2 = area->ymax;
  }
  else
  {
    sx1 = 0;
    sx2 = Width;
    sy1 = 0;
    sy2 = Height;
  }

  caca_draw_bitmap(0, 0, caca_get_width()-1, caca_get_height()-1, 
    caca_context, (void*)Memory);
  caca_refresh();

  /* Get all events from keyboard and mouse and put them into system queue */
  int event;
  while ((event = caca_get_event(CACA_EVENT_ANY)) != 0)
  {
    int evtype = event & CACA_EVENT_ANY;
    int evrest = event & ~CACA_EVENT_ANY;
    int mousex = caca_get_mouse_x()*Width/caca_get_width();
    int mousey = caca_get_mouse_y()*Height/caca_get_height();
    switch(evtype)
    {
    case CACA_EVENT_KEY_PRESS:
      EventOutlet->Key(MapKey(evrest), MapKey(evrest), true);
      break;
    case CACA_EVENT_KEY_RELEASE:
      EventOutlet->Key(MapKey(evrest), MapKey(evrest), false);
      break;
    case CACA_EVENT_MOUSE_PRESS:
      EventOutlet->Mouse(evrest, true, mousex, mousey);
      break;
    case CACA_EVENT_MOUSE_RELEASE:
      EventOutlet->Mouse(evrest, false, mousex, mousey);
      break;
    case CACA_EVENT_MOUSE_MOTION:
      EventOutlet->Mouse(0, false, mousex, mousey);
      break;
    case CACA_EVENT_ANY:
    case CACA_EVENT_NONE:
    case CACA_EVENT_RESIZE:
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
  caca_set_window_title(title);
}
