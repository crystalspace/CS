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
#include "csgeom/polyclip.h"
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
#include "ivideo/material.h"
#include "engine.h"
#include "material.h"
#include "impmesh.h"
#include "meshobj.h"
#include "impprctx.h"

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
  csRef<iImage> thisImage = new csImageMemory (256, 256,
    CS_IMGFMT_ALPHA && CS_IMGFMT_TRUECOLOR );
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
  printf("animating imposter... ");
  csRef<iTextureHandle> handle = tex->GetTextureHandle ();
  if (!mesh) return;

  g3d->SetRenderTarget (handle);

  csRef<iMeshWrapper> originalmesh = mesh->GetParent ();

  g3d->BeginDraw (CSDRAW_3DGRAPHICS | engine->GetBeginDrawFlags ()
    | CSDRAW_CLEARZBUFFER);
  
  g3d->GetDriver2D ()->Clear (
    g3d->GetDriver2D ()->FindRGB (0, 100, 200, 0));
 
  int num;
  csRef<iMeshObject> meshobj = originalmesh->GetMeshObject ();

  iMovable* movable = originalmesh->GetMovable ();
  csVector3 mesh_pos = movable->GetFullPosition ();
  iCamera* cam = rview->GetCamera ();
  const csOrthoTransform camt = cam->GetTransform ();

  const csVector3& cam_pos = cam->GetTransform ().GetOrigin ();
  cam->GetTransform ().LookAt(mesh_pos-cam_pos, csVector3(0,1,0));
  csVector3 new_cam_pos = mesh_pos + 1 * (cam_pos-mesh_pos).Unit ();
  cam->GetTransform ().SetOrigin (new_cam_pos);

  csRenderMesh** rendermeshes = meshobj->GetRenderMeshes (num, rview, 
    originalmesh->GetMovable (), ~0);

  csRenderMesh* rendermesh = rendermeshes[0];
  csRenderMeshModes mode (*rendermesh);
  csShaderVarStack sva;

  int persx, persy;
  g3d->GetPerspectiveCenter ( persx, persy );
  int oldFOV = cam->GetFOV ();

  g3d->SetPerspectiveCenter ( 125,125 );
  cam->SetFOV (250,250);
  iClipper2D* clip = new csBoxClipper(0,0,250,250);
  g3d->SetClipper(clip, CS_CLIPPER_TOPLEVEL);
  //g3d->ResetNearPlane ();
  g3d->SetPerspectiveAspect (cam->GetFOV ());

  g3d->SetWorldToCamera (cam->GetTransform ().GetInverse ());

  iMaterial* hdl = rendermesh->material->GetMaterial ();
  csRef<iStringSet> stringSet = engine->globalStringSet;
  iShader* meshShader = hdl->GetShader (stringSet->Request("standard"));

  if (meshShader == 0)
  {
   printf("No 'standard' Shader!\n");
   meshShader = engine->defaultShader;
  }

  size_t shaderTicket = meshShader->GetTicket (mode, sva);
  size_t passCount = meshShader->GetNumberOfPasses (shaderTicket);

  for (size_t p = 0; p < passCount; p++)
  {
    meshShader->ActivatePass (shaderTicket, p);

    csShaderVariableContext svc;
    csRef<csShaderVariable> sv;
    sv = svc.GetVariableAdd(stringSet->Request("light ambient"));
    csColor ambient;
    engine->GetAmbientLight (ambient);
    sv->SetValue (ambient + s->GetDynamicAmbientLight());
    svc.PushVariables (sva);

    if (rendermesh->variablecontext)
      rendermesh->variablecontext->PushVariables (sva);
    meshShader->PushVariables (sva);
    if (hdl)
      hdl->PushVariables (sva);

    meshShader->SetupPass (shaderTicket, rendermesh, mode, sva);
    g3d->DrawMesh(rendermesh, mode, sva);
    meshShader->TeardownPass (shaderTicket);
    meshShader->DeactivatePass (shaderTicket);
  }

printf("rendered ");

  g3d->SetPerspectiveCenter (persx, persy);
  cam->SetFOV (oldFOV, 2*persx);
  cam->SetTransform (camt);

  printf("updating mesh... ");
  mesh->FindImposterRectangle (cam);

  mesh->SetImposterReady (true);

  //debuging output
  csDebugImageWriter diw = csDebugImageWriter();
  csRef<iImage> pic;
  pic.AttachNew(g3d->GetDriver2D()->ScreenShot());
  csString name = csString();
  name += random();
  name += "imposter.png";
  diw.DebugImageWrite(pic,name);

  g3d->FinishDraw ();
  printf("done\n");
}

