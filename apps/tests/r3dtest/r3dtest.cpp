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
#include "cssys/sysfunc.h"
#include "iutil/vfs.h"
#include "csgeom/polyclip.h"
#include "csgeom/transfrm.h"
#include "csutil/cscolor.h"
#include "cstool/csview.h"
#include "cstool/initapp.h"
#include "iengine/camera.h"
#include "iengine/engine.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "iengine/sector.h"
#include "igeom/clip2d.h"
#include "iutil/eventq.h"
#include "iutil/event.h"
#include "iutil/objreg.h"
#include "iutil/csinput.h"
#include "iutil/virtclk.h"
#include "imesh/thing/polygon.h"
#include "imesh/thing/thing.h"
#include "imesh/object.h"
#include "ivideo/graph2d.h"
#include "ivideo/txtmgr.h"
#include "ivideo/texture.h"
#include "ivideo/material.h"
#include "ivideo/fontserv.h"
#include "igraphic/imageio.h"
#include "imap/parser.h"
#include "ivaria/reporter.h"
#include "ivaria/stdrep.h"
#include "csutil/cmdhelp.h"
#include "ivideo/render3d.h"
#include "ivideo/rndbuf.h"
#include "imesh/terrfunc.h"
#include "ivideo/shader/shader.h"
//#include "ilight/testlt.h"

/*#include "csengine/material.h"
#include "csengine/texture.h"*/

#include "r3dtest.h"

CS_IMPLEMENT_APPLICATION

class csTestMesh : public iStreamSource
{
private:
  csRef<iRender3D> r3d;
  csRef<iRenderBuffer> vertices;
  csRef<iRenderBuffer> indices;
  csRef<iRenderBuffer> texcoords;

  csStringID vertices_name, indices_name, texcoords_name;

public:

  SCF_DECLARE_IBASE;

  csTestMesh (iRender3D* r3d)
  {
    SCF_CONSTRUCT_IBASE (NULL)

    csTestMesh::r3d = r3d;

    vertices = r3d->GetBufferManager ()->GetBuffer (
      sizeof (csVector3)*8, CS_BUF_STATIC);
    texcoords = r3d->GetBufferManager ()->GetBuffer (
      sizeof (csVector2)*8, CS_BUF_STATIC);
    indices = r3d->GetBufferManager ()->GetBuffer (
      sizeof (unsigned int)*36, CS_BUF_INDEX);

    csVector3* vbuf = (csVector3*)vertices->Lock(iRenderBuffer::CS_BUF_LOCK_NORMAL);
    vbuf[0] = csVector3 (-1,  1, -1);
    vbuf[1] = csVector3 ( 1,  1, -1);
    vbuf[2] = csVector3 (-1,  1,  1);
    vbuf[3] = csVector3 ( 1,  1,  1);
    vbuf[4] = csVector3 (-1, -1, -1);
    vbuf[5] = csVector3 ( 1, -1, -1);
    vbuf[6] = csVector3 (-1, -1,  1);
    vbuf[7] = csVector3 ( 1, -1,  1);
    vertices->Release();

    unsigned int* ibuf = (unsigned int*)indices->Lock(iRenderBuffer::CS_BUF_LOCK_NORMAL);
    ibuf[ 0] = 0;  ibuf[ 1] = 1;  ibuf[ 2] = 4;
    ibuf[ 3] = 1;  ibuf[ 4] = 5;  ibuf[ 5] = 4;
    ibuf[ 6] = 0;  ibuf[ 7] = 3;  ibuf[ 8] = 1;
    ibuf[ 9] = 0;  ibuf[10] = 2;  ibuf[11] = 3;
    ibuf[12] = 4;  ibuf[13] = 7;  ibuf[14] = 6;
    ibuf[15] = 4;  ibuf[16] = 5;  ibuf[17] = 7;
    ibuf[18] = 1;  ibuf[19] = 3;  ibuf[20] = 5;
    ibuf[21] = 3;  ibuf[22] = 7;  ibuf[23] = 5;
    ibuf[24] = 2;  ibuf[25] = 0;  ibuf[26] = 6;
    ibuf[27] = 0;  ibuf[28] = 4;  ibuf[29] = 6;
    ibuf[30] = 2;  ibuf[31] = 6;  ibuf[32] = 3;
    ibuf[33] = 3;  ibuf[34] = 6;  ibuf[35] = 7;
    indices->Release();

    csVector2* tcbuf = (csVector2*)texcoords->Lock(iRenderBuffer::CS_BUF_LOCK_NORMAL);
    tcbuf[0] = csVector2 (0, 0);
    tcbuf[1] = csVector2 (1, 0);
    tcbuf[2] = csVector2 (0, 1);
    tcbuf[3] = csVector2 (1, 1);
    tcbuf[4] = csVector2 (0, 0);
    tcbuf[5] = csVector2 (1, 0);
    tcbuf[6] = csVector2 (0, 1);
    tcbuf[7] = csVector2 (1, 1);
    texcoords->Release();


    vertices_name = r3d->GetStringContainer ()->Request ("vertices");
    indices_name = r3d->GetStringContainer ()->Request ("indices");
    texcoords_name = r3d->GetStringContainer ()->Request ("texture coordinates");
  }

  iRenderBuffer* GetBuffer(csStringID name)
  {
    if (name == vertices_name)
      return vertices;
    if (name == indices_name)
      return indices;
    if (name == texcoords_name)
      return texcoords;
    return NULL;
  }
};

SCF_IMPLEMENT_IBASE (csTestMesh)
  SCF_IMPLEMENTS_INTERFACE (iStreamSource)
SCF_IMPLEMENT_IBASE_END

//-----------------------------------------------------------------------------

// The global pointer to simple
R3DTest *r3dtest;

R3DTest::R3DTest (iObjectRegistry* object_reg)
{
  R3DTest::object_reg = object_reg;
}

R3DTest::~R3DTest ()
{
}

void R3DTest::SetupFrame ()
{
  // First get elapsed time from the virtual clock.
  csTicks elapsed_time = vc->GetElapsedTicks ();


  // Now rotate the camera according to keyboard state

  float speed = (elapsed_time / 1000.0) * (0.03 * 20);
  static int FPS = 0;
  static int framecount = 0;
  static int timeaccum = 0;
  framecount++;
  timeaccum += elapsed_time;

  if ((framecount % 60) == 0)
  {
    FPS = 60000.0/(float)timeaccum;
    timeaccum = 0;
  }


  if (kbd->GetKeyState (CSKEY_RIGHT))
    view->GetCamera ()->GetTransform ().RotateOther (CS_VEC_ROT_RIGHT, speed * 5.0);
  if (kbd->GetKeyState (CSKEY_LEFT))
    view->GetCamera ()->GetTransform ().RotateOther (CS_VEC_ROT_LEFT, speed * 5.0);
  if (kbd->GetKeyState (CSKEY_PGUP))
    view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_TILT_UP, speed * 5.0);
  if (kbd->GetKeyState (CSKEY_PGDN))
    view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_TILT_DOWN, speed * 5.0);

  if (kbd->GetKeyState (CSKEY_UP))
    view->GetCamera ()->Move (CS_VEC_FORWARD * speed * 50.0);
  if (kbd->GetKeyState (CSKEY_DOWN))
    view->GetCamera ()->Move (CS_VEC_BACKWARD * speed * 50.0);
  if (kbd->GetKeyState (CSKEY_HOME))
    view->GetCamera ()->Move (CS_VEC_UP * speed * 50.0);
  if (kbd->GetKeyState (CSKEY_END))
    view->GetCamera ()->Move (CS_VEC_DOWN * speed * 50.0);

  r3d->SetPerspectiveAspect (r3d->GetDriver2D ()->GetHeight ());
  r3d->SetPerspectiveCenter (r3d->GetDriver2D ()->GetWidth ()/2,
                             r3d->GetDriver2D ()->GetHeight ()/2);

  // Tell 3D driver we're going to display 3D things.
  if (!r3d->BeginDraw (CSDRAW_3DGRAPHICS | CSDRAW_CLEARSCREEN | CSDRAW_CLEARZBUFFER))
    return;

  csRenderMesh mesh;
  mesh.SetIndexRange (0, 36);
  mesh.SetMaterialWrapper (matwrap);
  mesh.SetStreamSource (testmesh);
  mesh.SetType (csRenderMesh::MESHTYPE_TRIANGLES);

  csReversibleTransform trans;
  static float a = 0;
/*
  csVector2 clipshape[10];
  clipshape[0] = csVector2 (sin(a*5.0)*100.0+200, 100);
  clipshape[1] = csVector2 (sin(a*5.0+0.5)*100.0+150, 200);
  clipshape[2] = csVector2 (sin(a*5.0+1.0)*100.0+300, 300);
  clipshape[3] = csVector2 (sin(a*5.0+1.5)*100.0+450, 200);
  clipshape[4] = csVector2 (sin(a*5.0+2.0)*100.0+400, 100);
  csPolygonClipper polyclip (clipshape, 5);
  csRef<iClipper2D> clipper = SCF_QUERY_INTERFACE (&polyclip, iClipper2D);
  r3d->SetClipper (clipper, CS_CLIPPER_TOPLEVEL);
*/
  a += speed;
  trans.RotateOther (csVector3 (1,0,0), a*2.0);
  trans.RotateOther (csVector3 (0,1,0), a*1.5);
  trans.RotateOther (csVector3 (0,0,1), a*1.0);
  trans.SetOrigin (csVector3 (0,0,5));
  trans = trans.GetInverse ();
  r3d->SetObjectToCamera (&trans);
  mesh.clip_plane = CS_CLIP_NOT;
  mesh.clip_z_plane = CS_CLIP_NOT;
  mesh.clip_portal = CS_CLIP_NOT;
  mesh.z_buf_mode = CS_ZBUF_NONE;
  mesh.do_mirror = false;
  r3d->DrawMesh (&mesh);

  /*light->GetMovable ()->SetPosition (csVector3 (0, 1+sin(a*4.0), sin(a*2.0)*4));
  light->GetMovable ()->UpdateMove ();*/

  // Tell the camera to render into the frame buffer.
  //view->Draw ();

  r3d->SetClipper (NULL, CS_CLIPPER_NONE);

  r3d->FinishDraw ();
  
  if (!r3d->BeginDraw (CSDRAW_2DGRAPHICS))
    return;

  iFontServer* fntsvr = r3d->GetDriver2D ()->GetFontServer ();
  CS_ASSERT (fntsvr != NULL);
  csRef<iFont> fnt (fntsvr->GetFont (0));
  if (fnt == NULL)
  {
    fnt = fntsvr->LoadFont (CSFONT_COURIER);
  }
  char text[1024];
  sprintf (text, "Ah, it iz le test!      Le FPS c'est cyrrentlee %d, frame %d", FPS, framecount);
  r3d->GetDriver2D ()->Write (fnt, 10, 50, 0x00FF00FF, -1, text);
  r3d->FinishDraw ();
}

void R3DTest::FinishFrame ()
{
  r3d->Print (NULL);
}

bool R3DTest::HandleEvent (iEvent& ev)
{
  if (ev.Type == csevBroadcast && ev.Command.Code == cscmdProcess)
  {
    r3dtest->SetupFrame ();
    return true;
  }
  else if (ev.Type == csevBroadcast && ev.Command.Code == cscmdFinalProcess)
  {
    r3dtest->FinishFrame ();
    return true;
  }
  else if (ev.Type == csevKeyDown && ev.Key.Code == CSKEY_ESC)
  {
    csRef<iEventQueue> q (CS_QUERY_REGISTRY (object_reg, iEventQueue));
    if (q) q->GetEventOutlet()->Broadcast (cscmdQuit);
    return true;
  }

  return false;
}

bool R3DTest::SimpleEventHandler (iEvent& ev)
{
  return r3dtest->HandleEvent (ev);
}

bool R3DTest::Initialize ()
{
  if (!csInitializer::RequestPlugins (object_reg,
  	CS_REQUEST_VFS,
        CS_REQUEST_PLUGIN ("crystalspace.render3d.opengl", iRender3D),
        CS_REQUEST_ENGINE,
	CS_REQUEST_IMAGELOADER,
	CS_REQUEST_LEVELLOADER,
	CS_REQUEST_FONTSERVER,
	CS_REQUEST_REPORTER,
	CS_REQUEST_REPORTERLISTENER,
 	CS_REQUEST_END))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.r3dtest",
	"Can't initialize plugins!");
    return false;
  }

  if (!csInitializer::SetupEventHandler (object_reg, SimpleEventHandler))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.r3dtest",
	"Can't initialize event handler!");
    return false;
  }

  // Check for commandline help.
  if (csCommandLineHelper::CheckHelp (object_reg))
  {
    csCommandLineHelper::Help (object_reg);
    return false;
  }

  vc = CS_QUERY_REGISTRY (object_reg, iVirtualClock);
  if (vc == NULL)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.r3dtest",
    	"No iVirtualClock plugin!");
    return false;
  }

  vfs = CS_QUERY_REGISTRY (object_reg, iVFS);
  if (vfs == NULL)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.r3dtest",
    	"No iVFS plugin!");
    return false;
  }

  r3d = CS_QUERY_REGISTRY (object_reg, iRender3D);
  if (r3d == NULL)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.r3dtest",
    	"No iRender3D plugin!");
    return false;
  }


  engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  if (engine == NULL)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.r3dtest",
    	"No iEngine plugin!");
    return false;
  }

  kbd = CS_QUERY_REGISTRY (object_reg, iKeyboardDriver);
  if (kbd == NULL)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.r3dtest",
    	"No iKeyboardDriver plugin!");
    return false;
  }

  csRef<iLoader> loader = CS_QUERY_REGISTRY (object_reg, iLoader);
  if (loader == NULL)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.r3dtest",
    	"No iLoader plugin!");
    return false;
  }


  // Open the main system. This will open all the previously loaded plug-ins.
  if (!csInitializer::OpenApplication (object_reg))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.r3dtest",
    	"Error opening system!");
    return false;
  }

  testmesh = new csTestMesh (r3d);

  if (!loader->LoadTexture ("portal", "/lib/std/portal.jpg"))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.r3dtest",
    	"Error loading 'portal' texture!");
    return false;
  }
  matwrap = engine->GetMaterialList ()->FindByName ("portal");

  // Just disregard this. It's for testing. /Anders Stenberg
  //vfs->Mount ("/level", "./data/r3dtest.zip");
  //vfs->ChDir ("/level");
  /*vfs->ChDir ("/this/data/r3dtest");
  loader->LoadMapFile ("world.xml", false);*/

  csRef<iSector> room = engine->CreateSector ("room");
  //csRef<iSector> room = engine->FindSector ("room");

  /*light = engine->CreateMeshWrapper (
    "crystalspace.mesh.object.testlight", 
    "testlight", 
    room, 
    csVector3 (0, 1, 1));

  csRef<iTestLightState> lightstate = SCF_QUERY_INTERFACE (
    light->GetMeshObject (),
    iTestLightState);

  lightstate->SetRange (1.0);*/

  view = csPtr<iView> (new csView (engine, r3d));
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 1.8, 0));
  csRef<iGraphics2D> g2d = r3d->GetDriver2D ();
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  engine->Prepare ();

  iFontServer* fntsvr = r3d->GetDriver2D ()->GetFontServer ();
  CS_ASSERT (fntsvr != NULL);
  csRef<iFont> fnt (fntsvr->GetFont (0));
  if (fnt == NULL)
  {
    fnt = fntsvr->LoadFont (CSFONT_COURIER);
  }
  char text[1024];
  r3d->SetRenderTarget (matwrap->GetMaterial ()->GetTexture (), true);

  // Tell 3D driver we're going to display 3D things.
  if (!r3d->BeginDraw (CSDRAW_2DGRAPHICS))
    return false;

  sprintf (text, "Le SetRenderTargette iz workeeng!");
  r3d->GetDriver2D ()->Write (fnt, 10, 10, 0x00FF00FF, -1, text);

  r3d->FinishDraw ();
  r3d->SetRenderTarget (NULL);

  csRef<iShaderManager> shmgr = CS_QUERY_REGISTRY(object_reg, iShaderManager);
  csRef<iShader> shader = shmgr->CreateShader();
  shader->SetName("rainbow");

  csRef<iShaderTechnique> shtech = shader->CreateTechnique();
  shtech->SetPriority(100);

  csRef<iShaderPass> shpass = shtech->CreatePass();
  shpass->SetVertexProgram ( csRef<iShaderProgram>(shmgr->CreateShaderProgramFromFile("/shader/ms.avp","gl_arb_vp")) );

  matwrap->GetMaterial()->SetShader(shader);

  return true;
}

void R3DTest::Start ()
{
  csDefaultRunLoop (object_reg);
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  iObjectRegistry* object_reg = csInitializer::CreateEnvironment (argc, argv);

  r3dtest = new R3DTest (object_reg);
  if (r3dtest->Initialize ())
    r3dtest->Start ();
  delete r3dtest;

  csInitializer::DestroyApplication (object_reg);
  return 0;
}

