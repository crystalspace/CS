/*  -*- Mode: C++; -*- */
/*
    Copyright (C) 1998 by Jorrit Tyberghein
    The DrawPixmap routine in a separate include file

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

/*
    This is the generalized DrawPixmapXX routine. Since the only difference
    between DrawPixmap routines was the datatype of a pixel, the former three
    different routines have been unified into a single file.

    Before including this file you should define the following macros:

    DP_NAME - The name of routine (DrawPixmap8 etc).
    DP_PIXTYPE - The type of a pixel (uint8, uint16, uint32)
*/

#if defined (DP_PIXFORM_R5G5B5)
#  define ALPHA_MIX(dst,src)						     \
  {									     \
    uint s = src, d = dst;						     \
    dst = (((((s & 0x7c1f) * srcf) + ((d & 0x7c1f) * dstf)) >> 5) & 0x7c1f)  \
        | (((((s & 0x03e0) * srcf) + ((d & 0x03e0) * dstf)) >> 5) & 0x03e0); \
  }
#elif defined (DP_PIXFORM_R5G6B5)
#  define ALPHA_MIX(dst,src)						     \
  {									     \
    uint s = src, d = dst;						     \
    dst = (((((s & 0xf81f) * srcf) + ((d & 0xf81f) * dstf)) >> 5) & 0xf81f)  \
        | (((((s & 0x07e0) * srcf) + ((d & 0x07e0) * dstf)) >> 5) & 0x07e0); \
  }
#elif defined (DP_PIXFORM_R8G8B8)
#  define ALPHA_MIX(dst,src)						     \
  {									     \
    uint s = src, d = dst;						     \
    dst = (((((s & 0xff00ff) * srcf) + ((d & 0xff00ff) * dstf)) >> 8) & 0xff00ff)\
        | (((((s & 0x00ff00) * srcf) + ((d & 0x00ff00) * dstf)) >> 8) & 0x00ff00);\
  }
#endif

#define NORM_ALPHA(a) (a)
#define INV_ALPHA(a)  (255 - a)

static void DP_NAME (iGraphics2D *G2D, iTextureHandle *hTex,
  int sx, int sy, int sw, int sh, int tx, int ty, int tw, int th, uint8 Alpha)
{
  if (Alpha == 255)
    return;

  bool alpha_chan = hTex->GetAlphaMap ();
  bool transp = hTex->GetKeyColor ();

  csSoftwareTextureHandle *tex_mm = (csSoftwareTextureHandle *)hTex->GetPrivateObject ();
  csSoftwareTexture *txt_unl = (csSoftwareTexture *)tex_mm->get_texture (0);

  int bw = txt_unl->get_width ();
  int bh = txt_unl->get_height ();

  uint32* bitmap = txt_unl->get_bitmap ();

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

  if (Alpha && alpha_chan)
  {
    /*register uint8 dstscale = Alpha, srcscale = 256 - dstscale*/;
    if (tile)
    {
      uint bwm = (bw << 16) - 1, bhm = (bh << 16) - 1;
      tx <<= 16;
      for (ty = (ty << 16) & bhm; sh > 0; sh--, ty = (ty + dy) & bhm, sy++)
      {
        DP_PIXTYPE *VRAM = (DP_PIXTYPE *)G2D->GetPixelAt (sx, sy);
        uint32* data = bitmap + (ty >> 16) * bw;
        int _tx = tx;
        if (transp)
          for (int w = sw; w; w--)
          {
            uint32 pixel = data [(_tx & bwm) >> 16];
            if (pixel)
            {
              register uint8
                srcf = /*(alpha [(_tx & bwm) >> 16] * srcscale) >> 8*/
		  pixel >> 24,
                dstf = INV_ALPHA (srcf);
              ALPHA_MIX (*VRAM, pixel);
            }
            VRAM++;
            _tx += dx;
          } /* endfor */
        else
          for (int w = sw; w; w--)
          {
            uint32 pixel = data [(_tx & bwm) >> 16];
            register uint8
              srcf = /*(alpha [(_tx & bwm) >> 16] * srcscale) >> 8*/
		pixel >> 24,
              dstf = INV_ALPHA (srcf);
            ALPHA_MIX (*VRAM, pixel);
            VRAM++;
            _tx += dx;
          } /* endfor */
      } /* endfor */
    }
    else
    {
      for (; sh > 0; sh--, ty += dy, sy++)
      {
        DP_PIXTYPE *VRAM = (DP_PIXTYPE *)G2D->GetPixelAt (sx, sy);
        uint32 *data = bitmap + (ty >> 16) * bw;
        int tx = 0;
        if (transp)
          for (int w = sw; w; w--)
          {
            uint32 pixel = data [tx >> 16];
            if (pixel)
            {
              register uint8
                srcf = /*(alpha [tx >> 16] * srcscale) >> 8*/
		  pixel >> 24,
                dstf = INV_ALPHA (srcf);
              ALPHA_MIX (*VRAM, pixel);
            }
            VRAM++;
            tx += dx;
          } /* endfor */
        else
          for (int w = sw; w; w--)
          {
            uint32 pixel = data [tx >> 16];
            register uint8
              srcf = /*(alpha [tx >> 16] * srcscale) >> 8*/
		pixel >> 24,
              dstf = INV_ALPHA (srcf);
            ALPHA_MIX (*VRAM, pixel);
            VRAM++;
            tx += dx;
          } /* endfor */
      } /* endfor */
    }
  }
  else if (alpha_chan)
  {
    if (tile)
    {
      uint bwm = (bw << 16) - 1, bhm = (bh << 16) - 1;
      tx <<= 16;
      for (ty = (ty << 16) & bhm; sh > 0; sh--, ty = (ty + dy) & bhm, sy++)
      {
        DP_PIXTYPE *VRAM = (DP_PIXTYPE *)G2D->GetPixelAt (sx, sy);
        uint32* data = bitmap + (ty >> 16) * bw;
        int _tx = tx;
        if (transp)
          for (int w = sw; w; w--)
          {
            uint32 pixel = data [(_tx & bwm) >> 16];
            if (pixel)
	    {
              register uint8
                srcf = /*alpha [(_tx & bwm) >> 16]*/
		  pixel >> 24,
                dstf = INV_ALPHA (srcf);
              ALPHA_MIX (*VRAM, pixel);
	    }
            VRAM++;
            _tx += dx;
          } /* endfor */
        else
          for (int w = sw; w; w--)
          {
            uint32 pixel = data [(_tx & bwm) >> 16];
            register uint8
              srcf = /*alpha [(_tx & bwm) >> 16]*/
		pixel >> 24,
              dstf = INV_ALPHA (srcf);
            ALPHA_MIX (*VRAM, pixel);
            VRAM++;
            _tx += dx;
          } /* endfor */
      } /* endfor */
    }
    else
    {
      for (; sh > 0; sh--, ty += dy, sy++)
      {
        DP_PIXTYPE *VRAM = (DP_PIXTYPE *)G2D->GetPixelAt (sx, sy);
        uint32* data = bitmap + (ty >> 16) * bw;
        int tx = 0;
        if (transp)
          for (int w = sw; w; w--)
          {
            uint32 pixel = data [tx >> 16];
            if (pixel)
	    {
              register uint8
                srcf = /*alpha [tx >> 16]*/pixel >> 24,
                dstf = INV_ALPHA (srcf);
              ALPHA_MIX (*VRAM, pixel);
	    }
            VRAM++;
            tx += dx;
          } /* endfor */
        else
          for (int w = sw; w; w--)
          {
            uint32 pixel = data [tx >> 16];
            register uint8
              srcf = /*alpha [tx >> 16]*/pixel >> 24,
              dstf = INV_ALPHA (srcf);
            ALPHA_MIX (*VRAM, pixel);
            VRAM++;
            tx += dx;
          } /* endfor */
      } /* endfor */
    }
  }
  else if (Alpha)
  {
    register uint8
      dstf = NORM_ALPHA (Alpha),
      srcf = INV_ALPHA (dstf);
    if (tile)
    {
      uint bwm = (bw << 16) - 1, bhm = (bh << 16) - 1;
      tx <<= 16;
      for (ty = (ty << 16) & bhm; sh > 0; sh--, ty = (ty + dy) & bhm, sy++)
      {
        DP_PIXTYPE *VRAM = (DP_PIXTYPE *)G2D->GetPixelAt (sx, sy);
        uint32 *data = bitmap + (ty >> 16) * bw;
        int _tx = tx;
        if (transp)
          for (int w = sw; w; w--)
          {
            uint32 pixel = data [(_tx & bwm) >> 16];
            if (pixel)
              ALPHA_MIX (*VRAM, pixel);
            VRAM++;
            _tx += dx;
          } /* endfor */
        else
          for (int w = sw; w; w--)
          {
            ALPHA_MIX (*VRAM, data [(_tx & bwm) >> 16]);
            VRAM++;
            _tx += dx;
          } /* endfor */
      } /* endfor */
    }
    else
    {
      for (; sh > 0; sh--, ty += dy, sy++)
      {
        DP_PIXTYPE *VRAM = (DP_PIXTYPE *)G2D->GetPixelAt (sx, sy);
        uint32 *data = bitmap + (ty >> 16) * bw;
        int tx = 0;
        if (transp)
          for (int w = sw; w; w--)
          {
            uint32 pixel = data [tx >> 16];
            if (pixel)
              ALPHA_MIX (*VRAM, pixel);
            VRAM++;
            tx += dx;
          } /* endfor */
        else
          for (int w = sw; w; w--)
          {
            ALPHA_MIX (*VRAM, data [tx >> 16]);
            VRAM++;
            tx += dx;
          } /* endfor */
      } /* endfor */
    }
  }
  else if (tile)
  {
    uint bwm = (bw << 16) - 1, bhm = (bh << 16) - 1;
    tx <<= 16;
    for (ty = (ty << 16) & bhm; sh > 0; sh--, ty = (ty + dy) & bhm, sy++)
    {
      DP_PIXTYPE *VRAM = (DP_PIXTYPE *)G2D->GetPixelAt (sx, sy);
      uint32 *data = bitmap + (ty >> 16) * bw;
      int _tx = tx;
      if (transp)
        for (int w = sw; w; w--)
        {
          uint32 pixel = data [(_tx & bwm) >> 16];
          if (pixel)
            *VRAM = pixel;
          VRAM++;
          _tx += dx;
        } /* endfor */
      else
        for (int w = sw; w; w--)
        {
          *VRAM++ = data [(_tx & bwm) >> 16];
          _tx += dx;
        } /* endfor */
    } /* endfor */
  }
  else if (sw == tw)
    for (; sh > 0; sh--, ty += dy, sy++)
    {
      DP_PIXTYPE *VRAM = (DP_PIXTYPE *)G2D->GetPixelAt (sx, sy);
      uint32 *data = bitmap + (ty >> 16) * bw;
      if (transp)
        for (int w = sw; w; w--)
        {
          uint32 pixel = *data;
          if (pixel)
            *VRAM = pixel;
          VRAM++;
          data++;
        } /* endfor */
      else
        for (int w = sw; w; w--)
          *VRAM++ = *data++;
    } /* endfor */
  else
    for (; sh > 0; sh--, ty += dy, sy++)
    {
      DP_PIXTYPE *VRAM = (DP_PIXTYPE *)G2D->GetPixelAt (sx, sy);
      uint32 *data = bitmap + (ty >> 16) * bw;
      int tx = 0;
      if (transp)
        for (int w = sw; w; w--)
        {
          uint32 pixel = data [tx >> 16];
          if (pixel)
            *VRAM = pixel;
          VRAM++;
          tx += dx;
        } /* endfor */
      else
        for (int w = sw; w; w--)
        {
          *VRAM++ = data [tx >> 16];
          tx += dx;
        } /* endfor */
    } /* endfor */
}

#undef NORM_ALPHA
#undef INV_ALPHA
#undef ALPHA_MIX
#undef DP_NAME
#undef DP_PIXTYPE
#undef DP_PIXFORM_R5G5B5
#undef DP_PIXFORM_R5G6B5
#undef DP_PIXFORM_R8G8B8
