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

#include <crystalspace.h>

class csEngineProcTex : public csProcTexture
{
private:
  csRef<iEngine> Engine;
  csRef<iView> View;
  
  struct Target
  {
    csRef<iTextureHandle> texh;
    csRenderTargetAttachment attachment;
    const char* format;
  };
  csArray<Target> targets;
  size_t currentTarget;

  csString currentTargetStr;
  csString availableFormatsStr;
  bool renderTargetState;
public:
  csEngineProcTex ();
  ~csEngineProcTex ();

  bool LoadLevel ();
  iTextureWrapper* CreateTexture (iObjectRegistry* object_reg);
  virtual bool PrepareAnim ();
  virtual void Animate (csTicks current_time);
  
  const char* GetCurrentTarget () const { return currentTargetStr; }
  const char* GetAvailableFormats() const { return availableFormatsStr; }
  bool GetRenderTargetState() const { return renderTargetState; }
  void CycleTarget();
};

class Simple
{
public:
  iObjectRegistry* object_reg;
  csEventID Process;
  csEventID FinalProcess;
  csEventID KeyboardDown;

private:
  csRef<iEngine> engine;
  csRef<iLoader> loader;
  csRef<iGraphics3D> g3d;
  csRef<iKeyboardDriver> kbd;
  iSector* room;
  csRef<iView> view;
  csRef<iRenderManager> rm;
  csRef<iVirtualClock> vc;
  csEngineProcTex* ProcTexture;
  csRef<iMeshWrapper> genmesh;
  csRef<iGeneralFactoryState> factstate;
  csRef<iFont> font;

  csRef<iTextureWrapper> targetTexture;
  csRef<iView> targetView;

  void CreatePolygon (iGeneralFactoryState *th, int v1, int v2, int v3, int v4);

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

#endif // __SIMPLEPT_H__

