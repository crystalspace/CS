/*
    Copyright (C) 2004 by Jorrit Tyberghein
	      (C) 2004 by Frank Richter

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

#ifndef __CS_CANVAS_OPENGLCOMMON_DRIVERDB_H__
#define __CS_CANVAS_OPENGLCOMMON_DRIVERDB_H__

#include "csextern.h"
#include "csutil/leakguard.h"

struct iConfigDocument;
class csGraphics2DGLCommon;

class CS_CSPLUGINCOMMON_OGL_EXPORT csGLDriverDatabase
{
public:
  CS_LEAKGUARD_DECLARE (csGLDriverDatabase);

  csGraphics2DGLCommon* ogl2d;
  csRefArray<iConfigFile> addedConfigs;
  const char* rulePhase;

  csStringHash tokens;
#define CS_TOKEN_ITEM_FILE "csplugincommon/opengl/driverdb.tok"
#include "cstool/tokenlist.h"

  csGLDriverDatabase ();
  ~csGLDriverDatabase ();

  void Report (int severity, const char* msg, ...);

  void Open (csGraphics2DGLCommon* ogl2d, const char* phase = 0);
  void Close ();
};

#endif // __CS_CANVAS_OPENGLCOMMON_DRIVERDB_H__
