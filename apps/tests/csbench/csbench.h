/*
    Copyright (C) 2004 by Jorrit Tyberghein

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

#ifndef __CSBENCH_H__
#define __CSBENCH_H__

class CsBench;
struct iMaterialHandle;
struct iImageIO;
struct iObjectRegistry;
struct iVirtualClock;
struct iGraphics3D;
struct iGraphics2D;
struct iVFS;
struct iEvent;
struct iEngine;
struct iLoader;
struct iView;
struct iSector;
struct iShaderManager;
struct iDocumentSystem;
struct iGeneralMeshState;
struct iStringSet;

#define BIGOBJECT_DIM 160
#define SMALLOBJECT_DIM 17
#define SMALLOBJECT_NUM 100
#define BENCHTIME 3000

class CsBench
{
public:
  iObjectRegistry* object_reg;

private:
  csRef<iEngine> engine;
  csRef<iLoader> loader;
  csRef<iImageIO> imageio;
  csRef<iGraphics3D> g3d;
  csRef<iVFS> vfs;
  csRef<iVirtualClock> vc;
  csRef<iShaderManager> shader_mgr;
  csRef<iDocumentSystem> docsys;
  csRef<iCommandLineParser> cmdline;
  csRef<iView> view;
  csRef<iGeneralMeshState> gmSingle;
  csRef<iStringSet> strings;
  iSector* room_single;
  iSector* room_multi;
  iMaterialWrapper* material;

  iMeshFactoryWrapper* CreateGenmeshLattice (int dim, float size,
  	const char* name);

  bool SetupMaterials ();
  iSector* CreateRoom (const char* name, const char* meshname,
  	const csVector3& p1, const csVector3& p2);
  bool CreateTestCaseSingleBigObject ();
  bool CreateTestCaseMultipleObjects ();

  float BenchMark (const char* name, const char* description,
  	uint drawFlags = 0);
  iDocumentSystem* GetDocumentSystem ();
  iShaderManager* GetShaderManager ();
  void PerformShaderTest (const char* shaderPath, const char* shtype, 
    const char* shaderPath2, const char* shtype2, 
    iGeneralMeshState* genmesh);

public:
  CsBench ();
  virtual ~CsBench ();

  void Report (const char* msg, ...);
  bool ReportError (const char* msg, ...);

  bool Initialize (int argc, const char* const argv[],
    const char *iConfigName);
  void PerformTests ();
};

#endif // __CSBENCH_H__

