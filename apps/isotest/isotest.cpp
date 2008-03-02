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
#include "isotest.h"
#include "cstool/csview.h"
#include "cstool/initapp.h"
#include "csutil/cmdhelp.h"
#include "csutil/cscolor.h"
#include "csutil/event.h"
#include "csutil/stringarray.h"
#include "csutil/sysfunc.h"
#include "iengine/camera.h"
#include "iengine/campos.h"
#include "iengine/engine.h"
#include "iengine/light.h"
#include "iengine/material.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "iengine/sector.h"
#include "iengine/texture.h"
#include "igraphic/imageio.h"
#include "imap/loader.h"
#include "imesh/object.h"
#include "imesh/sprite3d.h"
#include "imesh/skeleton.h"
#include "iutil/csinput.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "iutil/vfs.h"
#include "iutil/virtclk.h"
#include "ivaria/reporter.h"
#include "ivaria/stdrep.h"
#include "ivideo/fontserv.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/material.h"
#include "ivideo/texture.h"
#include "ivideo/txtmgr.h"
#include "imesh/genmesh.h"
#include "imesh/gmeshskel2.h"
#include "imesh/nskeleton.h"
#include "imesh/skelanim.h"

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

// The global pointer to isotest
IsoTest *isotest;

IsoTest::IsoTest (iObjectRegistry* object_reg)
{
  IsoTest::object_reg = object_reg;
  selboneid = 0;
  selbone = 0;
  skeleton = 0;
  myskel = 0;
  //manipmode = TRAN_ROTATE;
  //transaxis = AXIS_Z;

  feather = STAND;
  pfeather = STAND;
  feather_duration = 0.0f;
  pfeather_duration = 0.0f;

  current_view = 0;
  views[0].SetOrigOffset (csVector3 (-2, 2, -2)); // true isometric perspective.
  views[1].SetOrigOffset (csVector3 (-9, 9, -9)); // zoomed out.
  views[2].SetOrigOffset (csVector3 (4, 3, -4)); // diablo style perspective.
  views[3].SetOrigOffset (csVector3 (0, 4, -4)); // zelda style perspective.

  actor_is_walking = false;
}

IsoTest::~IsoTest ()
{
}

void IsoTest::SetupFrame ()
{
  static float currweight = 0.0f;
  if (!selbone)
    selbone = skeleton->GetBone (selboneid);

  // First get elapsed time from the virtual clock.
  csTicks elapsed_time = vc->GetElapsedTicks ();

  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.0) * (0.03 * 90);

  //if (kbd->GetModifierState (CSKEY_SHIFT_LEFT) 
  //  || kbd->GetModifierState (CSKEY_SHIFT_RIGHT))
  if (kbd->GetModifierState (CSKEY_SHIFT_RIGHT))
  {
    if (kbd->GetKeyState (CSKEY_RIGHT))
      views[current_view].angle += speed*15.f;
    if (kbd->GetKeyState (CSKEY_LEFT))
      views[current_view].angle -= speed*15.f;
    if (kbd->GetKeyState (CSKEY_UP))
      views[current_view].distance -= 0.25f*speed;
    if (kbd->GetKeyState (CSKEY_DOWN))
      views[current_view].distance += 0.25f*speed;
    SetupIsoView(views[current_view]);
  }
  else if (kbd->GetModifierState (CSKEY_SHIFT_LEFT))
  {
    float facing = 0.f; // in degrees
    bool moved = false;
    if (kbd->GetKeyState (CSKEY_RIGHT))
    {
      moved = true;
      actor->GetMovable ()->MovePosition (csVector3 (speed, 0, 0));
      facing = 270.f;
    }
    if (kbd->GetKeyState (CSKEY_LEFT))
    {
      moved = true;
      actor->GetMovable ()->MovePosition (csVector3 (-speed, 0, 0));
      facing = 90.f;
    }
    if (kbd->GetKeyState (CSKEY_UP))
    {
      moved = true;
      actor->GetMovable ()->MovePosition (csVector3 (0, 0, speed));
      facing = 0.f;
    }
    if (kbd->GetKeyState (CSKEY_DOWN))
    {
      moved = true;
      actor->GetMovable ()->MovePosition (csVector3 (0, 0, -speed));
      facing = 180.f;
    }

    if(kbd->GetKeyState (CSKEY_DOWN) && kbd->GetKeyState (CSKEY_LEFT))
      facing = 135;
    if(kbd->GetKeyState (CSKEY_DOWN) && kbd->GetKeyState (CSKEY_RIGHT))
      facing = 225;
    if(kbd->GetKeyState (CSKEY_UP) && kbd->GetKeyState (CSKEY_LEFT))
      facing = 45;
    if(kbd->GetKeyState (CSKEY_UP) && kbd->GetKeyState (CSKEY_RIGHT))
      facing = 315;

    if(moved)
    {
      csYRotMatrix3 r(facing*PI/180.0);
      actor->GetMovable ()->SetTransform(r);
    }
    // update animation state
    if(actor_is_walking && !moved)
    {
      skeleton->StopAll();
      skeleton->Execute("idle");
    }
    if(!actor_is_walking && moved)
    {
      skeleton->StopAll();
      skeleton->Execute("run");
    }
    actor_is_walking = moved;
  }
  else
  {
    if (kbd->GetKeyState (CSKEY_RIGHT))
    {
      selboneid++;
      if (selboneid > int(myskel->GetChildrenCount () - 1))
        selboneid = 0;
    }
    else if (kbd->GetKeyState (CSKEY_LEFT))
    {
      selboneid--;
      if (selboneid < 0)
        selboneid = myskel->GetChildrenCount () - 1;
    }
    else if (kbd->GetKeyState (CSKEY_UP))
    {
      currweight += 0.01;
      printf ("currweight: %f\n", currweight);
      blpen->SetWeight (punchid, currweight);
      /*Skeleton::iSkeleton::iBone* mybone = myskel->GetChild (selboneid);
      csQuaternion rot (mybone->GetRotation ());
      csVector3 axis;
      float angle;
      rot.GetAxisAngle (axis, angle);
      rot.SetAxisAngle (axis, angle + 0.1f);
      mybone->SetRotation (rot);*/
    }
    else if (kbd->GetKeyState (CSKEY_DOWN))
    {
      currweight -= 0.01;
      blpen->SetWeight (punchid, currweight);
      /*Skeleton::iSkeleton::iBone* mybone = myskel->GetChild (selboneid);
      csQuaternion rot (mybone->GetRotation ());
      csVector3 axis;
      float angle;
      rot.GetAxisAngle (axis, angle);
      rot.SetAxisAngle (axis, angle - 0.1f);
      mybone->SetRotation (rot);*/
    }

    if (kbd->GetKeyState ('w'))
    {
      if (feather != WALK)
      {
        feather = STAND_WALK;
      }
    }
    else
    {
      if (feather != STAND)
      {
        feather = WALK_STAND;
      }
    }

    if (kbd->GetKeyState ('p'))
    {
      if (pfeather == STAND)
      {
        pfeather = STAND_WALK;
        pfeather_duration = 0.0f;
        anim_punch->SetPlayCount (1);
      }
    }
    /*else
    {
      if (pfeather != STAND)
      {
        pfeather = WALK_STAND;
      }
    }*/

    /*if (kbd->GetKeyState ('p'))
    {
      anim_punch->SetPlayCount (1);
    }*/
    /*bool moved = kbd->GetKeyState ('a');
    if (kbd->GetKeyState (CSKEY_RIGHT))
    {
      selboneid++;
      if (selboneid > skeleton->GetBonesCount () - 1)
        selboneid = 0;
      selbone = skeleton->GetBone (selboneid);
    }
    else if (kbd->GetKeyState (CSKEY_LEFT))
    {
      selboneid--;
      if (selboneid < 0)
        selboneid = skeleton->GetBonesCount () - 1;
      selbone = skeleton->GetBone (selboneid);
    }
    else if (kbd->GetKeyState (CSKEY_UP))
    {
      csQuaternion &rot (selbone->GetQuaternion ());
      csVector3 axis;
      float angle;
      rot.GetAxisAngle (axis, angle);
      rot.SetAxisAngle (axis, angle + 0.1f);
      csReversibleTransform &tr (selbone->GetTransform ());
      tr.SetO2T (csMatrix3 (rot));
    }
    else if (kbd->GetKeyState (CSKEY_DOWN))
    {
      csQuaternion &rot (selbone->GetQuaternion ());
      csVector3 axis;
      float angle;
      rot.GetAxisAngle (axis, angle);
      rot.SetAxisAngle (axis, angle - 0.1f);
      csReversibleTransform &tr (selbone->GetTransform ());
      tr.SetO2T (csMatrix3 (rot));
    }*/
  }

  size_t timenow = vc->GetCurrentTicks (), delta = timenow - last_time;
  last_time = timenow;
  if (feather == STAND_WALK || feather == WALK_STAND)
  {
    feather_duration += delta;
  }
  switch (feather)
  {
    case (STAND):
      break;
    case (WALK):
      break;
    case (STAND_WALK):
      blend->SetWeight (walkid, feather_duration / 200.0f);
      blend->SetWeight (standid, 1 - (feather_duration / 200.0f));
      if (feather_duration > 200)
      {
        blend->SetWeight (walkid, 1.0f);
        blend->SetWeight (standid, 0.0f);
        feather = WALK;
        feather_duration = 0.0f;
      }
      break;
    case (WALK_STAND):
      blend->SetWeight (standid, feather_duration / 200.0f);
      blend->SetWeight (walkid, 1 - (feather_duration / 200.0f));
      if (feather_duration > 200)
      {
        blend->SetWeight (standid, 1.0f);
        blend->SetWeight (walkid, 0.0f);
        feather = STAND;
        feather_duration = 0.0f;
      }
      break;
    default:
      puts ("wtf? what piece of shit establishment is this?");
      break;
  }

  float timeuntilend = anim_punch->GetAnimationLength () - anim_punch->GetTimeline ();
  //printf ("endtime %f %f\n", timeuntilend, pfeather_duration);
  if (timeuntilend <= 200 && pfeather != STAND && pfeather != WALK_STAND)
  {
    //puts ("DEACTIVATING NOW!");
        //blpen->SetWeight (otherid, 1.0f);
        //blpen->SetWeight (punchid, 0.0f);
        pfeather = WALK_STAND;
        pfeather_duration = 0.0f;
  }
  if (pfeather == STAND_WALK || pfeather == WALK_STAND)
  {
    pfeather_duration += delta * anim_punch->GetPlaySpeed ();
  }
  switch (pfeather)
  {
    case (STAND):
      //puts ("nothing...");
      break;
    case (WALK):
      //puts ("punching...");
      break;
    case (STAND_WALK):
      //puts (" -> punch");
      blpen->SetWeight (punchid, pfeather_duration / 200.0f);
      //blpen->SetWeight (otherid, 1 - (pfeather_duration / 200.0f));
      if (pfeather_duration > 200)
      {
        //puts ("faseout");
        blpen->SetWeight (punchid, 1.0f);
        //blpen->SetWeight (otherid, 0.0f);
        pfeather = WALK;
        pfeather_duration = 0.0f;
      }
      break;
    case (WALK_STAND):
      //puts ("punch ->");
      //blpen->SetWeight (otherid, pfeather_duration / 200.0f);
      blpen->SetWeight (punchid, 1 - (pfeather_duration / 200.0f));
      if (pfeather_duration > 200)
      {
        //puts ("faseout");
        //blpen->SetWeight (otherid, 1.0f);
        blpen->SetWeight (punchid, 0.0f);
        pfeather = STAND;
        pfeather_duration = 0.0f;
      }
      break;
    default:
      puts ("wtf? what piece of shit establishment is this?");
      break;
  }

  // Make sure actor is constant distance above plane.
  csVector3 actor_pos = actor->GetMovable ()->GetPosition ();
  actor_pos.y += 10.0;	// Make sure we start beam high enough.
  csVector3 end_pos, isect;
  end_pos = actor_pos; end_pos.y -= 100.0;
  csHitBeamResult rc = plane->HitBeamObject (actor_pos, end_pos);
  actor_pos.y = rc.isect.y;

  actor->GetMovable ()->SetPosition (actor_pos);
  actor->GetMovable ()->UpdateMove ();

  // Move the light.
  actor_light->SetCenter (actor_pos+csVector3 (0, 2, -1));

  CameraIsoLookat(view->GetCamera(), views[current_view], actor_pos);
  
  // Tell 3D driver we're going to display 3D things.
  if (!g3d->BeginDraw (engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS))
    return;

  // Tell the camera to render into the frame buffer.
  view->Draw ();

  //DrawBone (1.0f, g3d);
  //puts ("DrawDebugBones");
  //skeleton->DrawDebugBones (g3d);
  //myskel->Update (vc->GetCurrentTicks () - last_time);
  //last_time = vc->GetCurrentTicks ();
  //skelgrave->Debug ();
  myskel->DrawDebugBones (g3d);

  /*bool error = false;
  for (size_t i = 0; i < myskel->GetChildrenCount (); i++)
  {
    const Skeleton::iSkeleton::iBone* b = myskel->GetChild (i);
    iSkeletonBone *bo = skeleton->GetBone (i);
    if (!bo)
      printf ("Erroroman skeleton is missing bone '%s'!\n", b->GetFactory ()->GetName ());
    const csQuaternion &q (b->GetRotation ()), &q0 (bo->GetQuaternion ());
    if (!(q.v - q0.v).IsZero () || q.w != q0.w)
    {
      printf ("Bones '%s' and Erroroman's bone '%s' have differing rotations!\n", b->GetFactory ()->GetName (), bo->GetName ());
      csQuaternion q = b->GetRotation ();
      printf ("Mine:       (%f, %f, %f, %f)\n", q.v.x, q.v.y, q.v.z, q.w);
      q = bo->GetQuaternion ();
      printf ("Erroromans: (%f, %f, %f, %f)\n", q.v.x, q.v.y, q.v.z, q.w);
      error = true;
    }
    const csVector3 &p (b->GetPosition ()), &p0 (bo->GetTransform ().GetOrigin ());
    if (!(p - p0).IsZero ())
    {
      printf ("Bones '%s' and Erroroman's bone '%s' have differing positions!\n", b->GetFactory ()->GetName (), bo->GetName ());
      printf ("Mine:       (%f, %f, %f)\n", p.x, p.y, p.z);
      printf ("Erroromans: (%f, %f, %f)\n", p0.x, p0.y, p0.z);
      error = true;
    }
    const csReversibleTransform &t (b->GetTransform ()), &to (bo->GetFullTransform ());
    if (!(t.GetOrigin () - to.GetOrigin ()).IsZero ())
    {
      printf ("Bones '%s' and Erroroman's bone '%s' have differing transform origins!\n", b->GetFactory ()->GetName (), bo->GetName ());
      const csVector3 &d (t.GetOrigin ()), &d0 (to.GetOrigin ());
      printf ("Mine:       (%f, %f, %f)\n", d.x, d.y, d.z);
      printf ("Erroromans: (%f, %f, %f)\n", d0.x, d0.y, d0.z);
      error = true;
    }
    if (false && t.GetO2T () != to.GetO2T ())
    {
      printf ("Bones '%s' and Erroroman's bone '%s' have differing transform matrices!\n", b->GetFactory ()->GetName (), bo->GetName ());
      csQuaternion q;
      q.SetMatrix (t.GetO2T());
      printf ("Mine:       (%f, %f, %f, %f)\n", q.v.x, q.v.y, q.v.z, q.w);
      q.SetMatrix (to.GetO2T());
      printf ("Erroromans: (%f, %f, %f, %f)\n", q.v.x, q.v.y, q.v.z, q.w);
      error = true;
    }
  }
  if (error)
    exit (-1);*/

  if (!g3d->BeginDraw (CSDRAW_2DGRAPHICS))
    return;

  csVector2 lpos(0,0);
  lpos = view->GetCamera()->Perspective(
    view->GetCamera()->GetTransform ().Other2This(csVector3 (-4.7f, 1.0f, 5.5f)));

  /*csQuaternion rot (selbone->GetQuaternion ());
  csVector3 pos (selbone->GetTransform ().GetOrigin ());
  csString line (selbone->GetName ());*/
  const Skeleton::iSkeleton::iBone* mybone = myskel->GetChild (selboneid);
  csQuaternion rot (mybone->GetRotation ());
  csVector3 pos (mybone->GetPosition ());
  csString line (mybone->GetFactory ()->GetName ());

  line += ", ";
  line += selboneid;

  csStringArray text;
  text.Push (line);

  line = "  rx: ";
  line += rot.v.x;
  text.Push (line);

  line = "  ry: ";
  line += rot.v.y;
  text.Push (line);

  line = "  rz: ";
  line += rot.v.z;
  text.Push (line);

  line = "  rw: ";
  line += rot.w;
  text.Push (line);

  text.Push ("");

  line = "  vx: ";
  line += pos.x;
  text.Push (line);

  line = "  vy: ";
  line += pos.y;
  text.Push (line);

  line = "  vz: ";
  line += pos.z;
  text.Push (line);

  int txtw=0, txth=0;
  font->GetMaxSize(txtw, txth);
  if(txth == -1) txth = 20;
  int white = g3d->GetDriver2D ()->FindRGB (255, 255, 255);
  int ypos = g3d->GetDriver2D ()->GetHeight () - txth*text.GetSize () - 1;
  for (size_t i = 0; i < text.GetSize (); i++)
  {
    const csString &str = text.Get (i);
    g3d->GetDriver2D ()->Write (font, 1, ypos, white, -1, str.GetData ());
    ypos += txth;
  }

  // display a helpful little text.
  //g3d->GetDriver2D ()->DrawBox((int)lpos.x-2,
  //  g3d->GetDriver2D ()->GetHeight()-(int)lpos.y-2,4,4,white);

  /*g3d->GetDriver2D ()->Write (font, 1, ypos, white, -1,
    "Isometric demo keys (esc to exit):");
  ypos += txth;
  g3d->GetDriver2D ()->Write (font, 1, ypos, white, -1, 
    "   arrow keys: move around");
  ypos += txth;
  g3d->GetDriver2D ()->Write (font, 1, ypos, white, -1, 
    "   shift+arrow keys: rotate/zoom camera");
  ypos += txth;
  g3d->GetDriver2D ()->Write (font, 1, ypos, white, -1, 
    "   tab key: cycle through camera presets");*/
}

void IsoTest::DrawBone (float length, iGraphics3D* g3d)
{
  csSimpleRenderMesh mesh;
  mesh.object2world.Identity();

  float w = length/10;
  csVector3 verts[16];
  verts[0].Set (-w, 0, -w);
  verts[1].Set (w, 0, -w);
  verts[2].Set (-w, 0, w);
  verts[3].Set (w, 0, w);
  verts[4].Set (-w, 0, -w);
  verts[5].Set (-w, 0, w);
  verts[6].Set (w, 0, -w);
  verts[7].Set (w, 0, w);

  verts[8].Set (-w, 0, -w);
  verts[9].Set (0, length, 0);
  verts[10].Set (w, 0, -w);
  verts[11].Set (0, length, 0);
  verts[12].Set (-w, 0, w);
  verts[13].Set (0, length, 0);
  verts[14].Set (w, 0, w);
  verts[15].Set (0, length, 0);

  mesh.vertices = verts;
  mesh.vertexCount = 16;
  mesh.meshtype = CS_MESHTYPE_LINES;
  g3d->DrawSimpleMesh (mesh, 0);
}

void IsoTest::FinishFrame ()
{
  g3d->FinishDraw ();
  g3d->Print (0);
}

bool IsoTest::HandleEvent (iEvent& ev)
{
  if (ev.Name == Process)
  {
    isotest->SetupFrame ();
    return true;
  }
  else if (ev.Name == FinalProcess)
  {
    isotest->FinishFrame ();
    return true;
  }
  else if (ev.Name == KeyboardDown)
  {
    utf32_char c = csKeyEventHelper::GetCookedCode (&ev);
    if (c == CSKEY_ESC)
    {
      csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (object_reg));
      if (q)
        q->GetEventOutlet()->Broadcast (csevQuit (object_reg));
      return true;
    }
    else if (c == CSKEY_TAB)
    {
      current_view++;
      if (current_view >= 4) current_view = 0;
    }
    else if (c == 'a')
    {
      csRef<iGeneralMeshState> spstate (
        scfQueryInterface<iGeneralMeshState> (actor->GetMeshObject ()));
      csRef<iGenMeshSkeletonControlState> animcontrol (
        scfQueryInterface<iGenMeshSkeletonControlState> (spstate->GetAnimationControl ()));
      iSkeleton* skeleton = animcontrol->GetSkeleton ();
      //skeleton->StopAll();
      skeleton->Execute("wave", 1.0f);
    }
    else if (c == 'q')
    {
      myskel->Update (30.0f);
    }
  }

  return false;
}

void IsoTest::CameraIsoLookat(csRef<iCamera> cam, const IsoView& isoview,
    const csVector3& lookat)
{
  // Let the camera look at the actor.
  // so the camera is set to look at 'actor_pos'
  //int isofactor = 50; // 98.3% isometric (=GetFovAngle()/180.0)
  //int isofactor = 100; // 99.2% isometric (=GetFovAngle()/180.0)
  int isofactor = 200; // 99.6% isometric (=GetFovAngle()/180.0)

  // set center and lookat
  csOrthoTransform& cam_trans = cam->GetTransform ();
  cam_trans.SetOrigin (lookat + float(isofactor)*isoview.camera_offset);
  cam_trans.LookAt (lookat-cam_trans.GetOrigin (), csVector3 (0, 1, 0));
  // set fov more isometric, could be done in initialisation once.
  cam->SetFOV (g3d->GetHeight()*isofactor, g3d->GetWidth());

  // due to moving the camera so far away, depth buffer accuracy is
  // impaired, repair that by using smaller coordinate system
  csOrthoTransform repair_trans = cam->GetTransform();
  repair_trans.SetT2O (repair_trans.GetT2O()/repair_trans.GetOrigin().Norm());
  cam->SetTransform (repair_trans);
}

void IsoTest::SetupIsoView(IsoView& isoview)
{
  // clamp
  if(isoview.angle < 0.f) isoview.angle += 360.f;
  if(isoview.angle > 360.f) isoview.angle -= 360.f;
  if(isoview.distance < 0.05f) isoview.distance = 0.05f;
  if(views[current_view].distance > 10.f) isoview.distance = 10.f;
  // setup
  csYRotMatrix3 r(isoview.angle * PI / 180.0);
  isoview.camera_offset = (r*isoview.original_offset)*isoview.distance;
}

bool IsoTest::IsoTestEventHandler (iEvent& ev)
{
  if (isotest)
    return isotest->HandleEvent (ev);
  else
    return false;
}

bool IsoTest::LoadMap ()
{
  // First disable the lighting cache. Our map uses stencil
  // lighting.
  engine->SetLightingCacheMode (0);

  // Set VFS current directory to the level we want to load.
  csRef<iVFS> VFS (csQueryRegistry<iVFS> (object_reg));
  VFS->ChDir ("/lev/isomap");
  // Load the level file which is called 'world'.
  if (!loader->LoadMapFile ("world"))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.isotest",
    	"Couldn't load level!");
    return false;
  }

  // Find the starting position in this level.
  csVector3 pos (0, 0, 0);
  if (engine->GetCameraPositions ()->GetCount () > 0)
  {
    // There is a valid starting position defined in the level file.
    iCameraPosition* campos = engine->GetCameraPositions ()->Get (0);
    room = engine->GetSectors ()->FindByName (campos->GetSector ());
    pos = campos->GetPosition ();
  }
  else
  {
    // We didn't find a valid starting position. So we default
    // to going to room called 'room' at position (0,0,0).
    room = engine->GetSectors ()->FindByName ("room");
  }
  if (!room)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.isotest",
      	"Can't find a valid starting position!");
    return false;
  }

  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (pos);

  iLightList* ll = room->GetLights ();
  actor_light = engine->CreateLight (0, csVector3 (1, 5, -1), 5,
    csColor (1, 1, 1));
  ll->Add (actor_light);

  csRef<iLight> statuelight = engine->CreateLight ("statuelight",
    csVector3 (-4.7f, 1.0f, 5.5f), 4, csColor(1.2f,0.2f,0.2f));
  statuelight->CreateNovaHalo (1278, 15, 0.3f);
  ll->Add (statuelight);

  plane = engine->FindMeshObject ("Plane");

  return true;
}

#include <iutil/stringarray.h>

bool IsoTest::CreateActor ()
{
  bool load_kirchdorfer = true;

  csRef<iVFS> vfs (csQueryRegistry<iVFS> (object_reg));
  vfs->PushDir ();
  if (load_kirchdorfer)
  {
    vfs->ChDir ("/lib/kirchdorfer");
    if (!loader->LoadLibraryFile ("library"))
    {
      csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
          "crystalspace.application.isotest",
          "Error loading kirchdorfer!");
      return false;
    }
  }
  else
  {
    vfs->ChDir ("/lib/kwartz");
    if (!loader->LoadLibraryFile ("kwartz.lib"))
    {
      csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
          "crystalspace.application.isotest",
          "Error loading kwartz!");
      return false;
    }
  }
  vfs->PopDir ();

  iMeshFactoryWrapper* imeshfact;
  if (load_kirchdorfer)
  {
    imeshfact = engine->FindMeshFactory ("genkirchdorfer");
    if (!imeshfact)
    {
      csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
          "crystalspace.application.isotest",
          "Error finding kirchdorfer factory!");
      return false;
    }
  }
  else
  {
    imeshfact = engine->FindMeshFactory ("kwartz_fact");
    if (!imeshfact)
    {
      csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
          "crystalspace.application.isotest",
          "Error finding kwartz factory!");
      return false;
    }
  }

  //csMatrix3 m; m.Identity ();
  //imeshfact->HardTransform (csReversibleTransform (m, csVector3 (0, -1, 0)));

  // Create the sprite and add it to the engine.
  actor = engine->CreateMeshWrapper (
    imeshfact, "MySprite", room, csVector3 (1, 1, -1));
  actor->GetMovable ()->UpdateMove ();
  csRef<iGeneralMeshState> spstate (
    scfQueryInterface<iGeneralMeshState> (actor->GetMeshObject ()));
  csRef<iGenMeshSkeletonControlState> animcontrol (
     
    scfQueryInterface<iGenMeshSkeletonControlState> (spstate->GetAnimationControl ()));
  skeleton = animcontrol->GetSkeleton ();
  skeleton->StopAll();
  skeleton->Execute("idle");

  // The following two calls are not needed since CS_ZBUF_USE and
  // Object render priority are the default but they show how you
  // can do this.
  actor->SetZBufMode (CS_ZBUF_USE);
  actor->SetRenderPriority (engine->GetObjectRenderPriority ());

#if 0
  Skeleton::iSkeletonFactory *s = skelgrave->CreateFactory ("amir");

  /*Skeleton::iSkeletonFactory::iBoneFactory *b[3];
  b[0] = s->CreateBoneFactory ("thigh");
  b[1] = s->CreateBoneFactory ("shin");
  b[2] = s->CreateBoneFactory ("foot");

  b[0]->AddChild (b[1]);
  b[1]->AddChild (b[2]);

  b[0]->SetDefaultRotation (csQuaternion (1.1f, 2.1f, 3.1415926535f, 4.0f));
  b[1]->SetDefaultRotation (csQuaternion (4.0f, 4.1f, 4.3f, 4.1001f));
  b[2]->SetDefaultRotation (csQuaternion (6.0f, 6.1f, 6.3f, 6.1001f));

  b[0]->SetDefaultPosition (csVector3 (1.9f, 2.9f, 3.999991415926535f));
  b[1]->SetDefaultPosition (csVector3 (4.9f, 4.19f, 4.93f));
  b[2]->SetDefaultPosition (csVector3 (6.9f, 6.19f, 6.93f));*/

  // create the static root bone
  Skeleton::iSkeletonFactory::iBoneFactory *srb;
  if (load_kirchdorfer)
  {
    srb = s->CreateBoneFactory ("StaticRootBone");
    srb->SetRotation (csQuaternion ());
    srb->SetPosition (csVector3 (0));
  }

  iSkeletonFactory* hrf = skeleton->GetFactory ();
  // create the bones
  for (size_t i = 0; i < hrf->GetBonesCount (); i++)
  {
    iSkeletonBoneFactory* hbf = hrf->GetBone (i);
    Skeleton::iSkeletonFactory::iBoneFactory *b = s->CreateBoneFactory (hbf->GetName ());
    csReversibleTransform &hrt = hbf->GetTransform ();
    csQuaternion rot_quat;
    rot_quat.SetMatrix (hrt.GetO2T());
    b->SetRotation (rot_quat);
    b->SetPosition (hrt.GetOrigin ());
  }
  // do the parenting now we know all bones are created
  for (size_t i = 0; i < hrf->GetBonesCount (); i++)
  {
    iSkeletonBoneFactory* hbf = hrf->GetBone (i);
    Skeleton::iSkeletonFactory::iBoneFactory *b = s->FindBoneFactoryByName (hbf->GetName ());
    // hack for making SRB
    if (load_kirchdorfer)
    {
      if (!strcmp (hbf->GetName (), "Neck") || !strcmp (hbf->GetName (), "Arm.R") || !strcmp (hbf->GetName (), "Arm.L"))
      {
        srb->AddChild (b);
      }
    }
    for (size_t j = 0; j < hbf->GetChildrenCount (); j++)
    {
      iSkeletonBoneFactory *hrchild = hbf->GetChild (j);
      Skeleton::iSkeletonFactory::iBoneFactory *c = s->FindBoneFactoryByName (hrchild->GetName ());
      b->AddChild (c);
    }
  }
  s->SetRootBone (0);
#endif
  myskel = skelgrave->CreateSkeleton ("amir", "human");
  skelgrave->Debug ();

/*  if (load_kirchdorfer)
  {
    if (!LoadKirchdorferAnim ())
      return false;
  }
  else
  {
    if (!LoadKwartzAnim ())
      return false;
  }*/

  if (load_kirchdorfer)
  {
  csRef<Skeleton::Animation::iAnimationFactoryLayer> animfactlay = myskel->GetFactory ()->GetAnimationFactoryLayer ();
  csRef<Skeleton::Animation::iAnimationLayer> animlay = myskel->GetAnimationLayer ();

  csRef<Skeleton::Animation::iAnimationFactory> animfact_stand = animfactlay->FindAnimationFactoryByName ("stand");
  csRef<Skeleton::Animation::iAnimation> anim_stand = animfact_stand->CreateAnimation ();
  anim_stand->SetPlayCount (-1);

  csRef<Skeleton::Animation::iAnimationFactory> animfact_walk = animfactlay->FindAnimationFactoryByName ("walk");
  csRef<Skeleton::Animation::iAnimation> anim_walk = animfact_walk->CreateAnimation ();
  anim_walk->SetPlayCount (-1);

  blend = animlay->CreateBlendNode ();
  //csRef<Skeleton::Animation::iMixingNode> animmix;
  //animmix = scfQueryInterface<Skeleton::Animation::iMixingNode> (anim_stand);
  standid = blend->AddNode (1.0f, anim_stand);
  //animmix = scfQueryInterface<Skeleton::Animation::iMixingNode> (anim_walk);
  walkid = blend->AddNode (0.0f, anim_walk);

  csRef<Skeleton::Animation::iAnimationFactory> animfact_punch = animfactlay->FindAnimationFactoryByName ("punch");
  anim_punch = animfact_punch->CreateAnimation ();
  anim_punch->SetPlayCount (0);
  //anim_punch->SetPlaySpeed (0.1f);

  csRef<Skeleton::Animation::iAccumulateNode> overwrite = animlay->CreateAccumulateNode ();
  blpen = overwrite;
  //animmix = scfQueryInterface<Skeleton::Animation::iMixingNode> (blend);
  otherid = overwrite->AddNode (1.0f, blend);
  //animmix = scfQueryInterface<Skeleton::Animation::iMixingNode> (anim_punch);
  punchid = overwrite->AddNode (0.0f, anim_punch);

  //animmix = scfQueryInterface<Skeleton::Animation::iMixingNode> (overwrite);
  animlay->SetRootMixingNode (overwrite);

  /*csRef<Skeleton::Animation::iAnimationFactory> animfact_run = animfactlay->FindAnimationFactoryByName ("jump");
  csRef<Skeleton::Animation::iAnimation> anim_run = animfact_run->CreateAnimation ();

  csRef<Skeleton::Animation::iAnimationFactory> animfact_idle = animfactlay->FindAnimationFactoryByName ("stand");
  csRef<Skeleton::Animation::iAnimation> anim_idle = animfact_idle->CreateAnimation ();
  anim_idle->SetPlayCount (-1);
  anim_idle->SetPlaySpeed (10.0f);

  csRef<Skeleton::Animation::iAnimationFactory> animfact_wave = animfactlay->FindAnimationFactoryByName ("walk");
  csRef<Skeleton::Animation::iAnimation> anim_wave = animfact_wave->CreateAnimation ();

  csRef<Skeleton::Animation::iBlendNode> blend = animlay->CreateBlendNode ();
  csRef<Skeleton::Animation::iMixingNode> animmix;
  animmix = scfQueryInterface<Skeleton::Animation::iMixingNode> (anim_run);
  blend->AddNode (1.0f, animmix);
  animmix = scfQueryInterface<Skeleton::Animation::iMixingNode> (anim_idle);
  blend->AddNode (4.0f, animmix);

  animmix = scfQueryInterface<Skeleton::Animation::iMixingNode> (blend);

  csRef<Skeleton::Animation::iOverwriteNode> overwrite = animlay->CreateOverwriteNode ();
  animmix = scfQueryInterface<Skeleton::Animation::iMixingNode> (blend);
  overwrite->AddNode (animmix);
  animmix = scfQueryInterface<Skeleton::Animation::iMixingNode> (anim_wave);
  overwrite->AddNode (animmix);

  animmix = scfQueryInterface<Skeleton::Animation::iMixingNode> (overwrite);
  animlay->SetRootMixingNode (animmix);*/
  }
  else
  {
  csRef<Skeleton::Animation::iAnimationFactoryLayer> animfactlay = myskel->GetFactory ()->GetAnimationFactoryLayer ();
  csRef<Skeleton::Animation::iAnimationLayer> animlay = myskel->GetAnimationLayer ();

  csRef<Skeleton::Animation::iAnimationFactory> animfact_run = animfactlay->FindAnimationFactoryByName ("run");
  csRef<Skeleton::Animation::iAnimation> anim_run = animfact_run->CreateAnimation ();
  anim_run->SetPlaySpeed (1.0f);
  anim_run->SetPlayCount (-1);

  csRef<Skeleton::Animation::iMixingNode> animmix;
  animmix = scfQueryInterface<Skeleton::Animation::iMixingNode> (anim_run);
  animlay->SetRootMixingNode (animmix);
  }

  return true;
}

bool IsoTest::Initialize ()
{
  if (!csInitializer::RequestPlugins (object_reg,
  	CS_REQUEST_VFS,
	CS_REQUEST_OPENGL3D,
	CS_REQUEST_ENGINE,
	CS_REQUEST_PLUGIN("crystalspace.font.server.multiplexer", iFontServer),
	"crystalspace.font.server.freetype2", "iFontServer.1", 
	  scfInterfaceTraits<iFontServer>::GetID(), 
	  scfInterfaceTraits<iFontServer>::GetVersion(),
	"crystalspace.font.server.default", "iFontServer.2", 
	  scfInterfaceTraits<iFontServer>::GetID(), 
	  scfInterfaceTraits<iFontServer>::GetVersion(),
	CS_REQUEST_PLUGIN("crystalspace.nskeleton.graveyard", Skeleton::iGraveyard),
	CS_REQUEST_FONTSERVER,
	CS_REQUEST_IMAGELOADER,
	CS_REQUEST_LEVELLOADER,
	CS_REQUEST_REPORTER,
	CS_REQUEST_REPORTERLISTENER,
	CS_REQUEST_END))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.isotest",
	"Can't initialize plugins!");
    return false;
  }

  Process = csevProcess (object_reg);
  FinalProcess = csevFinalProcess (object_reg);
  KeyboardDown = csevKeyboardDown (object_reg);

  const csEventID evs[] = {
    Process,
    FinalProcess,
    KeyboardDown,
    CS_EVENTLIST_END
  };
  if (!csInitializer::SetupEventHandler (object_reg, IsoTestEventHandler, evs))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.isotest",
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
  vc = csQueryRegistry<iVirtualClock> (object_reg);
  if (!vc)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.isotest",
	"Can't find the virtual clock!");
    return false;
  }
  last_time = vc->GetCurrentTicks ();

  skelgrave = csQueryRegistry<Skeleton::iGraveyard> (object_reg);
  if (!skelgrave)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.isotest",
	"Can't find the graveyard!");
    return false;
  }

  // Find the pointer to engine plugin
  engine = csQueryRegistry<iEngine> (object_reg);
  if (!engine)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.isotest",
	"No iEngine plugin!");
    return false;
  }

  loader = csQueryRegistry<iLoader> (object_reg);
  if (!loader)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.isotest",
    	"No iLoader plugin!");
    return false;
  }

  g3d = csQueryRegistry<iGraphics3D> (object_reg);
  if (!g3d)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.isotest",
    	"No iGraphics3D plugin!");
    return false;
  }

  kbd = csQueryRegistry<iKeyboardDriver> (object_reg);
  if (!kbd)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.isotest",
    	"No iKeyboardDriver plugin!");
    return false;
  }

  // Open the main system. This will open all the previously loaded plug-ins.
  if (!csInitializer::OpenApplication (object_reg))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.isotest",
    	"Error opening system!");
    return false;
  }

  view = csPtr<iView> (new csView (engine, g3d));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  font = g3d->GetDriver2D ()->GetFontServer()->LoadFont
    ("/fonts/ttf/Vera.ttf", 10);
  if(!font) // fallback
    font = g3d->GetDriver2D ()->GetFontServer()->LoadFont(CSFONT_LARGE);

  if (!LoadMap ()) return false;
  if (!CreateActor ()) return false;
  engine->Prepare ();

  return true;
}

void IsoTest::Start ()
{
  csDefaultRunLoop (object_reg);
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  iObjectRegistry* object_reg = csInitializer::CreateEnvironment (argc, argv);
  if (!object_reg) return -1;
  isotest = new IsoTest (object_reg);

  if (isotest->Initialize ())
    isotest->Start ();

  delete isotest;
  isotest = 0;

  csInitializer::DestroyApplication (object_reg);
  return 0;
}
