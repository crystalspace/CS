/*
    Crystal Space Windowing System: button class
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

#ifndef __CSBUTTON_H__
#define __CSBUTTON_H__

#include "cscomp.h"

/// Button style flags: does bitmap and/or text shift when button is pressed?
#define CSBS_SHIFT		0x00000001
/// Is this the default button?
#define CSBS_DEFAULT		0x00000002
/// Never draw a "default button" border
#define CSBS_NODEFAULTBORDER	0x00000004
/// Is this button selectable?
#define CSBS_SELECTABLE		0x00000008
/// Do not focus button when clicked by mouse
#define CSBS_NOMOUSEFOCUS	0x00000010
/// Do not focus button when activated with keyboard
#define CSBS_NOKEYBOARDFOCUS	0x00000020
/**
 * Button is a multi-choose button<p>
 * A multi-choose button sends a cscmdButtonDeselect to all its neightbours
 * until a group bound is encountered. This is used for panels of buttons of
 * which only one can be selected (and pressed) at one time.
 */
#define CSBS_MULTICHOOSE	0x00000040
/// Dismiss parent dialog when this button is pressed
#define CSBS_DISMISS		0x00000080
/// Text placement relative to bitmap: value mask
#define CSBS_TEXTPLACEMENT	0x00030000
/// Draw text above bitmap
#define CSBS_TEXTABOVE		0x00000000
/// Draw text below the bitmap
#define CSBS_TEXTBELOW		0x00010000
/// Draw text on top of the bitmap (bitmap as background)
#define CSBS_TEXTONTOP		0x00020000

/// Default button styles
#define CSBS_DEFAULTVALUE	(CSBS_SHIFT | CSBS_SELECTABLE | \
				 CSBS_TEXTBELOW)

enum
{
  /**
   * Button down notification<p>
   * This notification is posted each time a button switches from 'normal'
   * state into 'pressed' state.
   * <pre>
   * IN: (csButton *)source
   * </pre>
   */
  cscmdButtonDown = 0x00000500,
  /**
   * Button up notification<p>
   * This notification is posted each time a button switches from 'pressed'
   * state into 'normal' state.
   * <pre>
   * IN: (csButton *)source
   * </pre>
   */
  cscmdButtonUp,
  /**
   * Right-click button notification message<p>
   * This message is sent by a button to its parent when user
   * clicks the button with right mouse button.
   * <pre>
   * IN: (csButton *)source
   * </pre>
   */
  cscmdButtonRightClick,
  /**
   * This message is sent by a button with CSBS_MULTICHOOSE style to its
   * neightbours to deselect them.<p>
   * Upon receiving of this message button should depress if it is pressed.
   * <pre>
   * IN: (csButton *)source
   * </pre>
   */
  cscmdButtonDeselect
};

/// Possible button frame styles
enum csButtonFrameStyle
{
  /// Button has no frame
  csbfsNone,
  /// Button has a thick rectangular frame with oblique corners
  csbfsOblique,
  /// Button has a thick rectangular frame
  csbfsThickRect,
  /// Button has a thin rectangular frame
  csbfsThinRect,
  /// Button has no frame in unpressed state and a thin frame when it is pressed
  csbfsVeryThinRect
};

/**
 * The Button class implements different types of push buttons.
 * Buttons can contain a text string and/or a bitmap.
 */
class csButton : public csComponent
{
protected:
  // Button images in normal and pressed state
  csPixmap *ImageNormal, *ImagePressed;
  // Should images be automatically deleted?
  bool delImages;
  // Command code emmited when button is pressed
  int CommandCode;
  // Character number that should be underlined (-1 == none)
  int underline_pos;
  /// Button style
  int ButtonStyle;
  // Button frame style
  csButtonFrameStyle FrameStyle;

public:
  /// Current button state
  bool Pressed;

  /// Create button object
  csButton (csComponent *iParent, int iCommandCode, int iButtonStyle =
    CSBS_DEFAULTVALUE, csButtonFrameStyle iFrameStyle = csbfsOblique);
  /// Destroy button object
  virtual ~csButton ();

  /// Set button text
  virtual void SetText (const char *iText)
  { PrepareLabel (iText, text, underline_pos); Invalidate (); }

  /**
   * Set button bitmaps in normal and pressed states<p>
   * If iDelete is true, bitmaps will be automatically deleted when they
   * will not be needed anymore (i.e. button disposal or another SetBitmap)
   */
  void SetBitmap (csPixmap *iNormal, csPixmap *iPressed, bool iDelete = true);

  /// Query button bitmaps
  void GetBitmap (csPixmap **iNormal, csPixmap **iPressed);

  /// Delete image bitmaps if iDelete was true on SetBitmap
  void FreeBitmaps ();

  /// Handle external events
  virtual bool HandleEvent (iEvent &Event);

  /// Pre-handle keyboard events
  virtual bool PostHandleEvent (iEvent &Event);

  /// Override SetState method to redraw button if it is default
  virtual void SetState (int mask, bool enable);

  /// Return the recommended minimal size of button
  virtual void SuggestSize (int &w, int &h);

  /// Query this button's command code
  void SetCommandCode (int iCommandCode)
  { CommandCode = iCommandCode; }

  /// Query this button's command code
  int GetCommandCode ()
  { return CommandCode; }

  /// Set button pressed state
  virtual void SetPressed (bool state);

  /// Get button style flags
  inline int GetButtonStyle ()
  { return ButtonStyle; }

  /// Get button frame style
  inline csButtonFrameStyle GetFrameStyle ()
  { return FrameStyle; }

  /// Get the character number to be underlined (hotkey)
  inline int GetUnderlinePos ()
  { return underline_pos; }

  /// Get the name of the skip slice for this component
  virtual char *GetSkinName ()
  { return "Button"; }

protected:
  /// Emulate a button press (generate command)
  virtual void Press ();

  /// Handle a key down event (called for HandleEvent and PreHandleEvent)
  bool HandleKeyPress (iEvent &Event);

  /// Deselect all button's neightbours in his group
  void DeselectNeighbours ();
};

#endif // __CSBUTTON_H__
