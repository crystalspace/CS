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

  texFlags = CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS;
}

csEngineProcTex::~csEngineProcTex ()
{
}

bool csEngineProcTex::LoadLevel ()
{
  Engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  csRef<iVFS> vfs = CS_QUERY_REGISTRY (object_reg, iVFS);
  csRef<iLoader> loader = CS_QUERY_REGISTRY (object_reg, iLoader);
  // load a map file to display
  vfs->PushDir ();
  vfs->ChDir ("/lev/flarge/");
  bool Success = (loader->LoadMapFile ("world", false));
  vfs->PopDir ();
  if (!Success) return false;
  return true;
}

bool csEngineProcTex::PrepareAnim ()
{
  if (anim_prepared) return true;
  if (!csProcTexture::PrepareAnim ()) return false;

  return true;
}

void csEngineProcTex::Animate (csTicks CurrentTime)
{
  g3d->SetRenderTarget (tex->GetTextureHandle ());

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
  g3d->BeginDraw (CSDRAW_3DGRAPHICS | Engine->GetBeginDrawFlags () 
    | CSDRAW_CLEARSCREEN | CSDRAW_CLEARZBUFFER);
  //g3d->GetDriver2D()->Clear (g3d->GetDriver2D()->FindRGB (0, 255, 0));
  View->Draw ();
  g3d->FinishDraw ();

  // switch back to the old context
  Engine->SetContext (oldContext);
}

