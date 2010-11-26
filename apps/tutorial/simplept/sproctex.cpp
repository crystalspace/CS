/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#include "simplept.h"

csEngineProcTex::csEngineProcTex() : csProcTexture ()
{
  mat_w = 256;
  mat_h = 256;

  texFlags = CS_TEXTURE_3D;
}

csEngineProcTex::~csEngineProcTex ()
{
}

bool csEngineProcTex::LoadLevel ()
{
  Engine = csQueryRegistry<iEngine> (object_reg);
  csRef<iVFS> vfs = csQueryRegistry<iVFS> (object_reg);
  csRef<iLoader> loader = csQueryRegistry<iLoader> (object_reg);
  // load a map file to display
  vfs->PushDir ();
  vfs->ChDir ("/lev/partsys/");
  bool Success = (loader->LoadMapFile ("world", false));
  vfs->PopDir ();
  if (!Success) return false;
  return true;
}

struct TargetToUse
{
  csRenderTargetAttachment attachment;
  const char* format;
};
static const TargetToUse targetsToUse[] = {
  {rtaColor0, "rgb8"},
  {rtaColor0, "rgb16_f"},
  {rtaDepth,  "d32"}
};

static const char* AttachmentToStr (csRenderTargetAttachment a)
{
  switch(a)
  {
  case rtaColor0: return "color0";
  case rtaDepth:  return "depth";
  default: return 0;
  }
}

iTextureWrapper* csEngineProcTex::CreateTexture (iObjectRegistry* object_reg)
{
  iTextureWrapper* tex = 0;

  csRef<iEngine> engine (csQueryRegistry<iEngine> (object_reg));
  
  for (size_t n = 0; n < sizeof(targetsToUse)/sizeof(targetsToUse[0]); n++)
  {
    if (!g3d->CanSetRenderTarget (targetsToUse[n].format,
      targetsToUse[n].attachment))
    {
      csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
	  "crystalspace.application.simplept",
	  "Format unsupported: %s(%s)", targetsToUse[n].format,
          AttachmentToStr (targetsToUse[n].attachment));
      continue;
    }
      
    csRef<iTextureHandle> texHandle = 
      g3d->GetTextureManager()->CreateTexture (mat_w, mat_h, csimg2D, 
        targetsToUse[n].format, CS_TEXTURE_3D | texFlags);
    if (!texHandle) continue;
    
    Target target;
    target.texh = texHandle;
    target.format = targetsToUse[n].format;
    target.attachment = targetsToUse[n].attachment;
    targets.Push (target);
    
    availableFormatsStr.AppendFmt ("%s ", target.format);
    
    if (!tex)
    {
      tex = engine->GetTextureList()->NewTexture (texHandle);
      currentTargetStr.Format ("%s(%s)", AttachmentToStr (target.attachment),
        target.format);
    }
  }

  return tex;
}

bool csEngineProcTex::PrepareAnim ()
{
  if (anim_prepared) return true;
  if (!csProcTexture::PrepareAnim ()) return false;

  return true;
}

void csEngineProcTex::Animate (csTicks CurrentTime)
{
  if (!(renderTargetState = g3d->SetRenderTarget (targets[currentTarget].texh,
    false, 0, targets[currentTarget].attachment)))
    return;
  if (!(renderTargetState = g3d->ValidateRenderTargets ()))
    return;

  if (!View.IsValid())
  {
    // @@@ Here to get the render target size, not screen size
    iSector *room = Engine->GetSectors ()->FindByName ("room");
    View = csPtr<iView> (new csView (Engine, g3d));
    View->GetCamera ()->GetTransform ().SetOrigin (csVector3 (-0.5,0,0));
    View->GetCamera ()->SetSector (room);
    View->SetRectangle (0, 0, 256, 256);
    View->GetCamera ()->SetPerspectiveCenter (128, 128);
    View->GetCamera ()->SetFOVAngle (View->GetCamera ()->GetFOVAngle(), 256);
  }

  // move the camera
  csVector3 Position (-0.5, 0, 3 + sin (CurrentTime / (10*1000.0))*3);
  View->GetCamera ()->Move (Position - View->GetCamera ()
  	->GetTransform ().GetOrigin ());

  // Switch to the context of the procedural texture.
  iTextureHandle *oldContext = Engine->GetContext ();
  Engine->SetContext (tex->GetTextureHandle ());

  // Draw the engine view.
  g3d->BeginDraw (CSDRAW_3DGRAPHICS);
  //g3d->GetDriver2D()->Clear (g3d->GetDriver2D()->FindRGB (0, 255, 0));
  View->Draw ();
  g3d->FinishDraw ();

  // switch back to the old context
  Engine->SetContext (oldContext);
}

void csEngineProcTex::CycleTarget()
{
  currentTarget = (currentTarget + 1) % targets.GetSize();
  const Target& target = targets[currentTarget];
  currentTargetStr.Format ("%s(%s)", AttachmentToStr (target.attachment),
    target.format);
  tex->SetTextureHandle (targets[currentTarget].texh);
}
