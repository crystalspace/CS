/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein

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
#include "csqint.h"
#include "walktest.h"
#include "bot.h"
#include "infmaze.h"
#include "command.h"
#include "imesh/thing.h"
#include "iengine/camera.h"
#include "iengine/mesh.h"
#include "iengine/sector.h"
#include "imesh/object.h"
#include "csutil/csuctransform.h"
#include "csutil/scanstr.h"
#include "csgeom/math3d.h"
#include "cstool/collider.h"
#include "cstool/cspixmap.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "isound/wrapper.h"
#include "isound/data.h"
#include "ivaria/reporter.h"
#include "ivaria/view.h"
#include "iutil/event.h"
#include "iutil/csinput.h"
#include "iutil/virtclk.h"
#include "csutil/event.h"

extern WalkTest* Sys;

csKeyMap* mapping = 0;

iMeshWrapper *FindNextClosestMesh (iMeshWrapper *baseMesh, iCamera *camera, csVector2 *screenCoord);

//===========================================================================
// Everything for key mapping and binding.
//===========================================================================

void map_key (const char* _keyname, csKeyMap* map)
{
  map->shift = 0;
  map->alt = 0;
  map->ctrl = 0;
  map->need_status = 0;
  char* keyname = new char[strlen(_keyname) + 1];
  strcpy (keyname, _keyname);
  char* dash = strchr (keyname, '-');
  while (dash)
  {
    *dash = 0;
    if (!strcmp (keyname, "shift")) map->shift = 1;
    else if (!strcmp (keyname, "alt")) map->alt = 1;
    else if (!strcmp (keyname, "ctrl")) map->ctrl = 1;
    else if (!strcmp (keyname, "status")) map->need_status = 1;
    else Sys->Report (CS_REPORTER_SEVERITY_NOTIFY,
    	"Bad modifier '%s'!", keyname);

    *dash = '-';
    strcpy (keyname, dash+1);
    dash = strchr (keyname, '-');
  }

  if (!strcmp (keyname, "tab")) map->key = CSKEY_TAB;
  else if (!strcmp (keyname, "space")) map->key = ' ';
  else if (!strcmp (keyname, "esc")) map->key = CSKEY_ESC;
  else if (!strcmp (keyname, "enter")) map->key = CSKEY_ENTER;
  else if (!strcmp (keyname, "bs")) map->key = CSKEY_BACKSPACE;
  else if (!strcmp (keyname, "up")) map->key = CSKEY_UP;
  else if (!strcmp (keyname, "down")) map->key = CSKEY_DOWN;
  else if (!strcmp (keyname, "right")) map->key = CSKEY_RIGHT;
  else if (!strcmp (keyname, "left")) map->key = CSKEY_LEFT;
  else if (!strcmp (keyname, "pgup")) map->key = CSKEY_PGUP;
  else if (!strcmp (keyname, "pgdn")) map->key = CSKEY_PGDN;
  else if (!strcmp (keyname, "home")) map->key = CSKEY_HOME;
  else if (!strcmp (keyname, "end")) map->key = CSKEY_END;
  else if (!strcmp (keyname, "ins")) map->key = CSKEY_INS;
  else if (!strcmp (keyname, "del")) map->key = CSKEY_DEL;
  else if (!strcmp (keyname, "f1")) map->key = CSKEY_F1;
  else if (!strcmp (keyname, "f2")) map->key = CSKEY_F2;
  else if (!strcmp (keyname, "f3")) map->key = CSKEY_F3;
  else if (!strcmp (keyname, "f4")) map->key = CSKEY_F4;
  else if (!strcmp (keyname, "f5")) map->key = CSKEY_F5;
  else if (!strcmp (keyname, "f6")) map->key = CSKEY_F6;
  else if (!strcmp (keyname, "f7")) map->key = CSKEY_F7;
  else if (!strcmp (keyname, "f8")) map->key = CSKEY_F8;
  else if (!strcmp (keyname, "f9")) map->key = CSKEY_F9;
  else if (!strcmp (keyname, "f10")) map->key = CSKEY_F10;
  else if (!strcmp (keyname, "f11")) map->key = CSKEY_F11;
  else if (!strcmp (keyname, "f12")) map->key = CSKEY_F12;
  /*
  else if (*(keyname+1) != 0) Sys->Report (CS_REPORTER_SEVERITY_NOTIFY,
  	"Bad key '%s'!", keyname);
  else if ((*keyname >= 'A' && *keyname <= 'Z') ||
    strchr ("!@#$%^&*()_+", *keyname))
  {
    map->shift = 1;
    map->key = *keyname;
  }
  */
  else
  {
    utf32_char key;
    size_t nameLen = strlen (keyname);
    bool charValid;
    int encLen = csUnicodeTransform::UTF8Decode ((utf8_char*)keyname, 
      nameLen, key, &charValid);
    if (!charValid || ((size_t)encLen < nameLen))
    {
      Sys->Report (CS_REPORTER_SEVERITY_NOTIFY, "Bad key '%s'!", keyname);
    }
    else
      map->key = key;
  }
  delete[] keyname;
}

char* keyname (csKeyMap* map)
{
  static char buf[100];
  buf[0] = 0;
  if (map->need_status) strcat (buf, "status-");
  if (map->shift) strcat (buf, "shift-");
  if (map->ctrl) strcat (buf, "ctrl-");
  if (map->alt) strcat (buf, "alt-");
  switch (map->key)
  {
    case CSKEY_TAB: strcat (buf, "tab"); break;
    case ' ': strcat (buf, "space"); break;
    case CSKEY_ESC: strcat (buf, "esc"); break;
    case CSKEY_ENTER: strcat (buf, "enter"); break;
    case CSKEY_BACKSPACE: strcat (buf, "bs"); break;
    case CSKEY_UP: strcat (buf, "up"); break;
    case CSKEY_DOWN: strcat (buf, "down"); break;
    case CSKEY_RIGHT: strcat (buf, "right"); break;
    case CSKEY_LEFT: strcat (buf, "left"); break;
    case CSKEY_PGUP: strcat (buf, "pgup"); break;
    case CSKEY_PGDN: strcat (buf, "pgdn"); break;
    case CSKEY_HOME: strcat (buf, "home"); break;
    case CSKEY_END: strcat (buf, "end"); break;
    case CSKEY_INS: strcat (buf, "ins"); break;
    case CSKEY_DEL: strcat (buf, "del"); break;
    case CSKEY_F1: strcat (buf, "f1"); break;
    case CSKEY_F2: strcat (buf, "f2"); break;
    case CSKEY_F3: strcat (buf, "f3"); break;
    case CSKEY_F4: strcat (buf, "f4"); break;
    case CSKEY_F5: strcat (buf, "f5"); break;
    case CSKEY_F6: strcat (buf, "f6"); break;
    case CSKEY_F7: strcat (buf, "f7"); break;
    case CSKEY_F8: strcat (buf, "f8"); break;
    case CSKEY_F9: strcat (buf, "f9"); break;
    case CSKEY_F10: strcat (buf, "f10"); break;
    case CSKEY_F11: strcat (buf, "f11"); break;
    case CSKEY_F12: strcat (buf, "f12"); break;
    default:
    {
      char* s = strchr (buf, 0);
      *s++ = map->key;
      *s = 0;
    }
  }
  return buf;
}

csKeyMap* find_mapping (const char* keyname)
{
  csKeyMap map;
  map_key (keyname, &map);

  csKeyMap* m = mapping;
  while (m)
  {
    if (map.key == m->key && map.shift == m->shift && map.ctrl == m->ctrl && map.alt == m->alt
    	&& map.need_status == m->need_status)
      return m;
    m = m->next;
  }
  return 0;
}

void bind_key (const char* _arg)
{
  if (!_arg)
  {
    csKeyMap* map = mapping;
    while (map)
    {
      Sys->Report (CS_REPORTER_SEVERITY_NOTIFY,
      	"Key '%s' bound to '%s'.", keyname (map), map->cmd);
      map = map->next;
    }
    return;
  }
  char* arg = new char[strlen(_arg) + 1];
  strcpy (arg, _arg);
  char* space = strchr (arg, ' ');
  if (space)
  {
    *space = 0;
    csKeyMap* map = find_mapping (arg);
    if (map)
    {
      delete [] map->cmd;
    }
    else
    {
      map = new csKeyMap ();
      map->next = mapping;
      map->prev = 0;
      if (mapping) mapping->prev = map;
      mapping = map;
      map_key (arg, map);
    }
    map->cmd = new char [strlen (space+1)+1];
    strcpy (map->cmd, space+1);
    *space = ' ';
  }
  else
  {
    csKeyMap* map = find_mapping (arg);
    if (map) Sys->Report (CS_REPORTER_SEVERITY_NOTIFY,
    	"Key bound to '%s'!", map->cmd);
    else Sys->Report (CS_REPORTER_SEVERITY_NOTIFY, "Key not bound!");
  }
  delete[] arg;
}

void free_keymap ()
{
  csKeyMap *prev, *cur = mapping;
  while (cur)
  {
    prev = cur;
    cur = cur->next;
    delete [] prev->cmd;
    delete prev;
  }
  mapping = 0;
}

extern iCamera* c;
extern WalkTest* Sys;

void WalkTest::strafe (float speed,int keep_old)
{
  if (move_3d) return;
  static bool pressed = false;
  static float strafe_speed = 0;
  static long start_time = vc->GetCurrentTicks ();

  long cur_time = vc->GetCurrentTicks ();
  if (!keep_old)
  {
    bool new_pressed = ABS (speed) > 0.001;
    if (new_pressed != pressed)
    {
      pressed = new_pressed;
      strafe_speed = speed * cfg_walk_accelerate;
      start_time = cur_time - 100;
    }
  }

  while ((cur_time - start_time) >= 100)
  {
    if (pressed)
    {
      // accelerate
      if (ABS (velocity.x) < cfg_walk_maxspeed * cfg_walk_maxspeed_multreal)
        velocity.x += strafe_speed;
    }
    else
    {
      // brake!
      if (velocity.x > cfg_walk_brake)
        velocity.x -= cfg_walk_brake;
      else if (velocity.x < -cfg_walk_brake)
        velocity.x += cfg_walk_brake;
      else
        velocity.x = 0;
    }
    start_time += 100;
  }
}

void WalkTest::step (float speed,int keep_old)
{
  if (move_3d) return;

  static bool pressed = false;
  static float step_speed = 0;
  static long start_time = vc->GetCurrentTicks ();

  long cur_time = vc->GetCurrentTicks ();
  if (!keep_old)
  {
    bool new_pressed = ABS (speed) > 0.001;
    if (new_pressed != pressed)
    {
      pressed = new_pressed;
      step_speed = speed * cfg_walk_accelerate;
      start_time = cur_time - 100;
    }
  }

  float max_speed = cfg_walk_maxspeed * cfg_walk_maxspeed_multreal
  	* (kbd->GetKeyState (CSKEY_SHIFT) ? 2 : 1);

  while ((cur_time - start_time) >= 100)
  {
    if (pressed)
    {
      // accelerate
      if (ABS (velocity.z) < max_speed)
        velocity.z += step_speed;
      else if (ABS (velocity.z) > max_speed)
        velocity.z -= step_speed;
    }
    else
    {
      // brake!
      if (velocity.z > cfg_walk_brake)
        velocity.z -= cfg_walk_brake;
      else if (velocity.z < -cfg_walk_brake)
        velocity.z += cfg_walk_brake;
      else
        velocity.z = 0;
    }
    start_time += 100;
  }
}

void WalkTest::rotate (float speed,int keep_old)
{
  if (move_3d) return;

  static bool pressed = false;
  static float angle_accel = 0;
  static long start_time = vc->GetCurrentTicks ();

  long cur_time = vc->GetCurrentTicks ();
  if (!keep_old)
  {
    bool new_pressed = ABS (speed) > 0.001;
    if (new_pressed != pressed)
    {
      pressed = new_pressed;
      angle_accel = speed * cfg_rotate_accelerate;
      start_time = cur_time - 100;
    }
  }

  float max_speed = cfg_rotate_maxspeed * (kbd->GetKeyState (CSKEY_SHIFT) ? 2 : 1);

  while ((cur_time - start_time) >= 100)
  {
    if (pressed)
    {
      // accelerate rotation
      if (ABS (angle_velocity.y) < max_speed)
        angle_velocity.y += angle_accel;
      else if (ABS (angle_velocity.y) > max_speed)
        angle_velocity.y -= angle_accel;
    }
    else
    {
      // brake!
      if (angle_velocity.y > cfg_rotate_brake)
        angle_velocity.y -= cfg_rotate_brake;
      else if (angle_velocity.y < -cfg_rotate_brake)
        angle_velocity.y += cfg_rotate_brake;
      else
        angle_velocity.y = 0;
    }
    start_time += 100;
  }
}

void WalkTest::look (float speed,int keep_old)
{
  (void) speed; (void) keep_old;
  if (move_3d) return;
  static float step_speed = 0;
  if (!keep_old)
    step_speed = speed*cfg_look_accelerate;
  
  //XXX: how to do this without angle?
#if 0
  if (ABS (angle.x+step_speed) <= (355.0/113.0/4))
    angle.x += step_speed;
#endif
  RotateCam (-step_speed, 0);
}

void WalkTest::RotateCam(float x, float y)
{
  csMatrix3 mat = view->GetCamera ()->GetTransform ().GetO2T ();
  if(x)
    mat = csXRotMatrix3(x) * mat;
  if(y)
    mat *= csYRotMatrix3(y);
  view->GetCamera ()->SetTransform ( csOrthoTransform 
	  (mat, view->GetCamera ()->GetTransform ().GetOrigin ()));
}

void WalkTest::imm_forward (float speed, bool slow, bool fast)
{
  if (slow)
    view->GetCamera ()->Move (speed * 0.01 * CS_VEC_FORWARD, do_cd);
  else if (fast)
    view->GetCamera ()->Move (speed * 4.0 * CS_VEC_FORWARD, do_cd);
  else
    view->GetCamera ()->Move (speed * 1.0 * CS_VEC_FORWARD, do_cd);
}

void WalkTest::imm_backward (float speed, bool slow, bool fast)
{
  if (slow)
    view->GetCamera ()->Move (speed*.01*CS_VEC_BACKWARD, do_cd);
  else if (fast)
    view->GetCamera ()->Move (speed*1.2*CS_VEC_BACKWARD, do_cd);
  else
    view->GetCamera ()->Move (speed*.6*CS_VEC_BACKWARD, do_cd);
}

void WalkTest::imm_left (float speed, bool slow, bool fast)
{
  if (slow)
    view->GetCamera ()->Move (speed * 0.01 * CS_VEC_LEFT, do_cd);
  else if (fast)
    view->GetCamera ()->Move (speed * 4.0 * CS_VEC_LEFT, do_cd);
  else
    view->GetCamera ()->Move (speed * 1.0 * CS_VEC_LEFT, do_cd);
}

void WalkTest::imm_right (float speed, bool slow, bool fast)
{
  if (slow)
    view->GetCamera ()->Move (speed * 0.01 * CS_VEC_RIGHT, do_cd);
  else if (fast)
    view->GetCamera ()->Move (speed * 4.0 * CS_VEC_RIGHT, do_cd);
  else
    view->GetCamera ()->Move (speed * 1.0 * CS_VEC_RIGHT, do_cd);
}

void WalkTest::imm_up (float speed, bool slow, bool fast)
{
  if (slow)
    view->GetCamera ()->Move (speed * 0.01 * CS_VEC_UP, do_cd);
  else if (fast)
    view->GetCamera ()->Move (speed * 4.0 * CS_VEC_UP, do_cd);
  else
    view->GetCamera ()->Move (speed * 1.0 * CS_VEC_UP, do_cd);
}

void WalkTest::imm_down (float speed, bool slow, bool fast)
{
  if (slow)
    view->GetCamera ()->Move (speed * 0.01 * CS_VEC_DOWN, do_cd);
  else if (fast)
    view->GetCamera ()->Move (speed * 4.0 * CS_VEC_DOWN, do_cd);
  else
    view->GetCamera ()->Move (speed * 1.0 * CS_VEC_DOWN, do_cd);
}

void WalkTest::imm_rot_left_camera (float speed, bool slow, bool fast)
{
  if (slow)
    view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_ROT_LEFT, speed * .01);
  else if (fast)
    view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_ROT_LEFT, speed * .4);
  else
    view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_ROT_LEFT, speed * .2);
}

void WalkTest::imm_rot_left_world (float speed, bool slow, bool fast)
{
  if (slow)
    view->GetCamera ()->GetTransform ().RotateOther (CS_VEC_ROT_LEFT, speed * .01);
  else if (fast)
    view->GetCamera ()->GetTransform ().RotateOther (CS_VEC_ROT_LEFT, speed * .4);
  else
    view->GetCamera ()->GetTransform ().RotateOther (CS_VEC_ROT_LEFT, speed * .2);
}

void WalkTest::imm_rot_right_camera (float speed, bool slow, bool fast)
{
  if (slow)
    view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_ROT_RIGHT, speed * .01);
  else if (fast)
    view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_ROT_RIGHT, speed * .4);
  else
    view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_ROT_RIGHT, speed * .2);
}

void WalkTest::imm_rot_right_world (float speed, bool slow, bool fast)
{
  if (slow)
    view->GetCamera ()->GetTransform ().RotateOther (CS_VEC_ROT_RIGHT, speed * .01);
  else if (fast)
    view->GetCamera ()->GetTransform ().RotateOther (CS_VEC_ROT_RIGHT, speed * .4);
  else
    view->GetCamera ()->GetTransform ().RotateOther (CS_VEC_ROT_RIGHT, speed * .2);
}

void WalkTest::imm_rot_left_xaxis (float speed, bool slow, bool fast)
{
  if (slow)
    view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_TILT_DOWN, speed * .01);
  else if (fast)
    view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_TILT_DOWN, speed * .4);
  else
    view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_TILT_DOWN, speed * .2);
}

void WalkTest::imm_rot_right_xaxis (float speed, bool slow, bool fast)
{
  if (slow)
    view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_TILT_UP, speed * .01);
  else if (fast)
    view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_TILT_UP, speed * .4);
  else
    view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_TILT_UP, speed * .2);
}

void WalkTest::imm_rot_left_zaxis (float speed, bool slow, bool fast)
{
  if (slow)
    view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_TILT_LEFT, speed * .01);
  else if (fast)
    view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_TILT_LEFT, speed * .4);
  else
    view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_TILT_LEFT, speed * .2);
}

void WalkTest::imm_rot_right_zaxis (float speed, bool slow, bool fast)
{
  if (slow)
    view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_TILT_RIGHT, speed * .01);
  else if (fast)
    view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_TILT_RIGHT, speed * .4);
  else
    view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_TILT_RIGHT, speed * .2);
}

void WalkTest::eatkeypress (iEvent* Event)
{
  uint32 key = 0;
  Event->Retrieve ("keyCodeRaw", key);

  uint8 evType = 0;
  Event->Retrieve ("keyEventType", evType);
  bool status = (evType == csKeyEventTypeDown);

  if (myConsole && myConsole->GetVisible () && status)
  {
    if (ConsoleInput)
      ConsoleInput->HandleEvent (*Event);
    //@@KLUDGE
    if (key != CSKEY_TAB)
      return;
  }

  csKeyModifiers modifiers;
  csKeyEventHelper::GetModifiers (Event, modifiers);

  bool shift = modifiers.modifiers[csKeyModifierTypeShift] != 0;
  bool alt = modifiers.modifiers[csKeyModifierTypeAlt] != 0;
  bool ctrl = modifiers.modifiers[csKeyModifierTypeCtrl] != 0;

  csKeyMap *m = mapping;
  while (m)
  {
    if (key == m->key
     && shift == m->shift && alt == m->alt && ctrl == m->ctrl)
    {
      if (m->need_status)
      {
        // Don't perform the command again if the key is already down
        if (m->is_on != status)
        {
          char buf [256];
          sprintf (buf,"%s %d", m->cmd, status);
          csCommandProcessor::perform_line (buf);
          m->is_on = status;
        }
      }
      else
      {
        if (status)
          csCommandProcessor::perform_line (m->cmd);
      }
    }
    else if (!status && key == m->key && m->is_on && m->need_status)
    {
      if (key == m->key || shift != m->shift || alt != m->alt || ctrl != m->ctrl)
      {
        char buf [256];
        sprintf (buf,"%s 0", m->cmd);
        csCommandProcessor::perform_line (buf);
        m->is_on = 0;
      }
    }
    m = m->next;
  }
}


// left mouse button
void WalkTest::MouseClick1Handler(iEvent &Event)
{
  (void)Event;
  move_forward = true;
}

// middle mouse button
void WalkTest::MouseClick2Handler(iEvent &Event)
{
  csVector3 v;
  csVector2 p (Event.Mouse.x, FRAME_HEIGHT-Event.Mouse.y);

  view->GetCamera ()->InvPerspective (p, 1, v);
  csVector3 vw = view->GetCamera ()->GetTransform ().This2Other (v);

  iSector* sector = view->GetCamera ()->GetSector ();
  csVector3 origin = view->GetCamera ()->GetTransform ().GetO2TTranslation ();
  csVector3 isect;
  int sel;
  iMeshWrapper* mesh = sector->HitBeamPortals (origin,
  	origin + (vw-origin) * 20, isect, &sel);

  vw = isect;
  v = view->GetCamera ()->GetTransform ().Other2This (vw);
  Sys->Report (CS_REPORTER_SEVERITY_NOTIFY,
  	"LMB down : cam:(%f,%f,%f) world:(%f,%f,%f)",
	v.x, v.y, v.z, vw.x, vw.y, vw.z);
  Sys->Report (CS_REPORTER_SEVERITY_NOTIFY,
  	"LMB down : cam:(%f,%f,%f) world:(%f,%f,%f)",
	v.x, v.y, v.z, vw.x, vw.y, vw.z);

  if (mesh && sel != -1)
  {
    csRef<iThingState> ps = SCF_QUERY_INTERFACE (mesh->GetMeshObject (),
    	iThingState);
    Sys->selected_polygon = sel;

    iMeshObject* obj = mesh->GetMeshObject ();
    csRef<iObject> psobj = SCF_QUERY_INTERFACE (obj->GetLogicalParent (),
    	iObject);
    Sys->Report (CS_REPORTER_SEVERITY_DEBUG, "Hit polygon '%s/%s'",
    	psobj ? psobj->GetName () : "<null>",
	ps ? ps->GetFactory ()->GetPolygonName (sel) : "<null>");
    //Dumper::dump (sel);
  }

  extern csVector2 coord_check_vector;
  coord_check_vector.x = Event.Mouse.x;
  coord_check_vector.y = FRAME_HEIGHT-Event.Mouse.y;
  extern bool check_light;
  extern void select_object (iRenderView* rview, int type, void* entity);
  check_light = true;
  //view->GetEngine ()->DrawFunc (view->GetCamera (),
    //view->GetClipper (), select_object);
}

// right mouse button
void WalkTest::MouseClick3Handler(iEvent &Event)
{
  csVector2   screenPoint;
  iMeshWrapper *closestMesh;

  screenPoint.x = Event.Mouse.x;
  screenPoint.y = Event.Mouse.y;
  closestMesh = FindNextClosestMesh (0, view->GetCamera(), &screenPoint);
  if (closestMesh)
    Sys->Report (CS_REPORTER_SEVERITY_NOTIFY,
    	"Selected mesh %s", closestMesh->
    	QueryObject ()->GetName ());
  else
    Sys->Report (CS_REPORTER_SEVERITY_NOTIFY, "No mesh selected!");
}


bool WalkTest::WalkHandleEvent (iEvent &Event)
{
  switch (Event.Type)
  {
    case csevBroadcast:
    {
      if (Event.Command.Code == cscmdCanvasHidden)
      {
	canvas_exposed = false;
      #ifdef CS_DEBUG
	Report (CS_REPORTER_SEVERITY_NOTIFY, "canvas hidden");
      #endif
	break;
      }
      else if (Event.Command.Code == cscmdCanvasExposed)
      {
	canvas_exposed = true;
      #ifdef CS_DEBUG
	Report (CS_REPORTER_SEVERITY_NOTIFY, "canvas exposed");
      #endif
	break;
      }
      else if (Event.Command.Code == cscmdContextResize)
      {
	Sys->FrameWidth = Sys->myG2D->GetWidth();
	Sys->FrameHeight = Sys->myG2D->GetHeight();
	break;
      }
      break;
    }
    case csevKeyboard:
      eatkeypress (&Event);
      break;
    case csevMouseDown:
      {
      if (Event.Mouse.Button == 1)
        MouseClick1Handler(Event);
      else if (Event.Mouse.Button == 2)
        MouseClick2Handler(Event);
      else if (Event.Mouse.Button == 3)
        MouseClick3Handler(Event);
      break;
      }

    case csevMouseMove:
      // additional command by Leslie Saputra -> freelook mode.
      {
        static bool first_time = true;
        if (do_freelook)
        {
          int last_x, last_y;
          last_x = Event.Mouse.x;
          last_y = Event.Mouse.y;

          myG2D->SetMousePosition (FRAME_WIDTH / 2, FRAME_HEIGHT / 2);
          if (!first_time)
          {
	    RotateCam (
		-((float)(last_y - (FRAME_HEIGHT / 2) )) / (FRAME_HEIGHT*2)*(1-2*(int)inverse_mouse),
		((float)(last_x - (FRAME_WIDTH / 2) )) / (FRAME_WIDTH*2));
          }
          else
            first_time = false;
        }
        else
          first_time = true;
      }
      break;
    case csevMouseUp:
      if (Event.Mouse.Button == 1)
      {
        move_forward = false;
	step (0, 0);
      }
      break;
  }

  return false;
}

iMeshWrapper *FindNextClosestMesh (iMeshWrapper *baseMesh,
	iCamera *camera, csVector2 *screenCoord)
{
  int meshIndex;
  float thisZLocation;
  float closestZLocation;
  iMeshWrapper *closestMesh;
  iMeshWrapper *nextMesh;
  csBox2 screenBoundingBox;
  csBox3 bbox3;

  if (baseMesh)
  {
    closestMesh = baseMesh;
    closestZLocation = baseMesh->GetScreenBoundingBox
    	(camera, screenBoundingBox, bbox3);
    // if the baseMesh isn't in front of the camera, return
    if (closestZLocation < 0)
      return 0;
  }
  else
  {
    closestMesh = 0;
    closestZLocation = 32000;
  }

  // @@@ This routine ignores 2D meshes for the moment.
  iMeshList* meshes = Sys->Engine->GetMeshes ();
  for (meshIndex = 0; meshIndex < meshes->GetCount (); meshIndex++)
  {
    nextMesh = meshes->Get (meshIndex);

    if (nextMesh != baseMesh)
    {
      thisZLocation = nextMesh->GetScreenBoundingBox(camera,
      	screenBoundingBox, bbox3);
      if ((thisZLocation > 0) && (thisZLocation < closestZLocation))
      {
        if (screenBoundingBox.In(screenCoord->x, screenCoord->y))
        {
          closestZLocation = thisZLocation;
          closestMesh = nextMesh;
        }
      }
    }
  }

  return closestMesh;
}
