/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#ifndef __MDLTEST_H__
#define __MDLTEST_H__

#include <stdarg.h>
#include "imesh/mdldata.h"

struct iEngine;
struct iGraphics3D;
struct iGraphics2D;
struct iKeyboardDriver;
struct iLoader;
struct iSector;
struct iView;
struct iCrossBuilder;
struct iThingState;
struct iMaterialWrapper;
struct iModelConverter;
struct iObjectRegistry;
struct iVirtualClock;
struct iEvent;

class Simple
{
public:
  iObjectRegistry* object_reg;

private:
  iEngine* engine;
  iLoader* loader;
  iGraphics3D* g3d;
  iSector* room;
  iView* view;
  iCrossBuilder* crossbuilder;
  iModelConverter* converter;
  iVFS *vfs;
  iKeyboardDriver* kbd;
  iVirtualClock* vc;

public:
  Simple ();
  virtual ~Simple ();

  void Report (int severity, const char* msg, ...);

  bool Initialize (int argc, const char* const argv[],
	const char *iConfigName);
  bool HandleEvent (iEvent&);
  void SetupFrame ();
  void FinishFrame ();
  iModelDataVertices *CreateDefaultModelVertexFrame ();
  iModelData *ImportModel (const char *Filename);
  iModelData *CreateDefaultModel ();
  iMaterialWrapper *LoadTexture (const char *Name, const char *Filename);
};

#endif // __MDLTEST_H__
