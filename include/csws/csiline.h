/*
    Crystal Space Windowing System: input line class
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

#ifndef __CS_CSILINE_H__
#define __CS_CSILINE_H__

/**\file
 * Crystal Space Windowing System: input line class
 */

/**
 * \addtogroup csws_comps_iline
 * @{ */
 
#include "csextern.h"
 
#include "cscomp.h"

/// Possible input line frame styles
enum csInputLineFrameStyle
{
  /// Input line has no frame
  csifsNone,
  /// Button has a thin 3D rectangular frame
  csifsThinRect,
  /// Input line has a thick 3D rectangular frame
  csifsThickRect
};

/// Default input line maximal field length
#define CSIL_DEFAULTLENGTH		256

class csTimer;

/**
 * The Input Line class implements a rectangular are where user can
 * enter any text. The class has a method called IsValidChar() which can
 * be overriden to implement specific needs, for example if you would
 * like to implement a input line which accepts only numbers you can
 * just override the method IsValidChar() and analyze entered characters.
 * There is also a more general method called IsValidString() which
 * checks the just-modified string for correctness. If method decides
 * that string is incorrect, the changes are undone.
 */
class CS_CRYSTALSPACE_EXPORT csInputLine : public csComponent
{
  /// Input line frame style
  csInputLineFrameStyle FrameStyle;
  /// Maximal text line length
  size_t maxlen;
  /// Number of first visible text character
  size_t firstchar;
  /// Cursor character
  size_t cursorpos;
  /// Cursor coordinates
  csRect cursorrect;
  /// true if cursor is visible
  bool cursorvis;
  /// Text selection
  size_t selstart, selend;
  /// true if in insert mode
  bool insert;
  /// Top-Left corner of text
  int textx, texty;
  /// The timer used for cursor flashing
  csTimer *timer;

public:
  /// Create input line object
  csInputLine (csComponent *iParent, int iMaxLen = CSIL_DEFAULTLENGTH,
    csInputLineFrameStyle iFrameStyle = csifsThickRect);

  /// Set text field
  virtual void SetText (const char *iText);

  /// Draw the input line
  virtual void Draw ();

  /// Handle external events
  virtual bool HandleEvent (iEvent &Event);

  /// Override SetState method to redraw input line when it is switched
  virtual void SetState (int mask, bool enable);

  /// Select text from character iStart to character iEnd
  void SetSelection (size_t iStart, size_t iEnd);

  /// Set new cursor position and extend selection if extendsel == true
  void SetCursorPos (size_t NewPos, bool ExtendSel);

  /// Report the minimal size of inputline
  virtual void SuggestSize (int &w, int &h);

  /// Check whenever new cursor position is valid
  virtual bool IsValidPos (size_t NewPos);

  /// Check whenever a character is valid for inserting into string
  virtual bool IsValidChar (char iChar);

  /// Check if string after modification is valid
  virtual bool IsValidString (const char *iText);

  /// Delete selection
  void DeleteSelection ();

protected:
  /// Query character X position within component
  int GetCharX (size_t iNum);
  /// Set text, drop the selection and don't move the cursor
  void SetTextExt (const char *iText);
};

/** @} */

#endif // __CS_CSILINE_H__
