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

    DRAWSPRITE_NAME - The name of routine (DrawPixmap8 etc).
    DRAWSPRITE_PIXTYPE - The type of a pixel (UByte, UShort, ULong)
*/

static void DRAWSPRITE_NAME (iGraphics2D *G2D, iTextureHandle *hTex,
  int sx, int sy, int sw, int sh, int tx, int ty, int tw, int th)
{
  bool transp = hTex->GetAlphaMap ();

  csTextureMMNull *tex_mm = (csTextureMMNull *)hTex->GetPrivateObject ();
  csTextureNull *txt_unl = (csTextureNull *)tex_mm->get_texture (0);

  int bw = txt_unl->get_width ();
  //  int bh = txt_unl->get_height ();

  UByte *bitmap = txt_unl->get_bitmap ();
  DRAWSPRITE_PIXTYPE *palette = (DRAWSPRITE_PIXTYPE *)tex_mm->GetPaletteToGlobal ();

  /// Retrieve clipping rectangle
  int ClipX1, ClipY1, ClipX2, ClipY2;
  G2D->GetClipRect (ClipX1, ClipY1, ClipX2, ClipY2);

  int dx = (tw << 16) / sw;
  int dy = (th << 16) / sh;

  // Clipping
  if ((sx >= ClipX2) || (sy >= ClipY2) ||
      (sx + sw <= ClipX1) || (sy + sh <= ClipY1))
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

  bitmap += ty * bw + tx;
  ty = 0;

  if (sw == tw)
    for (; sh > 0; sh--, ty += dy, sy++)
    {
      DRAWSPRITE_PIXTYPE *VRAM = (DRAWSPRITE_PIXTYPE *)G2D->GetPixelAt (sx, sy);
      UByte *data = bitmap + (ty >> 16) * bw;
      if (transp)
        for (int w = sw; w; w--)
        {
          UByte pixel = *data;
          if (pixel)
            *VRAM = palette [pixel];
          VRAM++;
          data++;
        } /* endfor */
      else
        for (int w = sw; w; w--)
          *VRAM++ = palette [*data++];
    } /* endfor */
  else
    for (; sh > 0; sh--, ty += dy, sy++)
    {
      DRAWSPRITE_PIXTYPE *VRAM = (DRAWSPRITE_PIXTYPE *)G2D->GetPixelAt (sx, sy);
      UByte *data = bitmap + (ty >> 16) * bw;
      int tx = 0;
      if (transp)
        for (int w = sw; w; w--)
        {
          UByte pixel = data [tx >> 16];
          if (pixel)
            *VRAM = palette [pixel];
          VRAM++;
          tx += dx;
        } /* endfor */
      else
        for (int w = sw; w; w--)
        {
          *VRAM++ = palette [data [tx >> 16]];
          tx += dx;
        } /* endfor */
    } /* endfor */
}

#undef DRAWSPRITE_NAME
#undef DRAWSPRITE_PIXTYPE
