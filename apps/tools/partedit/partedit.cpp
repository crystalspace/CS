/*
    Copyright (C) 2003 by Andrew Mann

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
#include "csutil/sysfunc.h"
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
#include "iengine/texture.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "iengine/material.h"
#include "imesh/thing.h"
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
#include "csqsqrt.h"
#include "csutil/event.h"

#include "imesh/emit.h"
#include "iutil/plugin.h"

#include "awssink.h"
#include "partedit.h"

CS_IMPLEMENT_APPLICATION

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
      // Sanity checking to avoid some crashes
      if (estate->particle_count<0)
      {
        estate->particle_count=0;
        s->UpdateEmitterStateDisplay();
      }
      if (estate->reg_number<0)
      {
        estate->reg_number=0;
        s->UpdateEmitterStateDisplay();
      }
      if (estate->particle_max_age<1)
      {
        estate->particle_max_age=1;
        s->UpdateEmitterStateDisplay();
      }
      memcpy(&state_emitter_new,estate,sizeof(EmitterState));
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

    FieldState *fieldstate;
    if (s->FieldSpeedStateChanged())
    {
      fieldstate=s->GetFieldSpeedState();
      memcpy(&state_field_speed,fieldstate,sizeof(FieldState));
      s->ClearFieldSpeedStateChanged();
      update=true;
    }
    if (s->FieldAccelStateChanged())
    {
      fieldstate=s->GetFieldAccelState();
      memcpy(&state_field_accel,fieldstate,sizeof(FieldState));
      s->ClearFieldAccelStateChanged();
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
   
    if (s->LoadSaveStateChanged())
    {
      SaveEmitterToFile();
      s->ClearLoadSaveStateChanged();
    }
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
  else if (ev.Type == csevBroadcast && ev.Command.Code == cscmdSystemClose)
  {
      // System window closed, app shutting down
      return true;
  }
  return aws->HandleEvent(ev);
}

bool PartEdit::EventHandler (iEvent& ev)
{
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
  else
  {
    return System ? System->HandleEvent (ev) : false;
  }
}

bool PartEdit::InitEmitterList(EmitterList *elist)
{
  elist->point=EmitFactoryState->CreateFixed();
  elist->line=EmitFactoryState->CreateLine();
  elist->box=EmitFactoryState->CreateBox();
  elist->cylinder=EmitFactoryState->CreateCylinder();
  elist->cone=EmitFactoryState->CreateCone();
  elist->sphere=EmitFactoryState->CreateSphere();
  elist->spheretangent=EmitFactoryState->CreateSphereTangent();
  elist->cylindertangent=EmitFactoryState->CreateCylinderTangent();
  elist->mix=EmitFactoryState->CreateMix();
  elist->current_type=EMITTER_NONE;
  elist->current=NULL;
  elist->point_used=false;
  elist->line_used=false;
  elist->box_used=false;
  elist->cylinder_used=false;
  elist->cone_used=false;
  elist->sphere_used=false;
  elist->spheretangent_used=false;
  elist->cylindertangent_used=false;

  return true;
}

bool PartEdit::ClearGen3D(EmitterList *elist)
{
  bool had_old_value=false;
  
  if (elist->current_type!=EMITTER_NONE)
    had_old_value=true;

  elist->current_type=EMITTER_NONE;
  elist->current=NULL;
  elist->point_used=false;
  elist->line_used=false;
  elist->box_used=false;
  elist->cylinder_used=false;
  elist->cone_used=false;
  elist->sphere_used=false;
  elist->spheretangent_used=false;
  elist->cylindertangent_used=false;

  return had_old_value;
}

bool PartEdit::UpdateGen3D(EmitterList *elist,Emitter3DState *emitter_state)
{
  bool different_emitter=false;
  // We start out with no clear emitter to use
  Emitters use_emitter=EMITTER_NONE;

  // Determine which emitter we'll be using
  if (emitter_state->fixed_weight>0) { use_emitter= (use_emitter==EMITTER_NONE ? EMITTER_POINT : EMITTER_MIX); }
  if (emitter_state->line_weight>0) { use_emitter= (use_emitter==EMITTER_NONE ? EMITTER_LINE : EMITTER_MIX); }
  if (emitter_state->box_weight>0) { use_emitter= (use_emitter==EMITTER_NONE ? EMITTER_BOX : EMITTER_MIX); }
  if (emitter_state->sphere_weight>0) { use_emitter= (use_emitter==EMITTER_NONE ? EMITTER_SPHERE : EMITTER_MIX); }
  if (emitter_state->cylinder_weight>0) { use_emitter= (use_emitter==EMITTER_NONE ? EMITTER_CYLINDER : EMITTER_MIX); }
  if (emitter_state->cone_weight>0) { use_emitter= (use_emitter==EMITTER_NONE ? EMITTER_CONE : EMITTER_MIX); }
  if (emitter_state->spheretangent_weight>0) { use_emitter= (use_emitter==EMITTER_NONE ? EMITTER_SPHERETANGENT : EMITTER_MIX); }
  if (emitter_state->cylindertangent_weight>0) { use_emitter= (use_emitter==EMITTER_NONE ? EMITTER_CYLINDERTANGENT : EMITTER_MIX); }

  // No emitter means use point emitter
  if (use_emitter==EMITTER_NONE)
    use_emitter=EMITTER_POINT;

  // If this is different from the current emitter, we'll need to change
  if (use_emitter!=elist->current_type)
    different_emitter=true;

  // Update the values of all the emitters
  elist->point->SetValue(emitter_state->fixed_position);
  elist->line->SetContent(emitter_state->line_start,emitter_state->line_end);
  elist->box->SetContent(emitter_state->box_min,emitter_state->box_max);
  elist->cylinder->SetContent(emitter_state->cylinder_start,emitter_state->cylinder_end,
                    emitter_state->cylinder_min,emitter_state->cylinder_max);
  elist->cone->SetContent(emitter_state->cone_origin,emitter_state->cone_elevation,
                    emitter_state->cone_azimuth,emitter_state->cone_aperture,
                    emitter_state->cone_min,emitter_state->cone_max);
  elist->sphere->SetContent(emitter_state->sphere_center,emitter_state->sphere_min,emitter_state->sphere_max);
  elist->spheretangent->SetContent(emitter_state->spheretangent_center,emitter_state->spheretangent_min,emitter_state->spheretangent_max);
  elist->cylindertangent->SetContent(emitter_state->cylindertangent_start,emitter_state->cylindertangent_end,
                    emitter_state->cylindertangent_min,emitter_state->cylindertangent_max);

  if (!different_emitter)
  {
    // One last way that we may have to use a different emitter - if we need to create a new mix.
    if (elist->current_type==EMITTER_MIX)
    {
      // We need to know if any were removed first
      bool removed=false;

      removed = removed || (emitter_state->fixed_weight==0.0f && elist->point_used);
      removed = removed || (emitter_state->line_weight==0.0f && elist->line_used);
      removed = removed || (emitter_state->box_weight==0.0f && elist->box_used);
      removed = removed || (emitter_state->cylinder_weight==0.0f && elist->cylinder_used);
      removed = removed || (emitter_state->cone_weight==0.0f && elist->cone_used);
      removed = removed || (emitter_state->sphere_weight==0.0f && elist->sphere_used);
      removed = removed || (emitter_state->spheretangent_weight==0.0f && elist->spheretangent_used);
      removed = removed || (emitter_state->cylindertangent_weight==0.0f && elist->cylindertangent_used);

      if (removed)
        different_emitter=true;
    }
  }

  // Now, if we're using a different emitter, we need to reset some things
  if (different_emitter)
  {
    // Clear out all the emitters-used-in-mix options
    elist->point_used=false;
    elist->line_used=false;
    elist->box_used=false;
    elist->cylinder_used=false;
    elist->cone_used=false;
    elist->sphere_used=false;
    elist->spheretangent_used=false;
    elist->cylindertangent_used=false;

    // Update the new type
    elist->current_type=use_emitter;

    switch (use_emitter)
    {
    case EMITTER_LINE:
      elist->current=(iEmitGen3D*)elist->line;
     break;
    case EMITTER_BOX:
      elist->current=(iEmitGen3D*)elist->box;
     break;
    case EMITTER_CYLINDER:
      elist->current=(iEmitGen3D*)elist->cylinder;
     break;
    case EMITTER_CONE:
      elist->current=(iEmitGen3D*)elist->cone;
     break;
    case EMITTER_SPHERE:
      elist->current=(iEmitGen3D*)elist->sphere;
     break;
    case EMITTER_SPHERETANGENT:
      elist->current=(iEmitGen3D*)elist->spheretangent;
     break;
    case EMITTER_CYLINDERTANGENT:
      elist->current=(iEmitGen3D*)elist->cylindertangent;
     break;
    case EMITTER_MIX:
      {
        // Clear out the mix emitter.  Since we have to reset the emitter here anyway
        // clearing saves us from having to check if any existing mix members were removed
        elist->mix=EmitFactoryState->CreateMix();

        elist->current=(iEmitGen3D*)elist->mix;
      }
     break;
    case EMITTER_POINT:
    default:
      elist->current=(iEmitGen3D*)elist->point;
     break;
    }
  }

  // Finally, if we're using a mix emitter, we either have a fresh emitter
  // Or no removed entries.  Handle any mix entries that need to be added
  if (use_emitter==EMITTER_MIX)
  {
    if (emitter_state->fixed_weight>0 && !(elist->point_used))
    {
      elist->mix->AddEmitter(emitter_state->fixed_weight,elist->point);
      elist->point_used=true;
    }
    if (emitter_state->line_weight>0 && !(elist->line_used))
    {
      elist->mix->AddEmitter(emitter_state->line_weight,elist->line);
      elist->line_used=true;
    }
    if (emitter_state->box_weight>0 && !(elist->box_used))
    {
      elist->mix->AddEmitter(emitter_state->box_weight,elist->box);
      elist->box_used=true;
    }
    if (emitter_state->cylinder_weight>0 && !(elist->cylinder_used))
    {
      elist->mix->AddEmitter(emitter_state->cylinder_weight,elist->cylinder);
      elist->cylinder_used=true;
    }
    if (emitter_state->cone_weight>0 && !(elist->cone_used))
    {
      elist->mix->AddEmitter(emitter_state->cone_weight,elist->cone);
      elist->cone_used=true;
    }
    if (emitter_state->sphere_weight>0 && !(elist->sphere_used))
    {
      elist->mix->AddEmitter(emitter_state->sphere_weight,elist->sphere);
      elist->sphere_used=true;
    }
    if (emitter_state->spheretangent_weight>0 && !(elist->spheretangent_used))
    {
      elist->mix->AddEmitter(emitter_state->spheretangent_weight,elist->spheretangent);
      elist->spheretangent_used=true;
    }
    if (emitter_state->cylindertangent_weight>0 && !(elist->cylindertangent_used))
    {
      elist->mix->AddEmitter(emitter_state->cylindertangent_weight,elist->cylindertangent);
      elist->cylindertangent_used=true;
    }
  }

  // And we're done
  return different_emitter;
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

  /* Clear out the current emitter settings since we're going to ditch them all
   *  and start fresh.  Don't worry, the state_* structures are still intact
   *  and we will use them to build new emitters afterwards.
   */
  InitEmitterList(&startpos);
  InitEmitterList(&startspeed);
  InitEmitterList(&startaccel);
  InitEmitterList(&fieldspeed);
  InitEmitterList(&fieldaccel);
  InitEmitterList(&attractor);

  // Force emitter setup.  Even if some parameters haven't changed, set them all.
  force_emitter_setup=true;
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

void PartEdit::SaveEmitter3DStateRecursive(Emitter3DState *emitter_state,Emitters use_emitter)
{
    switch (use_emitter)
    {
    case EMITTER_LINE:
      printf("<emitline>\n");
      printf("    <min x=\"%f\" y=\"%f\" z=\"%f\">\n",emitter_state->line_start.x,emitter_state->line_start.y,emitter_state->line_start.z);
      printf("    <max x=\"%f\" y=\"%f\" z=\"%f\">\n",emitter_state->line_end.x,emitter_state->line_end.y,emitter_state->line_end.z);
      printf("</emitline>\n");
      return;
    case EMITTER_BOX:
      printf("<emitbox>\n");
      printf("    <min x=\"%f\" y=\"%f\" z=\"%f\">\n",emitter_state->box_min.x,emitter_state->box_min.y,emitter_state->box_min.z);
      printf("    <max x=\"%f\" y=\"%f\" z=\"%f\">\n",emitter_state->box_max.x,emitter_state->box_max.y,emitter_state->box_max.z);
      printf("</emitbox>\n");
      return;
    case EMITTER_CYLINDER:
      printf("<emitcylinder p=\"%f\" q=\"%f\">\n",emitter_state->cylinder_min,emitter_state->cylinder_max);
      printf("    <min x=\"%f\" y=\"%f\" z=\"%f\">\n",emitter_state->cylinder_start.x,emitter_state->cylinder_start.y,emitter_state->cylinder_start.z);
      printf("    <max x=\"%f\" y=\"%f\" z=\"%f\">\n",emitter_state->cylinder_end.x,emitter_state->cylinder_end.y,emitter_state->cylinder_end.z);
      printf("</emitcylinder>\n");
      return;
    case EMITTER_CONE:
      printf("<emitcone x=\"%f\" y=\"%f\" z=\"%f\" p=\"%f\" q=\"%f\" r=\"%f\" s=\"%f\" t=\"%f\"/>\n",
        emitter_state->cone_origin.x,emitter_state->cone_origin.y,emitter_state->cone_origin.z,
        emitter_state->cone_elevation,emitter_state->cone_azimuth,emitter_state->cone_aperture,
        emitter_state->cone_min,emitter_state->cone_max);
      return;
    case EMITTER_SPHERE:
      printf("<emitsphere x=\"%f\" y=\"%f\" z=\"%f\" p=\"%f\" q=\"%f\"/>\n",
        emitter_state->sphere_center.x,emitter_state->sphere_center.y,emitter_state->sphere_center.z,
        emitter_state->sphere_min,emitter_state->sphere_max);
      return;
    case EMITTER_SPHERETANGENT:
      printf("<emitspheretangent x=\"%f\" y=\"%f\" z=\"%f\" p=\"%f\" q=\"%f\"/>\n",
        emitter_state->spheretangent_center.x,emitter_state->spheretangent_center.y,emitter_state->spheretangent_center.z,
        emitter_state->spheretangent_min,emitter_state->spheretangent_max);
      return;
    case EMITTER_CYLINDERTANGENT:
      printf("<emitcylindertangent p=\"%f\" q=\"%f\">\n",emitter_state->cylindertangent_min,emitter_state->cylindertangent_max);
      printf("    <min x=\"%f\" y=\"%f\" z=\"%f\">\n",emitter_state->cylindertangent_start.x,emitter_state->cylindertangent_start.y,emitter_state->cylindertangent_start.z);
      printf("    <max x=\"%f\" y=\"%f\" z=\"%f\">\n",emitter_state->cylindertangent_end.x,emitter_state->cylindertangent_end.y,emitter_state->cylindertangent_end.z);
      printf("</emitcylindertangent>\n");
      return;
    case EMITTER_MIX:
      printf("<emitmix>\n");
      if (emitter_state->fixed_weight > 0.0f)
      {
        printf("    <weight>%f</weight>\n",emitter_state->fixed_weight);
        SaveEmitter3DStateRecursive(emitter_state,EMITTER_POINT);
      }
      if (emitter_state->line_weight > 0.0f)
      {
        printf("    <weight>%f</weight>\n",emitter_state->line_weight);
        SaveEmitter3DStateRecursive(emitter_state,EMITTER_LINE);
      }
      if (emitter_state->box_weight > 0.0f)
      {
        printf("    <weight>%f</weight>\n",emitter_state->box_weight);
        SaveEmitter3DStateRecursive(emitter_state,EMITTER_BOX);
      }
      if (emitter_state->cylinder_weight > 0.0f)
      {
        printf("    <weight>%f</weight>\n",emitter_state->cylinder_weight);
        SaveEmitter3DStateRecursive(emitter_state,EMITTER_CYLINDER);
      }
      if (emitter_state->cone_weight > 0.0f)
      {
        printf("    <weight>%f</weight>\n",emitter_state->cone_weight);
        SaveEmitter3DStateRecursive(emitter_state,EMITTER_CONE);
      }
      if (emitter_state->sphere_weight > 0.0f)
      {
        printf("    <weight>%f</weight>\n",emitter_state->sphere_weight);
        SaveEmitter3DStateRecursive(emitter_state,EMITTER_SPHERE);
      }
      if (emitter_state->spheretangent_weight > 0.0f)
      {
        printf("    <weight>%f</weight>\n",emitter_state->spheretangent_weight);
        SaveEmitter3DStateRecursive(emitter_state,EMITTER_SPHERETANGENT);
      }
      if (emitter_state->cylindertangent_weight > 0.0f)
      {
        printf("    <weight>%f</weight>\n",emitter_state->cylindertangent_weight);
        SaveEmitter3DStateRecursive(emitter_state,EMITTER_CYLINDERTANGENT);
      }
      printf("</emitmix>\n");
      return;
    case EMITTER_POINT:
    default:
      printf("<emitfixed x=\"%f\" y=\"%f\" z=\"%f\"/>\n",
        emitter_state->fixed_position.x,emitter_state->fixed_position.y,emitter_state->fixed_position.z);
      return;
    }
}

void PartEdit::SaveEmitter3DStateToFile(Emitter3DState *emitter_state)
{
  // We start out with no clear emitter to use
  Emitters use_emitter=EMITTER_NONE;

  // Determine which emitter we'll be using
  if (emitter_state->fixed_weight>0) { use_emitter= (use_emitter==EMITTER_NONE ? EMITTER_POINT : EMITTER_MIX); }
  if (emitter_state->line_weight>0) { use_emitter= (use_emitter==EMITTER_NONE ? EMITTER_LINE : EMITTER_MIX); }
  if (emitter_state->box_weight>0) { use_emitter= (use_emitter==EMITTER_NONE ? EMITTER_BOX : EMITTER_MIX); }
  if (emitter_state->sphere_weight>0) { use_emitter= (use_emitter==EMITTER_NONE ? EMITTER_SPHERE : EMITTER_MIX); }
  if (emitter_state->cylinder_weight>0) { use_emitter= (use_emitter==EMITTER_NONE ? EMITTER_CYLINDER : EMITTER_MIX); }
  if (emitter_state->cone_weight>0) { use_emitter= (use_emitter==EMITTER_NONE ? EMITTER_CONE : EMITTER_MIX); }
  if (emitter_state->spheretangent_weight>0) { use_emitter= (use_emitter==EMITTER_NONE ? EMITTER_SPHERETANGENT : EMITTER_MIX); }
  if (emitter_state->cylindertangent_weight>0) { use_emitter= (use_emitter==EMITTER_NONE ? EMITTER_CYLINDERTANGENT : EMITTER_MIX); }

  // No emitter means use point emitter
  if (use_emitter==EMITTER_NONE)
    use_emitter=EMITTER_POINT;

  SaveEmitter3DStateRecursive(emitter_state,use_emitter);
}

bool PartEdit::SaveEmitterToFile()
{
  printf("<!--PartEdit Emitter save BEGINS here-->\n\n");

  printf("<!--Paste the entries from this section into the plugins section in your map file-->\n");
  printf("<plugins>\n    <plugin name=\"emitFact\">crystalspace.mesh.loader.factory.emit</plugin>\n    <plugin name=\"emit\">crystalspace.mesh.loader.emit</plugin>\n</plugins>\n");

  // Dump texture def
  printf("<!--Paste the entries from this section into the textures section in your map file-->\n");
  printf("<textures>\n");
  printf("    <texture name=\"%s\"><file>%s</file></texture>\n",current_graphic.GetData(),current_graphic.GetData());
  printf("</textures>\n\n");

  // Dump material def
  printf("<!--Paste the entries from this section into the materials section in your map file-->\n");
  printf("<materials>\n");
  printf("    <material name=\"%s\"><texture>%s</texture></material>\n",current_graphic.GetData(),current_graphic.GetData());
  printf("</materials>\n\n");

  printf("<!--Paste the entries from this section into your map file-->\n");
  printf("<meshfact name=\"emitFact\"><plugin>emitFact</plugin><params /></meshfact>\n\n");

  printf("<!--Paste the entries from this section beneath a sector in your map file-->\n");
  printf("<!--Be sure to adjust the x,y,z values in the move section to position the effect in your world-->\n");
  printf("<!--You may want to adjust the priority and mix mode.  PartEdit does not currently handle these.-->\n");
  printf("<meshobj name=\"partsysteminstance\">\n    <plugin>emit</plugin>\n    <znone />\n    <priority>alpha</priority>\n    <move>\n        <v x=\"0\" y=\"0\" z=\"0\" />\n    </move>\n");
  printf("    <params>\n        <factory>emitFact</factory>\n        <mixmode>\n            <add />\n        </mixmode>\n");
  printf("        <number>%d</number>\n",state_emitter.particle_count);
  printf("        <material>mymaterial</material>\n");
  if (state_emitter.rectangular_particles)
    printf("        <rectparticles w=\"%f\" h=\"%f\" />\n",state_emitter.rect_w,state_emitter.rect_h);
  else
    printf("        <regularparticles sides=\"%d\" radius=\"%f\">\n",state_emitter.reg_number,state_emitter.reg_radius);
  printf("        <lighting>%s</lighting>\n",state_emitter.lighting ? "on" : "off");
  printf("        <totaltime>%d</totaltime>\n",state_emitter.particle_max_age);
  if (state_emitter.using_bounding_box)
    printf("        <containerbox>\n            <min x=\"%f\" y=\"%f\" z=\"%f\">\n            <max x=\"%f\" y=\"%f\" z=\"%f\">\n        </containerbox>\n",
        state_emitter.bbox_minx,state_emitter.bbox_miny,state_emitter.bbox_minz,
        state_emitter.bbox_maxx,state_emitter.bbox_maxy,state_emitter.bbox_maxz);
  // startpos
  printf("<startpos>\n");
  SaveEmitter3DStateToFile(&state_initial_position);
  printf("</startpos>\n");
  // startspeed
  printf("<startspeed>\n");
  SaveEmitter3DStateToFile(&state_initial_speed);
  printf("</startspeed>\n");
  // startaccel
  printf("<startaccel>\n");
  SaveEmitter3DStateToFile(&state_initial_acceleration);
  printf("</startaccel>\n");
  // fieldspeed
  if (state_field_speed.active)
  {
    printf("<fieldspeed>\n");
    SaveEmitter3DStateToFile(&(state_field_speed.e3d_state));
    printf("</fieldspeed>\n");
  }
  // fieldaccel
  if (state_field_accel.active)
  {
    printf("<fieldaccel>\n");
    SaveEmitter3DStateToFile(&(state_field_accel.e3d_state));
    printf("</fieldaccel>\n");
  }
  // attractor
  if (state_attractor.force< -0.0001f || state_attractor.force>0.0001f)
  {
    printf("<attractorforce>%f</attractorforce>\n",state_attractor.force);
    printf("<attractor>\n");
    SaveEmitter3DStateToFile(&(state_attractor.e3d_state));
    printf("</attractor>\n");
  }

  // aging

  printf("    </params>\n");
  printf("</meshobj>\n");
  printf("<!--PartEdit Emitter save ENDS here-->\n\n");
  return true;
}

bool PartEdit::UpdateParticleSystem()
{

  // Update stats for each emitter.  If true is returned, we need to update the reference
  if (UpdateGen3D(&startpos,&state_initial_position))
    emitState->SetStartPosEmit(startpos.current);
  if (UpdateGen3D(&startspeed,&state_initial_speed))
    emitState->SetStartSpeedEmit(startspeed.current);
  if (UpdateGen3D(&startaccel,&state_initial_acceleration))
    emitState->SetStartAccelEmit(startaccel.current);

  if (state_attractor.force>0.00001f || state_attractor.force<-0.00001f)
  {
    if (UpdateGen3D(&attractor,&state_attractor.e3d_state))
      emitState->SetAttractorEmit(attractor.current);
    emitState->SetAttractorForce(state_attractor.force);
  }
  else
  {
    if (ClearGen3D(&attractor))
    {
      emitState->SetAttractorEmit(NULL);    
      emitState->SetAttractorForce(0.0f);
    }
  }

  if (state_field_speed.active)
  {
    if (UpdateGen3D(&fieldspeed,&(state_field_speed.e3d_state)))
      emitState->SetFieldSpeedEmit(fieldspeed.current);
  }
  else
  {
    if (ClearGen3D(&fieldspeed))
      emitState->SetFieldSpeedEmit(NULL);
  }

  if (state_field_accel.active)
  {
    if (UpdateGen3D(&fieldaccel,&(state_field_accel.e3d_state)))
      emitState->SetFieldAccelEmit(fieldaccel.current);
  }
  else
  {
    if (ClearGen3D(&fieldaccel))
      emitState->SetFieldAccelEmit(NULL);
  }

  if (force_emitter_setup || state_emitter_new.particle_count != state_emitter.particle_count)
    emitState->SetParticleCount(state_emitter_new.particle_count);
  if (force_emitter_setup || state_emitter_new.lighting != state_emitter.lighting)
    emitState->SetLighting (state_emitter_new.lighting);
  if (force_emitter_setup || state_emitter_new.rectangular_particles != state_emitter.rectangular_particles ||
    (state_emitter_new.rectangular_particles && (state_emitter_new.rect_h != state_emitter.rect_h ||
      state_emitter_new.rect_w != state_emitter.rect_w)) || 
      (!state_emitter_new.rectangular_particles && (state_emitter_new.reg_number != state_emitter.reg_number ||
      state_emitter_new.reg_radius != state_emitter.reg_radius)))
  {
    if (state_emitter_new.rectangular_particles)
      emitState->SetRectParticles(state_emitter_new.rect_w,state_emitter_new.rect_h);
    else
      emitState->SetRegularParticles(state_emitter_new.reg_number,state_emitter_new.reg_radius);
  }

  if (force_emitter_setup || state_emitter_new.particle_max_age != state_emitter.particle_max_age)
    emitState->SetParticleTime(state_emitter_new.particle_max_age);
  if (force_emitter_setup || state_emitter_new.using_bounding_box != state_emitter.using_bounding_box ||
    state_emitter_new.bbox_minx != state_emitter.bbox_minx ||
    state_emitter_new.bbox_miny != state_emitter.bbox_miny ||
    state_emitter_new.bbox_minz != state_emitter.bbox_minz ||
    state_emitter_new.bbox_maxx != state_emitter.bbox_maxx ||
    state_emitter_new.bbox_maxy != state_emitter.bbox_maxy ||
    state_emitter_new.bbox_maxz != state_emitter.bbox_maxz)
  {
    csVector3 min(state_emitter_new.bbox_minx,state_emitter_new.bbox_miny,state_emitter_new.bbox_minz);
    csVector3 max(state_emitter_new.bbox_maxx,state_emitter_new.bbox_maxy,state_emitter_new.bbox_maxz);
    emitState->SetContainerBox(state_emitter_new.using_bounding_box,min,max);
  }

  memcpy(&state_emitter,&state_emitter_new,sizeof(state_emitter));
  force_emitter_setup=false;
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

  csRef<iLight> light;
  iLightList* ll = room->GetLights ();

  light = engine->CreateLight (0, csVector3 (-3, 5, 0), 10,
  	csColor (1, 0, 0));
  ll->Add (light);

  light = engine->CreateLight (0, csVector3 (3, 5,  0), 10,
  	csColor (0, 0, 1));
  ll->Add (light);

  light = engine->CreateLight (0, csVector3 (0, 5, -3), 10,
  	csColor (0, 1, 0));
  ll->Add (light);

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
  memcpy(&state_emitter_new,&state_emitter,sizeof(state_emitter));

  memset(&state_initial_position,0,sizeof(state_initial_position));
  memset(&state_initial_speed,0,sizeof(state_initial_speed));
  state_initial_speed.fixed_position.y=0.5;
  memset(&state_initial_acceleration,0,sizeof(state_initial_acceleration));
  memset(&state_field_speed,0,sizeof(state_field_speed));
  memset(&state_field_accel,0,sizeof(state_field_accel));
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
  s = new awsSink();
  iAwsSink* sink = aws->GetSinkMgr()->CreateSink((intptr_t)s);

  s->SetSink(sink);
  s->SetWindowManager(aws);
  s->SetVFS(vfs);

  aws->GetSinkMgr()->RegisterSink("parteditSink", sink);

  // now load preferences
  if (!aws->GetPrefMgr()->Load("/varia/partedit.def"))
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
  iAwsWindow *iawswindow_FieldSpeed = aws->CreateWindowFrom("FieldSpeed");
  if (iawswindow_FieldSpeed) iawswindow_FieldSpeed->Hide();
  iAwsWindow *iawswindow_FieldAccel = aws->CreateWindowFrom("FieldAccel");
  if (iawswindow_FieldAccel) iawswindow_FieldAccel->Hide();

  iAwsWindow *iawswindow_Attractor = aws->CreateWindowFrom("Attractor");
  if (iawswindow_Attractor) iawswindow_Attractor->Hide();

  iAwsWindow *iawswindow_LoadSave = aws->CreateWindowFrom("LoadSaveNotAvailable");
  if (iawswindow_LoadSave) iawswindow_LoadSave->Hide();
  iAwsWindow *iawswindow_Aging = aws->CreateWindowFrom("AgingNotAvailable");
  if (iawswindow_Aging) iawswindow_Aging->Hide();


  iAwsWindow *iawswindow_FreeScroll = aws->CreateWindowFrom("FreeScroll");
  if (iawswindow_FreeScroll) iawswindow_FreeScroll->Show();

  s->SetGraphicCWD("/lib/std/");
  s->SetGraphicFilter("");
  s->SetGraphicFile("cslogo2.png");
  s->SetEmitterState(&state_emitter);
  s->SetInitialPositionState(&state_initial_position);
  s->SetInitialSpeedState(&state_initial_speed);
  s->SetInitialAccelerationState(&state_initial_acceleration);
  s->SetFieldSpeedState(&state_field_speed);
  s->SetFieldAccelState(&state_field_accel);
  s->SetAttractorState(&state_attractor);
  s->ClearLoadSaveStateChanged();

  return true;
}

void PartEdit::Start ()
{
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
