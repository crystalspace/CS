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

#include "cssysdef.h"
#include "simplept.h"
#include "cstool/csview.h"
#include "csgfx/memimage.h"
#include "csutil/cscolor.h"

#include "ivideo/txtmgr.h"
#include "ivideo/texture.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "imap/parser.h"
#include "iengine/engine.h"
#include "iengine/camera.h"

csEngineProcTex::csEngineProcTex()
{
  TexHandle = NULL;
  ptG3D = NULL;
  Engine = NULL;
  View = NULL;
}

csEngineProcTex::~csEngineProcTex ()
{
  SCF_DEC_REF (TexHandle);
  SCF_DEC_REF (View);
}

bool csEngineProcTex::Initialize (iGraphics3D *g3d, iEngine *engine,
	iVFS *vfs, iLoader *Loader)
{
  // copy the engine pointer
  Engine = engine;

  // load a map file to display
  vfs->PushDir ();
  vfs->ChDir ("/lev/flarge/");
  bool Success = (Loader->LoadMapFile ("world", false));
  vfs->PopDir ();
  if (!Success) return false;

  // create a procedural texture
  iImage *Image = new csImageMemory (256, 256);
  TexHandle = g3d->GetTextureManager ()->RegisterTexture (Image,
    CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS | CS_TEXTURE_PROC);
  TexHandle->Prepare ();
  ptG3D = TexHandle->GetProcTextureInterface ();

  // set up a view for the engine
  iSector *room = Engine->FindSector ("room");
  View = new csView (Engine, ptG3D);
  View->GetCamera ()->GetTransform ().SetOrigin (csVector3 (-0.5,0,0));
  View->GetCamera ()->SetSector (room);
  View->SetRectangle (0, 0, 256, 256);
  View->GetCamera ()->SetPerspectiveCenter (128, 128);

  return true;
}

void csEngineProcTex::Update (csTime CurrentTime)
{
  // move the camera
  csVector3 Position (-0.5, 0, 3 + sin (CurrentTime / 1000.0)*3);
  View->GetCamera ()->Move (Position - View->GetCamera ()->GetTransform ().GetOrigin ());

  // switch to the context of the procedural texture
  iGraphics3D *oldContext = Engine->GetContext ();
  Engine->SetContext (ptG3D);

  // draw the engine view
  ptG3D->BeginDraw (CSDRAW_3DGRAPHICS);
  View->Draw ();
  ptG3D->FinishDraw ();
  ptG3D->Print (NULL);

  // switch back to the old context
  Engine->SetContext (oldContext);
}
