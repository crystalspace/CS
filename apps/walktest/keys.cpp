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
#include "walktest.h"
#include "bot.h"
#include "infmaze.h"
#include "hugeroom.h"
#include "command.h"
#include "csengine/dumper.h"
#include "csengine/camera.h"
#include "csengine/octree.h"
#include "csengine/engine.h"
#include "csengine/csview.h"
#include "csengine/wirefrm.h"
#include "csengine/meshobj.h"
#include "csengine/polygon.h"
#include "csengine/light.h"
#include "csengine/sector.h"
#include "csengine/collider.h"
#include "csutil/scanstr.h"
#include "csparser/impexp.h"
#include "csobject/dataobj.h"
#include "isound/data.h"
#include "csparser/snddatao.h"
#include "csparser/crossbld.h"
#include "csgeom/math3d.h"
#include "cssys/system.h"
#include "csfx/cspixmap.h"
#include "qint.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"

extern WalkTest* Sys;

csKeyMap* mapping = NULL;

iMeshWrapper *FindNextClosestMesh (iMeshWrapper *baseMesh, iCamera *camera, csVector2 *screenCoord);

//===========================================================================
// Everything for key mapping and binding.
//===========================================================================

void map_key (const char* keyname, csKeyMap* map)
{
  map->shift = 0;
  map->alt = 0;
  map->ctrl = 0;
  map->need_status = 0;
  char* dash = strchr (keyname, '-');
  while (dash)
  {
    *dash = 0;
    if (!strcmp (keyname, "shift")) map->shift = 1;
    else if (!strcmp (keyname, "alt")) map->alt = 1;
    else if (!strcmp (keyname, "ctrl")) map->ctrl = 1;
    else if (!strcmp (keyname, "status")) map->need_status = 1;
    else Sys->Printf (MSG_CONSOLE, "Bad modifier '%s'!\n", keyname);

    *dash = '-';
    keyname = dash+1;
    dash = strchr (dash+1, '-');
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
  else if (*(keyname+1) != 0) Sys->Printf (MSG_CONSOLE, "Bad key '%s'!\n", keyname);
  else if ((*keyname >= 'A' && *keyname <= 'Z') || strchr ("!@#$%^&*()_+", *keyname))
  {
    map->shift = 1;
    map->key = *keyname;
  }
  else
    map->key = *keyname;
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
  return NULL;
}

void bind_key (const char* arg)
{
  if (!arg)
  {
    csKeyMap* map = mapping;
    while (map)
    {
      Sys->Printf (MSG_CONSOLE, "Key '%s' bound to '%s'.\n", keyname (map), map->cmd);
      map = map->next;
    }
    return;
  }
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
      map->prev = NULL;
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
    if (map) Sys->Printf (MSG_CONSOLE, "Key bound to '%s'!\n", map->cmd);
    else Sys->Printf (MSG_CONSOLE, "Key not bound!\n");
  }
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
  mapping = NULL;
}

/*------------------------------------------------------------------
 * The following handle_key_... routines are general movement
 * routines that are called by do_update() for the new movement
 * system and by do_keypress() for the old movement system (see
 * system.h for an explanation of the difference between the two
 * systems).
 *------------------------------------------------------------------*/

extern iCamera* c;
extern WalkTest* Sys;

void WalkTest::strafe (float speed,int keep_old)
{
  if (move_3d || map_mode) return;
  static bool pressed = false;
  static float strafe_speed = 0;
  static long start_time = Time ();

  long cur_time = Time ();
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
      if (ABS (velocity.x) < cfg_walk_maxspeed)
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
  if (move_3d || map_mode) return;

  static bool pressed = false;
  static float step_speed = 0;
  static long start_time = Time ();

  long cur_time = Time ();
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

  float max_speed = cfg_walk_maxspeed * (Keyboard.GetKeyState (CSKEY_SHIFT) ? 2 : 1);

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
  if (move_3d || map_mode) return;

  static bool pressed = false;
  static float angle_accel = 0;
  static long start_time = Time ();

  long cur_time = Time ();
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

  float max_speed = cfg_rotate_maxspeed * (Keyboard.GetKeyState (CSKEY_SHIFT) ? 2 : 1);

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
  if (move_3d || map_mode) return;
  static float step_speed = 0;
  if (!keep_old)
    step_speed = speed*cfg_look_accelerate;
  if (ABS (angle.x+step_speed) <= (355.0/113.0/4))
    angle.x += step_speed;
}

void WalkTest::imm_forward (float speed, bool slow, bool fast)
{
  if (map_mode) { wf->KeyUp (speed, slow, fast); return; }
  if (slow)
    view->GetCamera ()->Move (speed * 0.01 * VEC_FORWARD, do_cd);
  else if (fast)
    view->GetCamera ()->Move (speed * 4.0 * VEC_FORWARD, do_cd);
  else
    view->GetCamera ()->Move (speed * 1.0 * VEC_FORWARD, do_cd);
}

void WalkTest::imm_backward (float speed, bool slow, bool fast)
{
  if (map_mode) { wf->KeyDown (speed, slow, fast); return; }
  if (slow)
    view->GetCamera ()->Move (speed*.01*VEC_BACKWARD, do_cd);
  else if (fast)
    view->GetCamera ()->Move (speed*1.2*VEC_BACKWARD, do_cd);
  else
    view->GetCamera ()->Move (speed*.6*VEC_BACKWARD, do_cd);
}

void WalkTest::imm_left (float speed, bool slow, bool fast)
{
  if (map_mode) return;
  if (slow)
    view->GetCamera ()->Move (speed * 0.01 * VEC_LEFT, do_cd);
  else if (fast)
    view->GetCamera ()->Move (speed * 4.0 * VEC_LEFT, do_cd);
  else
    view->GetCamera ()->Move (speed * 1.0 * VEC_LEFT, do_cd);
}

void WalkTest::imm_right (float speed, bool slow, bool fast)
{
  if (map_mode) return;
  if (slow)
    view->GetCamera ()->Move (speed * 0.01 * VEC_RIGHT, do_cd);
  else if (fast)
    view->GetCamera ()->Move (speed * 4.0 * VEC_RIGHT, do_cd);
  else
    view->GetCamera ()->Move (speed * 1.0 * VEC_RIGHT, do_cd);
}

void WalkTest::imm_up (float speed, bool slow, bool fast)
{
  if (map_mode) return;
  if (slow)
    view->GetCamera ()->Move (speed * 0.01 * VEC_UP, do_cd);
  else if (fast)
    view->GetCamera ()->Move (speed * 4.0 * VEC_UP, do_cd);
  else
    view->GetCamera ()->Move (speed * 1.0 * VEC_UP, do_cd);
}

void WalkTest::imm_down (float speed, bool slow, bool fast)
{
  if (map_mode) return;
  if (slow)
    view->GetCamera ()->Move (speed * 0.01 * VEC_DOWN, do_cd);
  else if (fast)
    view->GetCamera ()->Move (speed * 4.0 * VEC_DOWN, do_cd);
  else
    view->GetCamera ()->Move (speed * 1.0 * VEC_DOWN, do_cd);
}

void WalkTest::imm_rot_left_camera (float speed, bool slow, bool fast)
{
  if (map_mode == MAP_TXT) { wf->KeyLeftStrafe (speed, slow, fast); return; }
  if (map_mode) { wf->KeyLeft (speed, slow, fast); return; }
  if (slow)
    view->GetCamera ()->GetTransform ().RotateThis (VEC_ROT_LEFT, speed * .01);
  else if (fast)
    view->GetCamera ()->GetTransform ().RotateThis (VEC_ROT_LEFT, speed * .4);
  else
    view->GetCamera ()->GetTransform ().RotateThis (VEC_ROT_LEFT, speed * .2);
}

void WalkTest::imm_rot_left_world (float speed, bool slow, bool fast)
{
  if (map_mode) return;
  if (slow)
    view->GetCamera ()->GetTransform ().RotateOther (VEC_ROT_LEFT, speed * .01);
  else if (fast)
    view->GetCamera ()->GetTransform ().RotateOther (VEC_ROT_LEFT, speed * .4);
  else
    view->GetCamera ()->GetTransform ().RotateOther (VEC_ROT_LEFT, speed * .2);
}

void WalkTest::imm_rot_right_camera (float speed, bool slow, bool fast)
{
  if (map_mode == MAP_TXT) { wf->KeyRightStrafe (speed, slow, fast); return; }
  if (map_mode) { wf->KeyRight (speed, slow, fast); return; }
  if (slow)
    view->GetCamera ()->GetTransform ().RotateThis (VEC_ROT_RIGHT, speed * .01);
  else if (fast)
    view->GetCamera ()->GetTransform ().RotateThis (VEC_ROT_RIGHT, speed * .4);
  else
    view->GetCamera ()->GetTransform ().RotateThis (VEC_ROT_RIGHT, speed * .2);
}

void WalkTest::imm_rot_right_world (float speed, bool slow, bool fast)
{
  if (map_mode) return;
  if (slow)
    view->GetCamera ()->GetTransform ().RotateOther (VEC_ROT_RIGHT, speed * .01);
  else if (fast)
    view->GetCamera ()->GetTransform ().RotateOther (VEC_ROT_RIGHT, speed * .4);
  else
    view->GetCamera ()->GetTransform ().RotateOther (VEC_ROT_RIGHT, speed * .2);
}

void WalkTest::imm_rot_left_xaxis (float speed, bool slow, bool fast)
{
  if (map_mode) { wf->KeyPgDn (speed, slow, fast); return; }
  if (slow)
    view->GetCamera ()->GetTransform ().RotateThis (VEC_TILT_DOWN, speed * .01);
  else if (fast)
    view->GetCamera ()->GetTransform ().RotateThis (VEC_TILT_DOWN, speed * .4);
  else
    view->GetCamera ()->GetTransform ().RotateThis (VEC_TILT_DOWN, speed * .2);
}

void WalkTest::imm_rot_right_xaxis (float speed, bool slow, bool fast)
{
  if (map_mode) { wf->KeyPgUp (speed, slow, fast); return; }
  if (slow)
    view->GetCamera ()->GetTransform ().RotateThis (VEC_TILT_UP, speed * .01);
  else if (fast)
    view->GetCamera ()->GetTransform ().RotateThis (VEC_TILT_UP, speed * .4);
  else
    view->GetCamera ()->GetTransform ().RotateThis (VEC_TILT_UP, speed * .2);
}

void WalkTest::imm_rot_left_zaxis (float speed, bool slow, bool fast)
{
  if (map_mode) return;
  if (slow)
    view->GetCamera ()->GetTransform ().RotateThis (VEC_TILT_LEFT, speed * .01);
  else if (fast)
    view->GetCamera ()->GetTransform ().RotateThis (VEC_TILT_LEFT, speed * .4);
  else
    view->GetCamera ()->GetTransform ().RotateThis (VEC_TILT_LEFT, speed * .2);
}

void WalkTest::imm_rot_right_zaxis (float speed, bool slow, bool fast)
{
  if (map_mode) return;
  if (slow)
    view->GetCamera ()->GetTransform ().RotateThis (VEC_TILT_RIGHT, speed * .01);
  else if (fast)
    view->GetCamera ()->GetTransform ().RotateThis (VEC_TILT_RIGHT, speed * .4);
  else
    view->GetCamera ()->GetTransform ().RotateThis (VEC_TILT_RIGHT, speed * .2);
}

void WalkTest::eatkeypress (iEvent &Event)
{
  int key = Event.Key.Code;
  int status = (Event.Type == csevKeyDown);
  bool shift = (Event.Key.Modifiers & CSMASK_SHIFT) != 0;
  bool alt = (Event.Key.Modifiers & CSMASK_ALT) != 0;
  bool ctrl = (Event.Key.Modifiers & CSMASK_CTRL) != 0;

  if (Console && Console->GetVisible () && status)
  {
    if (ConsoleInput)
      ConsoleInput->HandleEvent (Event);
    //@@KLUDGE
    if (key != CSKEY_TAB)
      return;
  }

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
  iPolygon3D* sel = sector->HitBeam (origin, origin + (vw-origin) * 20, isect);

  vw = isect;
  v = view->GetCamera ()->GetTransform ().Other2This (vw);
  Sys->Printf (MSG_CONSOLE, "LMB down : cam:(%f,%f,%f) world:(%f,%f,%f)\n", v.x, v.y, v.z, vw.x, vw.y, vw.z);
  Sys->Printf (MSG_DEBUG_0, "LMB down : cam:(%f,%f,%f) world:(%f,%f,%f)\n", v.x, v.y, v.z, vw.x, vw.y, vw.z);

  if (sel)
  {
    if (Sys->selected_polygon == sel)
      Sys->selected_polygon = NULL;
    else
      Sys->selected_polygon = sel;

    iThingState* ps = sel->GetParent ();
    iObject* psobj = QUERY_INTERFACE (ps, iObject);
    Sys->Printf (MSG_DEBUG_0, "Hit polygon '%s/%s'\n",
    	psobj->GetName (), sel->QueryObject ()->GetName ());
    Dumper::dump (sel);
    psobj->DecRef ();
  }

  extern csVector2 coord_check_vector;
  coord_check_vector.x = Event.Mouse.x;
  coord_check_vector.y = FRAME_HEIGHT-Event.Mouse.y;
  extern bool check_light;
  extern void select_object (iRenderView* rview, int type, void* entity);
  check_light = true;
  view->GetEngine ()->DrawFunc (view->GetCamera (),
    view->GetClipper (), select_object);
}

// right mouse button
void WalkTest::MouseClick3Handler(iEvent &Event)
{
  csVector2   screenPoint;
  iMeshWrapper *closestMesh;

  screenPoint.x = Event.Mouse.x;
  screenPoint.y = Event.Mouse.y;
  closestMesh = FindNextClosestMesh (NULL, view->GetCamera(), &screenPoint);
  if (closestMesh)
    Sys->Printf (MSG_CONSOLE, "Selected mesh %s\n", closestMesh->
    	QueryObject ()->GetName ());
  else
    Sys->Printf (MSG_CONSOLE, "No mesh selected!\n");
}


bool WalkTest::HandleEvent (iEvent &Event)
{
  // First pass the event to all plugins
  if (SysSystemDriver::HandleEvent (Event))
    return true;

  switch (Event.Type)
  {
    case csevBroadcast:
    {
      if (Event.Command.Code == cscmdContextResize)
      {
	FRAME_WIDTH = G2D->GetWidth();
	FRAME_HEIGHT = G2D->GetHeight();
	view->GetCamera ()->SetPerspectiveCenter (FRAME_WIDTH/2, FRAME_HEIGHT/2);
	if (wf)
	  wf->GetCamera ()->SetPerspectiveCenter (FRAME_WIDTH/2, FRAME_HEIGHT/2);
	break;
      }
    }
    case csevKeyDown:
    case csevKeyUp:
      eatkeypress (Event);
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

          System->G2D->SetMousePosition (FRAME_WIDTH / 2, FRAME_HEIGHT / 2);
          if (!first_time)
          {
          /*
            if(move_3d)
              view->GetCamera ()->RotateThis (VEC_ROT_RIGHT, ((float)( last_x - (FRAME_WIDTH / 2) )) / (FRAME_WIDTH*2) );
            else
              view->GetCamera ()->RotateOther (VEC_ROT_RIGHT, ((float)( last_x - (FRAME_WIDTH / 2) )) / (FRAME_WIDTH*2) );
            view->GetCamera ()->RotateThis (VEC_TILT_UP, -((float)( last_y - (FRAME_HEIGHT / 2) )) / (FRAME_HEIGHT*2) );
          */

            this->angle.y+=((float)(last_x - (FRAME_WIDTH / 2) )) / (FRAME_WIDTH*2);
            this->angle.x+=((float)(last_y - (FRAME_HEIGHT / 2) )) / (FRAME_HEIGHT*2)*(1-2*(int)inverse_mouse);
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
      return NULL;
  }
  else
  {
    closestMesh = NULL;
    closestZLocation = 32000;
  }

  // @@@ This routine ignores 2D meshes for the moment.
  for (meshIndex = 0; meshIndex < Sys->Engine->GetNumMeshObjects (); meshIndex++)
  {
    nextMesh = Sys->Engine->GetMeshObject (meshIndex);

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
