/*
    Copyright (C) 2001 by Christopher Nelson

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

#ifndef __CS_AWS_LABEL_H__
#define __CS_AWS_LABEL_H__

#include "awscomp.h"

class awsLabel : public awsComponent
{
private:
  /// True when button is down, false if up.
  bool is_down;

  /// True if the component has the mouse over it.
  bool mouse_is_over;

  /// Alignment of text (defaults to left).
  int alignment;

  /// Caption text for this component.
  iString *caption;
public:
  awsLabel ();
  virtual ~awsLabel ();

  /// Signal constants.
  enum
  {
    /// An up and down motion for the button.
    signalClicked = 0x1,
    /// Component becomes focused.
    signalFocused = 0x2
  };

  /// Alignment constants.
  enum
  {
    /// Align text to left.
    alignLeft = 0x0,
    /// Align text to right.
    alignRight = 0x1,
    /// Align text centered.
    alignCenter = 0x2
  };

  /// Get the texture handle and the title, plus style if there is one.
  virtual bool Setup (iAws *wmgr, iAwsComponentNode *settings);

  /// Get properties.
  bool GetProperty (const char *name, intptr_t *parm);

  /// Set properties.
  bool SetProperty (const char *name, intptr_t parm);

  /// Returns the named TYPE of the component, like "Radio Button", etc.
  virtual const char *Type ();

  /// Returns the smallest this label can be.
  virtual csRect getMinimumSize ();

  /// Triggered when the component needs to draw.
  virtual void OnDraw (csRect clip);

  /// Triggered when the user presses a mouse button down.
  virtual bool OnMouseDown (int button, int x, int y);

  /// Triggered when the user unpresses a mouse button.
  virtual bool OnMouseUp (int button, int x, int y);

  /// Triggered when this component loses mouse focus.
  virtual bool OnMouseExit ();

  /// Triggered when this component gains mouse focus.
  virtual bool OnMouseEnter ();

  /// Triggered when the user presses a key.
  virtual bool OnKeyboard (const csKeyEventData& eventData);

  /// Triggered when this component becomes focused.
  virtual void OnSetFocus ();
};

class awsLabelFactory : public awsComponentFactory
{
public:
  /**
   * Calls register to register the component that it builds with the
   * window manager.
   */
  awsLabelFactory (iAws *wmgr);

  /// Does nothing.
  virtual ~awsLabelFactory ();

  /// Returns a newly created component of the type this factory handles.
  virtual iAwsComponent *Create ();
};

#endif // __CS_AWS_LABEL_H__
