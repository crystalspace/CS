/*
    Copyright (C) 2002 by Mårten Svanfeldt
                          Anders Stenberg

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

#include "csgeom/transfrm.h"

#include "csutil/objreg.h"
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "csutil/strset.h"

#include "iutil/comp.h"
#include "iutil/plugin.h"
#include "iutil/eventq.h"

#include "ivaria/reporter.h"

#include "ivideo/lighting.h"
#include "ivideo/txtmgr.h"
#include "ivideo/render3d.h"
#include "ivideo/rndbuf.h"

#include "gl_render3d.h"
#include "gl_sysbufmgr.h"
#include "glextmanager.h"

#include "ivideo/effects/efserver.h"
#include "ivideo/effects/efdef.h"
#include "ivideo/effects/eftech.h"
#include "ivideo/effects/efpass.h"
#include "ivideo/effects/eflayer.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csGLRender3D)

SCF_EXPORT_CLASS_TABLE (gl_render3d)
  SCF_EXPORT_CLASS_DEP (csGLRender3D, "crystalspace.render3d.opengl",
    "OpenGL Render3D graphics driver for Crystal Space", "crystalspace.font.server.")
SCF_EXPORT_CLASS_TABLE_END


SCF_IMPLEMENT_IBASE(csGLRender3D)
  SCF_IMPLEMENTS_INTERFACE(iRender3D)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iEffectClient)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGLRender3D::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGLRender3D::eiEffectClient)
  SCF_IMPLEMENTS_INTERFACE (iEffectClient)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csGLRender3D::EventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END


csGLRender3D::csGLRender3D (iBase *parent)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiEffectClient);

  scfiEventHandler = NULL;

  strings = new csStringSet ();
}

csGLRender3D::~csGLRender3D()
{
  delete strings;
}

void csGLRender3D::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csRef<iReporter> rep (CS_QUERY_REGISTRY (object_reg, iReporter));
  if (rep)
    rep->ReportV (severity, "crystalspace.render3d.opengl", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}


////////////////////////////////////////////////////////////////////
//                         iRender3D
////////////////////////////////////////////////////////////////////




bool csGLRender3D::Open ()
{
  if (!G2D->Open ())
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Error opening Graphics2D context.");
    return false;
  }
  
  int w = G2D->GetWidth ();
  int h = G2D->GetHeight ();
  SetDimensions (w, h);

  effectserver = CS_QUERY_REGISTRY(object_reg, iEffectServer);
  if( !effectserver )
  {
    csRef<iPluginManager> plugin_mgr (
      CS_QUERY_REGISTRY (object_reg, iPluginManager));
    effectserver = CS_LOAD_PLUGIN (plugin_mgr,
      "crystalspace.video.effects.stdserver", iEffectServer);
    object_reg->Register (effectserver, "iEffectServer");
  }

  csRef<iOpenGLInterface> gl = SCF_QUERY_INTERFACE (G2D, iOpenGLInterface);
  ext.InitExtensions (gl);

  buffermgr = new csSysRenderBufferManager ();

  return true;
}

void csGLRender3D::Close ()
{
  if (G2D)
    G2D->Close ();
}

bool csGLRender3D::BeginDraw (int drawflags)
{
  current_drawflags = drawflags;

  if (drawflags & CSDRAW_CLEARZBUFFER)
  {
    glDepthMask (GL_TRUE);
    if (drawflags & CSDRAW_CLEARSCREEN)
      glClear (GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    else
      glClear (GL_DEPTH_BUFFER_BIT);
  }
  else if (drawflags & CSDRAW_CLEARSCREEN)
    G2D->Clear (0);

  if (drawflags & CSDRAW_3DGRAPHICS)
  {
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();

    glOrtho (0., (GLdouble) (viewwidth+1), 0., (GLdouble) (viewheight+1), -1.0, 50.0);
    glViewport (1, -1, viewwidth+1, viewheight+1);
    glTranslatef (viewwidth/2, viewheight/2, 0);

    GLfloat matrixholder[16];
    for (int i = 0 ; i < 16 ; i++) matrixholder[i] = 0.0;
    matrixholder[0] = matrixholder[5] = 1.0;
    matrixholder[11] = 1.0/(float)viewheight;
    matrixholder[14] = -matrixholder[11];
    glMultMatrixf (matrixholder);
    return true;
  } else if (drawflags & CSDRAW_2DGRAPHICS)
    return G2D->BeginDraw ();

  current_drawflags = 0;
  return false;
}

void csGLRender3D::FinishDraw ()
{
  if (current_drawflags & CSDRAW_2DGRAPHICS)
    G2D->FinishDraw ();
}

void csGLRender3D::Print (csRect* area)
{
  G2D->Print (area);
}

csReversibleTransform* csGLRender3D::GetWVMatrix()
{
  return NULL;
}

void csGLRender3D::SetWVMatrix(csReversibleTransform* wvmatrix)
{
  glMatrixMode (GL_MODELVIEW);

  GLfloat m[16];
  csVector3 c1 = wvmatrix->GetT2O ().Col1 ();
  csVector3 c2 = wvmatrix->GetT2O ().Col2 ();
  csVector3 c3 = wvmatrix->GetT2O ().Col3 ();
  csVector3 o = wvmatrix->GetOrigin ();
  m[ 0] = c1.x; m[ 1] = c2.x; m[ 2] = c3.x; m[3] = 0.0;
  m[ 4] = c1.y; m[ 5] = c2.y; m[ 6] = c3.y; m[7] = 0.0;
  m[ 8] = c1.z; m[ 9] = c2.z; m[10] = c3.z; m[11] = 0.0;
  m[12] = o.x;  m[13] = o.y;  m[14] = o.z;  m[15] = 1.0;
  
  glLoadMatrixf (m);
}

void csGLRender3D::SetFOV(float fov)
{
}

void csGLRender3D::DrawMesh(csRenderMesh* mymesh)
{

  csRef<iStreamSource> source = mymesh->GetStreamSource ();
  csRef<iRenderBuffer> vertexbuf = source->GetBuffer (strings->Request ("vertices"));
  csRef<iRenderBuffer> indexbuf = source->GetBuffer (strings->Request ("indices"));

  //G2D->BeginDraw ();
  glVertexPointer (3, GL_FLOAT, 0, vertexbuf->GetFloatBuffer ());
  glEnableClientState (GL_VERTEX_ARRAY);
  /*glIndexPointer (GL_INT, 0, vertexbuf->GetUIntBuffer ());
  glEnableClientState (GL_INDEX_ARRAY);
  glDrawArrays (GL_TRIANGLES, 0, indexbuf->GetUIntLength ());*/
  glDrawElements (GL_TRIANGLES, indexbuf->GetUIntLength (), GL_UNSIGNED_INT, indexbuf->GetUIntBuffer ());
}



////////////////////////////////////////////////////////////////////
//                         iEffectClient
////////////////////////////////////////////////////////////////////




bool csGLRender3D::Validate (iEffectDefinition* effect, iEffectTechnique* technique)
{
  return false;
}




////////////////////////////////////////////////////////////////////
//                          iComponent
////////////////////////////////////////////////////////////////////




bool csGLRender3D::Initialize (iObjectRegistry* p)
{
  object_reg = p;
  
  if (!scfiEventHandler)
    scfiEventHandler = new EventHandler (this);
  
  csRef<iEventQueue> q (CS_QUERY_REGISTRY(object_reg, iEventQueue));
  if (q)
    q->RegisterListener (scfiEventHandler, CSMASK_Broadcast);


  csRef<iPluginManager> plugin_mgr (
    CS_QUERY_REGISTRY (object_reg, iPluginManager));


  // @@@ Should check what canvas to load
  G2D = CS_LOAD_PLUGIN (plugin_mgr, 
    "crystalspace.graphics2d.glwin32", iGraphics2D);
  if (!G2D)
    return false;

  return true;
}




////////////////////////////////////////////////////////////////////
//                         iEventHandler
////////////////////////////////////////////////////////////////////




bool csGLRender3D::HandleEvent (iEvent& Event)
{
  if (Event.Type == csevBroadcast)
    switch (Event.Command.Code)
    {
      case cscmdSystemOpen:
      {
        Open ();
        return true;
      }
      case cscmdSystemClose:
      {
        Close ();
        return true;
      }
    }
  return false;
}
