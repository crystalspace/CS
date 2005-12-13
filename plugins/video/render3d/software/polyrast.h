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
#include "csplugincommon/softshader/scanline.h"
#include "csplugincommon/softshader/types.h"

#include "sft3dcom.h"
#include "types.h"
#include "scan_blend.h"

namespace cspluginSoft3d
{
  using namespace CrystalSpace::SoftShader;

  template<typename Pix, typename SrcBlend, typename DstBlend>
  struct SLLogic_ScanlineRenderer
  {
    const iScanlineRenderer::RenderInfoMesh& srim;
    const iScanlineRenderer::RenderInfoTriangle& srit;
    const VertexBuffer* inBuffers;
    BuffersMask buffersMask;
    size_t floatsPerVert;
    const Pix& pix;

    SLLogic_ScanlineRenderer (const Pix& pix,
    const iScanlineRenderer::RenderInfoMesh& srim,
    const iScanlineRenderer::RenderInfoTriangle& srit,
      const VertexBuffer* inBuffers, BuffersMask buffersMask,
      size_t floatsPerVert) : 
      srim (srim), srit (srit), inBuffers (inBuffers), 
      buffersMask (buffersMask), floatsPerVert (floatsPerVert), pix (pix) {}

    CS_FORCEINLINE
    void RenderScanline (InterpolateEdgePersp& L, InterpolateEdgePersp& R, 
      int ipolStep, int ipolShift, uint32* temp, void* dest, uint len, 
      uint32 *zbuff)
    {
      srit.proc (srim.renderer, L, R, ipolStep, ipolShift, 
	temp, len, zbuff);

      // Blend
      typename_qualifier Pix::PixType* _dest = 
	(typename_qualifier Pix::PixType*)dest;
      typename_qualifier Pix::PixType* _destend = _dest + len;

      uint32* src = temp;
      while (_dest < _destend)
      {
	Pixel px (*src++);

	if (px.c.a & 0x80)
	{
	  px.c.a <<= 1;

	  SrcBlend srcFactor;
	  DstBlend dstFactor;
	  const Pixel dp = pix.GetPix (_dest);
	  // Some special cases...
	  if (dstFactor.GetBlendFact() == CS_MIXMODE_FACT_ZERO)
	  {
	    pix.WritePix (_dest, srcFactor.Apply (px, dp));
	  }
	  else if (srcFactor.GetBlendFact() == CS_MIXMODE_FACT_ZERO)
	  {
	    if (dstFactor.GetBlendFact() != CS_MIXMODE_FACT_ONE)
	      pix.WritePix (_dest, dstFactor.Apply (px, dp));
	  }
	  else
	    // General case
	    pix.WritePix (_dest, 
	      srcFactor.Apply (px, dp) + dstFactor.Apply (px, dp));
	}
 	_dest++;
      } /* endwhile */
    }
    void LinearizeBuffers (float* linearBuffers, size_t vertNum)
    {
      const size_t* compNum = srim.bufferComps;
      size_t bufOfs = 0;
      for (size_t i = 0; i < maxBuffers; i++)
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
    size_t GetFloatsPerVert() const { return floatsPerVert; }
  };

  struct SLLogic_ZFill
  {
    CS_FORCEINLINE
    void RenderScanline (InterpolateEdgePersp& L, InterpolateEdgePersp& R, 
      int ipolStep, int ipolShift, uint32* /*temp*/, void* /*dest*/, uint len, 
      uint32 *zbuff)
    {
      InterpolateScanlinePersp<0> ipol;
      ipol.Setup (L, R, 1.0f / len, ipolStep, ipolShift);

      while (len-- > 0)
      {
	*zbuff = ipol.Iz.GetFixed();
	ipol.Advance();
	zbuff++;
      } /* endwhile */
    }
    void LinearizeBuffers (float* /*linearBuffers*/, size_t /*vertNum*/)
    { }
    size_t GetFloatsPerVert() const { return 0; }
  };

  struct ScanlineIter
  {
    int vertNum;
    const csVector3* vertices;
    const size_t floatsPerVert;
    int height;
    size_t top, bot;
    int ipolStep;
    int ipolShift;
    float* linearBuffers;
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
    int fin_y;
    int sy;
    bool haveTrapezoid;
    int screenY;

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

    ScanlineIter (size_t vertNum, const csVector3* vertices,
      const size_t floatsPerVert, float* linearBuffers, int height) : 
      vertNum ((int)vertNum), vertices (vertices), 
      floatsPerVert (floatsPerVert), height (height),
      linearBuffers (linearBuffers), haveTrapezoid (false)
    {
      //-----
      // Get the values from the polygon for more convenient local access.
      // Also look for the top-most and bottom-most vertices.
      //-----
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
	top = bot = 0; // shaddap gcc 
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

	const float z1 = v1.z;
	const float z2 = v2.z;
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
      ipolStep = 16;
      ipolShift = 4;

      // Pick an interpolation step...
      /* @@@ FIXME: does that computation make sense?
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
    
    // Start of code to stop MSVC bitching about uninitialized variables
      L.sv = R.sv = (int)top;
      L.fv = R.fv = (int)top;
      sy = L.fy = R.fy = csQround (vertices[top].y);
    // End of MSVC specific code
    }
    bool NextScanline ()
    {
      if (!haveTrapezoid)
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
	      return false;
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
	      return false;
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
	if (L.fy > R.fy)
	  fin_y = L.fy;
	else
	  fin_y = R.fy;

	screenY = height - sy;
	haveTrapezoid = true;
      }
      else
      {
	if (sy > fin_y)
	  return true;
	else
	{
	  haveTrapezoid = false;
	  return NextScanline();
	}
      }

      return true;
    }
    void PostScanline()
    {
      L.edge.Advance (floatsPerVert);
      R.edge.Advance (floatsPerVert);

      sy--;
      screenY++;
    }
  };
  template <typename ScanlineLogic>
  class PolygonRasterizer
  {
    int width, height;
    int do_interlaced;

    uint32* z_buffer;
    uint8** line_table;
    int pixel_shift;
    uint32* line_buffer;
    int line_buffer_width;

  public:
    PolygonRasterizer() : do_interlaced(-1), line_buffer(0), 
      line_buffer_width (-1)
    {
    }
    ~PolygonRasterizer()
    {
      delete[] line_buffer;
    }

    void DrawPolygon (size_t vertNum, const csVector3* vertices,
      const ScanlineLogic& logic)
    {
      CS_ASSERT_MSG ("Degenerate polygon", vertNum >= 3);
      ScanlineLogic sll (logic);

      const size_t floatsPerVert = sll.GetFloatsPerVert();
      CS_ALLOC_STACK_ARRAY(float, linearBuffers, floatsPerVert * vertNum);
      sll.LinearizeBuffers (linearBuffers, vertNum);
      
      ScanlineIter si (vertNum, vertices, floatsPerVert, linearBuffers, 
	height);

      //-----
      // Main scanline loop.
      //-----
      while (si.NextScanline ())
      {
	if ((si.sy & 1) != do_interlaced)
	{
	  //-----
	  // Draw one scanline.
	  //-----
	  int xl = csQround (si.L.edge.x);
	  int xr = csQround (si.R.edge.x);
  
	  if (xr > xl)
	  {
	    int l = xr - xl;
  
	    uint32 *zbuff = z_buffer + width * si.screenY + xl;
	    unsigned char *dest = line_table [si.screenY] + (xl << pixel_shift);

	    sll.RenderScanline (si.L.edge, si.R.edge, si.ipolStep, si.ipolShift, 
	      line_buffer, dest, l, zbuff);
	  }
	}
	si.PostScanline();
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
      if (line_buffer_width < w)
      {
	delete line_buffer;
	line_buffer = new uint32[width];
      }
    }
  };
  
} // namespace cspluginSoft3d

#endif // __CS_SOFT3D_POLYRAST_H__
