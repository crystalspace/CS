/*
    Crystal Space Windowing System: floating hints class
    Copyright (C) 2000 by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __CSHINTS_H__
#define __CSHINTS_H__

#define CSWS_INTERNAL
#include "csws.h"
#include "cscomp.h"
#include "csutil/csvector.h"

/**
 * This component will display "floating hints", which will vanish
 * as soon as you move the mouse or press a key. A object of this class
 * is automatically created by csComponent class when it detects that
 * the mouse is unmoved for some time over some non-focused component
 * that has an associated hint.
 */
class csHint : public csComponent
{
  /// Old mouse owner (before the hint has popped up). Usually NULL.
  csComponent *oldmo;

public:
  /// Create a floating hint with an text string
  csHint (csComponent *iParent, const char *iText, iFont *Font = NULL,
    int iFontSize = -1);
  /// Cleanup before destruction
  virtual ~csHint ();

  /// Draw the hint object
  virtual void Draw ();
  /// Handle all events before any others get a chance to eat it
  virtual bool PreHandleEvent (iEvent &Event);
  /// Set the text of the hint (also resizes the window)
  virtual void SetText (const char *iText);
};

/// Default timeout for hints (in 1/1000 seconds)
#define CSHINT_DEFAULT_TIMEOUT      3000

/**
 * The "hint manager" keeps track of all hints and associated components,
 * and creates the appropiate csHint when it detects mouse is not moved
 * for too long time.
 */
class csHintManager : public csVector
{
  // This structure holds the data about one hint
  struct HintStore
  {
    // The component associated with the hint
    csComponent *comp;
    // The text string starts right after the end of this structure
    char text [1];
  };

  /// The application
  csApp *app;
  /// Last time when the mouse has been moved
  csTime time;
  /// The timeout
  csTime timeout;
  /// Font for hints
  iFont *font;
  /// Font size
  int fontsize;
  /// True if we haven't checked yet the component under the mouse
  bool check;

public:
  /// Initialize the hint manager object
  csHintManager (csApp *iApp);
  /// Destroy the hint manager
  virtual ~csHintManager ();
  /// Override FreeItem to correctly free hint store objects
  virtual bool FreeItem (csSome Item);
  /// Compare two hints (by csComponent's)
  virtual int Compare (csSome Item1, csSome Item2, int Mode) const;
  /// Compare a hint with a csComponent
  virtual int CompareKey (csSome Item, csConstSome Key, int Mode) const;
  /// Add a new hint
  void Add (const char *iText, csComponent *iComp);
  /// Remove the hint (if any) associated with this component
  void Remove (csComponent *iComp);
  /// Examine a mouse event
  void HandleEvent (iEvent &Event);
  /// Set hints timeout
  void SetTimeout (csTime iTimeout)
  { timeout = iTimeout; }
  /// Set the font and font size for hints
  void SetFont (iFont *iNewFont, int iSize);

private:
  // Check if this child has an associated hint
  static bool do_checkhint (csComponent *comp, void *data);
};

#endif // __CSHINTS_H__
