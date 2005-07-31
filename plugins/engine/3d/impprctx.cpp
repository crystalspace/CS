/*
    Copyright (C) 2002 by Keith Fulton and Jorrit Tyberghein

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
#include "ivideo/graph3d.h"
#include "cstool/csview.h"
#include "cstool/proctex.h"
#include "plugins/engine/3d/engine.h"
#include "plugins/engine/3d/impprctx.h"
#include "csgfx/memimage.h"
#include "csutil/cscolor.h"
#include "iutil/objreg.h"
#include "iengine/rview.h"

#include "ivideo/txtmgr.h"
#include "ivideo/texture.h"
#include "ivideo/graph2d.h"
//#include "imap/loader.h"
#include "iengine/engine.h"
#include "iengine/camera.h"
#include "iengine/sector.h"
#include "plugins/engine/3d/impmesh.h"
#include "plugins/engine/3d/meshobj.h"

//#include "iutil/vfs.h"

csImposterProcTex::csImposterProcTex (csImposterMesh *parent) : csProcTexture ()
{
  mesh = parent;

  mat_w = 256;
  mat_h = 256;

  texFlags = CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS;

  csProcTexture::Initialize (csEngine::objectRegistry);
}

csImposterProcTex::~csImposterProcTex ()
{
}

bool csImposterProcTex::PrepareAnim ()
{
  if (anim_prepared) return true;
  if (!csProcTexture::PrepareAnim ()) return false;

  // special things may be necessary here

  return true;
}

void csImposterProcTex::Animate (csTicks CurrentTime)
{
  CS_ASSERT_MSG("Cannot remove this", 0);
  // move the camera
  csVector3 Position (-0.5, 0, 3 + sin (CurrentTime / (10*1000.0))*1);
  View->GetCamera ()->Move (Position - View->GetCamera ()
  	->GetTransform ().GetOrigin ());

  g3d->SetRenderTarget (tex->GetTextureHandle ());

  // Switch to the context of the procedural texture.
  iTextureHandle *oldContext = Engine->GetContext ();
  Engine->SetContext (tex->GetTextureHandle ());

  // Draw the engine view.
  g3d->BeginDraw (CSDRAW_3DGRAPHICS | Engine->GetBeginDrawFlags ());

  // Determine and save the actual polygon on which the texture will be rendered
  mesh->FindImposterRectangle (View->GetCamera () );

  // This actually draws the mesh on the backbuffer
/*  mesh->GetParent()->GetMeshObject()->Draw (View, 
        &mesh->GetParent()->GetCsMovable().scfiMovable, 
	mesh->GetParent()->GetZBufMode());*/
  
  // This copies the backbuffer to the iTextureHandle I think.
  g3d->FinishDraw ();

  // switch back to the old context
  Engine->SetContext (oldContext);
}

