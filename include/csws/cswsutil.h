/*
  Crystal Space Windowing System: Miscelaneous CSWS utilites
  Copyright (C) 1998 by Jorrit Tyberghein
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

#ifndef __CSWSUTIL_H__
#define __CSWSUTIL_H__

#define CSWS_INTERNAL
#include "csws.h"
#include "cswindow.h"

/**
 * Window List class<p>
 * A window of this type is created when user clicks both mouse buttons
 * on application canvas.
 */
class csWindowList : public csWindow
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
  virtual bool HandleEvent (csEvent &Event);

protected:
  /// Rebuild window list
  void RebuildList ();
  /// Used in RebuildList ()
  static bool do_addtowindowlist (csComponent *child, void *param);
};

/**
 * Message box style flags (used in MessageBox (...) as last parameter)<p>
 * These style flags can be combined using logical OR (|) operation;
 * some styles are mutually exclusive (for example, a message box cannot
 * be CSMBS_INFORMATION and CSMBS_WARNING at same time)
 */
/// Message has no type
#define CSMBS_NOTYPE		0x00000000
/// Informational message
#define CSMBS_INFORMATION	0x00000001
/// Same for lazy ones (like me :-)
#define CSMBS_INFO		CSMBS_INFORMATION
/// Warning message
#define CSMBS_WARNING		0x00000002
/// Same but shorter
#define CSMBS_WARN		CSMBS_WARNING
/// Question message
#define CSMBS_QUESTION		0x00000003
/// Fatal error
#define CSMBS_ERROR		0x00000004
/// Invalid operation
#define CSMBS_STOP		0x00000005
/// This is the mask used to separate message box style from other flags
#define CSMBS_TYPEMASK		0x0000000F

/// Message box contains a "OK" button
#define CSMBS_OK		0x00000010
/// Message box contains a "Cancel" button
#define CSMBS_CANCEL		0x00000020
/// Message box contains a "Abort" button
#define CSMBS_ABORT		0x00000040
/// Message box contains a "Retry" button
#define CSMBS_RETRY		0x00000080
/// Message box contains a "Ignore" button
#define CSMBS_IGNORE		0x00000100
/// Message box contains a "OK" button and a "Cancel" button
#define CSMBS_OKCANCEL		(CSMBS_OK | CSMBS_CANCEL)
/// Message box contains a "Abort" button, a "Retry" button and a "Ignore" button
#define CSMBS_ABORTRETRYIGNORE	(CSMBS_ABORT | CSMBS_RETRY | CSMBS_IGNORE)

/// Message is not program-modal
#define CSMBS_NONMODAL		0x80000000

/// Display a message box and return ID of pressed button (0 for Esc)
extern int MessageBox (csComponent *iParent, char *iTitle, char *iMessage,
  int iFlags = CSMBS_INFO | CSMBS_OK);
/// Create and return a new file open dialog
extern csWindow *csFileDialog (csComponent *iParent, char *iTitle,
  char *iFileName = "./", char *iOpenButtonText = "~Load");
/// Query full name, filename and pathname from a file dialog
extern void csQueryFileDialog (csWindow *iFileDialog, char *iFileName,
  size_t iFileNameSize);

/// Compute the biggest union of a set of adjanced rectangles
/// (i.e. rectangles do not overlap and can have adjanced edges).
extern void RectUnion (csObjVector &rect, csRect &result);

/// Find a bitmap definition in one of CSWS.CFG bitmap arrays
extern void FindCFGBitmap (csStrVector &sv, char *id, int *x, int *y,
  int *w, int *h);

/// The short way to add a text button to a toolbar
extern csButton *csNewToolbarButton (csComponent *iToolbar, int iCommand,
  char *iText, csButtonFrameStyle iFrameStyle = csbfsThinRect,
  int iButtonStyle = CSBS_SHIFT | CSBS_TEXTBELOW);
/// The short way to add a icon button to a toolbar
extern csButton *csNewToolbarButton (csComponent *iToolbar, int iCommand,
  csSprite2D *bmpup = NULL, csSprite2D *bmpdn = NULL,
  csButtonFrameStyle iFrameStyle = csbfsThinRect, int iButtonStyle = CSBS_SHIFT);
/// Create and return a new bitmap (2D sprite)
extern csSprite2D *NewBitmap (csApp *app, char *texturename, int tx, int ty,
  int tw, int th);

#endif // __CSWSUTIL_H__
