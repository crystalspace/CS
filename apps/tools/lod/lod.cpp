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

#include <iostream>
#include <vector>
using namespace std;

#include <crystalspace.h>
#include "LodGen.h"
#include "lod.h"

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

Lod::Lod (): lod_level(0)
{
  SetApplicationName ("CrystalSpace.Lod");
}

Lod::~Lod ()
{
}

void Lod::Frame ()
{
  // First get elapsed time from the virtual clock.
  csTicks elapsed_time = vc->GetElapsedTicks ();
  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.0) * (0.06 * 20);
  
  iCamera* c = view->GetCamera ();
  
  if (kbd->GetKeyState (CSKEY_SHIFT))
  {
    // If the user is holding down shift, the arrow keys will cause
    // the camera to strafe up, down, left or right from it's
    // current position.
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
    // left and right cause the camera to rotate on the global Y
    // axis; page up and page down cause the camera to rotate on the
    // _camera's_ X axis (more on this in a second) and up and down
    // arrows cause the camera to go forwards and backwards.
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
  
  // We now assign a new rotation transformation to the camera.  You
  // can think of the rotation this way: starting from the zero
  // position, you first rotate "rotY" radians on your Y axis to get
  // the first rotation.  From there you rotate "rotX" radians on the
  // your X axis to get the final rotation.  We multiply the
  // individual rotations on each axis together to get a single
  // rotation matrix.  The rotations are applied in right to left
  // order .
  csMatrix3 rot = csXRotMatrix3 (rotX) * csYRotMatrix3 (rotY);
  csOrthoTransform ot (rot, c->GetTransform ().GetOrigin ());
  c->SetTransform (ot);
  
  rm->RenderView (view);
}

void Lod::UpdateLODLevel()
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
  int s, e;
  fstate->GetSlidingWindow(lod_level, s, e);
  cout << "Level: " << lod_level << " Triangles: " << (e-s)/3 << endl;
}

bool Lod::OnKeyboard (iEvent& ev)
{
  // We got a keyboard event.
  csKeyEventType eventtype = csKeyEventHelper::GetEventType (&ev);
  if (eventtype == csKeyEventTypeDown)
  {
    // The user pressed a key (as opposed to releasing it).
    utf32_char code = csKeyEventHelper::GetCookedCode (&ev);
    if (code == CSKEY_ESC)
    {
      // The user pressed escape to exit the application.
      // The proper way to quit a Crystal Space application
      // is by broadcasting a csevQuit event. That will cause the
      // main runloop to stop. To do that we get the event queue from
      // the object registry and then post the event.
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
  }
  
  return false;
}


// 1
bool Lod::OnInitialize (int /*argc*/, char* /*argv*/ [])
{
  //PointTriangleDistanceUnitTests();
  
  // RequestPlugins() will load all plugins we specify. In addition
  // it will also check if there are plugins that need to be loaded
  // from the config system (both the application config and CS or
  // global configs). In addition it also supports specifying plugins
  // on the commandline.
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

  // Now we need to register the event handler for our application.
  // Crystal Space is fully event-driven. Everything (except for this
  // initialization) happens in an event.
  // Rather than simply handling all events, we subscribe to the
  // particular events we're interested in.
  csEventID events[] = {
    csevFrame (GetObjectRegistry ()),
    csevKeyboardEvent (GetObjectRegistry ()),
    CS_EVENTLIST_END
  };

  if (!RegisterQueue (GetObjectRegistry (), events))
    return ReportError ("Failed to set up event handler!");
  
  // Report success
  return true;
}

void Lod::OnExit ()
{
  // Shut down the event handlers we spawned earlier.
  printer.Invalidate ();
}

// 2
bool Lod::Application ()
{
  // Open the main system. This will open all the previously loaded plug-ins.
  // i.e. all windows will be opened.
  if (!OpenApplication (GetObjectRegistry ()))
    return ReportError ("Error opening system!");

  if (SetupModules ())
  {
    // This calls the default runloop. This will basically just keep
    // broadcasting process events to keep the game going.
    Run ();
  }

  return true;
}

/*
struct VertexAttributes
{
  csVector2 uv;
  csVector3 normal;
  csColor4 color;
  VertexAttributes(const csVector2& u, const csVector3& n, const csColor4& c): uv(u), normal(n), color(c) {}
};

class UniqueVertMesh
{
public:
  csArray<csVector3> vertices;
  csArray<csTriangle> triangles;
  csArray<VertexAttributes> attr;
protected:
  csArray<int> vert_map;
public:
  void AddVertUnique(const csVector3& v, const VertexAttributes& a)
  {
    static const float epsilon = 0.00001;
    int vindex = -1;
    for (unsigned int i = 0; i < vertices.GetSize(); i++)
    {
      if (fabs(v[0] - vertices[i][0]) < epsilon && fabs(v[1] - vertices[i][1]) < epsilon && fabs(v[2] - vertices[i][2]) < epsilon)
      {
        vindex = i;
        break;
      }
    }
    if (vindex == -1)
    {
      vertices.Push(v);
      attr.Push(a);
      vert_map.Push(vertices.GetSize() - 1);
    }
    else
    {
      vert_map.Push(vindex);
    }
  }
  
  void AddTriangleMapped(const csTriangle& t)
  {
    csTriangle new_tri;
    for (int i = 0; i < 3; i++)
      new_tri[i] = vert_map[t[i]];
    triangles.Push(new_tri);
  }  
};
*/

void Lod::CreateLODs(const char* filename)
{
  loading = tloader->LoadFileWait("/lev/lodtest/", filename);
  
  if (!loading->WasSuccessful())
  {
    printf("Loading not successful\n");
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
    /*
    int i;
    for (i = 0 ; i < factories->GetCount () ; i++)
    {
      iMeshFactoryWrapper* f = factories->Get (i);
      if (collection->IsParentOf (f->QueryObject ()))
      {
        imeshfactw = f;
        break;
      }
    }
    */
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
  
  /*
  if (!loader->LoadLibraryFile(filename))
    cout << "Error loading library file " << filename << endl;
  */
  
  csRef<iMeshObjectFactory> fact = imeshfactw->GetMeshObjectFactory();
  assert(fact);
  
  csRef<iGeneralFactoryState> fstate = scfQueryInterface<iGeneralFactoryState>(fact);
  assert(fstate);

  /*
  int nv = fstate->GetVertexCount();
  UniqueVertMesh m;
  const csVector3* fstate_vertices = fstate->GetVertices();
  const csVector2* fstate_uv = fstate->GetTexels();
  const csVector3* fstate_normals = fstate->GetNormals();
  const csColor4*  fstate_colors = fstate->GetColors();
  for (int i = 0; i < nv; i++)
    m.AddVertUnique(fstate_vertices[i], VertexAttributes(fstate_uv[i], fstate_normals[i], fstate_colors[i]));
  int nt = fstate->GetTriangleCount();
  csTriangle* fstate_triangles = fstate->GetTriangles();
  for (int i = 0; i < nt; i++)
    m.AddTriangleMapped(fstate_triangles[i]);
  LodGen lodgen(m.vertices, m.triangles);
  lodgen.GenerateLODs();
  
  fstate->SetVertexCount(0);
  for (unsigned int i = 0; i < m.vertices.GetSize(); i++)
    fstate->AddVertex(m.vertices[i], m.attr[i].uv, m.attr[i].normal, m.attr[i].color);
  */
  
  LodGen lodgen;
  lodgen.Init(fstate);
  lodgen.GenerateLODs();
  
  assert(lodgen.GetSlidingWindowCount() >= 2);
  fstate->SetTriangleCount(0);
  for (int i = 0; i < lodgen.GetTriangleCount(); i++)
  {
    cout << lodgen.GetTriangle(i)[0] << " " << lodgen.GetTriangle(i)[1] << " " << lodgen.GetTriangle(i)[2] << endl;
    fstate->AddTriangle(lodgen.GetTriangle(i));
  }
  
  fstate->ClearSlidingWindows();
  for (int i = 0; i < lodgen.GetSlidingWindowCount(); i++)
  {
    fstate->AddSlidingWindow(lodgen.GetSlidingWindow(i).start_index*3, lodgen.GetSlidingWindow(i).end_index*3);
  }
  
  loading.Invalidate();
}

bool Lod::SetupModules ()
{
  // Now get the pointer to various modules we need. We fetch them
  // from the object registry. The RequestPlugins() call we did earlier
  // registered all loaded plugins with the object registry.

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
  
  collection = engine->CreateCollection ("lod_region");
  
  // We need a View to the virtual world.
  view.AttachNew (new csView (engine, g3d));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  // We use the full window to draw the world.
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());
 
  CreateLODs("lodbarrel");
  //CreateLODs("genMesh.002");
  //CreateLODs("lodbox");
  //CreateLODs("genbment2_tables");
  //CreateLODs("simple");
  //CreateLODs("kwartz.lib");

  // Here we create our world.
  CreateRoom ();

  // Here we create our world.
  CreateSprites ();

  // Let the engine prepare all lightmaps for use and also free all images 
  // that were loaded for the texture manager.
  engine->Prepare ();

  // Now calculate static lighting for our geometry.
  using namespace CS::Lighting;
  SimpleStaticLighter::ShineLights (room, engine, 4);
  
  rm = engine->GetRenderManager ();

  // These are used store the current orientation of the camera
  rotY = rotX = 0;

  // Now we need to position the camera in our world.
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 5, -3));

  // We use some other "helper" event handlers to handle 
  // pushing our work into the 3D engine and rendering it
  // to the screen.
  printer.AttachNew (new FramePrinter (object_reg));

  return true;
}

void Lod::CreateRoom ()
{
  // Load the texture from the standard library.  This is located in
  // CS/data/standard.zip and mounted as /lib/std using the Virtual
  // File System (VFS) plugin.
  if (!loader->LoadTexture ("stone", "/lib/std/stone4.gif"))
    ReportError ("Error loading 'stone4' texture!");
  //iMaterialWrapper* tm = engine->GetMaterialList ()->FindByName ("stone");

  // We create a new sector called "room".
  room = engine->CreateSector ("room");

  // Creating the walls for our room.

  // First we make a primitive for our geometry.
  using namespace CS::Geometry;
  DensityTextureMapper mapper (0.3f);
  TesselatedBox box (csVector3 (-5, 0, -5), csVector3 (5, 20, 5));
  box.SetLevel (3);
  box.SetMapper (&mapper);
  box.SetFlags (Primitives::CS_PRIMBOX_INSIDE);

  // Now we make a factory and a mesh at once.
  /*
  csRef<iMeshWrapper> walls = GeneralMeshBuilder::CreateFactoryAndMesh (
    engine, room, "walls", "walls_factory", &box);
  walls->GetMeshObject ()->SetMaterialWrapper (tm);
   */

  // Now we need light to see something.
  csRef<iLight> light;
  iLightList* ll = room->GetLights ();

  light = engine->CreateLight (0, csVector3 (-3, 5, 0), 10, csColor (1, .8, .8));
  ll->Add (light);

  light = engine->CreateLight (0, csVector3 (3, 5,  0), 10, csColor (.8, .8, 1));
  ll->Add (light);

  light = engine->CreateLight (0, csVector3 (0, 5, -3), 10, csColor (.8, 1, .8));
  ll->Add (light);
}

void Lod::CreateSprites ()
{
  // Load a texture for our sprite.
  iTextureWrapper* txt = loader->LoadTexture ("spark", "/lib/std/spark.png");
  if (txt == 0)
    ReportError("Error loading texture!");
  
  // Create the sprite and add it to the engine.
  csRef<iMeshWrapper> sprite (engine->CreateMeshWrapper (
    imeshfactw, "MySprite", room,
    csVector3 (-3, 5, 3)));
  csMatrix3 m; m.Identity ();
  sprite->GetMovable ()->SetTransform (m);
  sprite->GetMovable ()->UpdateMove ();
  /*
  csRef<iSprite3DState> spstate (
    scfQueryInterface<iSprite3DState> (sprite->GetMeshObject ()));
  spstate->SetAction ("default");
   */
  //spstate->SetMixMode (CS_FX_SETALPHA (.5));

  // The following two calls are not needed since CS_ZBUF_USE and
  // Object render priority are the default but they show how you
  // can do this.
  sprite->SetZBufMode (CS_ZBUF_USE);
  sprite->SetRenderPriority (engine->GetObjectRenderPriority ());
    
  csRef<iMeshObjectFactory> fact = imeshfactw->GetMeshObjectFactory();
  assert(fact);
  
  csRef<iGeneralFactoryState> fstate = scfQueryInterface<iGeneralFactoryState>(fact);
  assert(fstate);
  
  num_lod_levels = fstate->GetSlidingWindowSize();
}

/*-------------------------------------------------------------------------*
 * Main function
 *-------------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  /* Runs the application. 
   *
   * csApplicationRunner<> is a small wrapper to support "restartable" 
   * applications (ie where CS needs to be completely shut down and loaded 
   * again). Simple1 does not use that functionality itself, however, it
   * allows you to later use "Simple.Restart();" and it'll just work.
   */
  return csApplicationRunner<Lod>::Run (argc, argv);
}
