/*
  Copyright (C) 2008 by Joe Forte

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "deferred.h"
#include "iengine.h"
#include "ivideo.h"
#include "ivaria/reporter.h"
#include "csplugincommon/rendermanager/render.h"

CS_PLUGIN_NAMESPACE_BEGIN(RMDeferred)
{

SCF_IMPLEMENT_FACTORY(RMDeferred);

//----------------------------------------------------------------------
RMDeferred::RMDeferred(iBase *parent) : scfImplementationType(this, parent)
{}

//----------------------------------------------------------------------
bool RMDeferred::Initialize(iObjectRegistry *registry)
{
  const char *messageID = "crystalspace.rendermanager.deferred";

  objRegistry = registry;

  csRef<iGraphics3D> graphics3D = csQueryRegistry<iGraphics3D> (objRegistry);
  iGraphics2D *graphics2D = graphics3D->GetDriver2D ();

  // Creates the accumulation buffer.
  int flags = CS_TEXTURE_2D | CS_TEXTURE_NOMIPMAPS | CS_TEXTURE_CLAMP | CS_TEXTURE_NPOTS;
  /*
  accumBuffer = graphics3D->GetTextureManager ()->CreateTexture (graphics2D->GetWidth (),
    graphics2D->GetHeight (),
    csimg2D,
    "rgba16_f",
    flags,
    NULL);

  if (!accumBuffer)
  {
    csReport(objRegistry, CS_REPORTER_SEVERITY_ERROR, messageID, "Could not create accumulation buffer!");
    return false;
  }
  */
  return true;
}

//----------------------------------------------------------------------
bool RMDeferred::RenderView(iView *view)
{
  iEngine *engine = view->GetEngine ();
  iGraphics3D *graphics3D = view->GetContext ();
  iGraphics2D *graphics2D = graphics3D->GetDriver2D ();

  view->UpdateClipper ();

  int drawFlags = engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS;
  CS::RenderManager::BeginFinishDrawScope drawScope (graphics3D, drawFlags);

  engine->Draw (view->GetCamera (), view->GetClipper ());

  return true;
}

//----------------------------------------------------------------------
bool RMDeferred::PrecacheView(iView *view)
{
  return RenderView (view);
}

}
CS_PLUGIN_NAMESPACE_END(RMDeferred)
