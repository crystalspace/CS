/*
    Copyright (C) 2004 by Andrew Mann

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


typedef struct st_EmitterList {
  csRef<iEmitFixed> point;
  bool point_used;
  csRef<iEmitLine> line;
  bool line_used;
  csRef<iEmitBox> box;
  bool box_used;
  csRef<iEmitCylinder> cylinder;
  bool cylinder_used;
  csRef<iEmitCone> cone;
  bool cone_used;
  csRef<iEmitSphere> sphere;
  bool sphere_used;
  csRef<iEmitSphereTangent> spheretangent;
  bool spheretangent_used;
  csRef<iEmitCylinderTangent> cylindertangent;
  bool cylindertangent_used;
  csRef<iEmitMix> mix;
  csRef<iEmitGen3D> current;
  Emitters current_type;
} EmitterList;



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

  bool RefreshFileList();
  bool RecreateParticleSystem();
  bool UpdateParticleSystem();

  bool recreate_system;
  EmitterState state_emitter,state_emitter_new;
  bool force_emitter_setup;
  Emitter3DState state_initial_position;
  Emitter3DState state_initial_speed;
  Emitter3DState state_initial_acceleration;
  FieldState state_field_speed;
  FieldState state_field_accel;
  AttractorState state_attractor;

  EmitterList startpos,startspeed,startaccel,fieldspeed,fieldaccel,attractor;

  bool InitEmitterList(EmitterList *elist);
  bool ClearGen3D(EmitterList *elist);
  bool UpdateGen3D(EmitterList *elist,Emitter3DState *emitter_state);


  void SaveEmitter3DStateRecursive(Emitter3DState *emitter_state,Emitters use_emitter);
  void SaveEmitter3DStateToFile(Emitter3DState *emitter_state);
  bool SaveEmitterToFile();

public:
  PartEdit (iObjectRegistry* object_reg);
  ~PartEdit ();

  bool Initialize ();
  void Start ();
};

#endif // __PARTEDIT_H__


