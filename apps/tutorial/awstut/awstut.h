/*
    Copyright (C) 2002 by Jorrit Tyberghein

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

#ifndef __AWSTUT_H__
#define __AWSTUT_H__

#include <stdarg.h>
#include <crystalspace.h>

class AwsTutorial
{
public:
  iObjectRegistry* object_reg;

private:
  csRef<iAws> aws;
  iAwsPrefManager* awsprefs;
  csRef<iAwsCanvas> awsCanvas;
  csRef<iGraphics3D> myG3D;
  csRef<iGraphics2D> myG2D;
  csRef<iVirtualClock> vc;

  static void SetPass (intptr_t awstut, iAwsSource *source);
  static void SetUser (intptr_t awstut, iAwsSource *source);
  static void Login (intptr_t awstut, iAwsSource *source);

public:
  AwsTutorial();
  virtual ~AwsTutorial();

  bool Initialize (int argc, const char* const argv[]);
  void Report (int severity, const char* msg, ...);

  void SetupFrame ();
  void FinishFrame ();
  bool HandleEvent (iEvent &Event);
};

#endif // __AWSTUT_H__

