/*
    Copyright (C) 1998 by Jorrit Tyberghein

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
#include <stdarg.h>
#include "csplugincommon/render3d/txtmgr.h"
#include "csutil/util.h"
#include "csutil/debug.h"
#include "csgfx/imagemanipulate.h"
#include "iutil/objreg.h"
#include "csqint.h"
#include "igraphic/image.h"
#include "ivideo/material.h"
#include "iengine/material.h"
#include "ivideo/graph2d.h"

//----------------------------------------------------- csTextureHandle -----//


csTextureHandle::csTextureHandle (csTextureManager* texman, int Flags)
  : scfImplementationType (this), texman (texman)
{
  flags = Flags & ~CS_TEXTURE_NPOTS;

  transp = false;
  transp_color.red = transp_color.green = transp_color.blue = 0;

  texClass = texman->texClassIDs.Request ("default");
}

csTextureHandle::~csTextureHandle ()
{ 
}

void csTextureHandle::SetKeyColor (bool Enable)
{
  transp = Enable;
}

// This function must be called BEFORE calling TextureManager::Update().
void csTextureHandle::SetKeyColor (uint8 red, uint8 green, uint8 blue)
{
  transp_color.red = red;
  transp_color.green = green;
  transp_color.blue = blue;
  transp = true;
}

/// Get the transparent color
void csTextureHandle::GetKeyColor (uint8 &r, uint8 &g, uint8 &b) const
{
  r = transp_color.red;
  g = transp_color.green;
  b = transp_color.blue;
}

bool csTextureHandle::GetKeyColor () const
{
  return transp;
}

void csTextureHandle::AdjustSizePo2 (int width, int height, int depth, 
				     int& newwidth, int& newheight, int& newdepth)
{
  CalculateNextBestPo2Size (flags, width, newwidth);
  CalculateNextBestPo2Size (flags, height, newheight);
  CalculateNextBestPo2Size (flags, depth, newdepth);
}

void csTextureHandle::CalculateNextBestPo2Size (int /*texFlags*/, 
						const int orgDim, int& newDim)
{
  newDim = csFindNearestPowerOf2 (orgDim);
  if (newDim != orgDim)
  {
    int dU = newDim - orgDim;
    int dD = orgDim - (newDim >> 1);
    if (dD < dU)
      newDim >>= 1;
  }
}

void csTextureHandle::SetTextureClass (const char* className)
{
  texClass = texman->texClassIDs.Request (className ? className : "default");
}

const char* csTextureHandle::GetTextureClass ()
{
  return texman->texClassIDs.Request (texClass);
}

//----------------------------------------------------- csTextureManager -----//


csTextureManager::csTextureManager (iObjectRegistry* object_reg,
	iGraphics2D *iG2D)
  : scfImplementationType (this), textures (16, 16), object_reg (object_reg)
{
  pfmt = *iG2D->GetPixelFormat ();

  csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
    object_reg, "crystalspace.shared.stringset", iStringSet);
  CS_ASSERT(strings != 0);
  nameDiffuseTexture = strings->Request (CS_MATERIAL_TEXTURE_DIFFUSE);
}

csTextureManager::~csTextureManager()
{
  Clear ();
}

void csTextureManager::read_config (iConfigFile* /*config*/)
{
}
