/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter

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

#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/txtmgr.h"
#include "iutil/document.h"
#include "iutil/objreg.h"
#include "cstool/proctxtanim.h"

//----------------------------------------------------------------------------

csProcAnimated::csProcAnimated (iImage* img) : csProcTexture (0, img)
{
  image = img;
  animation = csPtr<iAnimatedImage>
    (SCF_QUERY_INTERFACE (image, iAnimatedImage));

  mat_w = image->GetWidth ();
  mat_h = image->GetHeight ();

  texFlags = CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS;

  last_time = (csTicks)-1;
}

csProcAnimated::~csProcAnimated ()
{
}

bool csProcAnimated::PrepareAnim ()
{
  if (anim_prepared) return true;
  if (!csProcTexture::PrepareAnim ()) return false;
  return true;
}

void csProcAnimated::Animate (csTicks current_time)
{
  bool first = (last_time == (csTicks)-1);
  
  if (first || animation)
  {
    if (first || animation->Animate (current_time - last_time))
    {
      tex->GetTextureHandle ()->Blit (0, 0, mat_w, mat_h, (unsigned char*)
	  image->GetImageData ());
    }
    last_time = current_time;
  }
}

