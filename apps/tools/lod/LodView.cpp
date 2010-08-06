/*
    Copyright (C) 2010 by Eduardo Poyart

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

#include <iostream>
#include <vector>
using namespace std;

#include <crystalspace.h>
#include "LodGen.h"
#include "LodView.h"

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

LodView::LodView (): lod_level(0)
{
  SetApplicationName ("CrystalSpace.Lod");
}

LodView::~LodView ()
{
}

void LodView::Frame ()
{
  csTicks elapsed_time = vc->GetElapsedTicks ();
  float speed = (elapsed_time / 1000.0) * (0.06 * 20);
  
  iCamera* c = view->GetCamera ();
  
  if (kbd->GetKeyState (CSKEY_SHIFT))
  {
    if (kbd->GetKeyState (CSKEY_RIGHT))
      c->Move (CS_VEC_RIGHT * 4 * speed);
    if (kbd->GetKeyState (CSKEY_LEFT))
      c->Move (CS_VEC_LEFT * 4 * speed);
    if (kbd->GetKeyState (CSKEY_UP))
      c->Move (CS_VEC_UP * 4 * speed);
    if (kbd->GetKeyState (CSKEY_DOWN))
      c->Move (CS_VEC_DOWN * 4 * speed);
  }
  else
  {
    if (kbd->GetKeyState (CSKEY_RIGHT))
      rotY += speed;
    if (kbd->GetKeyState (CSKEY_LEFT))
      rotY -= speed;
    if (kbd->GetKeyState (CSKEY_PGUP))
      rotX += speed;
    if (kbd->GetKeyState (CSKEY_PGDN))
      rotX -= speed;
    if (kbd->GetKeyState (CSKEY_UP))
      c->Move (CS_VEC_FORWARD * 4 * speed);
    if (kbd->GetKeyState (CSKEY_DOWN))
      c->Move (CS_VEC_BACKWARD * 4 * speed);
  }
  
  csMatrix3 rot = csXRotMatrix3 (rotX) * csYRotMatrix3 (rotY);
  csOrthoTransform ot (rot, c->GetTransform ().GetOrigin ());
  c->SetTransform (ot);
  
  rm->RenderView (view);
}

void LodView::UpdateLODLevel()
{
  csRef<iMeshWrapper> sprite = engine->FindMeshObject("MySprite");
  csRef<iMeshObject> mobj = sprite->GetMeshObject();
  assert(mobj);

  csRef<iGeneralMeshState> mstate = scfQueryInterface<iGeneralMeshState>(mobj);
  assert(mstate);

  mstate->ForceProgLODLevel(lod_level);
  
  csRef<iMeshFactoryWrapper> fact = sprite->GetFactory();
  csRef<iMeshObjectFactory> fobj = fact->GetMeshObjectFactory();
  csRef<iGeneralFactoryState> fstate = scfQueryInterface<iGeneralFactoryState>(fobj);
  int tris = 0;
  for (unsigned int submesh_index = 0; submesh_index < fstate->GetSubMeshCount(); submesh_index++)
  {
    csRef<iGeneralMeshSubMesh> submesh = fstate->GetSubMesh(submesh_index);
    if (submesh)
    {
      int num_sw = submesh->GetSlidingWindowSize();
      int s, e;
      submesh->GetSlidingWindow((lod_level > num_sw) ? num_sw : lod_level, s, e);
      tris += (e - s) / 3;
    }
  }
  cout << "Level: " << lod_level << " Triangles: " << tris << endl;
}

bool LodView::OnKeyboard (iEvent& ev)
{
  csKeyEventType eventtype = csKeyEventHelper::GetEventType (&ev);
  if (eventtype == csKeyEventTypeDown)
  {
    utf32_char code = csKeyEventHelper::GetCookedCode (&ev);
    if (code == CSKEY_ESC)
    {
      csRef<iEventQueue> q = 
        csQueryRegistry<iEventQueue> (GetObjectRegistry ());
      if (q.IsValid ()) q->GetEventOutlet ()->Broadcast (
        csevQuit (GetObjectRegistry ()));
    }
    else if (code == CSKEY_PGDN)
    {
      if (lod_level < num_lod_levels - 1)
      {
        lod_level++;
        UpdateLODLevel();
      }
    }
    else if (code == CSKEY_PGUP)
    {
      if (lod_level > 0)
      {
        lod_level--;
        UpdateLODLevel();
      }
    }
  }
  
  return false;
}

void LodView::Usage()
{
  cout << "LOD viewer" << endl;
  cout << "Usage:" << endl;
  cout << "lodview <filename>" << endl;
  cout << "When viewing the model: k reduces LOD level; l increases it." << endl; 
}

bool LodView::OnInitialize (int argc, char* argv [])
{
  if (argc < 2)
  {
    Usage();
    exit(1);
  }
  
  filename = argv[1];
  
  if (!csInitializer::RequestPlugins (GetObjectRegistry (),
    CS_REQUEST_VFS,
    CS_REQUEST_OPENGL3D,
    CS_REQUEST_ENGINE,
    CS_REQUEST_FONTSERVER,
    CS_REQUEST_IMAGELOADER,
    CS_REQUEST_LEVELLOADER,
    CS_REQUEST_REPORTER,
    CS_REQUEST_REPORTERLISTENER,
    CS_REQUEST_END))
    return ReportError ("Failed to initialize plugins!");

  csBaseEventHandler::Initialize (GetObjectRegistry ());
  csEventID events[] = {
    csevFrame (GetObjectRegistry ()),
    csevKeyboardEvent (GetObjectRegistry ()),
    CS_EVENTLIST_END
  };

  if (!RegisterQueue (GetObjectRegistry (), events))
    return ReportError ("Failed to set up event handler!");
  
  return true;
}

void LodView::OnExit ()
{
  printer.Invalidate ();
}

bool LodView::Application ()
{
  if (!OpenApplication (GetObjectRegistry ()))
    return ReportError ("Error opening system!");

  if (SetupModules ())
    Run ();

  return true;
}

void LodView::LoadLODs(const char* filename)
{
  loading = tloader->LoadFileWait("", filename);
  
  if (!loading->WasSuccessful())
  {
    printf("Loading not successful - file: %s\n", filename);
    loading.Invalidate();
    return;
  }
  
  csRef<iMeshWrapper> spritewrapper;
  
  if (!loading->GetResultRefPtr().IsValid())
  {
    // Library file. Find the first factory in our region.
    iMeshFactoryList* factories = engine->GetMeshFactories ();
    if (factories->GetCount() == 0)
    {
      cout << "No factories in file" << endl;
      return;
    }
    iMeshFactoryWrapper* f = factories->Get (0);
    imeshfactw = f;
  }
  else
  {
    imeshfactw = scfQueryInterface<iMeshFactoryWrapper> (loading->GetResultRefPtr());
    if(!imeshfactw)
    {
      spritewrapper = scfQueryInterface<iMeshWrapper> (loading->GetResultRefPtr());
      if (spritewrapper)
        imeshfactw = spritewrapper->GetFactory();
    }
  }
  
  if (!imeshfactw)
  {
    cout << "Could not find loaded mesh" << endl;
    return;
  }
  
  loading.Invalidate();
}

bool LodView::SetupModules ()
{
  g3d = csQueryRegistry<iGraphics3D> (GetObjectRegistry ());
  if (!g3d) return ReportError ("Failed to locate 3D renderer!");

  engine = csQueryRegistry<iEngine> (GetObjectRegistry ());
  if (!engine) return ReportError ("Failed to locate 3D engine!");

  vc = csQueryRegistry<iVirtualClock> (GetObjectRegistry ());
  if (!vc) return ReportError ("Failed to locate Virtual Clock!");

  kbd = csQueryRegistry<iKeyboardDriver> (GetObjectRegistry ());
  if (!kbd) return ReportError ("Failed to locate Keyboard Driver!");

  loader = csQueryRegistry<iLoader> (GetObjectRegistry ());
  if (!loader) return ReportError ("Failed to locate Loader!");

  tloader = csQueryRegistry<iThreadedLoader> (GetObjectRegistry());
  if (!tloader) return ReportError("Failed to locate threaded Loader!");
  
  view.AttachNew (new csView (engine, g3d));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());
 
  LoadLODs(filename);

  CreateRoom ();
  CreateSprites ();
  engine->Prepare ();
  using namespace CS::Lighting;
  SimpleStaticLighter::ShineLights (room, engine, 4);
  rm = engine->GetRenderManager ();
  rotY = rotX = 0;
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 5, -3));
  printer.AttachNew (new FramePrinter (object_reg));
  return true;
}

void LodView::CreateRoom ()
{
  if (!loader->LoadTexture ("stone", "/lib/std/stone4.gif"))
    ReportError ("Error loading 'stone4' texture!");
  iMaterialWrapper* tm = engine->GetMaterialList ()->FindByName ("stone");
  room = engine->CreateSector ("room");
  using namespace CS::Geometry;
  DensityTextureMapper mapper (0.3f);
  TesselatedBox box (csVector3 (-5, 0, -5), csVector3 (5, 20, 5));
  box.SetLevel (3);
  box.SetMapper (&mapper);
  box.SetFlags (Primitives::CS_PRIMBOX_INSIDE);
  csRef<iMeshWrapper> walls = GeneralMeshBuilder::CreateFactoryAndMesh (
    engine, room, "walls", "walls_factory", &box);
  walls->GetMeshObject ()->SetMaterialWrapper (tm);
  csRef<iLight> light;
  iLightList* ll = room->GetLights ();
  light = engine->CreateLight (0, csVector3 (-3, 5, 0), 15, csColor (1, 1, 1));
  ll->Add (light);
  light = engine->CreateLight (0, csVector3 (3, 5,  0), 15, csColor (1, 1, 1));
  ll->Add (light);
  light = engine->CreateLight (0, csVector3 (0, 5, -3), 15, csColor (1, 1, 1));
  ll->Add (light);
}

void LodView::CreateSprites ()
{
  iTextureWrapper* txt = loader->LoadTexture ("spark", "/lib/std/spark.png");
  if (txt == 0)
    ReportError("Error loading texture!");
  
  csRef<iMeshWrapper> sprite (engine->CreateMeshWrapper (
    imeshfactw, "MySprite", room,
    csVector3 (-3, 5, 3)));
  csMatrix3 m; m.Identity ();
  sprite->GetMovable ()->SetTransform (m);
  sprite->GetMovable ()->UpdateMove ();

  csRef<iMeshObjectFactory> fact = imeshfactw->GetMeshObjectFactory();
  assert(fact);
  csRef<iGeneralFactoryState> fstate = scfQueryInterface<iGeneralFactoryState>(fact);
  assert(fstate);
  num_lod_levels = fstate->GetNumProgLODLevels();
}

int main (int argc, char* argv[])
{
  return csApplicationRunner<LodView>::Run (argc, argv);
}
