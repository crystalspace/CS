/*
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
*/

#ifndef __CS_AWS_IMGVW_H__
#define __CS_AWS_IMGVW_H__

#include "awscomp.h"

class awsImageView : public awsComponent
{
private:
  /// True when button is down, false if up.
  bool is_down;

  /// True if the component has the mouse over it.
  bool mouse_is_over;

  /// True if button was down, and button is in switch mode (toggle = yes).
  bool was_down;

  /// Holds the texture handle for the image we are viewing.
  iTextureHandle *img1; // Via Image.
  iTextureHandle *img2; // Via Texture.

  /// True if we should draw a solid color instead.
  bool draw_color;

  /// The color to draw if draw_color is true.
  int color;

  /// Flags for frame style.
  int frame_style;

  /// Alpha level for this component.
  int alpha_level;
public:
  awsImageView ();
  virtual ~awsImageView ();

  /// Signal constants.
  enum
  {
    /// An up and down motion for the button.
    signalClicked    = 0x1,
    /// A down motion for the button.
    signalMouseDown  = 0x2,
    /// An up motion for the button.
    signalMouseUp    = 0x3,
    /// A movement of the mouse.
    signalMouseMoved = 0x4
  };

  /// Frame styles.
  enum
  {
    fsBump = 0x0,
    fsSimple = 0x1,
    fsRaised = 0x2,
    fsSunken = 0x3,
    fsFlat = 0x4,
    fsNone = 0x5,
    fsScaled = 0x8,
    fsTiled = 0x10,
    fsFixed = 0x20,
    frameMask = 0x7,
    imageMask = ~awsImageView::frameMask
  };

  /// Get the texture handle and the title, plus style if there is one.
  virtual bool Setup (iAws *wmgr, iAwsComponentNode *settings);

  /// Gets properties.
  bool GetProperty (const char *name, intptr_t *parm);

  /// Sets properties.
  bool SetProperty (const char *name, intptr_t parm);

  void SetColor(int color);

  int GetColor();

  /// Returns the named TYPE of the component, like "Radio Button", etc.
  virtual const char *Type ();

  /// Triggered when the component needs to draw.
  virtual void OnDraw (csRect clip);

  /// Triggered when the user presses a mouse button down.
  virtual bool OnMouseDown (int button, int x, int y);

  /// Triggered when the user unpresses a mouse button.
  virtual bool OnMouseUp (int button, int x, int y);

  /// Triggered when the user moves the mouse.
  virtual bool OnMouseMove (int button, int x, int y);

  /// Triggered when this component loses mouse focus.
  virtual bool OnMouseExit ();

  /// Triggered when this component gains mouse focus.
  virtual bool OnMouseEnter ();
};

class awsImageViewFactory : public awsComponentFactory
{
public:
  /**
   * Calls register to register the component that it builds with the
   * window manager.
   */
  awsImageViewFactory (iAws *wmgr);

  /// Does nothing.
  virtual ~awsImageViewFactory ();

  /// Returns a newly created component of the type this factory handles.
  virtual iAwsComponent *Create ();
};

#endif // __CS_AWS_IMGVW_H__
