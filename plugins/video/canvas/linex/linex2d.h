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
#include "isys/event.h"
#include "isys/plugin.h"

#define XK_MISCELLANY 1
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>

#ifdef XFREE86VM
#  include <X11/extensions/xf86vmode.h>
#endif

/// XLIB version.
class csGraphics2DLineXLib : public csGraphics2D, public iEventPlug
{
  int screen_num;
  int display_width, display_height;
  Window wm_window;
  int wm_width;
  int wm_height;
  Window window;
  Window leader_window;
  Window root_window;
  Pixmap back;
  GC gc, gc_back;
  Visual *visual;
  XVisualInfo vinfo;
  unsigned int vclass;

  int seg_color;
  XSegment segments[100];
  int nr_segments;

  // "WM_DELETE_WINDOW" atom
  Atom wm_delete_window;

  // Window colormap
  Colormap cmap;

  // Hardware mouse cursor or software emulation?
  bool do_hwmouse;
  /// Mouse cursors (if hardware mouse cursors are used)  
  Cursor MouseCursor [int(csmcWait) + 1];
  /// Empty mouse cursor (consist of EmptyPixmap)
  Cursor EmptyMouseCursor;
  /// A empty pixmap
  Pixmap EmptyPixmap;

  bool currently_full_screen;
  bool allow_canvas_resize;

  // The event outlet
  iEventOutlet *EventOutlet;

public:
  // The display context
  static Display* dpy;

  SCF_DECLARE_IBASE_EXT(csGraphics2D);

  csGraphics2DLineXLib (iBase *iParent);
  virtual ~csGraphics2DLineXLib ();

  virtual bool Initialize (iSystem *pSystem);
  virtual bool Open (const char *Title);
  virtual void Close ();

  virtual bool BeginDraw ();

  virtual void Print (csRect *area = NULL);
  virtual void SetRGB (int i, int r, int g, int b);

  virtual void DrawLine (float x1, float y1, float x2, float y2, int color);
  virtual void Clear (int color);
  virtual void Write (iFont*, int x, int y, int fg, int bg, const char *text);
  virtual void DrawBox (int x, int y, int w, int h, int color);

  virtual bool PerformExtensionV (char const* command, va_list);

  /// Set mouse position.
  virtual bool SetMousePosition (int x, int y);

  /// Set mouse cursor shape
  virtual bool SetMouseCursor (csMouseCursorID iShape);

  /// Called on every frame by system driver
  virtual bool HandleEvent (iEvent &Event);

  virtual void AllowCanvasResize (bool iAllow);

  /// helper function which allocates buffers
  bool AllocateMemory ();
  /// helper function which deallocates buffers
  void DeAllocateMemory ();
  bool ReallocateMemory ();

  //------------------------ iEventPlug interface ---------------------------//

  virtual unsigned GetPotentiallyConflictingEvents ()
  { return CSEVTYPE_Keyboard | CSEVTYPE_Mouse; }
  virtual unsigned QueryEventPriority (unsigned /*iType*/)
  { return 150; }

#include "plugins/video/canvas/softx/x2dfs.h"
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
    virtual uint8 *GetGlyphBitmap (uint8 c, int &oW, int &oH);
    virtual void GetDimensions (const char *text, int &oW, int &oH);
    virtual int GetLength (const char *text, int maxwidth);
    virtual void AddDeleteCallback (DeleteNotify, void *)
    { }
    virtual bool RemoveDeleteCallback (DeleteNotify, void *)
    { return true; }
    void Load ();
    SCF_DECLARE_IBASE;
  } font;

public:
  SCF_DECLARE_IBASE;

  csLineX2DFontServer (iBase *iParent);
  virtual ~csLineX2DFontServer () {}
  virtual bool Initialize (iSystem *)
  { return true; }
  virtual iFont *LoadFont (const char *filename);
  virtual int GetFontCount ()
  { return font.xfont ? 1 : 0; }
  virtual iFont *GetFont (int iIndex)
  { (void)iIndex; return font.xfont ? &font : (iFont*)NULL; }

  struct eiPlugIn : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csLineX2DFontServer);
    virtual bool Initialize (iSystem* p) { return scfParent->Initialize(p); }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
};

#endif // __LINEX2D_H__
