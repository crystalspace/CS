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

#ifndef __CS_AWS_STBAR_H__
#define __CS_AWS_STBAR_H__

#include "awscomp.h"

class awsStatusBar : public awsComponent
{
private:
  /// Flags for frame style.
  int frame_style;

  /// Flags for chart options.
  int chart_options;

  /// Alpha level for this component.
  int alpha_level;

  /// Alpha level for this component's textured bar.
  int bar_alpha_level;

  /// Handle for texture background.
  iTextureHandle *bkg;

  /// Handle for status bar bitmap.
  iTextureHandle *barimg;

  /// Color for status bar.
  int bar_color;

  /// A value from 0 to 1 saying how done you are.
  float status;
public:
  awsStatusBar ();
  virtual ~awsStatusBar();

  /// Frame Styles.
  enum
  {
    /// A frame that's a bump.
    fsBump = 0x0,
    /// A simple frame.
    fsSimple = 0x1,
    /// A frame that looks like a raised button.
    fsRaised = 0x2,
    /// A frame that looks like a sunk button.
    fsSunken = 0x3,
    /// A frame that looks flat.
    fsFlat = 0x4,
    /// No frame at all.
    fsNone = 0x5
  };

  /// Signal constants.
  enum
  {
    /// An up and down motion for the button.
    signalClicked = 0x1
  };

  /// Get the texture handle and the title, plus style if there is one.
  virtual bool Setup (iAws *wmgr, iAwsComponentNode *settings);

  /// Gets properties.
  bool GetProperty (const char *name, intptr_t *parm);

  /// Sets properties.
  bool SetProperty (const char *name, intptr_t parm);

  /// Performs "scripted" execution.
  bool Execute (const char *action, iAwsParmList* parmlist);

  /// Returns the named TYPE of the component, like "Radio Button", etc.
  virtual const char *Type ();

  /// Gets the insets of the frame depending on what style it's in.
  virtual csRect getInsets();

  /// Triggered when the component needs to draw.
  virtual void OnDraw (csRect clip);
};

class awsStatusBarFactory : public awsComponentFactory
{
public:
  /**
   * Calls register to register the component that it builds with
   * the window manager.
   */
  awsStatusBarFactory (iAws *wmgr);

  /// Does nothing.
  virtual ~awsStatusBarFactory ();

  /// Returns a newly created component of the type this factory handles.
  virtual iAwsComponent *Create ();
};

#endif // __CS_AWS_STBAR_H__
