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


#include "csutil/objreg.h"
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "csutil/strhash.h"

#include "iutil/comp.h"
#include "iutil/plugin.h"
#include "iutil/eventq.h"
#include "ivaria/reporter.h"

#include "ivideo/lighting.h"
#include "ivideo/txtmgr.h"
#include "ivideo/rndbuf.h"
#include "ivideo/render3d.h"

#include "gl_render3d.h"
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
}

csGLRender3D::~csGLRender3D()
{
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

  return true;
}

void csGLRender3D::Close ()
{
  if (G2D)
    G2D->Close ();
}

bool csGLRender3D::BeginDraw (int drawflags)
{
  return false;
}

void csGLRender3D::FinishDraw ()
{
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
}

void csGLRender3D::SetDimension(int width, int height)
{
}

void csGLRender3D::SetFOV(float fov)
{
}

void csGLRender3D::DrawMesh(csRenderMesh* mymesh)
{
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
