/*
    Copyright (C) 2000 by Jorrit Tyberghein
    Copyright (C) 2000 by Samuel Humphreys

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

#include <math.h>
#include "cssysdef.h"

#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "igraphic/image.h"
#include "isys/system.h"
#include "ivideo/txtmgr.h"

#include "csgfx/memimage.h"
#include "csengine/texture.h"
#include "csengine/material.h"
#include "csengine/engine.h"

#include "csfx/proctex.h"

IMPLEMENT_CSOBJTYPE (csProcTexture, csObject);

csProcTexture::csProcTexture ()
{
  ptReady = false;
  tex = NULL;
  ptG3D = NULL;
  ptG2D = NULL;
  ptTxtMgr = NULL;
  texFlags = 0;
  key_color = false;
  sys = NULL;
  use_cb = true;
  last_cur_time = 0;
}

csProcTexture::~csProcTexture ()
{
}

void csProcTexture::ProcCallback (iTextureWrapper*, void* data)
{
  csProcTexture* pt = (csProcTexture*)data;
  cs_time elapsed_time, current_time;
  pt->sys->GetElapsedTime (elapsed_time, current_time);
  if (pt->last_cur_time == current_time) return;
  pt->Animate (current_time);
  pt->last_cur_time = current_time;
}

bool csProcTexture::Initialize (iSystem* system)
{
  sys = system;
  iImage *proc_image;
  proc_image = (iImage*) new csImageMemory (mat_w, mat_h);
  
  tex = csEngine::current_engine->GetTextures ()->NewTexture (proc_image);
  if (!tex) 
    return false;

  if (key_color)
    tex->SetKeyColor (key_red, key_green, key_blue);

  tex->SetFlags (tex->GetFlags() | texFlags);
  tex->SetName (GetName ());
  if (use_cb)
    tex->SetUseCallback (ProcCallback, (void*)this);
  ptReady = true;
  return true;
}

bool csProcTexture::PrepareAnim ()
{
  iTextureHandle* txt_handle = tex->GetTextureHandle ();
  ptG3D = txt_handle->GetProcTextureInterface ();
  if (!ptG3D) return false;
  ptG2D = ptG3D->GetDriver2D ();
  ptTxtMgr = ptG3D->GetTextureManager ();
  return true;
}

csMaterialWrapper* csProcTexture::Initialize (iSystem * system,
    	csEngine* engine, iTextureManager* txtmgr, const char* name)
{
  SetName (name);
  Initialize (system);
  tex->Register (txtmgr);
  tex->GetTextureHandle ()->Prepare ();
  PrepareAnim ();
  csMaterial* material = new csMaterial ();
  csMaterialWrapper* mat = engine->GetMaterials ()->NewMaterial (material);
  mat->SetName (name);
  material->SetTextureWrapper (tex);
  material->DecRef ();
  mat->Register (txtmgr);
  mat->GetMaterialHandle ()->Prepare ();
  return mat;
}

#if 0
csMaterialWrapper* csProcTexture::Initialize2 (csEngine* engine, iTextureManager* txtmgr)
{
  //PrepareAnim ();

  csMaterial* material = new csMaterial ();
  csMaterialWrapper* mat = engine->GetMaterials ()->NewMaterial (material);
  mat->SetName (GetName ());
  material->SetTextureWrapper (tex);
  material->DecRef ();
  mat->Register (txtmgr);
  mat->GetMaterialHandle ()->Prepare ();
  return mat;
}
#endif

//--------------------------------------------------------------------------------
