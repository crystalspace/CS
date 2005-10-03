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
#include "csgeom/math.h"

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

    inline static void SelectInterpolationStep (float M, int InterpolMode,
      int& InterpolStep, int& InterpolShift)
    {
      /*
      * For the four interpolation modes.
      */
      struct csSft3DCom
      {
	int step1, shift1;
	int step2, shift2;
	int step3, shift3;
	int step4, shift4;
      };

      static const csSft3DCom inter_modes[4] =
      {
	{ 128, 7, 64, 6, 32, 5, 16, 4 },      // Selective
	{ 32, 5, 32, 5, 32, 5, 32, 5 },       // 32-steps
	{ 16, 4, 16, 4, 16, 4, 16, 4 },       // 16-steps
	{ 8, 3, 8, 3, 8, 3, 8, 3 }            // 8-steps
      };
      // Select the right interpolation factor based on the z-slope of our
      // polygon. This will greatly increase the speed of polygons which are
      // horizontally constant in z.
      if (ABS (M) < .000001)
      {
	InterpolStep = inter_modes[InterpolMode].step1;
	InterpolShift = inter_modes[InterpolMode].shift1;
      }
      else if (ABS (M) < .00005)
      {
	InterpolStep = inter_modes[InterpolMode].step2;
	InterpolShift = inter_modes[InterpolMode].shift2;
      }
      else if (ABS (M) < .001)
      {
	InterpolStep = inter_modes[InterpolMode].step3;
	InterpolShift = inter_modes[InterpolMode].shift3;
      }
      else
      {
	InterpolStep = inter_modes[InterpolMode].step4;
	InterpolShift = inter_modes[InterpolMode].shift4;
      }
    }

  public:
    PolygonRasterizer() : do_interlaced(-1)
    {
    }

    void DrawPolygon (size_t vertNum, const csVector3* vertices,
      const VertexBuffer* inBuffers, BuffersMask buffersMask,
      ScanlineRenderInfo& sri, size_t floatsPerVert)
    {
      CS_ASSERT_MSG ("Degenerate polygon", vertNum >= 3);
      //-----
      // Get the values from the polygon for more convenient local access.
      // Also look for the top-most and bottom-most vertices.
      //-----
      size_t top, bot;
      size_t i;
      float top_y, bot_y, min_x, max_x, min_z, max_z;
      size_t compareNum;
      if (vertNum & 1)
      {
	const size_t end = vertNum-1;
	const csVector3& v = vertices[end];
	top_y = v.y;
	bot_y = v.y;
	min_x = v.x;
	max_x = v.x;
	min_z = v.z;
	max_z = v.z;
	compareNum = top = bot = end;
      }
      else
      {
	top_y = -99999;
	bot_y = 99999;
	min_x = 99999;
	max_x = -99999;
	min_z = 99999;
	max_z = -99999;
	compareNum = vertNum;
      }
      for (i = 0 ; i < compareNum ; i += 2)
      {
	const csVector3& v1 = vertices[i];
	const csVector3& v2 = vertices[i+1];

	if (v1.y > v2.y)
	{
	  if (v1.y > top_y)
	  {
	    top_y = v1.y;
	    top = i;
	  }
	  if (v2.y < bot_y)
	  {
	    bot_y = v2.y;
	    bot = i+1;
	  }
	}
	else
	{
	  if (v2.y > top_y)
	  {
	    top_y = v2.y;
	    top = i+1;
	  }
	  if (v1.y < bot_y)
	  {
	    bot_y = v1.y;
	    bot = i;
	  }
	}

	if (v1.x > v2.x)
	{
	  if (v1.x > max_x) max_x = v1.x;
	  if (v2.x < min_x) min_x = v2.x;
	}
	else
	{
	  if (v2.x > max_x) max_x = v2.x;
	  if (v1.x < min_x) min_x = v1.x;
	}

	const float z1 = 1.0f/v1.z;
	const float z2 = 1.0f/v2.z;
	if (z1 > z2)
	{
	  if (z1 > max_z) max_z = z1;
	  if (z2 < min_z) min_z = z2;
	}
	else
	{
	  if (z2 > max_z) max_z = z2;
	  if (z1 < min_z) min_z = z1;
	}
      }

      CS_ALLOC_STACK_ARRAY(float, linearBuffers, floatsPerVert * vertNum);
      {
	const size_t* compNum = sri.bufferComps;
	size_t bufOfs = 0;
	for (i = 0; i < maxBuffers; i++)
	{
	  if (!(buffersMask & (1 << i))) continue;
	  float* dest = &linearBuffers[bufOfs];
	  uint8* src = inBuffers[i].data;
	  for (size_t v = 0; v < vertNum; v++)
	  {
	    memcpy (dest, src, *compNum * sizeof (float));
	    src += inBuffers[i].comp * sizeof (float);
	    dest += floatsPerVert;
	  }
	  bufOfs += *compNum;
	  compNum++;
	}
      }

      int ipolStep = 16;
      int ipolShift = 4;
      // Pick an interpolation step...
      /* @@@ FIXME: makes that computation sense?
       * SelectInterpolationStep() is taken verbatim from the old polygon
       * code, and I'm not certain about the Z slope values... [-res] */
      float dx = max_x - min_x;
      float zss = 0.0f;
      if (ABS(dx) > EPSILON)
	zss = (max_z - min_z) / dx;
      SelectInterpolationStep (zss, 0,
	ipolStep, ipolShift);
    
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
    
	    R.edge.Setup (vertices, linearBuffers, floatsPerVert, 
	      R.sv, R.fv, sy);
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
    
	    L.edge.Setup (vertices, linearBuffers, floatsPerVert, 
	      L.sv, L.fv, sy);
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
    
	      uint32 *zbuff = z_buffer + width * screenY + xl;
	      unsigned char *dest = line_table [screenY] + (xl << pixel_shift);

	      sri.proc (sri.renderer, L.edge, R.edge, ipolStep, ipolShift, 
		dest, l, zbuff);
	    }
	  }
    
	  L.edge.Advance (floatsPerVert);
	  R.edge.Advance (floatsPerVert);
    
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
