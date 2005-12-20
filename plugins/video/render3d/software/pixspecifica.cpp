/*
    Copyright (C) 2005 by Jorrit Tyberghein
              (C) 2005 by Frank Richter

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

#include "csgeom/math2d.h"
#include "csgeom/tri.h"
#include "csgeom/vector3.h"
#include "cstool/rbuflock.h"

#include "sft3dcom.h"
#include "clipper.h"
#include "clip_znear.h"
#include "clip_iclipper.h"
#include "scan_pix.h"
#include "tridraw.h"

namespace cspluginSoft3d
{
  template<typename Pix>
  class Specifica : public iPixTypeSpecifica
  {
    typedef typename_qualifier Pix::PixType PixType;
    Pix pix;

    template<typename SrcBlend, typename DstBlend, int alphaTest,
      int constantAlpha>
    void Draw (const Pix& pix, iGraphics2D *G2D,
      uint32* bitmap, int sx, int sy, int sw, int sh, 
      int tx, int ty, int dx, int dy, int bw, int bh,
      uint8 Alpha = 255)
    {
      SrcBlend srcFactor;
      DstBlend dstFactor;
      uint bwm = (bw << 16) - 1, bhm = (bh << 16) - 1;
      tx <<= 16; ty = (ty << 16) & bhm;
      for (; sh > 0; sh--, ty = (ty + dy) & bhm, sy++)
      {
	PixType *VRAM = (PixType*)G2D->GetPixelAt (sx, sy);
	uint32 *data = bitmap + (ty >> 16) * bw;
	int x = tx;
	for (int w = sw; w; w--)
	{
	  Pixel px (data [(x & bwm) >> 16]);
	  if (constantAlpha)
	    px *= Alpha;
	  if (!alphaTest || (px.c.a & 0x80))
	  {
	    const Pixel dp = pix.GetPix (VRAM);
	    if (dstFactor.GetBlendFact() == CS_MIXMODE_FACT_ZERO)
	      pix.WritePix (VRAM, srcFactor.Apply (px, dp));
	    else
	      pix.WritePix (VRAM, 
		srcFactor.Apply (px, dp) + dstFactor.Apply (px, dp));
	  }
	  VRAM++;
	  x += dx;
	} /* endfor */
      } /* endfor */
    }

    void Draw (const Pix& pix, iGraphics2D *G2D,
      uint32* bitmap, int sx, int sy, int sw, int sh, 
      int tx, int ty, int dx, int dy, int bw, int bh,
      uint8 Alpha, csAlphaMode::AlphaType alphaType)
    {
      if (Alpha > 0)
      {
	switch (alphaType)
	{
	  default:
	  case csAlphaMode::alphaNone:
	  case csAlphaMode::alphaSmooth:
	    Draw<Factor_SrcAlpha<FactorColorSrc, 0>, 
	      Factor_SrcAlpha<FactorColorDst, 1>, 0, 1> (
	      pix, G2D, bitmap, sx, sy, sw, sh, tx, ty, dx, dy, bw, bh,
	      ~Alpha);
	    break;
	  case csAlphaMode::alphaBinary:
	    Draw<Factor_SrcAlpha<FactorColorSrc, 0>, 
	      Factor_SrcAlpha<FactorColorDst, 1>, 1, 1> (
	      pix, G2D, bitmap, sx, sy, sw, sh, tx, ty, dx, dy, bw, bh,
	      ~Alpha);
	    break;
	}
      }
      else
      {
	switch (alphaType)
	{
	  default:
	  case csAlphaMode::alphaNone:
	    Draw<Factor_Zero<FactorColorSrc, 1>, 
	      Factor_Zero<FactorColorDst, 0>, 0, 0> (
	      pix, G2D, bitmap, sx, sy, sw, sh, tx, ty, dx, dy, bw, bh);
	    break;
	  case csAlphaMode::alphaSmooth:
	    Draw<Factor_SrcAlpha<FactorColorSrc, 0>, 
	      Factor_SrcAlpha<FactorColorDst, 1>, 0, 0> (
	      pix, G2D, bitmap, sx, sy, sw, sh, tx, ty, dx, dy, bw, bh);
	    break;
	  case csAlphaMode::alphaBinary:
	    Draw<Factor_SrcAlpha<FactorColorSrc, 0>, 
	      Factor_SrcAlpha<FactorColorDst, 1>, 1, 0> (
	      pix, G2D, bitmap, sx, sy, sw, sh, tx, ty, dx, dy, bw, bh);
	    break;
	}
      }
    }
  public:
    Specifica (const Pix& pix) : pix (pix) {}

    virtual void DrawPixmap (csSoftwareGraphics3DCommon* G3D, 
      iTextureHandle *hTex, int sx, int sy, int sw, int sh, 
      int tx, int ty, int tw, int th, 
      uint8 Alpha)
    {
      if (Alpha == 255)
	return;

      iGraphics2D* G2D = G3D->GetDriver2D();
      csSoftwareTextureHandle *tex_mm = 
	(csSoftwareTextureHandle *)hTex->GetPrivateObject ();
      csSoftwareTexture *txt_unl = 
	(csSoftwareTexture *)tex_mm->GetTexture (0);

      int bw = txt_unl->w;
      int bh = txt_unl->h;

      uint32* bitmap = txt_unl->bitmap;

      /// Retrieve clipping rectangle
      int ClipX1, ClipY1, ClipX2, ClipY2;
      G2D->GetClipRect (ClipX1, ClipY1, ClipX2, ClipY2);

      int dx = (tw << 16) / sw;
      int dy = (th << 16) / sh;

      // Clipping
      if ((sx >= ClipX2) || (sy >= ClipY2)
      || (sx + sw <= ClipX1) || (sy + sh <= ClipY1))
	return;                             // Sprite is totally invisible
      if (sx < ClipX1)                      // Left margin crossed?
      {
	int nw = sw - (ClipX1 - sx);        // New width
	tx += (ClipX1 - sx) * tw / sw;      // Adjust X coord on texture
	tw = (tw * nw) / sw;                // Adjust width on texture
	sw = nw; sx = ClipX1;
      } /* endif */
      if (sx + sw > ClipX2)                 // Right margin crossed?
      {
	int nw = ClipX2 - sx;               // New width
	tw = (tw * nw) / sw;                // Adjust width on texture
	sw = nw;
      } /* endif */
      if (sy < ClipY1)                      // Top margin crossed?
      {
	int nh = sh - (ClipY1 - sy);        // New height
	ty += (ClipY1 - sy) * th / sh;      // Adjust Y coord on texture
	th = (th * nh) / sh;                // Adjust height on texture
	sh = nh; sy = ClipY1;
      } /* endif */
      if (sy + sh > ClipY2)                 // Bottom margin crossed?
      {
	int nh = ClipY2 - sy;               // New height
	th = (th * nh) / sh;                // Adjust height on texture
	sh = nh;
      } /* endif */

      bool tile = (tx < 0) || (tx + tw > bw) || (ty < 0) || (ty + th > bh);
      if (!tile)
      {
	bitmap += ty * bw + tx;
	ty = 0;
      }

      csAlphaMode::AlphaType alphaType = hTex->GetAlphaType();
      if ((G3D->DrawMode & CSDRAW_2DGRAPHICS) 
	&& (alphaType == csAlphaMode::alphaBinary))
	alphaType = csAlphaMode::alphaSmooth;

      Draw (pix, G2D, bitmap, sx, sy, sw, sh, tx, ty, dx, dy, bw, bh,
	Alpha, alphaType);
    }
    void BlitScreenToTexture (uint8** line_table, int txt_w, int txt_h,
      int scr_w, int scr_h, uint32* bitmap)
    {
      const int blitW = csMin (txt_w, scr_w);
      const int linePad = txt_w - blitW;
      const int blitH = csMin (txt_h, scr_h);
      for (int y = 0 ; y < blitH ; y++)
      {
	PixType* d = (PixType*)line_table[y];
	for (int x = 0 ; x < blitW; x++)
	{
	  const Pixel px (pix.GetPix (d++));
	  *bitmap++ = px.ui32;
	}
	bitmap += linePad;
      }
    }
    void BlitTextureToScreen (uint8** line_table, int txt_w, int txt_h,
      int scr_w, int scr_h, uint32* bitmap)
    {
      const int blitW = csMin (txt_w, scr_w);
      const int linePad = txt_w - blitW;
      const int blitH = csMin (txt_h, scr_h);
      for (int y = 0 ; y < blitH ; y++)
      {
	PixType* d = (PixType*)line_table[y];
	for (int x = 0 ; x < blitW; x++)
	{
	  const Pixel px (*bitmap++);
	  pix.WritePix (d++, px);
	}
	bitmap += linePad;
      }
    }
  };

  template<typename Pix>
  static void SetupPixTypeSpecifica (csSoftwareGraphics3DCommon* g3d)
  {
    TriDrawMatrixFiller<Pix>::Fill (g3d, g3d->triDraw);
    g3d->specifica = new Specifica<Pix> (Pix (
      *g3d->GetDriver2D()->GetPixelFormat()));
  }

  void csSoftwareGraphics3DCommon::SetupSpecifica ()
  {
    if (pfmt.PixelBytes == 4)
    {
      if (((pfmt.BlueMask == 0x0000ff) || (pfmt.RedMask == 0x0000ff))
	&& (pfmt.GreenMask == 0x00ff00)
	&& ((pfmt.RedMask == 0xff0000) || (pfmt.BlueMask == 0xff0000)))
	SetupPixTypeSpecifica <Pix_Fix<uint32, 24, 0xff,
					       16, 0xff,
					       8,  0xff,
					       0,  0xff> > (this);
      else
	SetupPixTypeSpecifica <Pix_Generic<uint32> > (this);
    }
    else
    {
      if (((pfmt.RedMask == 0xf800) || (pfmt.BlueMask == 0xf800))
	&& (pfmt.GreenMask == 0x07e0)
	&& ((pfmt.BlueMask == 0x001f) || (pfmt.RedMask == 0x001f)))
	SetupPixTypeSpecifica <Pix_Fix<uint16, 0,  0,
					       8,  0xf8,
					       3,  0xfc,
					      -3, 0xf8> > (this);
      else if (((pfmt.RedMask == 0x7c00) || (pfmt.BlueMask == 0x7c00))
	&& (pfmt.GreenMask == 0x03e0)
	&& ((pfmt.BlueMask == 0x001f) || (pfmt.RedMask == 0x001f)))
	SetupPixTypeSpecifica <Pix_Fix<uint16, 0,  0,
					       7,  0xf8,
					       2,  0xf8,
					       -3, 0xf8> > (this);
      else
	SetupPixTypeSpecifica <Pix_Generic<uint16> > (this);
    }
  }

} // namespace cspluginSoft3d
