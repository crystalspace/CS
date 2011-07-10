/*
    Copyright (C) 2007 by Seth Yastrov

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
#include "csutil/scf.h"

#include "cstool/enginetools.h"
#include "csgeom/plane3.h"
#include "csgeom/math3d.h"
#include "csutil/event.h"
#include "csutil/sysfunc.h"
#include "cstool/csview.h"
#include "iutil/csinput.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/virtclk.h"
#include "iengine/campos.h"
#include "iengine/sector.h"
#include "iengine/scenenode.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "iengine/engine.h"
#include "iengine/camera.h"
#include "imesh/object.h"
#include "imesh/objmodel.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/natwin.h"
#include "ivideo/wxwin.h"

#include "cs3dpanel.h"

#include <wx/wx.h>


CS_PLUGIN_NAMESPACE_BEGIN(CSE)
{

SCF_IMPLEMENT_FACTORY (CS3DPanel);

BEGIN_EVENT_TABLE(CS3DPanel::Panel, wxPanel)
  EVT_SIZE(CS3DPanel::Panel::OnSize)
END_EVENT_TABLE()


CS3DPanel::CS3DPanel (iBase* parent)
 : scfImplementationType (this, parent), initializedColliderActor (false)
{
}

bool CS3DPanel::Initialize (iObjectRegistry* obj_reg)
{
  object_reg = obj_reg;
  
  panelManager = csQueryRegistry<iPanelManager> (object_reg);
  if (!panelManager)
    return false;

  editor = csQueryRegistry<iEditor> (object_reg);
  if (!editor)
    return false;

  engine = csQueryRegistry<iEngine> (object_reg);
  if (!engine)
    return false;
  
  g3d = csQueryRegistry<iGraphics3D> (object_reg);
  if (!g3d)
    return false;

  vc = csQueryRegistry<iVirtualClock> (object_reg);
  if (!vc)
    return false;

  cdsys = csQueryRegistry<iCollideSystem> (obj_reg);
  if (!cdsys)
    return false;

  kbd = csQueryRegistry<iKeyboardDriver> (obj_reg);
  if (!kbd)
    return false;

  q = csQueryRegistry<iEventQueue> (object_reg);
  if (!q)
    return false;

  iGraphics2D* g2d = g3d->GetDriver2D ();
  g2d->AllowResize (true);

  csRef<iWxWindow> wxwin = scfQueryInterface<iWxWindow> (g2d);
  if( !wxwin )
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
              "crystalspace.application.editor",
              "Canvas is no iWxWindow plugin!");
    return false;
  }
  window = new CS3DPanel::Panel (this, editor->GetWindow (), -1, wxPoint(0,0), wxSize(200,150));
  wxwin->SetParent (window);
  
  window->SetDropTarget (new MyDropTarget (this));
  
  // Add it to the panel manager
  panelManager->AddPanel (this);
  
  view = csPtr<iView> (new csView (engine, g3d));
  view->SetAutoResize (false);
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  // Let editor know we're interested in map load events
  editor->AddMapListener (this);
  
  // Listen for selection events
  selection = editor->GetSelection ();
  selection->AddListener (this);
  objects = editor->GetObjects ();

  CS_INITIALIZE_EVENT_SHORTCUTS (object_reg);
  
  //iEventNameRegistry* name_reg = csEventNameRegistry::GetRegistry (object_reg);
  // Register the event handler
  csEventID events[] = {
    csevFrame (object_reg),
    csevMouseMove (object_reg, 0),
    csevMouseDown (object_reg, 0),
    csevKeyboardEvent (object_reg),
    CS_EVENTLIST_END
  };
  
  q->RegisterListener (this, events);
  
  /* This triggers a timer event every 20 milliseconds, which will yield
  a maximum framerate of 1000/20 = 50 FPS.  Obviously if it takes longer
  than 20 ms to render a frame the framerate will be lower :-)
  You may wish to tweak this for your own application.  Note that
  this also lets you throttle the CPU usage of your app, because
  the application will yield the CPU and wait for events in the
  time between when it completes rendering the current frame and
  the timer triggers the next frame.
  */
  
  pump = new Pump(this);
  pump->Start (20);
  
  return true;
}

CS3DPanel::~CS3DPanel()
{
  q->RemoveListener (this);
  delete pump;
}

wxWindow* CS3DPanel::GetWindow ()
{
  return window;
}

const wxChar* CS3DPanel::GetCaption () const
{
  return wxT("3D View");
}

PanelDockPosition CS3DPanel::GetDefaultDockPosition () const
{
  return DockPositionCenter;
}

bool CS3DPanel::HandleEvent (iEvent& ev)
{
  // Frame event
  if (ev.Name == Frame)
  {
    ProcessFrame ();
    FinishFrame ();
    return true;
  }

  // Mouse events
  iEventNameRegistry* name_reg = csEventNameRegistry::GetRegistry (object_reg);
  csRef<iMouseDriver> mousedrv = csQueryRegistry<iMouseDriver> (object_reg);

  int mouse_but;
  if (ev.Name == csevMouseMove (name_reg, 0))
  {
    if (mousedrv->GetLastButton (csmbLeft)) mouse_but = 1;
    else if (mousedrv->GetLastButton (csmbRight)) mouse_but = 2;
    else if (mousedrv->GetLastButton (csmbMiddle)) mouse_but = 3;
    else mouse_but = 0;
  }
  else
  {
    mouse_but = csMouseEventHelper::GetButton(&ev);
    mouse_but++;	// CS uses 0,1,2.
  }

  bool mouse_down = ev.Name == csevMouseDown (name_reg, 0);
  //bool mouse_up = ev.Name == csevMouseUp (name_reg, 0);
  int mouse_x = csMouseEventHelper::GetX(&ev);
  int mouse_y = csMouseEventHelper::GetY(&ev);
  if (ev.Name == csevMouseMove (name_reg, 0))
  {
    mousepos.Set (mouse_x, mouse_y);

    if ((editor->GetTransformStatus () == iEditor::MOVEX) ||
      (editor->GetTransformStatus () == iEditor::MOVEY) ||
      (editor->GetTransformStatus () == iEditor::MOVEZ))
    {
      csVector3 position(0);
      size_t selc = 0;
      for (csRef<iEditorObjectIterator> it = selection->GetIterator () ; it->HasNext (); )
      {
        iEditorObject* eo = it->Next ();
        csRef<iMeshWrapper> mesh = scfQueryInterface<iMeshWrapper> (eo->GetIBase ());
        if (mesh)
        {
          position += mesh->GetMovable ()->GetFullPosition ();
          selc++;
        }
      }
      if (selc > 0)
      {
        position = position/(int)selc;
  
        iCamera* cam = view->GetCamera ();
        csRef<iPerspectiveCamera> pcam =
          scfQueryInterface<iPerspectiveCamera> (cam);
        float sy;
        if (pcam)
          sy = pcam->GetShiftY ();
        else
          sy = 0.0f;
        csVector2 p (mouse_x, sy * 2.0f - mouse_y);
        csVector3 v = cam->InvPerspective (p, 1.0f);
        csVector3 end = cam->GetTransform ().This2Other(v);
        csVector3 origin = cam->GetTransform ().GetO2TTranslation ();
        csVector3 rel = (end - origin).Unit ();
        end = origin + (rel*10000.0f);
  
        //float m = (position.y - origin.y) / 10000.0f;
        //float newx = m * (end.x - origin.x) + origin.x - position.x;
  
        csVector3 axis (0, 0, 1);
        if (editor->GetTransformStatus () == iEditor::MOVEZ)
          axis.Set (1, 0, 0);

        csPlane3 plane (axis, 0);
        plane.SetOrigin (position);
        csVector3 isect;
        float dist;
        csIntersect3::SegmentPlane (origin, end, plane, isect, dist);
  
        csVector3 delta (0);
        switch (editor->GetTransformStatus ())
        {
          case (iEditor::MOVEX):
            delta.x = isect.x - position.x;
            break;
          case (iEditor::MOVEY):
            delta.y = isect.y - position.y;
            break;
          case (iEditor::MOVEZ):
            delta.z = isect.z - position.z;
            break;
          default:
            break;
        }
        for (csRef<iEditorObjectIterator> it = selection->GetIterator () ; it->HasNext (); )
        {
          iEditorObject* eo = it->Next ();
          csRef<iMeshWrapper> mesh = scfQueryInterface<iMeshWrapper> (eo->GetIBase ());
          if (mesh)
          {
            mesh->GetMovable ()->MovePosition (delta);
            mesh->GetMovable ()->UpdateMove ();
          }
        }
      }
    }

    opstartpos = mousepos;
  }
  iCamera* camera = view->GetCamera ();

  if (mouse_down && csMouseEventHelper::GetButton (&ev) == 1)
  {
    iEditor::TransformStatus trans = editor->GetTransformStatus ();
    if (trans != iEditor::NOTHING && trans != iEditor::MOVING && trans != iEditor::ROTATING && trans != iEditor::SCALING)
    {
      editor->SetTransformStatus (iEditor::MOVING);
    }
    else if (camera->GetSector ())
    {
      csScreenTargetResult result = csEngineTools::FindScreenTarget (
        csVector2 (mouse_x, mouse_y), 100000.0f, camera);
      if (result.mesh)
      {
        selection->Clear ();
        iEditorObject* obj = objects->FindObject (result.mesh);
        if (obj)
        {
          selection->Add (obj);
        }
      }
    }
  }

  if (ev.Name == csevMouseMove (name_reg, 0))
    return true;

  // Keyboard events
  if (CS_IS_KEYBOARD_EVENT(object_reg, ev)) 
  {
    if (ev.Name == csevKeyboardDown(object_reg))
    {
      iEditor::TransformStatus trans = editor->GetTransformStatus ();
      switch (trans)
      {
        case iEditor::MOVING:
        case iEditor::MOVEX:
        case iEditor::MOVEY:
        case iEditor::MOVEZ:
          if (csKeyEventHelper::GetCookedCode (&ev) == 'X')
            editor->SetTransformStatus (iEditor::MOVEX);
          else if (csKeyEventHelper::GetCookedCode (&ev) == 'Y')
            editor->SetTransformStatus (iEditor::MOVEY);
          else if (csKeyEventHelper::GetCookedCode (&ev) == 'Z')
            editor->SetTransformStatus (iEditor::MOVEZ);
          else
            break;
          opstartpos.Set (mousepos);
          break;
        default:
	  return false;
          break;
      }

      return true;
    }
    else if (ev.Name == csevKeyboardDown(object_reg))
    {
      editor->SetTransformStatus (iEditor::NOTHING);
      return true;
    }
  }
  
  return false;
}

void CS3DPanel::MoveCamera ()
{
  if (!initializedColliderActor)
    return;
  
  // First get elapsed time from the virtual clock.
  csTicks elapsed_time = vc->GetElapsedTicks ();

  csVector3 obj_move (0);
  csVector3 obj_rotate (0);

  if (kbd->GetKeyState (CSKEY_SHIFT))
  {
    // If the user is holding down shift, the arrow keys will cause
    // the camera to strafe up, down, left or right from it's
    // current position.
    if (kbd->GetKeyState (CSKEY_RIGHT))
      obj_move = CS_VEC_RIGHT * 3.0f;
    if (kbd->GetKeyState (CSKEY_LEFT))
      obj_move = CS_VEC_LEFT * 3.0f;
    if (kbd->GetKeyState (CSKEY_UP))
      obj_move = CS_VEC_UP * 3.0f;
    if (kbd->GetKeyState (CSKEY_DOWN))
      obj_move = CS_VEC_DOWN * 3.0f;
  }
  else
  {
    // left and right cause the camera to rotate on the global Y
    // axis; page up and page down cause the camera to rotate on the
    // _camera's_ X axis (more on this in a second) and up and down
    // arrows cause the camera to go forwards and backwards.
    if (kbd->GetKeyState (CSKEY_RIGHT))
      obj_rotate.Set (0, 1, 0);
    if (kbd->GetKeyState (CSKEY_LEFT))
      obj_rotate.Set (0, -1, 0);
    if (kbd->GetKeyState (CSKEY_PGUP))
      obj_rotate.Set (1, 0, 0);
    if (kbd->GetKeyState (CSKEY_PGDN))
      obj_rotate.Set (-1, 0, 0);
    if (kbd->GetKeyState (CSKEY_UP))
      obj_move = CS_VEC_FORWARD * 3.0f;
    if (kbd->GetKeyState (CSKEY_DOWN))
      obj_move = CS_VEC_BACKWARD * 3.0f;
  }

  collider_actor.Move (float (elapsed_time) / 1000.0f, 1.0f,
    	obj_move, obj_rotate);

}

void CS3DPanel::ProcessFrame ()
{
  MoveCamera ();
  
  // Tell 3D driver we're going to display 3D things.
  if (!g3d->BeginDraw (CSDRAW_CLEARSCREEN | CSDRAW_3DGRAPHICS))
    return;

  // Tell the camera to render into the frame buffer.
  view->Draw ();
  
  // draw helpers here
  csArray<csSimpleRenderMesh>* helperm = editor->GetHelperMeshes ();
  if (helperm && editor->GetTransformStatus () != iEditor::NOTHING)
  {
    csVector3 position(0);
    size_t selc = 0;
    for (csRef<iEditorObjectIterator> it = selection->GetIterator () ; it->HasNext (); )
    {
      iEditorObject* eo = it->Next ();
      csRef<iMeshWrapper> mesh = scfQueryInterface<iMeshWrapper> (eo->GetIBase ());
      if (mesh)
      {
        position += mesh->GetMovable ()->GetFullPosition ();
        selc++;
      }
    }
    if (selc > 0)
    {
      position = position/(int)selc;
      iEditor::TransformStatus trans = editor->GetTransformStatus ();
      switch (trans)
      {
        case iEditor::MOVING:
          for (csArray<csSimpleRenderMesh>::Iterator it = helperm->GetIterator () ; it.HasNext () ; )
          {
            csSimpleRenderMesh& helper = it.Next ();
            helper.object2world.SetOrigin (position);
            g3d->DrawSimpleMesh (helper, 0);
          }
          break;
        case iEditor::MOVEX:
        {
          helperm->Get (1).object2world.SetOrigin (position);
          g3d->DrawSimpleMesh (helperm->Get(1), 0);
          break;
        }
        case iEditor::MOVEY:
        {
          helperm->Get (2).object2world.SetOrigin (position);
          g3d->DrawSimpleMesh (helperm->Get(2), 0);
          break;
        }
        case iEditor::MOVEZ:
        {
          helperm->Get (0).object2world.SetOrigin (position);
          g3d->DrawSimpleMesh (helperm->Get(0), 0);
          break;
        }
        default:
          break;
      }
    }
  }

  if (!g3d->BeginDraw (CSDRAW_2DGRAPHICS))
    return;
  
  csTransform tr_w2c = view->GetCamera ()->GetTransform ();
  int fov = g3d->GetDriver2D ()->GetHeight ();

  csRef<iEditorObjectIterator> it = selection->GetIterator ();
  while (it->HasNext ())
  {
    iEditorObject* obj = it->Next ();
    csRef<iSceneNode> node = scfQueryInterface<iSceneNode> (obj->GetIBase ());
    if (!node)
      continue;
    
    iMeshWrapper* mesh = node->QueryMesh ();
    if (!mesh)
      continue;
    
    iMovable* mov = mesh->GetMovable ();
    csReversibleTransform tr_o2c = tr_w2c / mov->GetFullTransform ();
    
    int bbox_color = g3d->GetDriver2D ()->FindRGB (128, 255, 128);
    const csBox3& bbox = mesh->GetMeshObject ()
        ->GetObjectModel ()->GetObjectBoundingBox ();
    csVector3 vxyz = tr_o2c * bbox.GetCorner (CS_BOX_CORNER_xyz);
    csVector3 vXyz = tr_o2c * bbox.GetCorner (CS_BOX_CORNER_Xyz);
    csVector3 vxYz = tr_o2c * bbox.GetCorner (CS_BOX_CORNER_xYz);
    csVector3 vxyZ = tr_o2c * bbox.GetCorner (CS_BOX_CORNER_xyZ);
    csVector3 vXYz = tr_o2c * bbox.GetCorner (CS_BOX_CORNER_XYz);
    csVector3 vXyZ = tr_o2c * bbox.GetCorner (CS_BOX_CORNER_XyZ);
    csVector3 vxYZ = tr_o2c * bbox.GetCorner (CS_BOX_CORNER_xYZ);
    csVector3 vXYZ = tr_o2c * bbox.GetCorner (CS_BOX_CORNER_XYZ);
    g3d->DrawLine (vxyz, vXyz, fov, bbox_color);
    g3d->DrawLine (vXyz, vXYz, fov, bbox_color);
    g3d->DrawLine (vXYz, vxYz, fov, bbox_color);
    g3d->DrawLine (vxYz, vxyz, fov, bbox_color);
    g3d->DrawLine (vxyZ, vXyZ, fov, bbox_color);
    g3d->DrawLine (vXyZ, vXYZ, fov, bbox_color);
    g3d->DrawLine (vXYZ, vxYZ, fov, bbox_color);
    g3d->DrawLine (vxYZ, vxyZ, fov, bbox_color);
    g3d->DrawLine (vxyz, vxyZ, fov, bbox_color);
    g3d->DrawLine (vxYz, vxYZ, fov, bbox_color);
    g3d->DrawLine (vXyz, vXyZ, fov, bbox_color);
    g3d->DrawLine (vXYz, vXYZ, fov, bbox_color);
  }
}

void CS3DPanel::FinishFrame ()
{
  g3d->FinishDraw ();
  g3d->Print (0);
}

void CS3DPanel::PushFrame ()
{
  vc->Advance();
  q->Process();
  wxYield();
}

void CS3DPanel::OnSize (wxSizeEvent& event)
{
  if (!g3d.IsValid () || !g3d->GetDriver2D () || !view.IsValid ())
    return;
  
  wxSize size = event.GetSize();
  iGraphics2D* g2d = g3d->GetDriver2D ();
  
  // Also resize the wxWindow
  csRef<iWxWindow> wxwin = scfQueryInterface<iWxWindow> (g2d);
  if (!wxwin->GetWindow ())
  {
    g2d->Resize (size.x, size.y);
    return;
  }
  
  wxwin->GetWindow()->SetSize (size);
  
  // Update the view ratio
  view->GetPerspectiveCamera ()->SetFOV ((float) (size.y) / (float) (size.x), 1.0f);
  view->SetRectangle (0, 0, size.x, size.y);

  event.Skip();
}

void CS3DPanel::OnDrop (wxCoord x, wxCoord y, iEditorObject* obj)
{
  printf ("CS3DPanel::OnDrop\n");

  iCamera* camera = view->GetCamera ();
    
  if (camera->GetSector ())
  {
    csScreenTargetResult result = csEngineTools::FindScreenTarget (
      csVector2 (x, y), 100000.0f, camera);
      
    csRef<iMeshFactoryWrapper> meshFact = scfQueryInterface<iMeshFactoryWrapper> (obj->GetIBase ());
  
    csRef<iMeshWrapper> mesh = engine->CreateMeshWrapper (meshFact, obj->GetName(),
							  result.mesh->GetMovable ()->GetSectors()->Get(0), result.isect);
    
    if (!mesh)
      return;
  
    wxBitmap* meshBmp = new wxBitmap (wxBITMAP(meshIcon));
    csRef<iEditorObject> editorObject (editor->CreateEditorObject (mesh, meshBmp));
    objects->Add (editorObject);
    
    delete meshBmp;
  }
}

void CS3DPanel::OnMapLoaded (const char* path, const char* filename)
{
  // If there are no sectors then invalidate the camera
  if (!engine->GetSectors ()->GetCount ())
  {
    view->GetCamera ()->SetSector (nullptr);
    initializedColliderActor = false;

    return;
  }

  // Move the camera to the starting sector/position
  csRef<iSector> room;
  csVector3 pos;
  
  if (engine->GetCameraPositions ()->GetCount () > 0)
  {
    // There is a valid starting position defined in the level file.
    iCameraPosition* campos = engine->GetCameraPositions ()->Get (0);
    room = engine->GetSectors ()->FindByName (campos->GetSector ());
    pos = campos->GetPosition ();
  }

  if (!room)
  {
    // We didn't find a valid starting position. So we default
    // to going to the sector called 'room', or the first sector, at position (0,0,0).
    room = engine->GetSectors ()->FindByName ("room");
    if (!room) room = engine->GetSectors ()->Get (0);
    pos = csVector3 (0, 0, 0);
  }

  iGraphics2D* g2d = g3d->GetDriver2D ();
  
  view.Invalidate ();
  view = csPtr<iView> (new csView (engine, g3d));
  view->SetAutoResize (false);  
  view->GetPerspectiveCamera ()->SetFOV ((float) (g2d->GetHeight ()) / (float) (g2d->GetWidth ()), 1.0f);
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (pos);

  // Initialize collision objects for all loaded objects.
  csColliderHelper::InitializeCollisionWrappers (cdsys, engine);
  
  // Initialize our collider actor.
  collider_actor.SetCollideSystem (cdsys);
  collider_actor.SetEngine (engine);
  csVector3 legs (.2f, .3f, .2f);
  csVector3 body (.2f, 1.2f, .2f);
  csVector3 shift (0, -1, 0);
  collider_actor.InitializeColliders (view->GetCamera (),
                                      legs, body, shift);
  initializedColliderActor = true;

  // Put back the focus on the window
  csRef<iWxWindow> wxwin = scfQueryInterface<iWxWindow> (g3d->GetDriver2D ());
  if (wxwin->GetWindow ())
    wxwin->GetWindow ()->SetFocus ();
}

void CS3DPanel::OnLibraryLoaded (const char* path, const char* filename, iCollection* collection)
{
}

void CS3DPanel::OnObjectAdded (iObjectList* list, iEditorObject* obj)
{
  UpdateSelection ();
}

void CS3DPanel::OnObjectRemoved (iObjectList* list, iEditorObject* obj)
{
  UpdateSelection ();
}

void CS3DPanel::OnObjectsCleared (iObjectList* list)
{
  UpdateSelection ();
}

void CS3DPanel::UpdateSelection ()
{
  /// TODO: Is this really needed?
}

}
CS_PLUGIN_NAMESPACE_END(CSE)

