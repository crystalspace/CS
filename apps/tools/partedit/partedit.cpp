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
#include "cssysdef.h"
#include "cssys/sysfunc.h"
#include "csutil/scfstr.h"
#include "iutil/vfs.h"
#include "csutil/cscolor.h"
#include "cstool/csview.h"
#include "cstool/initapp.h"
#include "iutil/eventq.h"
#include "iutil/event.h"
#include "iutil/objreg.h"
#include "iutil/csinput.h"
#include "iutil/virtclk.h"
#include "iengine/sector.h"
#include "iengine/engine.h"
#include "iengine/camera.h"
#include "iengine/light.h"
#include "iengine/statlght.h"
#include "iengine/texture.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "iengine/material.h"
#include "imesh/thing/polygon.h"
#include "imesh/thing/thing.h"
#include "imesh/object.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/txtmgr.h"
#include "ivideo/texture.h"
#include "ivideo/material.h"
#include "ivideo/fontserv.h"
#include "ivideo/natwin.h"
#include "igraphic/imageio.h"
#include "imap/parser.h"
#include "ivaria/reporter.h"
#include "ivaria/stdrep.h"
#include "csutil/cmdhelp.h"
#include "qsqrt.h"
#include "csutil/event.h"

#include "imesh/emit.h"
#include "iutil/plugin.h"

#include "awssink.h"
#include "partedit.h"

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

// The global pointer to PartEdit
PartEdit *System;

awsSink *s;

PartEdit::PartEdit (iObjectRegistry* object_reg)
{
  PartEdit::object_reg = object_reg;
  keydown=false;
  value = 0;
}

PartEdit::~PartEdit ()
{
}

void PartEdit::SetupFrame ()
{
  // First get elapsed time from the virtual clock.
  csTicks elapsed_time = vc->GetElapsedTicks ();
  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.0) * (0.03 * 20);

  iCamera* c = view->GetCamera();


  if (kbd->GetKeyState (CSKEY_RIGHT))
    c->GetTransform ().RotateThis (CS_VEC_ROT_RIGHT, speed);
  if (kbd->GetKeyState (CSKEY_LEFT))
    c->GetTransform ().RotateThis (CS_VEC_ROT_LEFT, speed);
  if (kbd->GetKeyState (CSKEY_PGUP) && keydown == false) 
  {
    c->GetTransform ().RotateThis (CS_VEC_TILT_UP, speed);
  }
  else
    // keydown = false;
    if (kbd->GetKeyState (CSKEY_PGDN))
      c->GetTransform ().RotateThis (CS_VEC_TILT_DOWN, speed);
  if (kbd->GetKeyState (CSKEY_UP))
    c->Move (CS_VEC_FORWARD * 4 * speed);
  if (kbd->GetKeyState (CSKEY_DOWN))
    c->Move (CS_VEC_BACKWARD * 4 * speed);

  // Tell 3D driver we're going to display 3D things.
  if (!g3d->BeginDraw (engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS))
    return;

  // Tell the camera to render into the frame buffer.
  view->Draw ();

  // Start drawing 2D graphics.
  if (!g3d->BeginDraw (CSDRAW_2DGRAPHICS))
    return;

  // Make sure invalidated areas get a chance to
  // redraw themselves.
  aws->Redraw ();

  // Draw the current view of the window system to a
  // graphics context with a certain alpha value.
  aws->Print (g3d, 64);

  csString filestr;
  if (s)
  {
    bool update=false;
    EmitterState *estate;

    if (s->EmitterStateChanged())
    {
      estate=s->GetEmitterState();
      memcpy(&state_emitter,estate,sizeof(EmitterState));
      s->ClearEmitterStateChanged();
      update=true;
    }

    Emitter3DState *e3dstate;
    if (s->InitialPositionStateChanged())
    {
      e3dstate=s->GetInitialPositionState();
      memcpy(&state_initial_position,e3dstate,sizeof(Emitter3DState));
      s->ClearInitialPositionStateChanged();
      update=true;
    }
    if (s->InitialSpeedStateChanged())
    {
      e3dstate=s->GetInitialSpeedState();
      memcpy(&state_initial_speed,e3dstate,sizeof(Emitter3DState));
      s->ClearInitialSpeedStateChanged();
      update=true;
    }
    if (s->InitialAccelerationStateChanged())
    {
      e3dstate=s->GetInitialAccelerationState();
      memcpy(&state_initial_acceleration,e3dstate,sizeof(Emitter3DState));
      s->ClearInitialAccelerationStateChanged();
      update=true;
    }


    if (s->AttractorStateChanged())
    {
      AttractorState *atstate;
      atstate=s->GetAttractorState();
      memcpy(&state_attractor,atstate,sizeof(AttractorState));
      s->ClearAttractorStateChanged();
      update=true;
    }


    filestr=s->GetGraphicFile();
    if (filestr.Length()>0 && current_graphic != filestr.GetData())
    {
	  csString temp=current_graphic;
	  current_graphic=filestr;
	  if (!RecreateParticleSystem())
	  {
		  current_graphic=temp;
	  }
	  else
		  update=false; // The update was taken care of when the system was recreated
    }

    if (update)
      UpdateParticleSystem();
    
  }
 
}

void PartEdit::FinishFrame ()
{
  g3d->FinishDraw ();
  g3d->Print (0);
}

bool PartEdit::HandleEvent (iEvent& ev)
{
  if ((ev.Type == csevKeyboard) && 
    (csKeyEventHelper::GetEventType (&ev) == csKeyEventTypeDown) &&
    (csKeyEventHelper::GetCookedCode (&ev) == CSKEY_ESC))
  {
    csRef<iEventQueue> q (CS_QUERY_REGISTRY (object_reg, iEventQueue));
    if (q)
      q->GetEventOutlet()->Broadcast (cscmdQuit);
    return true;
  }
  return aws->HandleEvent(ev);
}

bool PartEdit::EventHandler (iEvent& ev)
{
  if (!(System->running))
      return false;

  if (ev.Type == csevBroadcast && ev.Command.Code == cscmdProcess)
  {
    System->SetupFrame ();
    return true;
  }
  else if (ev.Type == csevBroadcast && ev.Command.Code == cscmdFinalProcess)
  {
    System->FinishFrame ();
    return true;
  }
  else if (ev.Type == csevBroadcast && ev.Command.Code == cscmdSystemClose)
  {
      // System window closed, app shutting down
      System->running=false;
      return true;
  }
  else
  {
    return System ? System->HandleEvent (ev) : false;
  }
}


csRef<iEmitGen3D> PartEdit::CreateGen3D(Emitter3DState *emitter_state)
{
  // We start out with no clear emitter to use
  Emitters use_emitter=EMITTER_NONE;

  /*  Create an iEmit* for each type.  We may only use one of these, or we may
   *  use a number of them (in mix mode).  To simplify things, we'll create them all
   *  and discard the ones we don't use.
   */
  csRef<iEmitFixed> e_pptr=csPtr<iEmitFixed> (EmitFactoryState->CreateFixed());
  csRef<iEmitLine> e_lptr=csPtr<iEmitLine> (EmitFactoryState->CreateLine());
  csRef<iEmitBox> e_bptr=csPtr<iEmitBox> (EmitFactoryState->CreateBox());
  csRef<iEmitCylinder> e_cyptr=csPtr<iEmitCylinder> (EmitFactoryState->CreateCylinder());
  csRef<iEmitCone> e_coptr=csPtr<iEmitCone> (EmitFactoryState->CreateCone());
  csRef<iEmitSphere> e_sptr=csPtr<iEmitSphere> (EmitFactoryState->CreateSphere());
  csRef<iEmitSphereTangent> e_stptr=csPtr<iEmitSphereTangent> (EmitFactoryState->CreateSphereTangent());
  csRef<iEmitCylinderTangent> e_cytptr=csPtr<iEmitCylinderTangent> (EmitFactoryState->CreateCylinderTangent());



  // Determine which emitter we'll be using
  if (emitter_state->fixed_weight>0) { use_emitter= (use_emitter==EMITTER_NONE ? EMITTER_POINT : EMITTER_MIX); }
  if (emitter_state->line_weight>0) { use_emitter= (use_emitter==EMITTER_NONE ? EMITTER_LINE : EMITTER_MIX); }
  if (emitter_state->box_weight>0) { use_emitter= (use_emitter==EMITTER_NONE ? EMITTER_BOX : EMITTER_MIX); }
  if (emitter_state->sphere_weight>0) { use_emitter= (use_emitter==EMITTER_NONE ? EMITTER_SPHERE : EMITTER_MIX); }
  if (emitter_state->cylinder_weight>0) { use_emitter= (use_emitter==EMITTER_NONE ? EMITTER_CYLINDER : EMITTER_MIX); }
  if (emitter_state->cone_weight>0) { use_emitter= (use_emitter==EMITTER_NONE ? EMITTER_CONE : EMITTER_MIX); }
  if (emitter_state->spheretangent_weight>0) { use_emitter= (use_emitter==EMITTER_NONE ? EMITTER_SPHERETANGENT : EMITTER_MIX); }
  if (emitter_state->cylindertangent_weight>0) { use_emitter= (use_emitter==EMITTER_NONE ? EMITTER_CYLINDERTANGENT : EMITTER_MIX); }



  // Initialize all the emitter values for all emitters
  e_pptr->SetValue(emitter_state->fixed_position);
  e_lptr->SetContent(emitter_state->line_start,emitter_state->line_end);
  e_bptr->SetContent(emitter_state->box_min,emitter_state->box_max);
  e_cyptr->SetContent(emitter_state->cylinder_start,emitter_state->cylinder_end,
                    emitter_state->cylinder_min,emitter_state->cylinder_max);
  e_coptr->SetContent(emitter_state->cone_origin,emitter_state->cone_elevation,
                    emitter_state->cone_azimuth,emitter_state->cone_aperture,
                    emitter_state->cone_min,emitter_state->cone_max);
  e_sptr->SetContent(emitter_state->sphere_center,emitter_state->sphere_min,emitter_state->sphere_max);
  e_stptr->SetContent(emitter_state->spheretangent_center,emitter_state->spheretangent_min,emitter_state->spheretangent_max);
  e_cytptr->SetContent(emitter_state->cylindertangent_start,emitter_state->cylindertangent_end,
                    emitter_state->cylindertangent_min,emitter_state->cylindertangent_max);


  // Return the appropriate emitter, or construct a mix emitter if needed.
  switch (use_emitter)
  {
    case EMITTER_LINE:
	 return e_lptr;
     break;
    case EMITTER_BOX:
	 return e_bptr;
     break;
    case EMITTER_CYLINDER:
	 return e_cyptr;
     break;
    case EMITTER_CONE:
     return e_coptr;
     break;
    case EMITTER_SPHERE:
     return e_sptr;
     break;
    case EMITTER_SPHERETANGENT:
     return e_stptr;
     break;
    case EMITTER_CYLINDERTANGENT:
     return e_cytptr;
     break;
    case EMITTER_MIX:
      {
        csRef<iEmitMix> e_mptr=csPtr<iEmitMix> (EmitFactoryState->CreateMix());
        if (emitter_state->fixed_weight>0)
          e_mptr->AddEmitter(emitter_state->fixed_weight,e_pptr);
        if (emitter_state->line_weight>0)
          e_mptr->AddEmitter(emitter_state->line_weight,e_lptr);
        if (emitter_state->box_weight>0)
          e_mptr->AddEmitter(emitter_state->box_weight,e_bptr);
        if (emitter_state->cylinder_weight>0)
          e_mptr->AddEmitter(emitter_state->cylinder_weight,e_cyptr);
        if (emitter_state->cone_weight>0)
          e_mptr->AddEmitter(emitter_state->cone_weight,e_coptr);
        if (emitter_state->sphere_weight>0)
          e_mptr->AddEmitter(emitter_state->sphere_weight,e_sptr);
        if (emitter_state->spheretangent_weight>0)
          e_mptr->AddEmitter(emitter_state->spheretangent_weight,e_stptr);
        if (emitter_state->cylindertangent_weight>0)
          e_mptr->AddEmitter(emitter_state->cylindertangent_weight,e_cytptr);
        return e_mptr;
      }
     break;
    case EMITTER_POINT:
    default:
     return e_pptr;
     break;
  }

  // Unreachable
  return e_pptr;
}


bool PartEdit::RecreateParticleSystem()
{
  
  printf("Attempting to load texture file '%s'\n",current_graphic.GetData());

  // Unload old texture here?
 
  if (!loader->LoadTexture(current_graphic.GetData(), current_graphic.GetData()))
    return false;

  iMaterialWrapper *mat_wrap=engine->GetMaterialList()->FindByName(current_graphic);
  if (mat_wrap == 0)
  {
    printf("Warning!  Could not find material named '%s' after load.\n",current_graphic.GetData());
    return false;
  }

  if (mw != 0 && mw->QueryObject() != 0)
    engine->RemoveObject(mw->QueryObject());

  csRef<iMeshObject> mesh = EmitObjectFactory->NewInstance();  
  mesh->SetMaterialWrapper(mat_wrap);

  emitState = SCF_QUERY_INTERFACE (mesh, iEmitState);

  UpdateParticleSystem();

  mw = engine->CreateMeshWrapper(mesh, "emit", room,csVector3 (0, 5, 0));


  if (state_emitter.alpha_blend)
  {
    mw->SetRenderPriority(4);
    mw->SetZBufMode(CS_ZBUF_TEST);
  }
  else
  {
    mw->SetZBufMode(CS_ZBUF_USE);
  }

  engine->Prepare ();

  return true;
}


bool PartEdit::UpdateParticleSystem()
{
	

  csRef<iEmitGen3D> startpos,startspeed,startaccel,attractor;

  // Create emitters for each property
  startpos=CreateGen3D(&state_initial_position);
  startspeed=CreateGen3D(&state_initial_speed);
  startaccel=CreateGen3D(&state_initial_acceleration);

  if (state_attractor.force>0.00001f || state_attractor.force<-0.00001f)
	  attractor=CreateGen3D(&(state_attractor.e3d_state));


  emitState->SetParticleCount(state_emitter.particle_count);
  emitState->SetLighting (state_emitter.lighting);
  if (state_emitter.rectangular_particles)
    emitState->SetRectParticles(state_emitter.rect_w,state_emitter.rect_h);
  else
    emitState->SetRegularParticles(state_emitter.reg_number,state_emitter.reg_radius);

  emitState->SetParticleTime(state_emitter.particle_max_age);
  csVector3 min(state_emitter.bbox_minx,state_emitter.bbox_miny,state_emitter.bbox_minz);
  csVector3 max(state_emitter.bbox_maxx,state_emitter.bbox_maxy,state_emitter.bbox_maxz);
  emitState->SetContainerBox(state_emitter.using_bounding_box,min,max);




  emitState->SetStartPosEmit(startpos);
  emitState->SetStartSpeedEmit(startspeed);
  emitState->SetStartAccelEmit(startaccel);
  if (attractor != 0)
  {
    emitState->SetAttractorEmit(attractor);
    emitState->SetAttractorForce(state_attractor.force);
  }

  return true;
}

bool PartEdit::Initialize ()
{
  if (!csInitializer::RequestPlugins (object_reg,
  	CS_REQUEST_VFS,
	CS_REQUEST_OPENGL3D,
	CS_REQUEST_ENGINE,
	CS_REQUEST_FONTSERVER,
	CS_REQUEST_IMAGELOADER,
	CS_REQUEST_LEVELLOADER,
	CS_REQUEST_REPORTER,
	CS_REQUEST_REPORTERLISTENER,	
	CS_REQUEST_PLUGIN("crystalspace.window.alternatemanager", iAws),
	CS_REQUEST_END))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.partedit",
	"Can't initialize plugins!");
    return false;
  }

  if (!csInitializer::SetupEventHandler (object_reg, EventHandler))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.partedit",
	"Can't initialize event handler!");
    return false;
  }

  // Check for commandline help.
  if (csCommandLineHelper::CheckHelp (object_reg))
  {
    csCommandLineHelper::Help (object_reg);
    return false;
  }

  // The virtual clock.
  vc = CS_QUERY_REGISTRY (object_reg, iVirtualClock);
  if (vc == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.partedit",
	"Can't find the virtual clock!");
    return false;
  }

  // Find the pointer to engine plugin
  engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  if (engine == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.partedit",
	"No iEngine plugin!");
    return false;
  }

  vfs = CS_QUERY_REGISTRY (object_reg, iVFS);
  if (vfs == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.partedit",
    	"No iVFS plugin!");
    return false;
  }


  loader = CS_QUERY_REGISTRY (object_reg, iLoader);
  if (loader == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.partedit",
    	"No iLoader plugin!");
    return false;
  }

  g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  if (g3d == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.partedit",
    	"No iGraphics3D plugin!");
    return false;
  }
  g2d = CS_QUERY_REGISTRY (object_reg, iGraphics2D);
  if (g2d == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.partedit",
    	"No iGraphics2D plugin!");
    return false;
  }

  kbd = CS_QUERY_REGISTRY (object_reg, iKeyboardDriver);
  if (kbd == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.partedit",
    	"No iKeyboardDriver plugin!");
    return false;
  }

  aws = CS_QUERY_REGISTRY (object_reg, iAws);
  if (aws == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.partedit",
    	"No iAws plugin!");
    return false;
  }

  g3d->SetDimensions(1024,768);
  g2d->Resize(1024,768);

  iNativeWindow* nw = g2d->GetNativeWindow ();
  if (nw) nw->SetTitle ("Particle System Editor");
  

  // Open the main system. This will open all the previously loaded plug-ins.
  if (!csInitializer::OpenApplication (object_reg))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.partedit",
    	"Error opening system!");
    return false;
  }  

  PluginManager = CS_QUERY_REGISTRY(object_reg, iPluginManager);
  if (!PluginManager)
    return "No iPluginManager plugin!";



  // First disable the lighting cache. Our app is PartEdit enough
  // not to need this.
  engine->SetLightingCacheMode (0);

  if (!loader->LoadTexture ("stone", "/lib/std/stone4.gif"))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.partedit",
    	"Error loading 'stone4' texture!");
    return false;
  }

  iMaterialWrapper* tm = engine->GetMaterialList ()->FindByName ("stone");

  room = engine->CreateSector ("room");
  csRef<iMeshWrapper> walls (engine->CreateSectorWallsMesh (room, "walls"));
  csRef<iThingState> ws = SCF_QUERY_INTERFACE (walls->GetMeshObject (), iThingState);
  csRef<iThingFactoryState> walls_state = ws->GetFactory ();
  walls_state->AddInsideBox (csVector3 (-5, 0, -5), csVector3 (5, 20, 5));
  walls_state->SetPolygonMaterial (CS_POLYRANGE_LAST, tm);
  walls_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST, 3);

  csRef<iStatLight> light;
  iLightList* ll = room->GetLights ();

  light = engine->CreateLight (0, csVector3 (-3, 5, 0), 10,
  	csColor (1, 0, 0), false);
  ll->Add (light->QueryLight ());

  light = engine->CreateLight (0, csVector3 (3, 5,  0), 10,
  	csColor (0, 0, 1), false);
  ll->Add (light->QueryLight ());

  light = engine->CreateLight (0, csVector3 (0, 5, -3), 10,
  	csColor (0, 1, 0), false);
  ll->Add (light->QueryLight ());

  // Get the emit mesh object plug-in.
  csRef<iMeshObjectType> emit_type (CS_LOAD_PLUGIN(PluginManager,
  	"crystalspace.mesh.object.emit", iMeshObjectType));  

   // Emit
  EmitObjectFactory = emit_type->NewFactory();
  EmitFactoryState = SCF_QUERY_INTERFACE(EmitObjectFactory, iEmitFactoryState);

  state_emitter.particle_count=25;
  state_emitter.lighting=false;
  state_emitter.using_bounding_box=false;
  state_emitter.bbox_maxx=state_emitter.bbox_maxy=state_emitter.bbox_maxz=0.0f;
  state_emitter.bbox_minx=state_emitter.bbox_miny=state_emitter.bbox_minz=0.0f;
  state_emitter.rect_w=0.05f;
  state_emitter.rect_h=0.05f;
  state_emitter.rectangular_particles=true;
  state_emitter.reg_number=1;
  state_emitter.reg_radius=1.0f;
  state_emitter.particle_max_age=2000;
  state_emitter.alpha_blend=true;

  memset(&state_initial_position,0,sizeof(state_initial_position));
  memset(&state_initial_speed,0,sizeof(state_initial_speed));
  state_initial_speed.fixed_position.y=0.5;
  memset(&state_initial_acceleration,0,sizeof(state_initial_acceleration));
  memset(&state_attractor,0,sizeof(state_attractor));

  current_graphic="/lib/std/cslogo2.png";
  RecreateParticleSystem();


  view = csPtr<iView> (new csView (engine, g3d));
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 5, -3));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  // awsCanvas = csPtr<iAwsCanvas> (aws->CreateCustomCanvas (g2d, g3d));
  aws->SetFlag (AWSF_AlwaysRedrawWindows);
  aws->SetupCanvas (NULL,g2d,g3d);

  // Setup sink.  
  s  = new awsSink();
  iAwsSink    *sink =aws->GetSinkMgr()->CreateSink(s);

  s->SetSink(sink);
  s->SetWindowManager(aws);
  s->SetVFS(vfs);



  aws->GetSinkMgr()->RegisterSink("parteditSink", sink);


  


  // now load preferences
  if (!aws->GetPrefMgr()->Load("/this/data/temp/partedit.def"))
    csReport(object_reg,CS_REPORTER_SEVERITY_ERROR,
	       "crystalspace.application.partedit",
	       "couldn't load skin definition file!");

  aws->GetPrefMgr()->SelectDefaultSkin("Normal Windows");



  iAwsWindow *iawswindow_SectionSelection = aws->CreateWindowFrom("SectionSelection");
  if (iawswindow_SectionSelection) iawswindow_SectionSelection->Show();

  // Dont show these by default
  iAwsWindow *iawswindow_GraphicSelection = aws->CreateWindowFrom("GraphicSelection");
  if (iawswindow_GraphicSelection) iawswindow_GraphicSelection->Hide();
  iAwsWindow *iawswindow_EmitterState = aws->CreateWindowFrom("EmitterState");
  if (iawswindow_EmitterState) iawswindow_EmitterState->Hide();
  iAwsWindow *iawswindow_InitialPosition = aws->CreateWindowFrom("InitialPosition");
  if (iawswindow_InitialPosition) iawswindow_InitialPosition->Hide();
  iAwsWindow *iawswindow_InitialSpeed = aws->CreateWindowFrom("InitialSpeed");
  if (iawswindow_InitialSpeed) iawswindow_InitialSpeed->Hide();
  iAwsWindow *iawswindow_InitialAcceleration = aws->CreateWindowFrom("InitialAcceleration");
  if (iawswindow_InitialAcceleration) iawswindow_InitialAcceleration->Hide();
  iAwsWindow *iawswindow_Attractor = aws->CreateWindowFrom("Attractor");
  if (iawswindow_Attractor) iawswindow_Attractor->Hide();



  s->SetGraphicCWD("/lib/std/");
  s->SetGraphicFilter("");
  s->SetGraphicFile("cslogo2.png");
  s->SetEmitterState(&state_emitter);
  s->SetInitialPositionState(&state_initial_position);
  s->SetInitialSpeedState(&state_initial_speed);
  s->SetInitialAccelerationState(&state_initial_acceleration);
  s->SetAttractorState(&state_attractor);


  return true;
}

void PartEdit::Start ()
{
  running=true;
  csDefaultRunLoop (object_reg);
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  iObjectRegistry* object_reg = csInitializer::CreateEnvironment (argc, argv);

  System = new PartEdit (object_reg);
  if (System->Initialize ())
    System->Start ();
  delete System;

  csInitializer::DestroyApplication (object_reg);
  return 0;
}


