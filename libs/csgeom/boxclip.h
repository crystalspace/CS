/*
    Crystal Space 3D engine
    Copyright (C) 2000-2005 by Jorrit Tyberghein

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
   This is the same algorithm contained in polyclip.h but it is optimized
   for clipping against a square rectangle (box).
*/

#ifndef __CS_CSGEOM_BOXCLIP_H__
#define __CS_CSGEOM_BOXCLIP_H__

#include "csgeom/math.h"

#include "edgeclip.h"

namespace CrystalSpace
{
  
  template<typename BoxTest, typename StatusOutput>
  class BoxClipper
  {
    BoxTest boxTest;
    StatusOutput statOut;

    const csBox2& region;
    
    // The currently clipped polygon
    csVector2 *InP;
    // Number of input vertices
    size_t InV;
    // The current output polygon
    csVector2 *OutP;
    // Number of output vertices
    size_t OutV;
    
    struct ClipMinX
    {
      const csBox2& region;
      ClipMinX (const csBox2& region) : region(region) {}

      bool Inside (float px, float py)
      { return (px >= region.MinX ()); }
      void Intersect (float px, float py, float cx, float cy,
	double& t, float& tx, float& ty)
      {
	/* t = (region.X - px) / (cx - px) */
	t = (region.MinX () - px) / (cx - px);
	tx = region.MinX ();
	if (t <= 0)
	  ty = py;
	else if (t >= 1)
	  ty = cy;
	else
	  ty = py + t * (cy - py);
      }
    };
    struct ClipMaxX
    {
      const csBox2& region;
      ClipMaxX (const csBox2& region) : region(region) {}

      bool Inside (float px, float py)
      { return (px <= region.MaxX ()); }
      void Intersect (float px, float py, float cx, float cy,
	double& t, float& tx, float& ty)
      {
	/* t = (region.X - px) / (cx - px) */
	t = (region.MaxX () - px) / (cx - px);
	tx = region.MaxX ();
	if (t <= 0)
	  ty = py;
	else if (t >= 1)
	  ty = cy;
	else
	  ty = py + t * (cy - py);
      }
    };
    struct ClipMinY
    {
      const csBox2& region;
      ClipMinY (const csBox2& region) : region(region) {}

      bool Inside (float px, float py)
      { return (py >= region.MinY ()); }
      void Intersect (float px, float py, float cx, float cy,
	double& t, float& tx, float& ty)
      {
	/* t = (region.Y - py) / (cy - py) */
	t = (region.MinY () - py) / (cy - py);
	ty = region.MinY ();
	if (t <= 0)
	  tx = px;
	else if (t >= 1)
	  tx = cx;
	else
	  tx = px + t * (cx - px);
      }
    };
    struct ClipMaxY
    {
      const csBox2& region;
      ClipMaxY (const csBox2& region) : region(region) {}

      bool Inside (float px, float py)
      { return (py <= region.MaxY ()); }
      void Intersect (float px, float py, float cx, float cy,
	double& t, float& tx, float& ty)
      {
	/* t = (region.Y - py) / (cy - py) */
	t = (region.MaxY () - py) / (cy - py);
	ty = region.MaxY ();
	if (t <= 0)
	  tx = px;
	else if (t >= 1)
	  tx = cx;
	else
	  tx = px + t * (cx - px);
      }
    };
  public:
    BoxClipper (const BoxTest& boxTest, const StatusOutput& statOut,
      const csBox2& region,
      csVector2 *InP, size_t InV, csVector2 *OutP) : 
      boxTest (boxTest), statOut (statOut), region (region),
      InP (InP), InV (InV), OutP (OutP), OutV (-1)
    {
    }
    uint8 Clip()
    {
      // Temporary storage for intermediate polygons
      csVector2 TempPoly [MAX_OUTPUT_VERTICES];
      // The currently clipped polygon
      csVector2 *InP = this->InP;
      // Number of input vertices
      size_t InV = this->InV;
      // The current output polygon
      csVector2 *OutP = (boxTest.GetClipCount() & 1) ? this->OutP : TempPoly;

      if (boxTest.GetClipCount() & 1)
	statOut.Flip();

      CLIP_PRINTF ("*** %s\n", CS_FUNCTION_NAME);

      uint8 Clipped = CS_CLIP_INSIDE;
      if (boxTest.ClipMinX())
      {
	ClipMinX const r(region);
	EdgeClipper<ClipMinX, StatusOutput> edgeClip (
	  r, statOut, InP, InV, OutP, OutV);
	Clipped = csMin (Clipped, edgeClip.ClipEdge());

	// Switch input/output polys: now we're going to clip
	// the polygon we just created against the next clipper edge
	InV = OutV;
	InP = OutP;
	OutP = (OutP == TempPoly) ? this->OutP : TempPoly;
	statOut.Flip();
      }
      if (boxTest.ClipMaxX())
      {
	ClipMaxX const r(region);
	EdgeClipper<ClipMaxX, StatusOutput> edgeClip (
	  r, statOut, InP, InV, OutP, OutV);
	Clipped = csMin (Clipped, edgeClip.ClipEdge());

	InV = OutV;
	InP = OutP;
	OutP = (OutP == TempPoly) ? this->OutP : TempPoly;
	statOut.Flip();
      }
      if (boxTest.ClipMinY())
      {
	ClipMinY const r(region);
	EdgeClipper<ClipMinY, StatusOutput> edgeClip (
	  r, statOut, InP, InV, OutP, OutV);
	Clipped = csMin (Clipped, edgeClip.ClipEdge());

	InV = OutV;
	InP = OutP;
	OutP = (OutP == TempPoly) ? this->OutP : TempPoly;
	statOut.Flip();
      }
      if (boxTest.ClipMaxY())
      {
	ClipMaxY const r(region);
	EdgeClipper<ClipMaxY, StatusOutput> edgeClip (
	  r, statOut, InP, InV, OutP, OutV);
	Clipped = csMin (Clipped, edgeClip.ClipEdge());

	InV = OutV;
	InP = OutP;
	OutP = (OutP == TempPoly) ? this->OutP : TempPoly;
	statOut.Flip();
      }

      // The polygon is fully inside clipper polygon?
      if (OutV == (size_t)-1)
      {
	OutV = InV;
	if (InP != this->OutP)
	  memcpy (this->OutP, InP, OutV * sizeof (csVector2));
	return CS_CLIP_INSIDE;
      }
      return Clipped;
    }
    size_t GetOutputCount() { return OutV; }
  };
 
} // namespace CrystalSpace

#endif // __CS_CSGEOM_BOXCLIP_H__
