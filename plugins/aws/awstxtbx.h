#ifndef __AWS_TEXT_BOX_H__
# define __AWS_TEXT_BOX_H__

/**************************************************************************
    Copyright (C) 2000-2001 by Christopher Nelson

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
*****************************************************************************/
# include "awscomp.h"

class awsTextBox :
  public awsComponent
{
  /// True if the component has the mouse over it
  bool mouse_is_over;

  /// True if the component has focus
  bool has_focus;

  /// True if the component should mask chars
  int should_mask;

  /// Holds the background texture: either global texture, or override in component.
  iTextureHandle *bkg;

  /// Flags for frame style.
  int frame_style;

  /// Alpha level for this component
  int alpha_level;

  /// Text of this component, editable.
  iString *text;

  /// Text that's not allowed to add
  iString *disallow;

  /// Character to replace text with
  iString *maskchar;

  /// Position of first character we display.
  int start;

  /// Position of cursor
  int cursor;

  /// The timer that makes the cursor blink.
  awsTimer *blink_timer;

  /// The current blink state.
  bool blink;
public:
  awsTextBox ();
  virtual ~awsTextBox ();

  /******* Frame Styles **********************/

  /// A "normal" textbox.  Is textured if there is a background texture.
  static const int fsNormal;

  /// A textbox whose background is defined entirely by the bitmap.
  static const int fsBitmap;

  /******* Signals **********************/

  /// Occurs whenever text is changed.
  static const int signalChanged;

  /// Occurs whenever the component loses keyboard focus.
  static const int signalLostFocus;

  /// Occurs when the "enter" key is pressed.
  static const int signalEnterPressed;

  /// Occures when the "tab" key is pressed.
  static const int signalTabPressed;

public:
  /// Get's the texture handle and the title, plus style if there is one.
  virtual bool Setup (iAws *wmgr, awsComponentNode *settings);

  /// Gets properties
  bool GetProperty (char *name, void **parm);

  /// Sets properties
  bool SetProperty (char *name, void *parm);

  /// Returns the named TYPE of the component, like "Radio Button", etc.
  virtual char *Type ();
public:

  /// Triggered when the component needs to draw
  virtual void OnDraw (csRect clip);

  /// Triggered when the user presses a mouse button down
  virtual bool OnMouseDown (int button, int x, int y);

  /// Triggered when the user unpresses a mouse button
  virtual bool OnMouseUp (int button, int x, int y);

  /// Triggered when the user moves the mouse
  virtual bool OnMouseMove (int button, int x, int y);

  /// Triggered when the user clicks the mouse
  virtual bool OnMouseClick (int button, int x, int y);

  /// Triggered when the user double clicks the mouse
  virtual bool OnMouseDoubleClick (int button, int x, int y);

  /// Triggered when this component loses mouse focus
  virtual bool OnMouseExit ();

  /// Triggered when this component gains mouse focus
  virtual bool OnMouseEnter ();

  /// Triggered when the user presses a key
  virtual bool OnKeypress (int key, int modifiers);

  /// Triggered when the keyboard focus is lost
  virtual bool OnLostFocus ();

  /// Triggered when the keyboard focus is gained
  virtual bool OnGainFocus ();
};

class awsTextBoxFactory :
  public awsComponentFactory
{
public:

  /// Calls register to register the component that it builds with the window manager
  awsTextBoxFactory (iAws *wmgr);

  /// Does nothing
  virtual ~awsTextBoxFactory ();

  /// Returns a newly created component of the type this factory handles.
  virtual iAwsComponent *Create ();
};
#endif
