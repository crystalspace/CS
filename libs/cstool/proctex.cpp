/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein
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
#include "isys/plugin.h"
#include "ivideo/txtmgr.h"
#include "ivideo/material.h"

#include "csgfx/memimage.h"
#include "iengine/texture.h"
#include "iengine/material.h"
#include "iengine/engine.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"

#include "cstool/proctex.h"

csProcTexture::csProcTexture ()
{
  ptReady = false;
  tex = NULL;
  ptG3D = NULL;
  ptG2D = NULL;
  ptTxtMgr = NULL;
  texFlags = 0;
  key_color = false;
  object_reg = NULL;
  use_cb = true;
  last_cur_time = 0;
  anim_prepared = false;
}

csProcTexture::~csProcTexture ()
{
}

struct ProcCallback : public iTextureCallback
{
  csProcTexture* pt;
  SCF_DECLARE_IBASE;
  ProcCallback () { SCF_CONSTRUCT_IBASE (NULL); }
  virtual ~ProcCallback () { }
  virtual void UseTexture (iTextureWrapper*);
};

SCF_IMPLEMENT_IBASE (ProcCallback)
  SCF_IMPLEMENTS_INTERFACE (iTextureCallback)
SCF_IMPLEMENT_IBASE_END

void ProcCallback::UseTexture (iTextureWrapper*)
{
  if (!pt->PrepareAnim ()) return;
  csTicks elapsed_time, current_time;
  iSystem* sys = CS_GET_SYSTEM (pt->object_reg);	//@@@
  sys->GetElapsedTime (elapsed_time, current_time);
  if (pt->last_cur_time == current_time) return;
  pt->Animate (current_time);
  pt->last_cur_time = current_time;
}

bool csProcTexture::Initialize (iObjectRegistry* object_reg)
{
  csProcTexture::object_reg = object_reg;
  iImage *proc_image;
  proc_image = (iImage*) new csImageMemory (mat_w, mat_h);

  iPluginManager* plugin_mgr = CS_QUERY_REGISTRY (
  	object_reg, iPluginManager);
  iEngine* engine = CS_QUERY_PLUGIN (plugin_mgr, iEngine);
  tex = engine->GetTextureList ()->NewTexture (proc_image);
  engine->DecRef ();
  proc_image->DecRef ();
  if (!tex) 
    return false;

  if (key_color)
    tex->SetKeyColor (key_red, key_green, key_blue);

  tex->SetFlags (tex->GetFlags() | texFlags);
  tex->QueryObject ()->SetName (GetName ());
  if (use_cb)
  {
    struct ProcCallback* cb = new struct ProcCallback ();
    cb->pt = this;
    tex->SetUseCallback (cb);
    cb->DecRef ();
  }
  ptReady = true;
  return true;
}

bool csProcTexture::PrepareAnim ()
{
  if (anim_prepared) return true;
  iTextureHandle* txt_handle = tex->GetTextureHandle ();
  if (!txt_handle) return false;
  ptG3D = txt_handle->GetProcTextureInterface ();
  if (!ptG3D) return false;
  ptG2D = ptG3D->GetDriver2D ();
  ptTxtMgr = ptG3D->GetTextureManager ();
  anim_prepared = true;
  return true;
}

iMaterialWrapper* csProcTexture::Initialize (iObjectRegistry * object_reg,
    	iEngine* engine, iTextureManager* txtmgr, const char* name)
{
  SetName (name);
  Initialize (object_reg);
  if (txtmgr)
  {
    tex->Register (txtmgr);
    tex->GetTextureHandle ()->Prepare ();
  }
  //PrepareAnim ();
  iMaterial* material = engine->CreateBaseMaterial (tex);
  iMaterialWrapper* mat = engine->GetMaterialList ()->NewMaterial (material);
  mat->QueryObject ()->SetName (name);
  material->DecRef ();
  if (txtmgr)
  {
    mat->Register (txtmgr);
    mat->GetMaterialHandle ()->Prepare ();
  }
  return mat;
}

//--------------------------------------------------------------------------------
