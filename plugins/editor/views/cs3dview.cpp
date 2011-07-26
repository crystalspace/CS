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

#include "cs3dview.h"

#include "ieditor/context.h"
#include "ieditor/operator.h"

#include <wx/wx.h>


CS_PLUGIN_NAMESPACE_BEGIN(CSE)
{

SCF_IMPLEMENT_FACTORY (CS3DView);

BEGIN_EVENT_TABLE(CS3DView::View, wxPanel)
  EVT_SIZE(CS3DView::View::OnSize)
END_EVENT_TABLE()


CS3DView::CS3DView (iBase* parent)
 : scfImplementationType (this, parent), initializedColliderActor (false)
{
}

bool CS3DView::Initialize (iObjectRegistry* obj_reg)
{
  object_reg = obj_reg;
  
  viewManager = csQueryRegistry<CS::EditorApp::iViewManager> (object_reg);
  if (!viewManager)
    return false;

  editor = csQueryRegistry<CS::EditorApp::iEditor> (object_reg);
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
  window = new CS3DView::View (this, editor->GetWindow (), -1, wxPoint(0,0), wxSize(200,150));
  wxwin->SetParent (window);
  
  window->SetDropTarget (new MyDropTarget (this));
  
  // Add it to the view manager
  viewManager->AddView (this);
  
  view = csPtr< ::iView> (new csView (engine, g3d));
  view->SetAutoResize (false);
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  // Let editor know we're interested in map load events
  editor->AddMapListener (this);
  
  //iEventNameRegistry* name_reg = csEventNameRegistry::GetRegistry (object_reg);
  // Register the event handler
  csEventID events[] = {
    csevFrame (object_reg),
    csevMouseMove (object_reg, 0),
    csevMouseDown (object_reg, 0),
    csevKeyboardEvent (object_reg),
    csevInput (object_reg),
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

CS3DView::~CS3DView()
{
  q->RemoveListener (this);
  delete pump;
}

wxWindow* CS3DView::GetWindow ()
{
  return window;
}

const wxChar* CS3DView::GetCaption () const
{
  return wxT("3D View");
}

CS::EditorApp::ViewDockPosition CS3DView::GetDefaultDockPosition () const
{
  return CS::EditorApp::DockPositionCenter;
}

bool CS3DView::HandleEvent (iEvent& ev)
{
  iEventNameRegistry* name_reg = csEventNameRegistry::GetRegistry (object_reg);
  
  // Frame event
  if (ev.Name == csevFrame(name_reg))
  {
    ProcessFrame ();
    FinishFrame ();
    return false; //Let other handlers have a say too
  }
  
  bool mouse_down = ev.Name == csevMouseDown (name_reg, 0);
  if (mouse_down && csMouseEventHelper::GetButton (&ev) == 1)
  {
    printf("CS3DView::HandleEvent mouse_down\n");
    csRef<CS::EditorApp::iContext> context = csQueryRegistry<CS::EditorApp::iContext> (object_reg);
    context->SetCamera(view->GetCamera ());
    csRef<CS::EditorApp::iOperatorManager> operatorManager = csQueryRegistry<CS::EditorApp::iOperatorManager> (object_reg);
    operatorManager->Invoke("cs.editor.operator.select", &ev);
    return true;
  }
  if (CS_IS_KEYBOARD_EVENT(object_reg, ev)) 
  {
    if (csKeyEventHelper::GetCookedCode (&ev) == 'G')
    {
      printf("CS3DView::HandleEvent csKeyEventHelper\n");
      csRef<CS::EditorApp::iContext> context = csQueryRegistry<CS::EditorApp::iContext> (object_reg);
      context->SetCamera(view->GetCamera ());
      csRef<CS::EditorApp::iOperatorManager> operatorManager = csQueryRegistry<CS::EditorApp::iOperatorManager> (object_reg);
      operatorManager->Invoke("cs.editor.operator.move", &ev);
      return true;
    }
  }

  return false;
}

void CS3DView::MoveCamera ()
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

void CS3DView::ProcessFrame ()
{
  MoveCamera ();
  
  // Tell 3D driver we're going to display 3D things.
  if (!g3d->BeginDraw (CSDRAW_CLEARSCREEN | CSDRAW_3DGRAPHICS))
    return;

  // Tell the camera to render into the frame buffer.
  view->Draw ();
 
  if (!g3d->BeginDraw (CSDRAW_2DGRAPHICS))
    return;
  
  csTransform tr_w2c = view->GetCamera ()->GetTransform ();
  int fov = g3d->GetDriver2D ()->GetHeight ();
  
  csRef<CS::EditorApp::iContext> context = csQueryRegistry<CS::EditorApp::iContext> (object_reg);
  csWeakRefArray<iObject>::ConstIterator it = context->GetSelectedObjects ().GetIterator ();
  while (it.HasNext ())
  {
    iObject* obj = it.Next ();
    csRef<iSceneNode> node = scfQueryInterface<iSceneNode> (obj);
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

void CS3DView::FinishFrame ()
{
  g3d->FinishDraw ();
  g3d->Print (0);
}

void CS3DView::PushFrame ()
{
  vc->Advance();
  q->Process();
  wxYield();
}

void CS3DView::OnSize (wxSizeEvent& event)
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
/*
void CS3DView::OnDrop (wxCoord x, wxCoord y, iEditorObject* obj)
{
  printf ("CS3DView::OnDrop\n");

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
*/
void CS3DView::OnMapLoaded (const char* path, const char* filename)
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
  view = csPtr< ::iView> (new csView (engine, g3d));
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

void CS3DView::OnLibraryLoaded (const char* path, const char* filename, iCollection* collection)
{
}

}
CS_PLUGIN_NAMESPACE_END(CSE)

