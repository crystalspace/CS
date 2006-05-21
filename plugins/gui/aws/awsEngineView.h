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

#ifndef __CS_AWS_ENGINEVIEW_H__
#define __CS_AWS_ENGINEVIEW_H__

#include "awscomp.h"

struct iView;

class awsEngineView : public awsComponent  
{
private:
  iView* view;
public:
  awsEngineView ();
  virtual ~awsEngineView ();

  virtual void OnDraw (csRect clip);

  /// Gets properties.
  bool GetProperty (const char *name, intptr_t *parm);

  /// Sets properties.
  bool SetProperty (const char *name, intptr_t parm);

  /// Returns the named TYPE of the component, like "Radio Button", etc.
  virtual const char *Type ();
};

class awsEngineViewFactory : public awsComponentFactory
{
public:
  awsEngineViewFactory (iAws* mgr);
  ~awsEngineViewFactory ();

  iAwsComponent* Create ();
};

#endif // __CS_AWS_ENGINEVIEW_H__
