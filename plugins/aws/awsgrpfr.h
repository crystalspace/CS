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

#ifndef __CS_AWS_GRPRF_H__
#define __CS_AWS_GRPRF_H__

#include "awsPanel.h"

class awsGroupFrame : public awsPanel
{
private:
  /// Caption text for this component.
  iString *caption;
public:
  awsGroupFrame ();
  virtual ~awsGroupFrame ();

  /// An up and down motion for the button.
  static const int signalClicked;

  /// Get the texture handle and the title, plus style if there is one.
  virtual bool Setup (iAws *wmgr, iAwsComponentNode *settings);

  /// Gets properties.
  bool GetProperty (const char *name, intptr_t *parm);

  /// Sets properties.
  bool SetProperty (const char *name, intptr_t parm);

  /// Returns the named TYPE of the component, like "Radio Button", etc.
  virtual const char *Type ();

  /// Triggered when the component needs to draw.
  virtual void OnDraw (csRect clip);
};

class awsGroupFrameFactory : public awsComponentFactory
{
public:
  /**
   * Calls register to register the component that it builds with the
   * window manager.
   */
  awsGroupFrameFactory (iAws *wmgr);

  /// Does nothing.
  virtual ~awsGroupFrameFactory ();

  /// Returns a newly created component of the type this factory handles.
  virtual iAwsComponent *Create ();
};

#endif // __CS_AWS_GRPRF_H__
