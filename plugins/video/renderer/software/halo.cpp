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

#include "sysdef.h"
#include "qint.h"
#include "csgeom/math2d.h"
#include "csutil/util.h"
#include "soft_g3d.h"
#include "scan.h"

// The structure that stays behind csHaloHandle
class csSoftHalo : public iHalo
{
  // Halo R,G,B components
  float R, G, B;
  // Halo alpha map
  unsigned char *Alpha;
  // Halo size
  int Width, Height;

  // Software 3D rasterizer
  csGraphics3DSoftware *G3D;

public:

  DECLARE_IBASE;

  // Create the halo object
  csSoftHalo (float iR, float iG, float iB, unsigned char *iAlpha,
    int iWidth, int iHeight, csGraphics3DSoftware *iG3D);

  // Destroy the halo object
  virtual ~csSoftHalo ();

  //---------------------------------// iHalo interface implementation //-----//
  /// Query halo width
  virtual int GetWidth () { return Width; }

  /// Query halo height
  virtual int GetHeight () { return Height; }

  /// Change halo color
  virtual void SetColor (float &iR, float &iG, float &iB)
  { R = iR; G = iG; B = iB; }

  /// Query halo color
  virtual void GetColor (float &oR, float &oG, float &oB)
  { oR = R; oG = G; oB = B; }

  /**
   * Draw the halo given a center point and an intensity. 
   * If either w and/or h is negative, the native width and/or height
   * is used instead. If the halo should be clipped against some
   * polygon, that polygon should be given, otherwise if a NULL pointer
   * is passed, the halo is clipped just against screen bounds.
   */
  virtual void Draw (float x, float y, float w, float h, float iIntensity,
    csVector2 *iVertices, int iVertCount);
};

//----------------// Halo scanline rasterizing functions for all modes //-----//

void halo_dscan_8 (void *src, void *dest, int count, int delta)
{
  unsigned char *A = (unsigned char *)src;
  unsigned char *D = (unsigned char *)dest;
  unsigned char *E = D + count;
  if (delta == 0x10000)
    while (D < E)
    {
      unsigned int a = (*A++ * Scan.FogDensity) & 0x1f00;
      if (a)
        *D = Scan.Fog8 [(a ^ 0x1f00) | *D];
      D++;
    }
  else
  {
    unsigned int ax = 0;
    while (D < E)
    {
      unsigned int a = (A [ax >> 16] * Scan.FogDensity) & 0x1f00;
      if (a)
        *D = Scan.Fog8 [(a ^ 0x1f00) | *D];
      D++;
      ax += delta;
    }
  }
}

#define HALO_NAME	halo_dscan_16_555
#define HALO_BPP	16
#define HALO_RM		0x7c00
#define HALO_GM		0x03e0
#define HALO_BM		0x001f
#include "haloscan.inc"

#define HALO_NAME	halo_dscan_16_555_c
#define HALO_BPP	16
#define HALO_RM		0x7c00
#define HALO_GM		0x03e0
#define HALO_BM		0x001f
#define HALO_CLAMP
#include "haloscan.inc"

#define HALO_NAME	halo_dscan_16_565
#define HALO_BPP	16
#define HALO_RM		0xf800
#define HALO_GM		0x07e0
#define HALO_BM		0x001f
#include "haloscan.inc"

#define HALO_NAME	halo_dscan_16_565_c
#define HALO_BPP	16
#define HALO_RM		0xf800
#define HALO_GM		0x07e0
#define HALO_BM		0x001f
#define HALO_CLAMP
#include "haloscan.inc"

#define HALO_NAME	halo_dscan_32
#define HALO_BPP	32
#define HALO_RM		0xff0000
#define HALO_GM		0x00ff00
#define HALO_BM		0x0000ff
#include "haloscan.inc"

#define HALO_NAME	halo_dscan_32_c
#define HALO_BPP	32
#define HALO_RM		0xff0000
#define HALO_GM		0x00ff00
#define HALO_BM		0x0000ff
#define HALO_CLAMP
#include "haloscan.inc"

//----------------------------------------------// Software halo class //-----//

IMPLEMENT_IBASE (csSoftHalo)
  IMPLEMENTS_INTERFACE (iHalo)
IMPLEMENT_IBASE_END

// Create the halo object
csSoftHalo::csSoftHalo (float iR, float iG, float iB, unsigned char *iAlpha,
  int iWidth, int iHeight, csGraphics3DSoftware *iG3D)
{
  CONSTRUCT_IBASE (NULL);
  R = iR; G = iG; B = iB;
  CHK (Alpha = new unsigned char [(Width = iWidth) * (Height = iHeight)]);
  memcpy (Alpha, iAlpha, Width * Height);
  (G3D = iG3D)->IncRef ();
}

// Destroy the halo object
csSoftHalo::~csSoftHalo ()
{
  CHKB (delete [] Alpha);
  G3D->DecRef ();
}

void csSoftHalo::Draw (float x, float y, float w, float h, float iIntensity,
  csVector2 *iVertices, int iVertCount)
{
  // Check if halo is visible
  if (iIntensity <= 0)
    return;

  if (w < 0) w = Width;
  if (h < 0) h = Height;

#if defined (TOP8BITS_R8G8B8_USED)
  // Used with R|G|B|A big-endian encoding
  int PostShift;
#endif
  // Draw a single scanline of halo
  void (*dscan) (void *src, void *dest, int count, int delta);

  if (G3D->pfmt.PixelBytes == 1)
  {
    Scan.FogR = QRound (R * 255);
    Scan.FogG = QRound (G * 255);
    Scan.FogB = QRound (B * 255);
    Scan.Fog8 = G3D->BuildIndexedFogTable ();
    Scan.FogPix = G3D->txtmgr->find_rgb (Scan.FogR, Scan.FogG, Scan.FogB);
    // halo intensity (0..31)
    Scan.FogDensity = QRound (iIntensity * 32);
  }
  else
  {
    Scan.FogR = QRound (R * G3D->pfmt.RedMask  ) & G3D->pfmt.RedMask;
    Scan.FogG = QRound (G * G3D->pfmt.GreenMask) & G3D->pfmt.GreenMask;
    Scan.FogB = QRound (B * G3D->pfmt.BlueMask ) & G3D->pfmt.BlueMask;
    // halo intensity (0..255)
    Scan.FogDensity = QRound (iIntensity * 255);
  }

  bool clamp = (R * iIntensity > 1) || (G * iIntensity > 1) || (B * iIntensity > 1);
  switch (G3D->pfmt.PixelBytes)
  {
    case 1:
      dscan = halo_dscan_8;
      break;
    case 2:
      if (G3D->pfmt.GreenBits == 6)
        dscan = clamp ? halo_dscan_16_565_c : halo_dscan_16_565;
      else
        dscan = clamp ? halo_dscan_16_555_c : halo_dscan_16_555;
      break;
    case 4:
    {
      dscan = clamp ? halo_dscan_32_c : halo_dscan_32;
      unsigned int rs = G3D->pfmt.RedShift;
      unsigned int gs = G3D->pfmt.GreenShift;
#if defined (TOP8BITS_R8G8B8_USED)
      PostShift = (G3D->pfmt.RedShift && G3D->pfmt.GreenShift &&
        G3D->pfmt.BlueShift) ? 8 : 0;
      rs -= PostShift; gs -= PostShift;
#endif
      unsigned int r = (rs == 16) ? Scan.FogR : (gs == 16) ? Scan.FogG : Scan.FogB;
      unsigned int g = (rs ==  8) ? Scan.FogR : (gs ==  8) ? Scan.FogG : Scan.FogB;
      unsigned int b = (rs ==  0) ? Scan.FogR : (gs ==  0) ? Scan.FogG : Scan.FogB;
      Scan.FogR = r; Scan.FogG = g; Scan.FogB = b;
      break;
    }
    default:
      return;
  }

  csVector2 HaloPoly [4];
  if (!iVertices)
  {
    iVertCount = 4;
    iVertices = HaloPoly;

    float x1 = x, y1 = y, x2 = x + w, y2 = y + h;
    if (x1 < 0) x1 = 0; if (x2 > G3D->width ) x2 = G3D->width ;
    if (y1 < 0) y1 = 0; if (y2 > G3D->height) y2 = G3D->height;
    if ((x1 >= x2) || (y1 >= y2))
      return;

    HaloPoly [0].Set (x1, y1);
    HaloPoly [1].Set (x1, y2);
    HaloPoly [2].Set (x2, y2);
    HaloPoly [3].Set (x2, y1);
  };

  // Draw the halo polygon
  int i, min_i, max_i;
  float min_y, max_y;
  min_i = max_i = 0;
  min_y = max_y = iVertices [0].y;
  for (i = 1; i < iVertCount; i++)
  {
    if (min_y > iVertices [i].y)
    { min_i = i; min_y = iVertices [i].y; }
    if (max_y < iVertices [i].y)
    { max_i = i; max_y = iVertices [i].y; }
  }

  int scanL1, scanL2, scanR1, scanR2;   // scan vertex left/right start/final
  float sxL, sxR, dxL, dxR;             // scanline X left/right and deltas
  int sy, fyL, fyR;                     // scanline Y, final Y left, final Y right

  sxL = sxR = dxL = dxR = 0;            // avoid GCC warnings about "uninitialized variables"
  scanL2 = scanR2 = min_i;
  sy = fyL = fyR = QRound (iVertices [scanL2].y);

  // Shift amount to get pixel address
  int pixel_shift = log2 (G3D->pfmt.PixelBytes);
  // Compute halo X and Y scale
  float scaleX = w / Width;
  float scaleY = h / Height;
  // Compute rounded top-left halo corner coordinate
  int xTL = QRound (x);
  int yTL = QRound (y);
  // Compute horizontal motion delta
  int delta = QInt16 (scaleX);

  // The halo polygon is counterclockwise (since the clipping polygon is
  // counterclockwise). Now we should draw it from top to bottom.

  for ( ; ; )
  {
    //-----
    // We have reached the next segment. Recalculate the slopes.
    //-----
    bool leave;
    do
    {
      leave = true;
      if (sy >= fyR)
      {
        // Check first if polygon has been finished
        if (scanR2 == max_i)
          return;
        scanR1 = scanR2;
	if (--scanR2 < 0)
	  scanR2 = iVertCount - 1;

        leave = false;
        fyR = QRound (iVertices [scanR2].y);
        if (sy >= fyR)
          continue;

        float dyR = (iVertices [scanR2].y - iVertices [scanR1].y);
        if (dyR)
        {
          sxR = iVertices [scanR1].x;
          dxR = (iVertices [scanR2].x - sxR) / dyR;
          // horizontal pixel correction
          sxR += dxR * (iVertices [scanR1].y - (float (sy) - 0.5));
        } /* endif */
      } /* endif */
      if (sy >= fyL)
      {
        scanL1 = scanL2;
	if (++scanL2 >= iVertCount)
	  scanL2 = 0;

        leave = false;
        fyL = QRound (iVertices [scanL2].y);
        if (sy >= fyL)
          continue;

        float dyL = (iVertices [scanL2].y - iVertices [scanL1].y);
        if (dyL)
        {
          sxL = iVertices [scanL1].x;
          dxL = (iVertices [scanL2].x - sxL) / dyL;
          // horizontal pixel correction
          sxL += dxL * (iVertices [scanL1].y - (float (sy) - 0.5));
        } /* endif */
      } /* endif */
    } while (!leave); /* enddo */

    // Find the trapezoid top
    int fin_y;
    if (fyL < fyR)
      fin_y = fyL;
    else
      fin_y = fyR;

    while (sy < fin_y)
    {
      if ((sy & 1) != G3D->do_interlaced)
      {
        // Compute the rounded screen coordinates of horizontal strip
        int xL = QRound (sxL);
        int xR = QRound (sxR);
        unsigned char *d = G3D->line_table [sy] + (xL << pixel_shift);
        unsigned char *s = Alpha + QRound (scaleY * (sy - yTL)) * Width +
          QRound (scaleX * (xL - xTL));
        dscan (s, d, xR - xL, delta);
      }

      sxL += dxL;
      sxR += dxR;
      sy++;
    } /* endwhile */
  } /* endfor */
}

iHalo *csGraphics3DSoftware::CreateHalo (float iR, float iG, float iB,
  unsigned char *iAlpha, int iWidth, int iHeight)
{
  CHK (csSoftHalo *h = new csSoftHalo (iR, iG, iB, iAlpha, iWidth, iHeight, this));
  return h;
}
