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
#include "ivideo/render3d.h"

#include "render3d.h"

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
}


////////////////////////////////////////////////////////////////////
//                         iRender3D
////////////////////////////////////////////////////////////////////




bool csGLRender3D::Open ()
{
  if (!g2d->Open ())
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
}

void csGLRender3D::Close ()
{
  if (g2d)
    g2d->Close ();
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



////////////////////////////////////////////////////////////////////
//                         iEffectClient
////////////////////////////////////////////////////////////////////




bool csGLRender3D::Validate (iEffectDefinition* effect, iEffectTechnique* technique);
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
  g2d = CS_LOAD_PLUGIN (plugin_mgr, 
    "crystalspace.graphics2d.glwin32", iGraphics2D);
  if (!g2d)
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
