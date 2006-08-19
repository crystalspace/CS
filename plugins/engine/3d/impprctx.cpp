/*
    Copyright (C) 2002 by Keith Fulton and Jorrit Tyberghein
    Rewritten during Sommer of Code 2006 by Christoph "Fossi" Mewes

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
#include "csgeom/math.h"
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


csImposterProcTex::csImposterProcTex  (csEngine* engine,  
  csImposterMesh *parent) : scfImplementationType(this), engine(engine)
{
  mesh = parent;

  //@@@ make dynamic
  w = h = 256;

  int texFlags = CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS;

  //initialize shortcuts
  g3d = engine->G3D;
  g2d = g3d->GetDriver2D ();
  
  csRef<iImage> thisImage = new csImageMemory (w, h,
    CS_IMGFMT_ALPHA && CS_IMGFMT_TRUECOLOR );
  tex = engine->GetTextureList ()->NewTexture (thisImage);
  tex->SetFlags (tex->GetFlags() | texFlags);
  thisImage = 0;

  csRef<iStringSet> stringSet = engine->globalStringSet;
  stringid_standard = stringSet->Request("standard");
  stringid_light_ambient = stringSet->Request("light ambient");

  clip = new csBoxClipper (0, 0, w, h);
}

csImposterProcTex::~csImposterProcTex ()
{
  delete clip;
}

void csImposterProcTex::Animate (iRenderView *rview, iSector *s)
{
  //printf("updating imposter... ");
  if (!mesh) return;

  csRef<iTextureHandle> handle = tex->GetTextureHandle ();
  g3d->SetRenderTarget (handle);

  csRef<iMeshWrapper> originalmesh = mesh->parent_mesh;

  g3d->BeginDraw (CSDRAW_3DGRAPHICS | engine->GetBeginDrawFlags ()
    | CSDRAW_CLEARZBUFFER);
  
  g2d->Clear (g2d->FindRGB (0, 0, 0, 0));
 
  csRef<iMeshObject> meshobj = originalmesh->GetMeshObject ();

  //Calculate camera position for imposter rendering
  iMovable* movable = originalmesh->GetMovable ();
  csVector3 mesh_pos = movable->GetFullPosition ();

  iCamera* cam = rview->GetCamera ();
  mesh->FindImposterRectangle (cam);

  const csOrthoTransform camt = cam->GetTransform ();
  int persx, persy;
  g3d->GetPerspectiveCenter ( persx, persy );
  int oldFOV = cam->GetFOV ();

  const csVector3& cam_pos = cam->GetTransform ().GetOrigin ();
  csVector3 camdir = mesh_pos-cam_pos;

  cam->GetTransform ().LookAt (camdir, csVector3 (0,1,0));

  float maxratio = csMax (
    mesh->width/engine->frameWidth,
    mesh->height/engine->frameHeight);
  
  csVector3 new_cam_pos = mesh_pos - maxratio * camdir;
  cam->GetTransform ().SetOrigin (new_cam_pos);

  //Setup rendering
  g3d->SetPerspectiveCenter (w/2, w/2);
  g3d->SetClipper (clip, CS_CLIPPER_TOPLEVEL);
  cam->SetFOV (w, h);
  g3d->SetPerspectiveAspect (cam->GetFOV ());
  g3d->SetWorldToCamera (cam->GetTransform ().GetInverse ());

  int num;
  csRenderMesh** rendermeshes = meshobj->GetRenderMeshes (num, rview, 
    originalmesh->GetMovable (), ~0);

  if (num != 1)
  {
	  engine->Warn("Only one Rendermesh supported by imposters!");
  }

  //@@@ Add support for no/multiple rendermeshes
  csRenderMesh* rendermesh = rendermeshes[0];
  csRenderMeshModes mode (*rendermesh);
  csShaderVarStack sva;

  iMaterial* hdl = rendermesh->material->GetMaterial ();
  iShader* meshShader = hdl->GetShader (stringid_standard);

  if (meshShader == 0)
  {
    static bool defaultshaderused = 0;
    if (!defaultshaderused)
	{
      defaultshaderused = 1;
      engine->Warn("No 'standard' Shader! Using default.");
	}
    meshShader = engine->defaultShader;
  }

  csShaderVariableContext svc;

  //add ambient shadervariable
  csRef<csShaderVariable> sv;
  sv = svc.GetVariableAdd(stringid_light_ambient);
  csColor ambient;
  engine->GetAmbientLight (ambient);
  if (s) sv->SetValue (ambient + s->GetDynamicAmbientLight());
  svc.PushVariables (sva);

  if (rendermesh->variablecontext)
    rendermesh->variablecontext->PushVariables (sva);
  meshShader->PushVariables (sva);
  if (hdl) hdl->PushVariables (sva);

  size_t shaderTicket = meshShader->GetTicket (mode, sva);
  size_t passCount = meshShader->GetNumberOfPasses (shaderTicket);

  for (size_t p = 0; p < passCount; p++)
  {
    meshShader->ActivatePass (shaderTicket, p);
    meshShader->SetupPass (shaderTicket, rendermesh, mode, sva);
    g3d->DrawMesh(rendermesh, mode, sva);
    meshShader->TeardownPass (shaderTicket);
    meshShader->DeactivatePass (shaderTicket);
  }

  //restore old camera values
  g3d->SetPerspectiveCenter (persx, persy);
  cam->SetFOV (oldFOV, engine->frameHeight);
  cam->SetTransform (camt);

  //printf("updating mesh... ");
  mesh->SetImposterReady (true);

/*
  //debuging output
  csDebugImageWriter diw = csDebugImageWriter();
  csRef<iImage> pic;
  pic.AttachNew(g3d->GetDriver2D()->ScreenShot());
  csString name = csString();
  name += random();
  name += "imposter.png";
  diw.DebugImageWrite(pic,name);
*/

  g3d->FinishDraw ();
  //printf("done\n");
}

