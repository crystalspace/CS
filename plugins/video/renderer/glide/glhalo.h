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

#ifndef GL_HALO_H
#define GL_HALO_H

#include "qint.h"
#include "csgeom/math2d.h"
#include "csutil/util.h"
#include "g3dgl.h"
#include "glcache.h"
#include "glalpha.h"
#include "video/canvas/glide2common/iglide2d.h"

class csGraphics3DGlide;

// The structure that stays behind csHaloHandle
class csGlideHalo : public iHalo
{
  // Halo R,G,B components
  int R, G, B;
  // Halo size
  int Width, Height;

  // Software 3D rasterizer
  csGraphics3DGlide *G3D;

  csGlideAlphaMap *am;
  csVector2 *vert;
  MyGrVertex *vx;
  int oldVerts;
public:

  SCF_DECLARE_IBASE;

  // Create the halo object
  csGlideHalo (float iR, float iG, float iB, int iWidth, int iHeight, csGraphics3DGlide *iG3D, csGlideAlphaMap *am );

  // Destroy the halo object
  virtual ~csGlideHalo ();

  //---------------------------------// iHalo interface implementation //-----//
  /// Query halo width
  virtual int GetWidth () { return Width; }

  /// Query halo height
  virtual int GetHeight () { return Height; }

  /// Change halo color
  virtual void SetColor (float &iR, float &iG, float &iB)
  { R = int(255. * iR); G = int(255. * iG); B = int(255. * iB); }

  /// Query halo color
  virtual void GetColor (float &oR, float &oG, float &oB)
  { oR = R/255.; oG = G/255.; oB = B/255.; }

  virtual void Draw (float x, float y, float w, float h, float iIntensity, csVector2 *iVertices, int iVertCount);
};

#endif
