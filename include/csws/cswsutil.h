/*
    Crystal Space Windowing System: Miscelaneous CSWS utilites
    Copyright (C) 1998,1999 by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __CS_CSWSUTIL_H__
#define __CS_CSWSUTIL_H__

/**
 * \addtogroup csws
 * @{ */
 
#include "csextern.h"
 
#define CSWS_INTERNAL
#include "csws.h"
#include "cswindow.h"

/**
 * Window List class<p>
 * A window of this type is created when user clicks both mouse buttons
 * on application canvas.
 */
class CS_CRYSTALSPACE_EXPORT csWindowList : public csWindow
{
protected:
  /// client dialog component
  csDialog *dialog;
  /// listbox containing window list
  csListBox *list;
  /// The buttons
  csButton *butshow, *butmaximize, *butclose;
  /// Window that was focused before WindowList itself
  csComponent *focusedwindow;
  /// Set to true when window list should close as soon as possible
  bool shouldclose;

public:
  /// Create a "window list" object
  csWindowList (csComponent *iParent);

  /// Set children positions on resize
  virtual bool SetRect (int xmin, int ymin, int xmax, int ymax);

  /// Override SetState method
  virtual void SetState (int mask, bool enable);

  /// Handle input events
  virtual bool HandleEvent (iEvent &Event);

protected:
  /// Rebuild window list
  void RebuildList ();
  /// Used in RebuildList ()
  static bool do_addtowindowlist (csComponent *child, intptr_t param);
};

/**
 * Compute the biggest union of a set of adjanced rectangles
 * (i.e. rectangles do not overlap and can have adjanced edges).
 */
extern CS_CRYSTALSPACE_EXPORT void RectUnion (cswsRectVector &rect, csRect &result);

/// Find a bitmap definition in one of CSWS.CFG bitmap arrays
extern CS_CRYSTALSPACE_EXPORT void ParseConfigBitmap (csApp *app, const char *prefix,
  const char *section, const char *id, int &x, int &y, int &w, int &h);

/// Convert HLS to RGB
extern CS_CRYSTALSPACE_EXPORT void csHLS2RGB (float h, float l, float s, float &r, 
  float &g, float &b);
/// Convert RGB to HLS
extern CS_CRYSTALSPACE_EXPORT void csRGB2HLS (float r, float g, float b, float &h, 
  float &l, float &s);
/// Get a color's R,G,B components (iColor as returned by csApp::FindColor)
extern CS_CRYSTALSPACE_EXPORT void csGetRGB (int iColor, csApp *iApp, float &r, 
  float &g, float &b);

/// The short way to add a text button to a toolbar
extern CS_CRYSTALSPACE_EXPORT csButton *csNewToolbarButton (csComponent *iToolbar, 
  int iCommand, char *iText, csButtonFrameStyle iFrameStyle = csbfsThinRect,
  int iButtonStyle = CSBS_SHIFT | CSBS_TEXTBELOW);
/// The short way to add a icon button to a toolbar
extern CS_CRYSTALSPACE_EXPORT csButton *csNewToolbarButton (csComponent *iToolbar, 
  int iCommand, csPixmap *bmpup = 0, csPixmap *bmpdn = 0,
  csButtonFrameStyle iFrameStyle = csbfsThinRect,
  int iButtonStyle = CSBS_SHIFT, bool iDeletePixmaps = true);
/// Create and return a new bitmap (2D sprite)
extern CS_CRYSTALSPACE_EXPORT csPixmap *NewBitmap (csApp *app, char *texturename, 
  int tx, int ty, int tw, int th);

/** @} */

#endif // __CS_CSWSUTIL_H__
