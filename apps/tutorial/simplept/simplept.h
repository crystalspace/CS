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

#ifndef __SIMPLEPT_H__
#define __SIMPLEPT_H__

#include <stdarg.h>
#include "csutil/ref.h"
#include "cstool/proctex.h"
#include "csgeom/vector3.h"

struct iEngine;
struct iLoader;
struct iGraphics3D;
struct iKeyboardDriver;
struct iSector;
struct iVFS;
struct iEvent;
struct iView;
struct iTextureHandle;
struct iObjectRegistry;
struct iVirtualClock;
struct iThingFactoryState;
struct iMaterialWrapper;
struct iMeshWrapper;
struct iGeneralFactoryState;
class csEngineProcTex;

class Simple
{
public:
  iObjectRegistry* object_reg;

private:
  csRef<iEngine> engine;
  csRef<iLoader> loader;
  csRef<iGraphics3D> g3d;
  csRef<iKeyboardDriver> kbd;
  iSector* room;
  csRef<iView> view;
  csRef<iVirtualClock> vc;
  csEngineProcTex* ProcTexture;
  csRef<iMeshWrapper> genmesh;
  csRef<iGeneralFactoryState> factstate;

  void CreatePolygon (iThingFactoryState *th, int v1, int v2, int v3, int v4,
    iMaterialWrapper *mat);

  int genmesh_resolution;
  csVector3 genmesh_scale;
  float* angle_table;
  float* angle_speed;
  csVector3* start_verts;
  bool CreateGenMesh (iMaterialWrapper* mat);
  void AnimateGenMesh (csTicks elapsed);

public:
  Simple (iObjectRegistry* object_reg);
  virtual ~Simple ();

  bool Initialize ();
  void Start ();
  bool HandleEvent (iEvent&);
  void SetupFrame ();
  void FinishFrame ();
};

class csEngineProcTex : public csProcTexture
{
private:
  csRef<iEngine> Engine;
  csRef<iView> View;

public:
  csEngineProcTex ();
  ~csEngineProcTex ();

  bool LoadLevel ();
  virtual bool PrepareAnim ();
  virtual void Animate (csTicks current_time);
};

#endif // __SIMPLEPT_H__

