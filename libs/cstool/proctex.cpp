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

#include "cssysdef.h"
#include <math.h>

#include "csgfx/memimage.h"
#include "cstool/proctex.h"
#include "csutil/hash.h"
#include "iengine/engine.h"
#include "iengine/material.h"
#include "iengine/texture.h"
#include "igraphic/image.h"
#include "iutil/comp.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/virtclk.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/material.h"
#include "ivideo/txtmgr.h"

//---------------------------------------------------------------------------

/*
 * Event handler that takes care of updating all procedural
 * textures that were visible last frame.
 */
class csProcTexEventHandler : public iEventHandler
{
private:
  iObjectRegistry* object_reg;
  // Set of textures that needs updating next frame.
  csSet<csProcTexture*> textures;

public:
  csProcTexEventHandler (iObjectRegistry* r)
  {
    SCF_CONSTRUCT_IBASE (0);
    object_reg = r;
  }
  virtual ~csProcTexEventHandler ()
  {
    SCF_DESTRUCT_IBASE ();
  }

  SCF_DECLARE_IBASE;
  virtual bool HandleEvent (iEvent& event);

  void PushTexture (csProcTexture* txt)
  {
    textures.Add (txt);
  }
  void PopTexture (csProcTexture* txt)
  {
    textures.Delete (txt);
  }
};

SCF_IMPLEMENT_IBASE (csProcTexEventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

bool csProcTexEventHandler::HandleEvent (iEvent& event)
{
  csRef<iVirtualClock> vc (CS_QUERY_REGISTRY (object_reg, iVirtualClock));
  csTicks elapsed_time, current_time;
  elapsed_time = vc->GetElapsedTicks ();
  current_time = vc->GetCurrentTicks ();
  csSet<csProcTexture*> keep_tex;
  if (event.Type == csevBroadcast && event.Command.Code == cscmdPreProcess)
  {
    {
      csSet<csProcTexture*>::GlobalIterator it = textures.GetIterator();
      while (it.HasNext ())
      {
        csProcTexture* pt = it.Next ();
	if (!pt->anim_prepared)
	  pt->PrepareAnim();
	if (pt->anim_prepared)
          pt->Animate (current_time);
	pt->visible = false;
	if (pt->always_animate) keep_tex.Add (pt);
        pt->last_cur_time = current_time;
      }
    }
    textures.DeleteAll ();
    // enqueue 'always animate' textures for next cycle
    csSet<csProcTexture*>::GlobalIterator it = keep_tex.GetIterator();
    while (it.HasNext ())
    {
      csProcTexture* pt = it.Next ();
      textures.Add (pt);
    }
    return true;
  }
  return false;
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE_EXT (csProcTexture)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iTextureWrapper)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iProcTexture)
SCF_IMPLEMENT_IBASE_EXT_END

csProcTexture::csProcTexture (iTextureFactory* p, iImage* image)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiTextureWrapper);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiProcTexture);

  ptReady = false;
  tex = 0;
  texFlags = 0;
  key_color = false;
  object_reg = 0;
  use_cb = true;
  last_cur_time = 0;
  anim_prepared = false;
  always_animate = false;
  visible = false;
  parent = p;
  proc_image = image;
}

csProcTexture::~csProcTexture ()
{
  if (proceh != 0)
    ((csProcTexEventHandler*)(iEventHandler*)(proceh))->PopTexture (this);

  SCF_DESTRUCT_EMBEDDED_IBASE (scfiProcTexture);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiTextureWrapper);
}

iEventHandler* csProcTexture::SetupProcEventHandler (
	iObjectRegistry* object_reg)
{
  csRef<iEventHandler> proceh = CS_QUERY_REGISTRY_TAG_INTERFACE (object_reg,
  	"crystalspace.proctex.eventhandler", iEventHandler);
  if (proceh) return proceh;
  proceh = csPtr<iEventHandler> (new csProcTexEventHandler (object_reg));
  csRef<iEventQueue> q (CS_QUERY_REGISTRY (object_reg, iEventQueue));
  if (q != 0)
  {
    q->RegisterListener (proceh, CSMASK_Nothing);
    object_reg->Register (proceh, "crystalspace.proctex.eventhandler");
  }
  return proceh;
}

struct csProcTexCallback : public iTextureCallback, iProcTexCallback
{
  csRef<csProcTexture> pt;
  SCF_DECLARE_IBASE;
  csProcTexCallback () { SCF_CONSTRUCT_IBASE (0); }
  virtual ~csProcTexCallback () { SCF_DESTRUCT_IBASE(); }
  virtual void UseTexture (iTextureWrapper*);
  virtual iProcTexture* GetProcTexture() const;
};

SCF_IMPLEMENT_IBASE (csProcTexCallback)
  SCF_IMPLEMENTS_INTERFACE (iTextureCallback)
  SCF_IMPLEMENTS_INTERFACE (iProcTexCallback)
SCF_IMPLEMENT_IBASE_END

void csProcTexCallback::UseTexture (iTextureWrapper*)
{
  if (!pt->PrepareAnim ()) return;
  pt->visible = true;
  ((csProcTexEventHandler*)(iEventHandler*)(pt->proceh))->PushTexture (pt);
}
iProcTexture* csProcTexCallback::GetProcTexture() const
{
  return &pt->scfiProcTexture;
}

bool csProcTexture::Initialize (iObjectRegistry* object_reg)
{
  csProcTexture::object_reg = object_reg;
  proceh = SetupProcEventHandler (object_reg);

  if (!proc_image.IsValid())
    proc_image.AttachNew (new csImageMemory (mat_w, mat_h));

  g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  g2d = CS_QUERY_REGISTRY (object_reg, iGraphics2D);

  csRef<iEngine> engine (CS_QUERY_REGISTRY (object_reg, iEngine));
  tex = engine->GetTextureList ()->NewTexture (proc_image);
  proc_image = 0;
  if (!tex)
    return false;

  if (key_color)
    tex->SetKeyColor (key_red, key_green, key_blue);

  tex->SetFlags (tex->GetFlags() | texFlags);
  tex->QueryObject ()->SetName (GetName ());
  if (use_cb)
  {
    csProcTexCallback* cb = new csProcTexCallback ();
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
  }
  //PrepareAnim ();
  csRef<iMaterial> material (engine->CreateBaseMaterial (tex));
  iMaterialWrapper* mat = engine->GetMaterialList ()->NewMaterial (material,
  	name);
  if (txtmgr)
  {
    mat->Register (txtmgr);
  }
  return mat;
}

bool csProcTexture::GetAlwaysAnimate () const
{
  return always_animate;
}

void csProcTexture::SetAlwaysAnimate (bool enable)
{
  always_animate = enable;
  if (always_animate)
  {
    ((csProcTexEventHandler*)(iEventHandler*)proceh)->PushTexture (this);
  }
}

//-----------------------------------------------------------------------------

SCF_IMPLEMENT_EMBEDDED_IBASE (csProcTexture::eiTextureWrapper)
  SCF_IMPLEMENTS_INTERFACE (iTextureWrapper)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

iObject* csProcTexture::eiTextureWrapper::QueryObject()
{
  return scfParent->tex->QueryObject();
}

iTextureWrapper* csProcTexture::eiTextureWrapper::Clone () const
{
  return scfParent->tex->Clone();
}

void csProcTexture::eiTextureWrapper::SetImageFile (iImage *Image)
{
  scfParent->tex->SetImageFile (Image);
}

iImage* csProcTexture::eiTextureWrapper::GetImageFile ()
{
  return scfParent->tex->GetImageFile();
}

void csProcTexture::eiTextureWrapper::SetTextureHandle (iTextureHandle *tex)
{
  scfParent->tex->SetTextureHandle (tex);
}

iTextureHandle* csProcTexture::eiTextureWrapper::GetTextureHandle ()
{
  return scfParent->tex->GetTextureHandle();
}

void csProcTexture::eiTextureWrapper::SetKeyColor (int red, int green, int blue)
{
  scfParent->tex->SetKeyColor (red, green, blue);
}

void csProcTexture::eiTextureWrapper::GetKeyColor (int &red, int &green, int &blue) const
{
  scfParent->tex->GetKeyColor (red, green, blue);
}

void csProcTexture::eiTextureWrapper::SetFlags (int flags)
{
  scfParent->tex->SetFlags (flags);
}

int csProcTexture::eiTextureWrapper::GetFlags () const
{
  return scfParent->tex->GetFlags();
}

void csProcTexture::eiTextureWrapper::Register (iTextureManager *txtmng)
{
  scfParent->tex->Register (txtmng);
}

void csProcTexture::eiTextureWrapper::SetUseCallback (iTextureCallback* callback)
{
  scfParent->tex->SetUseCallback (callback);
}

iTextureCallback* csProcTexture::eiTextureWrapper::GetUseCallback () const
{
  return scfParent->tex->GetUseCallback();
}

void csProcTexture::eiTextureWrapper::Visit ()
{
  scfParent->tex->Visit();
}

bool csProcTexture::eiTextureWrapper::IsVisitRequired () const
{
  return scfParent->tex->IsVisitRequired ();
}

void csProcTexture::eiTextureWrapper::SetKeepImage (bool k)
{
  scfParent->tex->SetKeepImage (k);
}

bool csProcTexture::eiTextureWrapper::KeepImage () const
{
  return scfParent->tex->KeepImage();
}

void csProcTexture::eiTextureWrapper::SetTextureClass (const char* className)
{
  scfParent->tex->SetTextureClass (className);
}

const char* csProcTexture::eiTextureWrapper::GetTextureClass ()
{
  return scfParent->tex->GetTextureClass();
}

//-----------------------------------------------------------------------------

SCF_IMPLEMENT_EMBEDDED_IBASE (csProcTexture::eiProcTexture)
  SCF_IMPLEMENTS_INTERFACE (iProcTexture)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

bool csProcTexture::eiProcTexture::GetAlwaysAnimate () const
{
  return scfParent->GetAlwaysAnimate();
}

void csProcTexture::eiProcTexture::SetAlwaysAnimate (bool enable)
{
  scfParent->SetAlwaysAnimate (enable);
}
iTextureFactory* csProcTexture::eiProcTexture::GetFactory()
{
  return scfParent->parent;
}

