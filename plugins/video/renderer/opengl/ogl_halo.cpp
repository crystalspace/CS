/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Contributions made by Robert Bergkvist <fragdance@hotmail.com>

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

/* This file is more or less a ripoff of Andrew Zabolotny's software
   halo renderer */

#include "cssysdef.h"
#include "qint.h"
#include "csgeom/math2d.h"
#include "csutil/util.h"
#include "ogl_g3dcom.h"
#include "ivideo/halo.h"
#include "ogl_txtcache.h"
#include "plugins/video/canvas/openglcommon/glstates.h"

class csOpenGLHalo : public iHalo
{
  /// The halo color
  float R, G, B;
  /// The width and height
  int Width, Height;
  /// Width and height factor
  float Wfact, Hfact;
  /// Blending method
  uint dstblend;
  /// Our OpenGL texture handle
  GLuint halohandle;
  /// The OpenGL 3D driver
  csGraphics3DOGLCommon *G3D;

public:
  SCF_DECLARE_IBASE;

  csOpenGLHalo (float iR, float iG, float iB, unsigned char *iAlpha,
    int iWidth, int iHeight, csGraphics3DOGLCommon *iG3D);

  virtual ~csOpenGLHalo ();

  virtual int GetWidth () { return Width; }
  virtual int GetHeight () { return Height; }

  virtual void SetColor (float &iR, float &iG, float &iB)
  { R = iR; G = iG; B = iB; }

  virtual void GetColor (float &oR, float &oG, float &oB)
  { oR = R; oG = G; oB = B; }

  virtual void Draw (float x, float y, float w, float h, float iIntensity,
    csVector2 *iVertices, int iVertCount);
};

SCF_IMPLEMENT_IBASE (csOpenGLHalo)
  SCF_IMPLEMENTS_INTERFACE (iHalo)
SCF_IMPLEMENT_IBASE_END

csOpenGLHalo::csOpenGLHalo (float iR, float iG, float iB, unsigned char *iAlpha,
  int iWidth, int iHeight, csGraphics3DOGLCommon *iG3D)
{
  SCF_CONSTRUCT_IBASE (0);

  // Initialization
  R = iR; G = iG; B = iB;
  // OpenGL can only use 2^n sized textures
  Width = csFindNearestPowerOf2 (iWidth);
  Height = csFindNearestPowerOf2 (iHeight);

  uint8 *Alpha = iAlpha;
  if ((Width != iWidth) || (Height != iHeight))
  {
    // Allocate our copy of the scanline which is power-of-two
    Alpha = new uint8 [Width * Height];
    int i;
    for (i = 0; i < iHeight; i++)
    {
      // Copy a scanline from the supplied alphamap
      memcpy (Alpha + (i * Width), iAlpha + (i * iWidth), iWidth);
      // Clear the tail of the scanline
      memset (Alpha + (i * Width) + iWidth, 0, Width - iWidth);
    }
  }

  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
  // Create handle
  glGenTextures (1, &halohandle);
  // Activate handle
  csGraphics3DOGLCommon::statecache->SetTexture (GL_TEXTURE_2D, halohandle);

  // Jaddajaddajadda
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D (GL_TEXTURE_2D, 0, GL_ALPHA, Width, Height, 0, GL_ALPHA,
    GL_UNSIGNED_BYTE, Alpha);

  if (Alpha != iAlpha)
    delete [] Alpha;
  (G3D = iG3D)->IncRef ();

  Wfact = float (iWidth) / Width;
  Hfact = float (iHeight) / Height;

  Width = iWidth;
  Height = iHeight;

  if (R > 1.0 || G > 1.0 || B > 1.0)
  {
    dstblend = CS_FX_HALOOVF;
    R /= 2; G /= 2; B /= 2;
  }
  else
    dstblend = CS_FX_ALPHA;
}

csOpenGLHalo::~csOpenGLHalo ()
{
  // Kill, crush and destroy
  // Delete generated OpenGL handle
  glDeleteTextures (1, &halohandle);
  G3D->DecRef ();
  SCF_DESTRUCT_IBASE();
}

// Draw the halo. Wasn't that a suprise
void csOpenGLHalo::Draw (float x, float y, float w, float h, float iIntensity,
  csVector2 *iVertices, int iVertCount)
{
  int swidth = G3D->width;
  int sheight = G3D->height;
  int i;

  G3D->FlushDrawPolygon();

  if (w < 0) w = Width;
  if (h < 0) h = Height;

  csVector2 HaloPoly [4];
  if (!iVertices)
  {
    iVertCount = 4;
    iVertices = HaloPoly;

    float x1 = x, y1 = y, x2 = x + w, y2 = y + h;
    if (x1 < 0) x1 = 0; if (x2 > swidth ) x2 = swidth ;
    if (y1 < 0) y1 = 0; if (y2 > sheight) y2 = sheight;
    if ((x1 >= x2) || (y1 >= y2))
      return;

    HaloPoly [0].Set (x1, y1);
    HaloPoly [1].Set (x1, y2);
    HaloPoly [2].Set (x2, y2);
    HaloPoly [3].Set (x2, y1);
  };

  /// The inverse width and height of the halo
  float inv_W = Wfact / w, inv_H = Hfact / h;

  //???@@@glMatrixMode (GL_MODELVIEW);
  glPushMatrix ();
  glTranslatef (0, 0, 0);

  csGraphics3DOGLCommon::SetGLZBufferFlags (CS_ZBUF_NONE);
  csGraphics3DOGLCommon::statecache->Enable_GL_TEXTURE_2D ();

  csGraphics3DOGLCommon::statecache->SetShadeModel (GL_FLAT);
  csGraphics3DOGLCommon::statecache->SetTexture (GL_TEXTURE_2D, halohandle);

  csGraphics3DOGLCommon::SetupBlend (dstblend, 0, false);
  glColor4f (R, G, B, iIntensity);

  glBegin (GL_POLYGON);
  for (i = iVertCount - 1; i >= 0; i--)
  {
    float vx = iVertices [i].x, vy = iVertices [i].y;
    glTexCoord2f ((vx - x) * inv_W, (vy - y) * inv_H);
    glVertex2f (vx, sheight - vy);
  }
  glEnd ();

  glPopMatrix ();
}

iHalo *csGraphics3DOGLCommon::CreateHalo (float iR, float iG, float iB,
  unsigned char *iAlpha, int iWidth, int iHeight)
{
  return new csOpenGLHalo (iR, iG, iB, iAlpha, iWidth, iHeight, this);
}
