/*
    Copyright (C) 2000 by Jorrit Tyberghein

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

#ifndef __LINEX2D_H__
#define __LINEX2D_H__

#include "csutil/scf.h"
#include "video/canvas/common/graph2d.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "ivideo/xwindow.h"

#define XK_MISCELLANY 1
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>

/// XLIB version.
class csGraphics2DLineXLib : public csGraphics2D, public iEventPlug
{
  iXWindow  *xwin;
  /// The event outlet
  iEventOutlet *EventOutlet;
  int screen_num;
  Window window;
  Pixmap back;
  GC gc, gc_back;
  XVisualInfo xvis;
  // Window colormap
  Colormap cmap;

  int seg_color;
  XSegment segments[100];
  int nr_segments;

  /// helper function which creates the appropriate visual resources
  bool CreateVisuals ();
  /// helper function which allocates buffers
  bool AllocateMemory ();
  /// helper function which deallocates buffers
  void DeAllocateMemory ();

public:
  SCF_DECLARE_IBASE_EXT(csGraphics2D);
  // The display context
  static Display* dpy;
  csGraphics2DLineXLib (iBase *iParent);
  virtual ~csGraphics2DLineXLib ();

  virtual bool Initialize (iObjectRegistry *object_reg);
  virtual bool Open ();
  virtual void Close ();

  void Report (int severity, const char* msg, ...);

  virtual bool BeginDraw ();

  virtual void Print (csRect *area = NULL);
  virtual void SetRGB (int i, int r, int g, int b);

  virtual void DrawLine (float x1, float y1, float x2, float y2, int color);
  virtual void Clear (int color);
  virtual void Write (iFont*, int x, int y, int fg, int bg, const char *text);
  virtual void DrawBox (int x, int y, int w, int h, int color);

  virtual bool PerformExtensionV (char const* command, va_list);

  virtual void AllowResize (bool iAllow);

  virtual bool Resize (int width, int height);

  virtual bool GetFullScreen ()
  { return xwin->GetFullScreen (); }

  virtual void SetFullScreen (bool yesno);

  virtual void SetTitle (const char* title)
  { xwin->SetTitle (title); }
  /// Set mouse position.
  // should be the window manager
  virtual bool SetMousePosition (int x, int y)
  { return xwin->SetMousePosition (x, y); }

  /// Set mouse cursor shape
  // should be the window manager
  virtual bool SetMouseCursor (csMouseCursorID iShape)
  { return xwin->SetMouseCursor (iShape); }

  //------------------------ iEventPlug interface ---------------------------//

  virtual unsigned GetPotentiallyConflictingEvents ()
  { return 0; }
  virtual unsigned QueryEventPriority (unsigned /*iType*/)
  { return 150; }

};

/**
 * A pretty simple font server that manages one single X11 font :-)
 */
class csLineX2DFontServer : public iFontServer
{
  // The font object
  struct csLineX2DFont : public iFont
  {
    XFontStruct *xfont;
    int FontW, FontH;
    csLineX2DFont ();
    virtual ~csLineX2DFont ();
    virtual void SetSize (int iSize) { (void)iSize; }
    virtual int GetSize () { return 0; }
    virtual void GetMaxSize (int &oW, int &oH)
    { oW = FontW; oH = FontH; }
    virtual bool GetGlyphSize (uint8 c, int &oW, int &oH);
    virtual bool GetGlyphSize (uint8 c, int &oW, int &oH, int &adv, int &left, int &top);
    virtual uint8 *GetGlyphBitmap (uint8 c, int &oW, int &oH);
    virtual uint8 *GetGlyphBitmap (uint8 c, int &oW, int &oH, int &adv, int &left, int &top);
    virtual void GetDimensions (const char *text, int &oW, int &oH);
    virtual void GetDimensions (const char *text, int &oW, int &oH, int &desc);
    virtual int GetLength (const char *text, int maxwidth);
    virtual void AddDeleteCallback (iFontDeleteNotify*)
    { }
    virtual bool RemoveDeleteCallback (iFontDeleteNotify*)
    { return true; }
    void Load ();
    SCF_DECLARE_IBASE;
  } font;

public:
  SCF_DECLARE_IBASE;

  csLineX2DFontServer (iBase *iParent);
  virtual ~csLineX2DFontServer () {}
  virtual bool Initialize (iObjectRegistry *)
  { return true; }
  virtual iFont *LoadFont (const char *filename);
  virtual int GetFontCount ()
  { return font.xfont ? 1 : 0; }
  virtual iFont *GetFont (int iIndex)
  { (void)iIndex; return font.xfont ? &font : (iFont*)NULL; }

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csLineX2DFontServer);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;
};

#endif // __LINEX2D_H__
