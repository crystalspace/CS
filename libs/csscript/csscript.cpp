/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

#include "sysdef.h"
#include "csscript/csscript.h"
#include "csscript/trigger.h"
#include "csengine/sysitf.h"
#include "csengine/cscoll.h"
#include "csengine/thing.h"
#include "csengine/light.h"
#include "csengine/camera.h"
#include "csengine/world.h"

LanguageLayer::LanguageLayer (csWorld* w, csCamera* c)
{
  world = w;
  camera = c;
  first_run = NULL;
}

LanguageLayer::~LanguageLayer ()
{
  while (first_run)
  {
    csRunScript* n = first_run->get_next ();
    CHK (delete first_run);
    first_run = n;
  }
}

void LanguageLayer::link_run (csRunScript* r)
{
  r->set_next (first_run);
  r->set_prev (NULL);
  if (first_run) first_run->set_prev (r);
  first_run = r;
}

void LanguageLayer::step_run ()
{
  csRunScript* r = first_run;
  while (r)
  {
    csRunScript* n = r->get_next ();
    if (r->step ())
    {
      // Finish run.
      if (r->get_prev ()) r->get_prev ()->set_next (r->get_next ());
      else first_run = r->get_next ();
      if (r->get_next ()) r->get_next ()->set_prev (r->get_prev ());
      CHK (delete r);
    }
    r = n;
  }
}

void LanguageLayer::message (char* msg)
{
  CsPrintf (MSG_CONSOLE, "%s\n", msg);
}

csObject* LanguageLayer::find_object (csObject* parent, char* name)
{
  (void)parent; (void)name;
  //@@@
  return NULL;
}

csObject* LanguageLayer::first_object (csObject* parent)
{
  (void)parent;
  //@@@
  return NULL;
}

csObject* LanguageLayer::next_object (csObject* parent, csObject* obj)
{
  (void)parent; (void)obj;
  //@@@
  return NULL;
}

const csIdType& LanguageLayer::object_type (csObject* obj)
{
  return obj->GetType ();
}

const char* LanguageLayer::object_name (csObject* obj)
{
  return obj->GetName();
}

csObject* LanguageLayer::get_world ()
{
  return (csObject*)world;
}

csObject* LanguageLayer::camera_sector ()
{
  return (csObject*)(camera->GetSector ());
}

csMatrix3 LanguageLayer::camera_matrix ()
{
  return camera->GetW2C ();
}

csVector3 LanguageLayer::camera_vector ()
{
  return camera->GetOrigin ();
}

void LanguageLayer::transform (csObject* obj, csMatrix3& m, csVector3& v)
{
  if (obj->GetType () == csThing::Type())
  {
    csThing* th = (csThing*)obj;
    th->Transform (m);
    th->Move (v);
    th->Transform (); // @@@ This should be delayed until it is sure that the
    		      // thing is visible.
  }
  else if (obj->GetType () == csCollection::Type())
  {
    csCollection* col = (csCollection*)obj;
    col->Transform (m);
    col->Move (v);
    col->Transform (); // @@@ This should be delayed until it is sure that the
    		       // collection is visible.
  }
}

void LanguageLayer::transform (csObject* obj, csMatrix3& m)
{
  if (obj->GetType () == csThing::Type())
  {
    csThing* th = (csThing*)obj;
    th->Transform (m);
    th->Transform (); // @@@ This should be delayed until it is sure that the
    		      // thing is visible.
  }
  else if (obj->GetType () == csCollection::Type())
  {
    csCollection* col = (csCollection*)obj;
    col->Transform (m);
    col->Transform (); // @@@ This should be delayed until it is sure that the
    		       // collection is visible.
  }
}

void LanguageLayer::transform (csObject* obj, csVector3& v)
{
  transform (obj, v.x, v.y, v.z);
}

void LanguageLayer::transform (csObject* obj, float dx, float dy, float dz)
{
  if (obj->GetType () == csThing::Type())
  {
    csThing* th = (csThing*)obj;
    th->Move (dx, dy, dz);
    th->Transform (); // @@@ This should be delayed until it is sure that the
    		      // thing is visible.
  }
  else if (obj->GetType () == csCollection::Type())
  {
    csCollection* col = (csCollection*)obj;
    col->Move (dx, dy, dz);
    col->Transform (); // @@@ This should be delayed until it is sure that the
    		       // collection is visible.
  }
}

void LanguageLayer::transform_rot_x (csObject* obj, float angle)
{
  csMatrix3 rot;
  matrix_rot_x (rot, angle);
  transform (obj, rot);
}

void LanguageLayer::transform_rot_y (csObject* obj, float angle)
{
  csMatrix3 rot;
  matrix_rot_y (rot, angle);
  transform (obj, rot);
}

void LanguageLayer::transform_rot_z (csObject* obj, float angle)
{
  csMatrix3 rot;
  matrix_rot_z (rot, angle);
  transform (obj, rot);
}

void LanguageLayer::matrix_rot_x (csMatrix3& m, float angle)
{
  m.m11 = 1; m.m12 = 0;           m.m13 = 0;
  m.m21 = 0; m.m22 = cos (angle); m.m23 = -sin(angle);
  m.m31 = 0; m.m32 = sin (angle); m.m33 =  cos(angle);
}

void LanguageLayer::matrix_rot_y (csMatrix3& m, float angle)
{
  m.m11 = cos (angle); m.m12 = 0; m.m13 = -sin(angle);
  m.m21 = 0;           m.m22 = 1; m.m23 = 0;
  m.m31 = sin (angle); m.m32 = 0; m.m33 =  cos(angle);
}

void LanguageLayer::matrix_rot_z (csMatrix3& m, float angle)
{
  m.m11 = cos (angle); m.m12 = -sin(angle); m.m13 = 0;
  m.m21 = sin (angle); m.m22 =  cos(angle); m.m23 = 0;
  m.m31 = 0;           m.m32 = 0;           m.m33 = 1;
}

void LanguageLayer::set_light_intensity(
  csLight* light, float l1, float l2, float l3)
{
  csIdType t = light->GetType ();
  if (t == csStatLight::Type() || t == csDynLight::Type())
    light->SetColor (csColor (l1, l2, l3));
}

float LanguageLayer::get_lightmap1_intensity (csLight* light)
{
  csIdType t = light->GetType ();
  if (t == csStatLight::Type() || t == csDynLight::Type())
    return light->GetColor ().red;
  return 0;
}

float LanguageLayer::get_lightmap2_intensity (csLight* light)
{
  csIdType t = light->GetType ();
  if (t == csStatLight::Type() || t == csDynLight::Type())
    return light->GetColor ().green;
  return 0;
}

float LanguageLayer::get_lightmap3_intensity (csLight* light)
{
  csIdType t = light->GetType ();
  if (t == csStatLight::Type() || t == csDynLight::Type())
    return light->GetColor ().blue;
  return 0;
}

void LanguageLayer::show (csObject* obj)
{
  (void)obj;
  //@@@
}

void LanguageLayer::hide (csObject* obj)
{
  (void)obj;
  //@@@
}

void LanguageLayer::send_event (csObject* obj, csScriptEvent event)
{
  (void)obj; (void)event;
  //@@@
}

csScript* LanguageLayer::find_script (char* name)
{
  (void)name;
  //@@@
  return NULL;
}

csRunScript* LanguageLayer::run_script (csScript* script, csObject* attached)
{
  return script->run_script (attached);
}

void LanguageLayer::suspend (csRunScript* run, long milli)
{
  (void)run; (void)milli;
  //@@@
}

//-----------------------------------------------------------------------------

CSOBJTYPE_IMPL(csScript,csObject);

csScript::csScript (LanguageLayer* layer) : csObject ()
{
  csScript::layer = layer;
}

csScript::~csScript ()
{
}

//-----------------------------------------------------------------------------

CSOBJTYPE_IMPL(csRunScript,csObject);

csRunScript::csRunScript (csScript* script, csObject* attached) : csObject ()
{
  csRunScript::script = script;
  csRunScript::attached = attached;
  layer = script->get_layer ();
}

csRunScript::~csRunScript ()
{
}


//---------------------------------------------------------------------------

TriggerList::TriggerList ()
{
  first_trigger = NULL;
}

TriggerList::~TriggerList ()
{
  while (first_trigger)
  {
    Trigger* n = first_trigger->next;
    CHK (delete first_trigger);
    first_trigger = n;
  }
}

void TriggerList::add_trigger (csScript* s, csObject* object)
{
  CHK (Trigger* t = new Trigger ());
  t->script = s;
  t->next = first_trigger;
  t->object = object;
  first_trigger = t;
}

void TriggerList::perform (csObject* object)
{
  Trigger* t = first_trigger;
  while (t)
  {
    t->script->run_script (t->object ? t->object : object);
    t = t->next;
  }
}
