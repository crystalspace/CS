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

#ifndef __PARTEDIT_H__
#define __PARTEDIT_H__

#include <stdarg.h>
#include "csutil/ref.h"
#include "iaws/aws.h"
#include "iaws/awscnvs.h"
#include "imesh/emit.h"

struct iEngine;
struct iLoader;
struct iGraphics3D;
struct iKeyboardDriver;
struct iVirtualClock;
struct iObjectRegistry;
struct iEvent;
struct iSector;
struct iView;
struct iAwsSource;



class PartEdit
{
private:
  iObjectRegistry* object_reg;
  csRef<iEngine> engine;
  csRef<iLoader> loader;
  csRef<iGraphics3D> g3d;
  csRef<iGraphics2D> g2d;
  csRef<iKeyboardDriver> kbd;
  csRef<iVFS> vfs;
  csRef<iVirtualClock> vc;
  csRef<iView> view;
  csRef<iPluginManager> PluginManager;
  csRef<iAws> aws;
  iAwsPrefManager* awsprefs;
  csRef<iAwsCanvas> awsCanvas;
  iSector* room;
  csRef<iEmitState> emitState;
  bool keydown;
  float value;
  csString current_graphic;
  csRef<iMeshWrapper> mw;  
  csRef<iMeshObjectFactory> EmitObjectFactory;
  csRef<iEmitFactoryState> EmitFactoryState;
   
  static bool EventHandler (iEvent& ev);
  bool HandleEvent (iEvent& ev);
  void SetupFrame ();
  void FinishFrame ();

  csRef<iEmitGen3D> CreateGen3D(Emitter3DState *emitter_state);
  bool RefreshFileList();
  bool RecreateParticleSystem();
  bool UpdateParticleSystem();

  bool recreate_system;
  EmitterState state_emitter;
  Emitter3DState state_initial_position;
  Emitter3DState state_initial_speed;
  Emitter3DState state_initial_acceleration;
  AttractorState state_attractor;

  /** Set to true on start.  Set to false when a quit event is received.
   *  When false, normal events are not processed (such as aws events) since the underlying objects
   *  needed to process the events may already be destructed.
   */

  bool running; 

public:
  PartEdit (iObjectRegistry* object_reg);
  ~PartEdit ();

  bool Initialize ();
  void Start ();
};

#endif // __PARTEDIT_H__


