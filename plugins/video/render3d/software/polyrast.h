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

#include "types.h"
#include "scanindex.h"
#include "sft3dcom.h"

#include "scanline.h"

namespace cspluginSoft3d
{
  class PolygonRasterizer
  {
    /*bool dpfx_valid;
    iTextureHandle* dpfx_tex_handle;
    uint dpfx_mixmode;
    bool do_gouraud;
    bool do_lighting;
    bool do_textured;
    /// Alpha mask used for 16-bit mode.
    uint16 alpha_mask;
    bool is_for_procedural_textures;

    csDrawPIScanline** ScanProcPI;
    csDrawPIScanlineGouraud** ScanProcPIG;

    csPixelFormat pfmt;
    csSoftwareTextureManager* texman;*/

    int width, height;
    int do_interlaced;

    uint32* z_buffer;
    uint8** line_table;
    int pixel_shift;
  public:
    PolygonRasterizer() : do_interlaced(-1)
    {
    }

    void DrawPolygonFX (size_t vertNum, const csVector3* vertices,
      const VertexBuffer* inBuffers, BuffersMask buffersMask,
      ScanlineRenderInfo& sri)
    {
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
	InterpolateEdgePersp edge;
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
    
	    R.edge.Setup (vertices, inBuffers, buffersMask, R.sv, R.fv, sy);
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
    
	    L.edge.Setup (vertices, inBuffers, buffersMask, L.sv, L.fv, sy);
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

	      InterpolateScanlinePersp scanline;
	      scanline.Setup (L.edge, R.edge, buffersMask, inv_l);
    
	      float dzz = (R.edge.Iz - L.edge.Iz) * inv_l;
	      //int uu = 0, duu = 0, vv = 0, dvv = 0;
	      //if (pqinfo.textured)
	      {
		/*const csVector4 tcL (L.edge.c[VATTR_BUFINDEX(TEXCOORD)]);
		const csVector4 tcR (R.edge.c[VATTR_BUFINDEX(TEXCOORD)]);
		int span_u = csQfixed16 (tcR.x - tcL.x);
		int span_v = csQfixed16 (tcR.y - tcL.y);
		uu = csQfixed16 (tcL.x); duu = csQint (span_u * inv_l);
		vv = csQfixed16 (tcL.y); dvv = csQint (span_v * inv_l);*/
    
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
		//pqinfo.drawline (dest, l, zbuff, uu, duu, vv, dvv,
		  //L.edge.z, dzz, pqinfo.bm, pqinfo.shf_w);

	      sri.proc (sri.renderer, scanline, dest, l, zbuff, 
		L.edge.Iz, dzz);
	    }
	  }
    
	  L.edge.Advance (buffersMask);
	  R.edge.Advance (buffersMask);
    
	  sy--;
	  screenY++;
	}
      }
    }

    void Init (const csPixelFormat& pfmt, int w, int h,
      uint32* z_buffer, uint8** line_table)
    {
      width = w;
      height = h;

      this->z_buffer = z_buffer;
      this->line_table = line_table;

      pixel_shift = csLog2 (pfmt.PixelBytes);
    }
  };
  
} // namespace cspluginSoft3d

#endif // __CS_SOFT3D_POLYRAST_H__
