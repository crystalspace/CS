/*
  Copyright (C) 2010 by Alin Baciu

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
 
#ifndef __VIDEOPLAY_H__
#define __VIDEOPLAY_H__

#include "crystalspace.h"

#include "ivaria/icegui.h"

#include <iostream>

#include <ividplay/vpl_structs.h>
#include <ividplay/vpl_data.h>
#include <ividplay/vpl_loader.h>

struct iEngine;
struct iObjectRegistry;
struct iGraphics3D;
struct iGraphics2D;

/**
 * The main class that runs the video player demo application.
 */
class VidPlay : public CS::Utility::DemoApplication
{
 private:
  bool CreateScene ();

  bool inWater;
  csRef<iTextureHandle> logoTex;

 public:
  VidPlay ();

  //-- CS::Utility::DemoApplication
  void PrintHelp ();
  void Frame ();
  const char* GetApplicationConfigFile()
  { return "/config/csisland.cfg"; }

  //-- csApplicationFramework
  bool Application ();
};

#endif // __VIDEOPLAY_H__
