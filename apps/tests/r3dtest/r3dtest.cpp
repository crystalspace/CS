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
#include "csgeom/transfrm.h"
#include "csutil/cscolor.h"
#include "cstool/csview.h"
#include "cstool/initapp.h"
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

#include "r3dtest.h"

CS_IMPLEMENT_APPLICATION

class csTestMesh : public iStreamSource
{
private:
  csRef<iRender3D> r3d;
  csRef<iRenderBuffer> vertices;
  csRef<iRenderBuffer> indices;

  csStringID vertices_name, indices_name;

public:

  SCF_DECLARE_IBASE;

  csTestMesh (iRender3D* r3d)
  {
    SCF_CONSTRUCT_IBASE (NULL)

    csTestMesh::r3d = r3d;

    vertices = r3d->GetBufferManager ()->GetBuffer (
      sizeof (csVector3)*8, CS_BUF_STATIC);
    indices = r3d->GetBufferManager ()->GetBuffer (
      sizeof (unsigned int)*36, CS_BUF_STATIC);

    csVector3* vbuf = vertices->GetVector3Buffer ();
    vbuf[0] = csVector3 (-1,  1, -1);
    vbuf[1] = csVector3 ( 1,  1, -1);
    vbuf[2] = csVector3 (-1,  1,  1);
    vbuf[3] = csVector3 ( 1,  1,  1);
    vbuf[4] = csVector3 (-1, -1, -1);
    vbuf[5] = csVector3 ( 1, -1, -1);
    vbuf[6] = csVector3 (-1, -1,  1);
    vbuf[7] = csVector3 ( 1, -1,  1);

    unsigned int* ibuf = indices->GetUIntBuffer ();
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
    ibuf[33] = 3;  ibuf[34] = 7;  ibuf[35] = 6;

    vertices_name = r3d->GetStringContainer ()->Request ("vertices");
    indices_name = r3d->GetStringContainer ()->Request ("indices");
  }

  iRenderBuffer* GetBuffer(csStringID name)
  {
    if (name == vertices_name)
      return vertices;
    if (name == indices_name)
      return indices;
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
  /*// First get elapsed time from the virtual clock.
  csTicks elapsed_time = vc->GetElapsedTicks ();

  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.0) * (0.03 * 20);*/

  iFontServer* fntsvr = r3d->Get2DDriver ()->GetFontServer ();
  CS_ASSERT (fntsvr != NULL);
  csRef<iFont> fnt (fntsvr->GetFont (0));
  if (fnt == NULL)
  {
    fnt = fntsvr->LoadFont (CSFONT_COURIER);
  }

  if (!r3d->BeginDraw (CSDRAW_2DGRAPHICS | CSDRAW_CLEARSCREEN | CSDRAW_CLEARZBUFFER))
    return;

  char asdf[1024];
  sprintf (asdf, "Le test!", testmesh);
  r3d->Get2DDriver ()->Write (fnt, 10, 50, -1, -1, asdf);
  r3d->FinishDraw ();

  // Tell 3D driver we're going to display 3D things.
  if (!r3d->BeginDraw (CSDRAW_3DGRAPHICS))
    return;

  // Tell the camera to render into the frame buffer.
  //view->Draw ();

  csRenderMesh mesh;

  mesh.SetStreamSource (testmesh);
  mesh.SetType (csRenderMesh::MESHTYPE_TRIANGLES);

  csReversibleTransform trans;
  static float a = 0;
  a += 0.005;
  trans.RotateThis (csVector3 (1,0,0), a);
  trans.RotateThis (csVector3 (0,1,0), a);
  trans.RotateThis (csVector3 (0,0,1), a);
  trans.SetOrigin (csVector3 (0,0,5));
  r3d->SetWVMatrix (&trans);
  r3d->DrawMesh (&mesh);
}

void R3DTest::FinishFrame ()
{
  r3d->FinishDraw ();
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
	CS_REQUEST_FONTSERVER,
	CS_REQUEST_IMAGELOADER,
	CS_REQUEST_REPORTER,
	CS_REQUEST_REPORTERLISTENER,
        CS_REQUEST_PLUGIN ("crystalspace.render3d.opengl", iRender3D),
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

  r3d = CS_QUERY_REGISTRY (object_reg, iRender3D);
  if (r3d == NULL)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.r3dtest",
    	"No iRender3D plugin!");
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

