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

#ifndef __CS_SOFT3D_POLYRAST_H__
#define __CS_SOFT3D_POLYRAST_H__

#include "csqint.h"

#include "sft3dcom.h"
#include "scanindex.h"

namespace cspluginSoft3d
{

  class PolygonRasterizer
  {
    bool dpfx_valid;
    bool dpfx_use_fog;
    iTextureHandle* dpfx_tex_handle;
    uint dpfx_mixmode;
    csZBufMode dpfx_z_buf_mode;
    csZBufMode z_buf_mode;
    bool do_gouraud;
    bool do_lighting;
    bool do_textured;
    /// Alpha mask used for 16-bit mode.
    uint16 alpha_mask;
    bool is_for_procedural_textures;

    csDrawPIScanline** ScanProcPI;
    csDrawPIScanlineGouraud** ScanProcPIG;

    csPixelFormat pfmt;
    csSoftwareTextureManager* texman;

    int width, height;
    int do_interlaced;

    uint32* z_buffer;
    uint8** line_table;
    int pixel_shift;

    // Calculate round (f) of a 16:16 fixed pointer number
    // and return a long integer.
    inline long round16 (long f)
    {
      return (f + 0x8000) >> 16;
    }
  public:
    PolygonRasterizer() : dpfx_valid(false), do_gouraud(true),
      do_lighting(true), do_textured(true), 
      is_for_procedural_textures(false), do_interlaced(-1)
    {
    }

    struct
    {
      iTextureHandle* tex_handle;
      int redFact, greenFact, blueFact;
      int max_r, max_g, max_b;
      int twfp, thfp;
      float tw, th;
      unsigned char *bm;
      int shf_w;
      bool keycolor;
      bool textured;
      bool tiling;
      uint mixmode;
      csDrawPIScanline *drawline;
      csDrawPIScanlineGouraud *drawline_gouraud;
    } pqinfo;
    
    void RealStartPolygonFX (iTextureHandle* handle,
      uint mode, bool use_fog)
    {
      if (!dpfx_valid ||
	    use_fog != dpfx_use_fog ||
	    handle != dpfx_tex_handle ||
	    z_buf_mode != dpfx_z_buf_mode ||
	    mode != dpfx_mixmode)
      {
	dpfx_valid = true;
	dpfx_use_fog = use_fog;
	dpfx_tex_handle = handle;
	dpfx_z_buf_mode = z_buf_mode;
	dpfx_mixmode = mode;
      }
      else return;
    
      if (!do_gouraud || !do_lighting)
	mode |= CS_FX_FLAT;
    
      pqinfo.tex_handle = handle;
    
      if (pqinfo.tex_handle)
      {
	csSoftwareTextureHandle *tex_mm = (csSoftwareTextureHandle*)
	    pqinfo.tex_handle->GetPrivateObject ();
	csSoftwareTexture *txt_unl = (csSoftwareTexture *)tex_mm->get_texture (0);
	csScan_InitDrawFX (tex_mm, txt_unl);
	pqinfo.bm = txt_unl->get_bitmap ();
	pqinfo.tw = txt_unl->get_width ();
	pqinfo.th = txt_unl->get_height ();
	pqinfo.shf_w = txt_unl->get_w_shift ();
	pqinfo.twfp = csQfixed16 (pqinfo.tw) - 1;
	pqinfo.thfp = csQfixed16 (pqinfo.th) - 1;
	pqinfo.keycolor = tex_mm->GetKeyColor ();
	pqinfo.textured = do_textured;
	pqinfo.tiling = !!(mode & CS_FX_TILING);
      }
      else
	pqinfo.textured = false;
    
      Scan.AlphaMask = alpha_mask;
    
      Scan.BlendTable = 0;
      // array to select blend tables from
      unsigned char **BlendingTable = Scan.BlendingTable;
      if(is_for_procedural_textures) // proc manager uses its own blend tables
	BlendingTable = Scan.BlendingTableProc;
      pqinfo.drawline = 0;
      pqinfo.drawline_gouraud = 0;
    
      if (pqinfo.textured && Scan.AlphaMap)
      {
	int scan_index =
	  (z_buf_mode == CS_ZBUF_USE) ? SCANPROC_PI_TEX_ALPHA_ZUSE :
	  (z_buf_mode == CS_ZBUF_FILL) ? SCANPROC_PI_TEX_ALPHA_ZFIL :
	  (z_buf_mode == CS_ZBUF_TEST) ? SCANPROC_PI_TEX_ALPHA_ZTEST :
	  SCANPROC_PI_TEX_ALPHA_ZNONE;
	pqinfo.drawline = ScanProcPI [scan_index];
      }
      switch (mode & CS_FX_MASK_MIXMODE)
      {
	case CS_FX_ADD:
	  Scan.BlendTable = BlendingTable [BLENDTABLE_ADD];
	  break;
	case CS_FX_MULTIPLY:
	  Scan.BlendTable = BlendingTable [BLENDTABLE_MULTIPLY];
	  break;
	case CS_FX_MULTIPLY2:
	  Scan.BlendTable = BlendingTable [BLENDTABLE_MULTIPLY2];
	  break;
	case CS_FX_ALPHA:
	{
	  int alpha = mode & CS_FX_MASK_ALPHA;
	  if (alpha < 12)
	    mode = (mode & ~CS_FX_MASK_MIXMODE) | CS_FX_COPY;
	  else if (alpha < 96)
	    Scan.BlendTable = BlendingTable [BLENDTABLE_ALPHA25];
	  else if (alpha < 160)
	    Scan.BlendTable = BlendingTable [BLENDTABLE_ALPHA50];
	  else if (alpha < 244)
	    Scan.BlendTable = BlendingTable [BLENDTABLE_ALPHA75];
	  else
	    goto zfill_only;
	  break;
	}
	case CS_FX_TRANSPARENT:
    zfill_only:
	  mode |= CS_FX_FLAT;
	  pqinfo.drawline = (z_buf_mode == CS_ZBUF_USE)
	    ? 0
	    : csScan_scan_pi_zfil;
	  break;
	default:
	  break;
      }
    
      // Select draw scanline routines
      int scan_index = pqinfo.textured
	    ? SCANPROC_PI_TEX_ZNONE
	    : SCANPROC_PI_FLAT_ZNONE;
      if (z_buf_mode == CS_ZBUF_FILL) scan_index++;
      else if (z_buf_mode == CS_ZBUF_USE) scan_index += 2;
      else if (z_buf_mode == CS_ZBUF_TEST) scan_index += 3;
      if (pqinfo.textured && pqinfo.keycolor)
	scan_index += 4;
      if ((mode & CS_FX_MASK_MIXMODE) != CS_FX_COPY)
	scan_index += 20;
      if (pqinfo.textured && (mode & CS_FX_TILING))
	scan_index += 8;
      if (!pqinfo.drawline)
	pqinfo.drawline = ScanProcPI [scan_index];
      if (!(mode & CS_FX_FLAT))
	pqinfo.drawline_gouraud = ScanProcPIG [scan_index];
    
      pqinfo.mixmode = mode;
      // We use #.16 fixed-point format for R,G,B factors
      // where # is the number of bits per component (with the exception of
      // 32bpp modes/textured where we use (#-2).16 format).
      int shift_amount =
	((pfmt.PixelBytes == 4) && (Scan.BlendTable || pqinfo.textured)) ? 6 : 8;
      pqinfo.redFact = (((pfmt.RedMask>>pfmt.RedShift)+1) << shift_amount)-1;
      pqinfo.greenFact = (((pfmt.GreenMask>>pfmt.GreenShift)+1) << shift_amount)-1;
      pqinfo.blueFact  = (((pfmt.BlueMask>>pfmt.BlueShift)+1)   << shift_amount)-1;
    
      pqinfo.max_r = (1 << (pfmt.RedBits   + shift_amount + 8)) - 1;
      pqinfo.max_g = (1 << (pfmt.GreenBits + shift_amount + 8)) - 1;
      pqinfo.max_b = (1 << (pfmt.BlueBits  + shift_amount + 8)) - 1;
    }

    void DrawPolygonFX (G3DPolygonDPFX& poly)
    {
      RealStartPolygonFX (poly.tex_handle, poly.mixmode, poly.use_fog);
    
      if (!pqinfo.drawline && !pqinfo.drawline_gouraud)
	return;
    
      // Determine the R/G/B of the polygon's flat color
      if (pqinfo.textured)
	Scan.FlatRGB.Set (255, 255, 255);
      else
      {
	Scan.FlatRGB.Set (
	  poly.flat_color_r, poly.flat_color_g, poly.flat_color_b);
      }
    
      // Get the same value as a pixel-format-encoded value
      Scan.FlatColor = texman->encode_rgb (Scan.FlatRGB.red, Scan.FlatRGB.green,
	Scan.FlatRGB.blue);
    
      //-----
      // Get the values from the polygon for more convenient local access.
      // Also look for the top-most and bottom-most vertices.
      //-----
      float uu[64], vv[64], iz[64];
      float rr[64], gg[64], bb[64];
      size_t top, bot;
      float top_y = -99999;
      float bot_y = 99999;
      top = bot = 0;                        // avoid GCC complains
      size_t i;
      for (i = 0 ; i < poly.num ; i++)
      {
	uu[i] = pqinfo.tw * poly.texels [i].x;
	vv[i] = pqinfo.th * poly.texels [i].y;
	iz[i] = poly.z [i];
	if (poly.colors [i].red > 2.0) poly.colors [i].red = 2.0;
	if (poly.colors [i].red < 0.0) poly.colors [i].red = 0.0;
	rr[i] = poly.colors [i].red * pqinfo.redFact   * Scan.FlatRGB.red;
	if (poly.colors [i].green > 2.0) poly.colors [i].green = 2.0;
	if (poly.colors [i].green < 0.0) poly.colors [i].green = 0.0;
	gg[i] = poly.colors [i].green * pqinfo.greenFact * Scan.FlatRGB.green;
	if (poly.colors [i].blue > 2.0) poly.colors [i].blue = 2.0;
	if (poly.colors [i].blue < 0.0) poly.colors [i].blue = 0.0;
	bb[i] = poly.colors [i].blue * pqinfo.blueFact  * Scan.FlatRGB.blue;
	if (poly.vertices [i].y > top_y)
	  top_y = poly.vertices [top = i].y;
	if (poly.vertices [i].y < bot_y)
	  bot_y = poly.vertices [bot = i].y;
      }
    
      // If the polygon exceeds the screen, it is an engine failure
    
      // Jorrit: Removed the test below because it causes polygons
      // to disappear.
      //if (((bot_y + EPSILON) < 0) ||
	  //((top_y - EPSILON) > height))
	//return;
    
      //-----
      // Scan from top to bottom.
      // The following structure contains all the data for one side
      // of the scanline conversion. 'L' is responsible for the left
      // side, 'R' for the right side respectively.
      //-----
      struct
      {
	// Start and final vertex number
	signed char sv, fv;
	// The X coordinates and its per-scanline delta; also the final Y coordinate
	int x, dxdy, fy;
	// The `U/V/Z' texture coordinates and their per-scanline delta
	int u, dudy, v, dvdy, z, dzdy;
	// The `R/G/B' colors and their per-scanline delta
	int r, drdy, g, dgdy, b, dbdy;
      } L,R;
    
    // Start of code to stop MSVC bitching about uninitialized variables
      L.sv = R.sv = (signed char)top;
      L.fv = R.fv = (signed char)top;
      int sy = L.fy = R.fy = csQround (poly.vertices [top].y);
    
      L.x = R.x = 0;
      L.dxdy = R.dxdy = 0;
    
      L.u = R.u = 0;
      L.dudy = R.dudy = 0;
      L.v = R.v = 0;
      L.dvdy = R.dvdy = 0;
      L.z = R.z = 0;
      L.dzdy = R.dzdy = 0;
    
      L.r = R.r = 0;
      L.drdy = R.drdy = 0;
      L.g = R.g = 0;
      L.dgdy = R.dgdy = 0;
      L.b = R.b = 0;
      L.dbdy = R.dbdy = 0;
    // End of MSVC specific code
    
      // Decide whenever we should use Gouraud or flat (faster) routines
      bool do_gouraud = (pqinfo.drawline_gouraud != 0)
	&& (!(pqinfo.mixmode & CS_FX_FLAT));
    
      //-----
      // Main scanline loop.
      //-----
      for ( ; ; )
      {
	//-----
	// We have reached the next segment. Recalculate the slopes.
	//-----
	bool leave;
	do
	{
	  leave = true;
	  if (sy <= R.fy)
	  {
	    // Check first if polygon has been finished
	    if (R.fv == (int)bot)
	      return;
	    R.sv = R.fv;
	    if (++R.fv >= (int)poly.num)
	      R.fv = 0;
    
	    leave = false;
	    R.fy = csQround (poly.vertices [(int)R.fv].y);
	    if (sy <= R.fy)
	      continue;
    
	    float dyR = poly.vertices [(int)R.sv].y - poly.vertices [(int)R.fv].y;
	    if (dyR)
	    {
	      float inv_dyR = 1 / dyR;
	      R.x = csQfixed16 (poly.vertices [(int)R.sv].x);
	      R.dxdy = csQfixed16 (
		(poly.vertices [(int)R.fv].x - poly.vertices [(int)R.sv].x) * inv_dyR);
	      R.dzdy = csQfixed24 ((iz [(int)R.fv] - iz [(int)R.sv]) * inv_dyR);
	      if (pqinfo.textured)
	      {
		R.dudy = csQfixed16 ((uu [(int)R.fv] - uu [(int)R.sv]) * inv_dyR);
		R.dvdy = csQfixed16 ((vv [(int)R.fv] - vv [(int)R.sv]) * inv_dyR);
	      }
	      if (!(pqinfo.mixmode & CS_FX_FLAT))
	      {
		R.drdy = csQround ((rr [(int)R.fv] - rr [(int)R.sv]) * inv_dyR);
		R.dgdy = csQround ((gg [(int)R.fv] - gg [(int)R.sv]) * inv_dyR);
		R.dbdy = csQround ((bb [(int)R.fv] - bb [(int)R.sv]) * inv_dyR);
	      }
    
	      // horizontal pixel correction
	      float deltaX = (R.dxdy * 1/65536.) *
		(poly.vertices [(int)R.sv].y - (float (sy) - 0.5));
	      R.x += csQfixed16 (deltaX);
    
	      // apply sub-pixel accuracy factor
	      float Factor;
	      if (poly.vertices [(int)R.fv].x != poly.vertices [(int)R.sv].x)
		Factor = deltaX / (poly.vertices [(int)R.fv].x - poly.vertices [(int)R.sv].x);
	      else
		Factor = 0;
	      if (pqinfo.textured)
	      {
		R.u = csQfixed16 (uu [(int)R.sv] + (uu [(int)R.fv] - uu [(int)R.sv]) * Factor);
		R.v = csQfixed16 (vv [(int)R.sv] + (vv [(int)R.fv] - vv [(int)R.sv]) * Factor);
	      }
	      R.z = csQfixed24 (iz [(int)R.sv] + (iz [(int)R.fv] - iz [(int)R.sv]) * Factor);
	      if (!(pqinfo.mixmode & CS_FX_FLAT))
	      {
		R.r = csQround (rr [(int)R.sv] + (rr [(int)R.fv] - rr [(int)R.sv]) * Factor);
		R.g = csQround (gg [(int)R.sv] + (gg [(int)R.fv] - gg [(int)R.sv]) * Factor);
		R.b = csQround (bb [(int)R.sv] + (bb [(int)R.fv] - bb [(int)R.sv]) * Factor);
	      }
	    } /* endif */
	  } /* endif */
	  if (sy <= L.fy)
	  {
	    if (L.fv == (int)bot)
	      return;
	    L.sv = L.fv;
	    if (--L.fv < 0)
	      L.fv = (int)poly.num - 1;
    
	    leave = false;
	    L.fy = csQround (poly.vertices [(int)L.fv].y);
	    if (sy <= L.fy)
	      continue;
    
	    float dyL = poly.vertices [(int)L.sv].y - poly.vertices [(int)L.fv].y;
	    if (dyL)
	    {
	      float inv_dyL = 1 / dyL;
	      L.x = csQfixed16 (poly.vertices [(int)L.sv].x);
	      L.dxdy = csQfixed16 (
		(poly.vertices [(int)L.fv].x - poly.vertices [(int)L.sv].x) * inv_dyL);
	      L.dzdy = csQfixed24 ((iz [(int)L.fv] - iz [(int)L.sv]) * inv_dyL);
	      if (pqinfo.textured)
	      {
		L.dudy = csQfixed16 ((uu [(int)L.fv] - uu [(int)L.sv]) * inv_dyL);
		L.dvdy = csQfixed16 ((vv [(int)L.fv] - vv [(int)L.sv]) * inv_dyL);
	      }
	      if (!(pqinfo.mixmode & CS_FX_FLAT))
	      {
		L.drdy = csQround ((rr [(int)L.fv] - rr [(int)L.sv]) * inv_dyL);
		L.dgdy = csQround ((gg [(int)L.fv] - gg [(int)L.sv]) * inv_dyL);
		L.dbdy = csQround ((bb [(int)L.fv] - bb [(int)L.sv]) * inv_dyL);
	      }
    
	      // horizontal pixel correction
	      float deltaX = (L.dxdy * 1/65536.) *
		(poly.vertices [(int)L.sv].y - (float (sy) - 0.5));
	      L.x += csQfixed16 (deltaX);
    
	      // apply sub-pixel accuracy factor
	      float Factor;
	      if (poly.vertices [(int)L.fv].x != poly.vertices [(int)L.sv].x)
		Factor = deltaX / (poly.vertices [(int)L.fv].x - poly.vertices [(int)L.sv].x);
	      else
		Factor = 0;
	      if (pqinfo.textured)
	      {
		L.u = csQfixed16 (uu [(int)L.sv] + (uu [(int)L.fv] - uu [(int)L.sv]) * Factor);
		L.v = csQfixed16 (vv [(int)L.sv] + (vv [(int)L.fv] - vv [(int)L.sv]) * Factor);
	      }
	      L.z = csQfixed24 (iz [(int)L.sv] + (iz [(int)L.fv] - iz [(int)L.sv]) * Factor);
	      if (!(pqinfo.mixmode & CS_FX_FLAT))
	      {
		L.r = csQround (rr [(int)L.sv] + (rr [(int)L.fv] - rr [(int)L.sv]) * Factor);
		L.g = csQround (gg [(int)L.sv] + (gg [(int)L.fv] - gg [(int)L.sv]) * Factor);
		L.b = csQround (bb [(int)L.sv] + (bb [(int)L.fv] - bb [(int)L.sv]) * Factor);
	      }
	    } /* endif */
	  } /* endif */
	} while (!leave); /* enddo */
    
	//-----
	// Now draw a trapezoid.
	//-----
	int fin_y;
	if (L.fy > R.fy)
	  fin_y = L.fy;
	else
	  fin_y = R.fy;
    
	int screenY = height - sy;
	while (sy > fin_y)
	{
	  if ((sy & 1) != do_interlaced)
	  {
	    //-----
	    // Draw one scanline.
	    //-----
	    int xl = round16 (L.x);
	    int xr = round16 (R.x);
    
	    if (xr > xl)
	    {
	      int l = xr - xl;
	      float inv_l = 1. / l;
    
	      int dzz = csQround ((R.z - L.z) * inv_l);
	      int uu = 0, duu = 0, vv = 0, dvv = 0;
	      if (pqinfo.textured)
	      {
		int span_u = R.u - L.u;
		int span_v = R.v - L.v;
		uu = L.u; duu = csQint (span_u * inv_l);
		vv = L.v; dvv = csQint (span_v * inv_l);
    
		if (!pqinfo.tiling)
		{
		  // Check for texture overflows
		  if (uu < 0) uu = 0; if (uu > pqinfo.twfp) uu = pqinfo.twfp;
		  if (vv < 0) vv = 0; if (vv > pqinfo.thfp) vv = pqinfo.thfp;
    
		  int tmpu = uu + span_u;
		  if (tmpu < 0 || tmpu > pqinfo.twfp)
		  {
		    if (tmpu < 0) tmpu = 0; if (tmpu > pqinfo.twfp)
		      tmpu = pqinfo.twfp;
		    duu = csQint ((tmpu - uu) * inv_l);
		  }
		  int tmpv = vv + span_v;
		  if (tmpv < 0 || tmpv > pqinfo.thfp)
		  {
		    if (tmpv < 0) tmpv = 0; if (tmpv > pqinfo.thfp)
		      tmpv = pqinfo.thfp;
		    dvv = csQint ((tmpv - vv) * inv_l);
		  }
		}
	      }
    
	      // R,G,B brightness can underflow due to subpixel correction
	      // Underflow will cause visual artifacts while small overflows
	      // will be neutralized by our "clamp to 1.0" circuit.
	      int rr = 0, drr = 0, gg = 0, dgg = 0, bb = 0, dbb = 0;
	      bool clamp = false;
	      if (!(pqinfo.mixmode & CS_FX_FLAT))
	      {
		int span_r = R.r - L.r;
		int span_g = R.g - L.g;
		int span_b = R.b - L.b;
		rr = L.r, drr = csQint (span_r * inv_l);
		gg = L.g, dgg = csQint (span_g * inv_l);
		bb = L.b, dbb = csQint (span_b * inv_l);
    
		if (rr < 0) rr = 0;
		if (gg < 0) gg = 0;
		if (bb < 0) bb = 0;
    
		int tmp = rr + span_r;
		if (tmp < 0) drr = - csQint (rr * inv_l);
		clamp |= (rr > pqinfo.max_r) || (tmp > pqinfo.max_r);
		tmp = gg + span_g;
		if (tmp < 0) dgg = - csQint (gg * inv_l);
		clamp |= (gg > pqinfo.max_g) || (tmp > pqinfo.max_g);
		tmp = bb + span_b;
		if (tmp < 0) dbb = - csQint (bb * inv_l);
		clamp |= (bb > pqinfo.max_b) || (tmp > pqinfo.max_b);
	      }
    
	      uint32 *zbuff = z_buffer + width * screenY + xl;
	      unsigned char *dest = line_table [screenY] + (xl << pixel_shift);
    
	      if (do_gouraud)
		pqinfo.drawline_gouraud (dest, l, zbuff, uu, duu, vv, dvv,
		  L.z, dzz, pqinfo.bm, pqinfo.shf_w, rr, gg, bb, drr, dgg,
		  dbb, clamp);
	      else
		pqinfo.drawline (dest, l, zbuff, uu, duu, vv, dvv,
		  L.z, dzz, pqinfo.bm, pqinfo.shf_w);
	    }
	  }
    
	  L.x += L.dxdy; R.x += R.dxdy;
	  L.z += L.dzdy; R.z += R.dzdy;
	  if (pqinfo.textured)
	    L.u += L.dudy, L.v += L.dvdy,
	    R.u += R.dudy, R.v += R.dvdy;
	  if (!(pqinfo.mixmode & CS_FX_FLAT))
	    L.r += L.drdy, L.g += L.dgdy, L.b += L.dbdy,
	    R.r += R.drdy, R.g += R.dgdy, R.b += R.dbdy;
    
	  sy--;
	  screenY++;
	}
      }
    }

    void SetZBufMode (csZBufMode mode)
    { z_buf_mode = mode; }

    void Init (const csPixelFormat& pfmt,
      csDrawPIScanline** ScanProcPI,
      csDrawPIScanlineGouraud** ScanProcPIG,
      csSoftwareTextureManager* texman, int w, int h,
      uint32* z_buffer, uint8** line_table)
    {
      this->pfmt = pfmt;

      alpha_mask = 0;
      alpha_mask |= 1 << (pfmt.RedShift);
      alpha_mask |= 1 << (pfmt.GreenShift);
      alpha_mask |= 1 << (pfmt.BlueShift);
      alpha_mask = ~alpha_mask;

      this->ScanProcPI = ScanProcPI;
      this->ScanProcPIG = ScanProcPIG;
      this->texman = texman;

      width = w;
      height = h;

      this->z_buffer = z_buffer;
      this->line_table = line_table;

      if (pfmt.PixelBytes == 4)
	pixel_shift = 2;
      else if (pfmt.PixelBytes == 2)
	pixel_shift = 1;
    }
  };
  
} // namespace cspluginSoft3d

#endif // __CS_SOFT3D_POLYRAST_H__
