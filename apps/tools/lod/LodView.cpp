/*
    Copyright (C) 2010 by Jorrit Tyberghein, Eduardo Poyart

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
#include "LodView.h"

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

LodView::LodView (): lod_level(-1), prev_lod_level(0)
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
  int tris = 0;
  for (size_t i = 0; i < sprites.GetSize(); i++)
  {
    csRef<iMeshObject> mobj = sprites[i]->GetMeshObject();
    assert(mobj);

    csRef<iGeneralMeshState> mstate = scfQueryInterface<iGeneralMeshState>(mobj);
    assert(mstate);

    mstate->ForceProgLODLevel(lod_level);

    if (lod_level > -1)
    {
      csRef<iMeshFactoryWrapper> fact = sprites[i]->GetFactory();
      csRef<iMeshObjectFactory> fobj = fact->GetMeshObjectFactory();
      csRef<iGeneralFactoryState> fstate = scfQueryInterface<iGeneralFactoryState>(fobj);
      for (unsigned int submesh_index = 0; submesh_index < fstate->GetSubMeshCount(); submesh_index++)
      {
        csRef<iGeneralMeshSubMesh> submesh = fstate->GetSubMesh(submesh_index);
        if (submesh)
        {
          csRef<iGeneralFactorySubMesh> fsm = scfQueryInterface<iGeneralFactorySubMesh>(submesh);
          int num_sw = fsm->GetSlidingWindowSize();
          int s, e;
          fsm->GetSlidingWindow((lod_level > num_sw-1) ? num_sw-1 : lod_level, s, e);
          tris += (e - s) / 3;
        }
      }
    }
  }
  if (lod_level > -1)
    csPrintf("Level: %d Triangles: %d\n", lod_level, tris);
  else
    csPrintf("Level: auto\n");
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
    else if (code == 'l')
    {
      if (lod_level < num_lod_levels - 1)
      {
        lod_level++;
        UpdateLODLevel();
      }
    }
    else if (code == 'k')
    {
      if (lod_level > 0)
      {
        lod_level--;
        UpdateLODLevel();
      }
    }
    else if (code == 'a')
    {
      if (lod_level >= 0)
      {
        prev_lod_level = lod_level;
        lod_level = -1;
        UpdateLODLevel();
      }
      else
      {
        lod_level = prev_lod_level;
        UpdateLODLevel();
      }
    }
  }
  
  return false;
}

void LodView::Usage()
{
  csPrintf("LOD viewer\n");
  csPrintf("Usage:\n");
  csPrintf("lodview <filename> [-m=<num>]\n");
  csPrintf("  -m:   multiple sprites, large room (will show num^2 sprites)\n");
  csPrintf("When viewing the model:\n");
  csPrintf("  'k' / 'l':  increase/reduce LOD resolution\n");
  csPrintf("  'a':        switch to auto LOD\n");
}

bool LodView::OnInitialize (int argc, char* argv [])
{
  csRef<iCommandLineParser> cmdline = csQueryRegistry<iCommandLineParser>(GetObjectRegistry());
  filename = csString(cmdline->GetName());
  if (filename == "")
  {
    Usage();
    exit(1);
  }
  
  csString smultiple = cmdline->GetOption("m");
  if (smultiple == "")
  {
    use_multiple_sprites = false;
  }
  else
  {
    use_multiple_sprites = true;
    csScanStr(smultiple, "%d", &num_multiple);
    if (num_multiple < 1)
      num_multiple = 1;
  }
  
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
  CS::Utility::SmartChDir (vfs, filename);
  loading = tloader->LoadFileWait(vfs->GetCwd (), filename);
  
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
      csPrintf("No factories in file.\n");
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
    csPrintf("Could not find loaded mesh.\n");
    return;
  }
  
  loading.Invalidate();
}

bool LodView::SetupModules ()
{
  vfs = csQueryRegistry<iVFS> (object_reg);
  if (!vfs) return ReportError("Failed to locate VFS!");

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

  csRef<iMeshObjectFactory> fact = imeshfactw->GetMeshObjectFactory();
  assert(fact);
  csRef<iGeneralFactoryState> fstate = scfQueryInterface<iGeneralFactoryState>(fact);
  assert(fstate);
  num_lod_levels = fstate->GetNumProgLODLevels();

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
  if (use_multiple_sprites)
    CreateLargeRoom();
  else
    CreateSmallRoom();
}

void LodView::CreateSmallRoom()
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

void LodView::CreateLargeRoom()
{
  if (!loader->LoadTexture ("stone", "/lib/std/stone4.gif"))
    ReportError ("Error loading 'stone4' texture!");
  iMaterialWrapper* tm = engine->GetMaterialList ()->FindByName ("stone");
  room = engine->CreateSector ("room");
  using namespace CS::Geometry;
  DensityTextureMapper mapper (0.3f);
  TesselatedBox box (csVector3 (-50, 0, -50), csVector3 (50, 20, 50));
  box.SetLevel (3);
  box.SetMapper (&mapper);
  box.SetFlags (Primitives::CS_PRIMBOX_INSIDE);
  csRef<iMeshWrapper> walls = GeneralMeshBuilder::CreateFactoryAndMesh (
    engine, room, "walls", "walls_factory", &box);
  walls->GetMeshObject ()->SetMaterialWrapper (tm);
  csRef<iLight> light;
  iLightList* ll = room->GetLights ();
  light = engine->CreateLight (0, csVector3 (-10, 10, 0), 40, csColor (1, 1, 1));
  ll->Add (light);
  light = engine->CreateLight (0, csVector3 (10, 10, 0), 40, csColor (1, 1, 1));
  ll->Add (light);
  light = engine->CreateLight (0, csVector3 (0, 10, -10), 40, csColor (1, 1, 1));
  ll->Add (light);
  room->SetDynamicAmbientLight(csColor(0.5, 0.5, 0.5));
}

void LodView::CreateSprites ()
{
  if (use_multiple_sprites)
    CreateManySprites(num_multiple, num_multiple);
  else
    CreateManySprites(1, 1);
}

void LodView::CreateOneSprite(const csVector3& pos)
{
  int id = sprites.GetSize();
  char name[50];
  snprintf(name, 50, "MySprite%d", id);
  csRef<iMeshWrapper> sprite (engine->CreateMeshWrapper (imeshfactw, name, room, pos));
  csMatrix3 m; m.Identity ();
  sprite->GetMovable ()->SetTransform (m);
  sprite->GetMovable ()->UpdateMove ();
  sprites.Push(sprite);
}

void LodView::CreateManySprites(int rows, int cols)
{
  for (int j = -cols/2; j < (cols+1)/2; j++)
    for (int i = -rows/2; i < (rows+1)/2; i++)
      CreateOneSprite(csVector3(i*100.0/rows, 3.0, j*100.0/cols));
}

int main (int argc, char* argv[])
{
  return csApplicationRunner<LodView>::Run (argc, argv);
}
