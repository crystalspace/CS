/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein
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

#include "cssysdef.h"
#include "csgeom/math.h"
#include "csutil/event.h"
#include "csutil/eventnames.h"
#include <stdarg.h>
#include <stdlib.h>
#include "csplugincommon/canvas/fontcache.h"
#include "csplugincommon/canvas/graph2d.h"
#include "csqint.h"
#include "iutil/cmdline.h"
#include "iutil/plugin.h"
#include "ivideo/graph3d.h"
#include "ivideo/texture.h"
#include "ivideo/txtmgr.h"
#include "iengine/texture.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "ivaria/reporter.h"

csGraphics2D::csGraphics2D (iBase* parent) : 
  scfImplementationType (this, parent), fontCache (0), hwMouse (hwmcOff)
{
  static uint g2d_count = 0;

  fbWidth = 640;
  fbHeight = 480;
  Depth = 16;
  DisplayNumber = 0;
  FullScreen = false;
  is_open = false;
  win_title = "Crystal Space Application";
  object_reg = 0;
  AllowResizing = false;
  refreshRate = 0;
  vsync = false;
  weakEventHandler = 0;

  name.Format ("graph2d.%x", g2d_count++);

  fontCache = 0;
}

csGraphics2D::~csGraphics2D ()
{
  if (weakEventHandler != 0)
  {
    csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (object_reg));
    if (q != 0)
      CS::RemoveWeakListener (q, weakEventHandler);
  }
  Close ();
}

bool csGraphics2D::Initialize (iObjectRegistry* r)
{
  CS_ASSERT (r != 0);
  object_reg = r;
  plugin_mgr = csQueryRegistry<iPluginManager> (object_reg);
  // Get the system parameters
  config.AddConfig (object_reg, "/config/video.cfg");
  vpWidth = fbWidth = config->GetInt ("Video.ScreenWidth", fbWidth);
  vpHeight = fbHeight = config->GetInt ("Video.ScreenHeight", fbHeight);
  Depth = config->GetInt ("Video.ScreenDepth", Depth);
  FullScreen = config->GetBool ("Video.FullScreen", FullScreen);
  DisplayNumber = config->GetInt ("Video.DisplayNumber", DisplayNumber);
  refreshRate = config->GetInt ("Video.DisplayFrequency", 0);
  vsync = config->GetBool ("Video.VSync", false);
  
  const char* hwMouseFlag = config->GetStr ("Video.SystemMouseCursor", "yes");
  if ((strcasecmp (hwMouseFlag, "yes") == 0)
      || (strcasecmp (hwMouseFlag, "true") == 0)
      || (strcasecmp (hwMouseFlag, "on") == 0)
      || (strcmp (hwMouseFlag, "1") == 0))
  {
    hwMouse = hwmcOn;
  }
  else if (strcasecmp (hwMouseFlag, "rgbaonly") == 0)
  {
    hwMouse = hwmcRGBAOnly;
  }
  else
  {
    hwMouse = hwmcOff;
  }
  csRef<iCommandLineParser> cmdline (
    csQueryRegistry<iCommandLineParser> (object_reg));
  if (cmdline->GetOption ("sysmouse") || cmdline->GetOption ("nosysmouse"))
  {
    hwMouse = cmdline->GetBoolOption ("sysmouse") ? hwmcOn : hwmcOff;
  }

  // Get the font server: A missing font server is NOT an error
  if (!FontServer)
  {
    FontServer = csQueryRegistry<iFontServer> (object_reg);
  }
#ifdef CS_DEBUG
  if (!FontServer)
  {
    csReport (r, CS_REPORTER_SEVERITY_WARNING,
      "crystalspace.graphics2d.common",
      "Canvas driver couldn't find a font server plugin!  "
      "This is normal if you don't want one (warning displays only in "
      "debug mode)");
  }
#endif

  csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (object_reg));
  if (q != 0)
  {
    csEventID events[3] = { csevSystemOpen (object_reg), 
			    csevSystemClose (object_reg), 
			    CS_EVENTLIST_END };
    CS::RegisterWeakListener (q, this, events, weakEventHandler);
  }
  return true;
}

void csGraphics2D::ChangeDepth (int d)
{
  if (Depth == d) return;
  Depth = d;
}

const char* csGraphics2D::GetName() const
{
  return name;
}

bool csGraphics2D::HandleEvent (iEvent& Event)
{
  if (Event.Name == csevSystemOpen (object_reg))
  {
    Open ();
    return true;
  }
  else if (Event.Name == csevSystemClose (object_reg))
  {
    Close ();
    return true;
  }
  else
  {
    return false;
  }
}

bool csGraphics2D::Open ()
{
  if (is_open) return true;
  is_open = true;

  vpLeft = 0;
  vpTop = 0;
  
  FrameBufferLocked = 0;

  SetClipRect (0, 0, fbWidth, fbHeight);

  return true;
}

void csGraphics2D::Close ()
{
  if (!is_open) return;
  is_open = false;
  delete fontCache;
  fontCache = 0;
}

bool csGraphics2D::BeginDraw ()
{
  FrameBufferLocked++;
  return true;
}

void csGraphics2D::FinishDraw ()
{
  if (FrameBufferLocked)
    FrameBufferLocked--;
}

void csGraphics2D::Clear(int color)
{
  DrawBox (0, 0, vpWidth, vpHeight, color);
}

void csGraphics2D::ClearAll (int color)
{
  if (!BeginDraw ())
    return;
  Clear (color);
  FinishDraw ();
  Print ();
}

void csGraphics2D::SetClipRect (int xmin, int ymin, int xmax, int ymax)
{
  if (xmin < 0) xmin = 0;
  else if (xmin > vpWidth) xmin = vpWidth;
  if (xmax < 0) xmax = 0;
  else if (xmax > vpWidth) xmax = vpWidth;
  if (ymin < 0) ymin = 0;
  else if (ymin > vpHeight) ymin = vpHeight;
  if (ymax < 0) ymax = 0;
  else if (ymax > vpHeight) ymax = vpHeight;
  ClipX1 = xmin; ClipX2 = xmax;
  ClipY1 = ymin; ClipY2 = ymax;
  
  if (fontCache)
    fontCache->SetClipRect (vpLeft+ClipX1, vpTop+ClipY1,
      vpLeft+ClipX2, vpTop+ClipY2);
}

void csGraphics2D::GetClipRect (int &xmin, int &ymin, int &xmax, int &ymax)
{
  xmin = ClipX1; xmax = ClipX2;
  ymin = ClipY1; ymax = ClipY2;
}

/* helper function for ClipLine below */
bool csGraphics2D::CLIPt(float denom, float num, float& tE, float& tL)
{
    float t;

    if(denom > 0)
    {
        t = num / denom;
        if(t > tL) return false;
        else if(t > tE) tE = t;
    }
    else if(denom < 0)
    {
        t = num / denom;
        if(t < tE) return false;
        else if(t < tL) tL = t; // note: there is a mistake on this line in the C edition of the book!
    }
    else 
      if(num > 0) return false;
    return true;
}

/* This function and the next one were taken
   from _Computer Graphics: Principals and Practice_ (2nd ed)
   by Foley et al
   This implements the Liang-Barsky efficient parametric
   line-clipping algorithm
*/
bool csGraphics2D::ClipLine (float &x0, float &y0, float &x1, float &y1,
                             int xmin, int ymin, int xmax, int ymax)
{
    // exclude the left/bottom edges (the Liang-Barsky algorithm will
    // clip to those edges exactly, whereas the documentation for
    // ClipLine specifies that the lower/bottom edges are excluded)
    xmax--;
    ymax--;

    float dx = x1 - x0;
    float dy = y1 - y0;
    bool visible = false;

    if(dx == 0 && dy == 0 && x0 >= xmin && y0 >= ymin && x0 < xmax && y0 < ymax) 
    {
        visible = true;
    }
    else
    {
        float tE = 0.0;
        float tL = 1.0;
        if(CLIPt(dx, xmin - x0, tE, tL))
            if(CLIPt(-dx, x0 - xmax, tE, tL))
                if(CLIPt(dy, ymin - y0, tE, tL))
                    if(CLIPt(-dy, y0 - ymax, tE, tL))
                    {
                        visible = true;
                        if(tL < 1.0)
			{
                            x1 = x0 + tL * dx;
                            y1 = y0 + tL * dy;
                        }
                        if(tE > 0)
			{
                            x0 += tE * dx;
                            y0 += tE * dy;
                        }
                    }
    }
    return !visible;
}

void csGraphics2D::Write (iFont *font, int x, int y, int fg, int bg, 
			  const char *text, uint flags) 
{ 
  if (!text || !*text) return;
  fontCache->WriteString (font, x, y, fg, bg, text, false, flags);
}

void csGraphics2D::Write (iFont *font, int x, int y, int fg, int bg, 
			  const wchar_t*text, uint flags) 
{ 
  if (!text || !*text) return;
  fontCache->WriteString (font, x, vpTop+y, fg, bg, text, true, flags);
}

bool csGraphics2D::PerformExtensionV (char const* command, va_list args)
{
  return false;
}

bool csGraphics2D::PerformExtension (char const* command, ...)
{
  va_list args;
  va_start (args, command);
  bool rc = PerformExtensionV(command, args);
  va_end (args);
  return rc;
}

void csGraphics2D::AlertV (int type, const char* title, const char* okMsg,
    const char* msg, va_list arg)
{
  (void)type; (void)title; (void)okMsg;
  csPrintf ("ALERT: ");
  csPrintfV (msg, arg);
  csPrintf ("\n");
  fflush (stdout);
}

void csGraphics2D::Alert (int type, const char* title, const char* okMsg, 
			  const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  AlertV (type, title, okMsg, msg, arg);
  va_end (arg);
}

void csGraphics2D::Alert (int type, const wchar_t* title, const wchar_t* okMsg, 
			  const wchar_t* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  AlertV (type, csString (title), csString (okMsg), csString (msg), arg);
  va_end (arg);
}

void csGraphics2D::AlertV (int type, const wchar_t* title, const wchar_t* okMsg,
    const wchar_t* msg, va_list arg)
{
  AlertV (type, csString (title), csString (okMsg), csString (msg), arg);
}

iNativeWindow* csGraphics2D::GetNativeWindow ()
{
  return static_cast<iNativeWindow*> (this);
}

void csGraphics2D::SetTitle (const char* title)
{
  win_title = title;
}

void csGraphics2D::SetIcon (iImage *image)
{
    
}

bool csGraphics2D::Resize (int w, int h)
{
  if (!is_open)
  {
    // Still in Initialization phase, configuring size of canvas
    vpWidth = fbWidth = w;
    vpHeight = fbHeight = h;
    return true;
  }

  if (!AllowResizing)
    return false;

  if (fbWidth != w || fbHeight != h)
  {
    if ((vpLeft == 0) && (vpTop == 0)
        && (vpWidth == fbWidth) && (vpHeight == fbHeight))
    {
      vpWidth = w;
      vpHeight = h;
    }
    fbWidth = w;
    fbHeight = h;
  }
  return true;
}

void csGraphics2D::SetFullScreen (bool b)
{
  if (FullScreen == b) return;
  FullScreen = b;
}

bool csGraphics2D::SetMousePosition (int x, int y)
{
  (void)x; (void)y;
  return false;
}

bool csGraphics2D::SetMouseCursor (csMouseCursorID iShape)
{
  return (iShape == csmcArrow);
}

bool csGraphics2D::SetMouseCursor (iImage *, const csRGBcolor*, int, int, 
                                   csRGBcolor, csRGBcolor)
{
  return false;
}

void csGraphics2D::SetViewport (int left, int top, int width, int height)
{ 
  vpLeft = left; vpTop = top; vpWidth = width; vpHeight = height;
  fontCache->SetViewportOfs (left, top);
}

bool csGraphics2D::DebugCommand (const char* /*cmd*/)
{
  return false;
}

//---------------------------------------------------------------------------

#define NUM_OPTIONS 3

static const csOptionDescription config_options [NUM_OPTIONS] =
{
  { 0, "depth", "Display depth", CSVAR_LONG },
  { 1, "fs", "Fullscreen if available", CSVAR_BOOL },
  { 2, "mode", "Window size or resolution", CSVAR_STRING },
};

bool csGraphics2D::SetOption (int id, csVariant* value)
{
  if (value->GetType () != config_options[id].type)
    return false;
  switch (id)
  {
    case 0: ChangeDepth (value->GetLong ()); break;
    case 1: SetFullScreen (value->GetBool ()); break;
    case 2:
    {
      const char* buf = value->GetString ();
      int wres, hres;
      if (sscanf (buf, "%dx%d", &wres, &hres) == 2)
        Resize (wres, hres);
      break;
    }
    default: return false;
  }
  return true;
}

bool csGraphics2D::GetOption (int id, csVariant* value)
{
  switch (id)
  {
    case 0: value->SetLong (Depth); break;
    case 1: value->SetBool (FullScreen); break;
    case 2:
    {
      csString buf;
      buf.Format ("%dx%d", GetWidth (), GetHeight ());
      value->SetString (buf);
      break;
    }
    default: return false;
  }
  return true;
}

bool csGraphics2D::GetOptionDescription (int idx, csOptionDescription* option)
{
  if (idx < 0 || idx >= NUM_OPTIONS)
    return false;
  *option = config_options[idx];
  return true;
}

