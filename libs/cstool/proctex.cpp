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
#include "iutil/plugin.h"
#include "ivideo/txtmgr.h"
#include "ivideo/material.h"

#include "csgfx/memimage.h"
#include "iengine/texture.h"
#include "iengine/material.h"
#include "iengine/engine.h"
#include "iutil/objreg.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/comp.h"
#include "iutil/virtclk.h"
#include "csutil/csvector.h"

#include "cstool/proctex.h"

//---------------------------------------------------------------------------

/*
 * Event handler that takes care of updating all procedural
 * textures that were visible last frame.
 */
class ProcEventHandler : public iEventHandler
{
private:
  iObjectRegistry* object_reg;
  // Array of textures that needs updating next frame.
  csVector textures;

public:
  ProcEventHandler (iObjectRegistry* object_reg)
  {
    SCF_CONSTRUCT_IBASE (NULL);
    ProcEventHandler::object_reg = object_reg;
  }
  virtual ~ProcEventHandler () { }

  SCF_DECLARE_IBASE;
  virtual bool HandleEvent (iEvent& event);

  void PushTexture (csProcTexture* txt)
  {
    textures.Push (txt);
  }
};

SCF_IMPLEMENT_IBASE (ProcEventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

bool ProcEventHandler::HandleEvent (iEvent& event)
{
  csRef<iVirtualClock> vc (CS_QUERY_REGISTRY (object_reg, iVirtualClock));
  csTicks elapsed_time, current_time;
  elapsed_time = vc->GetElapsedTicks ();
  current_time = vc->GetCurrentTicks ();
  if (event.Type == csevBroadcast && event.Command.Code == cscmdPreProcess)
  {
    int i;
    for (i = 0 ; i < textures.Length () ; i++)
    {
      csProcTexture* pt = (csProcTexture*)textures[i];
      pt->Animate (current_time);
      pt->last_cur_time = current_time;
    }
    textures.DeleteAll ();
    return true;
  }
  return false;
}

//---------------------------------------------------------------------------

csProcTexture::csProcTexture ()
{
  ptReady = false;
  tex = NULL;
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

iEventHandler* csProcTexture::SetupProcEventHandler (
	iObjectRegistry* object_reg)
{
  csRef<iEventHandler> proceh = CS_QUERY_REGISTRY_TAG_INTERFACE (object_reg,
  	"crystalspace.proctex.eventhandler", iEventHandler);
  if (proceh) return proceh;
  proceh = csPtr<iEventHandler> (new ProcEventHandler (object_reg));
  csRef<iEventQueue> q (CS_QUERY_REGISTRY (object_reg, iEventQueue));
  if (q != 0)
  {
    q->RegisterListener (proceh, CSMASK_Nothing);
    object_reg->Register (proceh, "crystalspace.proctex.eventhandler");
  }
  return proceh;
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
  ((ProcEventHandler*)(iEventHandler*)(pt->proceh))->PushTexture (pt);
}

bool csProcTexture::Initialize (iObjectRegistry* object_reg)
{
  csProcTexture::object_reg = object_reg;
  proceh = SetupProcEventHandler (object_reg);

  iImage *proc_image;
  proc_image = (iImage*) new csImageMemory (mat_w, mat_h);

#ifndef CS_USE_NEW_RENDERER
  g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
#else
  g3d = CS_QUERY_REGISTRY (object_reg, iRender3D);
#endif // CS_USE_NEW_RENDERER
  g2d = CS_QUERY_REGISTRY (object_reg, iGraphics2D);

  csRef<iEngine> engine (CS_QUERY_REGISTRY (object_reg, iEngine));
  tex = engine->GetTextureList ()->NewTexture (proc_image);
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
  csRef<iMaterial> material (engine->CreateBaseMaterial (tex));
  iMaterialWrapper* mat = engine->GetMaterialList ()->NewMaterial (material);
  mat->QueryObject ()->SetName (name);
  if (txtmgr)
  {
    mat->Register (txtmgr);
    mat->GetMaterialHandle ()->Prepare ();
  }
  return mat;
}

//-----------------------------------------------------------------------------

