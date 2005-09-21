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

#include "clipper.h" //@@@ for VertexBuffer
#include "scanindex.h"
#include "sft3dcom.h"

namespace cspluginSoft3d
{
  struct InterpolateLine
  {
    float dcdx[4];
  };

  class PolygonRasterizer
  {
    bool dpfx_valid;
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

    template <typename T>
    static inline T Lerp (const T& a, const T& b, float f)
    { return a + (b-a) * f; }
    struct InterpolateEdge
    {
      // The X coordinates and its per-scanline delta
      float x, dxdy;
      // The Z coordinates and its per-scanline delta
      float z, dzdy;
      // Buffer values and per-scanline delta
      csVector4 c[clipMaxBuffers];
      csVector4 dcdy[clipMaxBuffers];

      void Setup (const csVector3* vertices, const VertexBuffer* buffers, 
	ClipBuffersMask buffersMask, size_t sv, size_t fv, int sy)
      {
	const csVector3 vsv (vertices[sv]);
	const csVector3 vfv (vertices[fv]);

	float dy = vsv.y - vfv.y;
	if (dy)
	{
	  float inv_dy = 1 / dy;
	  x = vsv.x;
	  dxdy = (vfv.x - x) * inv_dy;
	  dzdy = (vfv.z - vsv.z) * inv_dy;

	  // horizontal pixel correction
	  float deltaX = dxdy *
	    (vsv.y - (float (sy) - 0.5));
	  x += deltaX;

	  // apply sub-pixel accuracy factor
	  float Factor;
	  if (vfv.x != vsv.x)
	    Factor = deltaX / (vfv.x - vsv.x);
	  else
	    Factor = 0;

	  z = Lerp (vsv.z, vfv.z, Factor);

	  for (size_t b = 0; b < clipMaxBuffers; b++)
	  {
	    if (!(buffersMask & (1 << b))) continue;

	    const csVector4 csv (((csVector4*)buffers[b].data)[sv]);
	    const csVector4 cfv (((csVector4*)buffers[b].data)[fv]);

	    c[b] = Lerp (csv, cfv, Factor);
	    dcdy[b] = (cfv - csv) * inv_dy;
	  }
	} /* endif */
      }
      void Advance (ClipBuffersMask buffersMask)
      {
	x += dxdy;
	z += dzdy;

	for (size_t b = 0; b < clipMaxBuffers; b++)
	{
	  if (!(buffersMask & (1 << b))) continue;
	  c[b] += dcdy[b];
	}
      }
    };
  public:
    PolygonRasterizer() : dpfx_valid(false), do_gouraud(false),
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
      uint mode)
    {
      if (!dpfx_valid ||
	    handle != dpfx_tex_handle ||
	    z_buf_mode != dpfx_z_buf_mode ||
	    mode != dpfx_mixmode)
      {
	dpfx_valid = true;
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

    void DrawPolygonFX (size_t vertNum, const csVector3* vertices,
      const VertexBuffer* inBuffers, ClipBuffersMask buffersMask, 
      iTextureHandle* tex_handle, uint mixmode)
    {
      RealStartPolygonFX (tex_handle, mixmode);
    
      if (!pqinfo.drawline && !pqinfo.drawline_gouraud)
	return;
    
      const csVector4* texcoords = 
	(csVector4*)inBuffers[VATTR_BUFINDEX(TEXCOORD)].data;
      const csVector4* colors = (buffersMask & (1 << VATTR_BUFINDEX(COLOR))) ?
	(csVector4*)inBuffers[VATTR_BUFINDEX(COLOR)].data : 0;

      Scan.FlatRGB.Set (255, 255, 255);
    
      // Get the same value as a pixel-format-encoded value
      Scan.FlatColor = texman->encode_rgb (Scan.FlatRGB.red, Scan.FlatRGB.green,
	Scan.FlatRGB.blue);
    
      //-----
      // Get the values from the polygon for more convenient local access.
      // Also look for the top-most and bottom-most vertices.
      //-----
      size_t top, bot;
      float top_y = -99999;
      float bot_y = 99999;
      top = bot = 0;                        // avoid GCC complains
      size_t i;
      for (i = 0 ; i < vertNum ; i++)
      {
	if (vertices[i].y > top_y)
	  top_y = vertices[top = i].y;
	if (vertices[i].y < bot_y)
	  bot_y = vertices[bot = i].y;
      }

      /* "Denormalize" some known buffers (currently colors and TCs) since
       * doing it anywhere further down is rather wasterful. */
      VertexBuffer actualBuffers[clipMaxBuffers];
      CS_ALLOC_STACK_ARRAY(csVector4, denormTC, vertNum);
      CS_ALLOC_STACK_ARRAY(csVector4, denormColor, vertNum);
      for (size_t b = 0; b < clipMaxBuffers; b++)
      {
	if (!(buffersMask & (1 << b))) continue;

	VertexBuffer& actualBuffer = actualBuffers[b];
	switch (b)
	{
	  case VATTR_BUFINDEX(TEXCOORD):
	    actualBuffer.data = (uint8*)denormTC;
	    actualBuffer.comp = 4;
	    for (i = 0 ; i < vertNum ; i++)
	    {
	      denormTC[i].Set (pqinfo.tw * texcoords[i].x,
		pqinfo.th * texcoords[i].y, 0.0f, 1.0f);
	    }
	    break;
	  case VATTR_BUFINDEX(COLOR):
	    actualBuffer.data = (uint8*)denormColor;
	    actualBuffer.comp = 4;
	    for (i = 0 ; i < vertNum ; i++)
	    {
	      denormColor[i].Set (
		colors[i].x * pqinfo.redFact,
		colors[i].y * pqinfo.greenFact,
		colors[i].z * pqinfo.blueFact,
		colors[i].w * 0xff0000);
	    }
	    break;
	  default:
	    actualBuffer = inBuffers[b];
	}
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
	int sv, fv;
	// The final Y coordinate
	int fy;
	
	// Edge interpolater; contains values as well as deltas
	InterpolateEdge edge;
      } L,R;
    
    // Start of code to stop MSVC bitching about uninitialized variables
      L.sv = R.sv = (int)top;
      L.fv = R.fv = (int)top;
      int sy = L.fy = R.fy = csQround (vertices[top].y);
    // End of MSVC specific code
    
      // Decide whenever we should use Gouraud or flat (faster) routines
      /*bool do_gouraud = (pqinfo.drawline_gouraud != 0)
	&& (!(pqinfo.mixmode & CS_FX_FLAT));*/
    
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
	    if (++R.fv >= (int)vertNum)
	      R.fv = 0;
    
	    leave = false;
	    R.fy = csQround (vertices[R.fv].y);
	    if (sy <= R.fy)
	      continue;
    
	    R.edge.Setup (vertices, actualBuffers, buffersMask, R.sv, R.fv, sy);
	  } /* endif */
	  if (sy <= L.fy)
	  {
	    if (L.fv == (int)bot)
	      return;
	    L.sv = L.fv;
	    if (--L.fv < 0)
	      L.fv = (int)vertNum - 1;
    
	    leave = false;
	    L.fy = csQround (vertices[L.fv].y);
	    if (sy <= L.fy)
	      continue;
    
	    L.edge.Setup (vertices, actualBuffers, buffersMask, L.sv, L.fv, sy);
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
	    int xl = csQround (L.edge.x);
	    int xr = csQround (R.edge.x);
    
	    if (xr > xl)
	    {
	      int l = xr - xl;
	      float inv_l = 1. / l;
    
	      float dzz = (R.edge.z - L.edge.z) * inv_l;
	      int uu = 0, duu = 0, vv = 0, dvv = 0;
	      if (pqinfo.textured)
	      {
		const csVector4 tcL (L.edge.c[VATTR_BUFINDEX(TEXCOORD)]);
		const csVector4 tcR (R.edge.c[VATTR_BUFINDEX(TEXCOORD)]);
		int span_u = csQfixed16 (tcR.x - tcL.x);
		int span_v = csQfixed16 (tcR.y - tcL.y);
		uu = csQfixed16 (tcL.x); duu = csQint (span_u * inv_l);
		vv = csQfixed16 (tcL.y); dvv = csQint (span_v * inv_l);
    
		/*
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
		*/
	      }
    
	      // R,G,B brightness can underflow due to subpixel correction
	      // Underflow will cause visual artifacts while small overflows
	      // will be neutralized by our "clamp to 1.0" circuit.
	      /*
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
	      */
    
	      uint32 *zbuff = z_buffer + width * screenY + xl;
	      unsigned char *dest = line_table [screenY] + (xl << pixel_shift);
    
	      /*
	      if (do_gouraud)
		pqinfo.drawline_gouraud (dest, l, zbuff, uu, duu, vv, dvv,
		  L.z, dzz, pqinfo.bm, pqinfo.shf_w, rr, gg, bb, drr, dgg,
		  dbb, clamp);
	      else
	      */
		pqinfo.drawline (dest, l, zbuff, uu, duu, vv, dvv,
		  L.edge.z, dzz, pqinfo.bm, pqinfo.shf_w);
	    }
	  }
    
	  L.edge.Advance (buffersMask);
	  R.edge.Advance (buffersMask);
    
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
