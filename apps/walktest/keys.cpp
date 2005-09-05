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
  char* wordstart = keyname;
  char* dash = strchr (wordstart, '-');
  while (dash)
  {
    *dash = 0;
    if (!strcmp (wordstart, "shift")) map->shift = 1;
    else if (!strcmp (wordstart, "alt")) map->alt = 1;
    else if (!strcmp (wordstart, "ctrl")) map->ctrl = 1;
    else if (!strcmp (wordstart, "status")) map->need_status = 1;
    else Sys->Report (CS_REPORTER_SEVERITY_NOTIFY,
    	"Bad modifier '%s'!", wordstart);

    *dash = '-';
    wordstart = dash+1;
    dash = strchr (wordstart, '-');
  }

  if (!strcmp (wordstart, "tab")) map->key = CSKEY_TAB;
  else if (!strcmp (wordstart, "space")) map->key = ' ';
  else if (!strcmp (wordstart, "esc")) map->key = CSKEY_ESC;
  else if (!strcmp (wordstart, "enter")) map->key = CSKEY_ENTER;
  else if (!strcmp (wordstart, "bs")) map->key = CSKEY_BACKSPACE;
  else if (!strcmp (wordstart, "up")) map->key = CSKEY_UP;
  else if (!strcmp (wordstart, "down")) map->key = CSKEY_DOWN;
  else if (!strcmp (wordstart, "right")) map->key = CSKEY_RIGHT;
  else if (!strcmp (wordstart, "left")) map->key = CSKEY_LEFT;
  else if (!strcmp (wordstart, "pgup")) map->key = CSKEY_PGUP;
  else if (!strcmp (wordstart, "pgdn")) map->key = CSKEY_PGDN;
  else if (!strcmp (wordstart, "home")) map->key = CSKEY_HOME;
  else if (!strcmp (wordstart, "end")) map->key = CSKEY_END;
  else if (!strcmp (wordstart, "ins")) map->key = CSKEY_INS;
  else if (!strcmp (wordstart, "del")) map->key = CSKEY_DEL;
  else if (!strcmp (wordstart, "f1")) map->key = CSKEY_F1;
  else if (!strcmp (wordstart, "f2")) map->key = CSKEY_F2;
  else if (!strcmp (wordstart, "f3")) map->key = CSKEY_F3;
  else if (!strcmp (wordstart, "f4")) map->key = CSKEY_F4;
  else if (!strcmp (wordstart, "f5")) map->key = CSKEY_F5;
  else if (!strcmp (wordstart, "f6")) map->key = CSKEY_F6;
  else if (!strcmp (wordstart, "f7")) map->key = CSKEY_F7;
  else if (!strcmp (wordstart, "f8")) map->key = CSKEY_F8;
  else if (!strcmp (wordstart, "f9")) map->key = CSKEY_F9;
  else if (!strcmp (wordstart, "f10")) map->key = CSKEY_F10;
  else if (!strcmp (wordstart, "f11")) map->key = CSKEY_F11;
  else if (!strcmp (wordstart, "f12")) map->key = CSKEY_F12;
  /*
  else if (*(wordstart+0) != 0) Sys->Report (CS_REPORTER_SEVERITY_NOTIFY,
  	"Bad key '%s'!", wordstart);
  else if ((*wordstart >= 'A' && *wordstart <= 'Z') ||
    strchr ("!@#$%^&*()_+", *wordstart))
  {
    map->shift = 1;
    map->key = *wordstart;
  }
  */
  else
  {
    utf32_char key;
    size_t nameLen = strlen (wordstart);
    bool charValid;
    int encLen = csUnicodeTransform::UTF8Decode ((utf8_char*)wordstart, 
      nameLen, key, &charValid);
    if (!charValid || ((size_t)encLen < nameLen))
    {
      Sys->Report (CS_REPORTER_SEVERITY_NOTIFY, "Bad key '%s'!", wordstart);
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

void WalkTest::Strafe (float speed)
{
  if (move_3d) return;
  speed *= cfg_walk_maxspeed_multreal;
  desired_velocity.x = 140.0f * speed * cfg_walk_maxspeed
  	* cfg_walk_maxspeed_multreal;
}

void WalkTest::Step (float speed)
{
  if (move_3d) return;
  speed *= cfg_walk_maxspeed_multreal;
  desired_velocity.z = 140.0f * speed * cfg_walk_maxspeed
  	* cfg_walk_maxspeed_multreal;
}

void WalkTest::Jump ()
{
  velocity.y = 110.0f * cfg_jumpspeed;
  desired_velocity.y = 0.0f;
}

void WalkTest::Look (float speed)
{
  if (move_3d) return;
  desired_angle_velocity.x = 150.0f * speed * cfg_rotate_maxspeed;
}

void WalkTest::Rotate (float speed)
{
  if (move_3d) return;
  desired_angle_velocity.y = 100.0f * speed * cfg_rotate_maxspeed
  	* cfg_walk_maxspeed_multreal;
}

#undef WLK_ACCELERATE
#define WLK_ACCELERATE(c) \
  if (velocity. c < desired_velocity. c) \
  { \
    velocity. c += cfg_walk_accelerate * elapsed; \
    if (velocity. c > desired_velocity. c) velocity. c = desired_velocity. c; \
  } \
  else \
  { \
    velocity. c -= cfg_walk_accelerate * elapsed; \
    if (velocity. c < desired_velocity. c) velocity. c = desired_velocity. c; \
  }

#undef WLK_ROT_ACCELERATE
#define WLK_ROT_ACCELERATE(c) \
  if (angle_velocity. c < desired_angle_velocity. c) \
  { \
    angle_velocity. c += cfg_rotate_accelerate * elapsed; \
    if (angle_velocity. c > desired_angle_velocity. c) angle_velocity. c = desired_angle_velocity. c; \
  } \
  else \
  { \
    angle_velocity. c -= cfg_rotate_accelerate * elapsed; \
    if (angle_velocity. c < desired_angle_velocity. c) angle_velocity. c = desired_angle_velocity. c; \
  }

void WalkTest::InterpolateMovement ()
{
  float elapsed = vc->GetElapsedTicks () / 1000.0f;
  elapsed *= 1700.0f;

  WLK_ACCELERATE(x)
  WLK_ACCELERATE(y)
  WLK_ACCELERATE(z)

  WLK_ROT_ACCELERATE(x)
  WLK_ROT_ACCELERATE(y)
  WLK_ROT_ACCELERATE(z)
}

void WalkTest::RotateCam (float x, float y)
{
  csVector3 rot = collider_actor.GetRotation ();
  rot.x += x;
  rot.y += y;
  collider_actor.SetRotation (rot);
}

void WalkTest::imm_forward (float speed, bool slow, bool fast)
{
  if (slow)
    view->GetCamera ()->Move (speed * 0.01 * CS_VEC_FORWARD, collider_actor.HasCD ());
  else if (fast)
    view->GetCamera ()->Move (speed * 4.0 * CS_VEC_FORWARD, collider_actor.HasCD ());
  else
    view->GetCamera ()->Move (speed * 1.0 * CS_VEC_FORWARD, collider_actor.HasCD ());
}

void WalkTest::imm_backward (float speed, bool slow, bool fast)
{
  if (slow)
    view->GetCamera ()->Move (speed*.01*CS_VEC_BACKWARD, collider_actor.HasCD ());
  else if (fast)
    view->GetCamera ()->Move (speed*1.2*CS_VEC_BACKWARD, collider_actor.HasCD ());
  else
    view->GetCamera ()->Move (speed*.6*CS_VEC_BACKWARD, collider_actor.HasCD ());
}

void WalkTest::imm_left (float speed, bool slow, bool fast)
{
  if (slow)
    view->GetCamera ()->Move (speed * 0.01 * CS_VEC_LEFT, collider_actor.HasCD ());
  else if (fast)
    view->GetCamera ()->Move (speed * 4.0 * CS_VEC_LEFT, collider_actor.HasCD ());
  else
    view->GetCamera ()->Move (speed * 1.0 * CS_VEC_LEFT, collider_actor.HasCD ());
}

void WalkTest::imm_right (float speed, bool slow, bool fast)
{
  if (slow)
    view->GetCamera ()->Move (speed * 0.01 * CS_VEC_RIGHT, collider_actor.HasCD ());
  else if (fast)
    view->GetCamera ()->Move (speed * 4.0 * CS_VEC_RIGHT, collider_actor.HasCD ());
  else
    view->GetCamera ()->Move (speed * 1.0 * CS_VEC_RIGHT, collider_actor.HasCD ());
}

void WalkTest::imm_up (float speed, bool slow, bool fast)
{
  if (slow)
    view->GetCamera ()->Move (speed * 0.01 * CS_VEC_UP, collider_actor.HasCD ());
  else if (fast)
    view->GetCamera ()->Move (speed * 4.0 * CS_VEC_UP, collider_actor.HasCD ());
  else
    view->GetCamera ()->Move (speed * 1.0 * CS_VEC_UP, collider_actor.HasCD ());
}

void WalkTest::imm_down (float speed, bool slow, bool fast)
{
  if (slow)
    view->GetCamera ()->Move (speed * 0.01 * CS_VEC_DOWN, collider_actor.HasCD ());
  else if (fast)
    view->GetCamera ()->Move (speed * 4.0 * CS_VEC_DOWN, collider_actor.HasCD ());
  else
    view->GetCamera ()->Move (speed * 1.0 * CS_VEC_DOWN, collider_actor.HasCD ());
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
          csString buf;
          buf.Format ("%s %d", m->cmd, status);
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
        csString buf;
        buf.Format ("%s 0", m->cmd);
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
  csVector2 p (csMouseEventHelper::GetX(&Event), 
	       FRAME_HEIGHT-csMouseEventHelper::GetY(&Event));

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
    csRef<iObject> psobj = SCF_QUERY_INTERFACE (obj->GetMeshWrapper (),
    	iObject);
    Sys->Report (CS_REPORTER_SEVERITY_DEBUG, "Hit polygon '%s/%s'",
    	psobj ? psobj->GetName () : "<null>",
	ps ? ps->GetFactory ()->GetPolygonName (sel) : "<null>");
    //Dumper::dump (sel);
  }

  extern csVector2 coord_check_vector;
  coord_check_vector.x = csMouseEventHelper::GetX(&Event);
  coord_check_vector.y = FRAME_HEIGHT-csMouseEventHelper::GetY(&Event);
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

  screenPoint.x = csMouseEventHelper::GetX(&Event);
  screenPoint.y = csMouseEventHelper::GetY(&Event);
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
      if (csCommandEventHelper::GetCode(&Event) == cscmdCanvasHidden)
      {
	canvas_exposed = false;
      #ifdef CS_DEBUG
	Report (CS_REPORTER_SEVERITY_NOTIFY, "canvas hidden");
      #endif
	break;
      }
      else if (csCommandEventHelper::GetCode(&Event) == cscmdCanvasExposed)
      {
	canvas_exposed = true;
      #ifdef CS_DEBUG
	Report (CS_REPORTER_SEVERITY_NOTIFY, "canvas exposed");
      #endif
	break;
      }
      else if (csCommandEventHelper::GetCode(&Event) == cscmdContextResize)
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
	switch(csMouseEventHelper::GetButton(&Event)) {
	case 1:
	  MouseClick1Handler(Event); break;
	case 2:
	  MouseClick2Handler(Event); break;
	case 3:
	  MouseClick3Handler(Event); break;
	}
      }

    case csevMouseMove:
      // additional command by Leslie Saputra -> freelook mode.
      {
        static bool first_time = true;
        if (do_freelook)
        {
          int last_x, last_y;
          last_x = csMouseEventHelper::GetX(&Event);
          last_y = csMouseEventHelper::GetY(&Event);

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
      if (csMouseEventHelper::GetButton(&Event) == 1)
      {
        move_forward = false;
	Step (0);
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
