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
#include "csgfx/memimage.h"
#include "cstool/csview.h"
#include "csutil/cscolor.h"
#include "iengine/camera.h"
#include "iengine/engine.h"
#include "iengine/rview.h"
#include "iengine/sector.h"
#include "iutil/objreg.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/texture.h"
#include "ivideo/txtmgr.h"
#include "plugins/engine/3d/engine.h"
#include "plugins/engine/3d/impmesh.h"
#include "plugins/engine/3d/meshobj.h"
#include "plugins/engine/3d/impprctx.h"

//@@@ debugging
#include "cstool/debugimagewriter.h"
#include "csgfx/memimage.h"


csImposterProcTex::csImposterProcTex (csEngine* engine, csImposterMesh 
*parent) : scfImplementationType(this), engine(engine)
{
  mesh = parent;

  int texFlags = CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS;

  mesh_on_texture = new csMeshOnTexture(engine->objectRegistry);

  g3d = csQueryRegistry<iGraphics3D>(engine->objectRegistry);
  g2d = csQueryRegistry<iGraphics2D>(engine->objectRegistry);
  engine->imposterUpdateList.Push(
    csWeakRef<csImposterProcTex>(this));

  //@@@ replace proctex initialize!
  csRef<iImage> thisImage = new csImageMemory (256,256);
  tex = engine->GetTextureList ()->NewTexture (thisImage);
  tex->SetFlags (tex->GetFlags() | texFlags);
  thisImage = 0;
}

csImposterProcTex::~csImposterProcTex ()
{
}

void csImposterProcTex::Animate (iRenderView *rview, iRenderLoop* rl, 
iSector *s)
{
  printf("animating imposter\n");
  csRef<iTextureHandle> handle = tex->GetTextureHandle ();
  if (!mesh) return;

//  iTextureHandle *oldContext = engine->GetContext ();

  g3d->SetRenderTarget (handle);
//  engine->SetContext (handle);

  csRef<iMeshWrapper> originalmesh = mesh->GetParent ();

  g3d->BeginDraw (CSDRAW_3DGRAPHICS | engine->GetBeginDrawFlags ()
    | CSDRAW_CLEARZBUFFER);
  
  g3d->GetDriver2D ()->Clear (
    g3d->GetDriver2D ()->FindRGB (0, 255, 255, 0));
 
  int num;
  csRef<iMeshObject> meshobj = originalmesh->GetMeshObject ();
  csRenderMesh** rendermeshes = meshobj->GetRenderMeshes (num, rview, 
    originalmesh->GetMovable (), 0xf);

  csRenderMesh* rendermesh = rendermeshes[0];
  csRenderMeshModes mode (*rendermesh);
  const csShaderVarStack sva;

  //SetWorldToCamera?
//  g3d->DrawMesh(rendermesh, mode, sva);
printf("rendered\n");
  mesh->FindImposterRectangle (rview->GetCamera ());
  mesh->SetImposterReady (true);

/*
  //debuging output
  csRef<csDebugImageWriter> diw = new csDebugImageWriter();
  csRef<iImage> pic;
  pic.AttachNew(g3d->GetDriver2D()->ScreenShot());
  diw->DebugImageWrite(pic,"imposter.png");
*/

  g3d->FinishDraw ();

  // switch back to the old context
//  engine->SetContext (oldContext);

  //debuging output
//  pic.AttachNew(g3d->GetDriver2D()->ScreenShot());
//  diw->DebugImageWrite(pic,"screen.png");

//assert(0);
}

