/*
    Copyright (C) 2002 by Anders Stenberg

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

#ifndef __WATERDEMO_H__
#define __WATERDEMO_H__

#include <stdarg.h>
#include "csutil/ref.h"

struct iEngine;
struct iObjectRegistry;
struct iEvent;
struct iVirtualClock;
struct iMaterialWrapper;
struct iKeyboardDriver;
struct iMouseDriver;
struct iVFS;


class csWaterDemo
{
private:
  iObjectRegistry* object_reg;
  csRef<iEngine> engine;
  csRef<iGraphics3D> r3d;
  csRef<iView> view;
  csRef<iKeyboardDriver> kbd;
  csRef<iMouseDriver> mouse;
  csRef<iConsoleOutput> console;

  csRef<iVFS> vfs;
  csRef<iVirtualClock> vc;
  
  csRef<iFont> font;

  csRef<iMeshObject> gMesh;
  csRef<iMeshObjectFactory> gFact;
  csRef<iGeneralMeshState> gMeshState;
  csRef<iGeneralFactoryState> gFactState;
  csRef<iMeshWrapper> gMeshW;

  int mdx, mdy;

  bool hasfocus;

  iSector* room;


  static bool SimpleEventHandler (iEvent& ev);
  bool HandleEvent (iEvent& ev);
  void SetupFrame ();
  void FinishFrame ();


  //surface-buffers
  float *water;  //current heightmap
  float *water1; //water at step -1
  float *water2; //water at step -2

  float WaveSpeed;
  float GridSize;
  float TimeDelta;
  float WaveLife;

  //constants
  float C1, C2, C3;

  int Width; // number of cells - m_fGridSize * m_iWidth = actual width
  int Height; // ditto

  float lastSimTime, nextSimTime;


  void updateWater(float time);
  void generateNormals();
  void pushDownPoint(float x, float z, float depth);

public:
  csWaterDemo (iObjectRegistry* object_reg);
  ~csWaterDemo ();

  bool Initialize ();
  void Start ();
};

#endif // __WATERDEMO_H__

