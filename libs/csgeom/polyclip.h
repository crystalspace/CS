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
   This is the well-known Southerland-Hodgemen polygon clipping algorithm
   which works by successively clipping target polygon against every clipper
   polygon edge.

   In the following we call the "clipping" (or "clipper") polygon the polygon
   against which we're clipping, and "clipped" polygon the polygon that we
   are clipping against the clipper.

   The algorithm does not care about the vertex order of both polygons
   (clockwise or anti-clockwise). However, the clipping polygon should be
   convex while the clipped polygon can be arbitrary.
*/

#ifndef __CS_CSGEOM_POLYCLIP_H__
#define __CS_CSGEOM_POLYCLIP_H__

#include "csgeom/math.h"

#include "edgeclip.h"

namespace CrystalSpace
{
  
  template<typename StatusOutput>
  class PolyClipper
  {
    StatusOutput statOut;

    // The currently clipped polygon
    csVector2 *InP;
    // Number of input vertices
    size_t InV;
    // The current output polygon
    csVector2 *OutP;
    // Number of output vertices
    size_t OutV;
    csVector2* ClipPoly;
    csVector2* ClipData;
    size_t ClipPolyVertices;
    
    struct ClipPolyEdge
    {
      csVector2* ClipPoly;
      csVector2* ClipData;
      size_t edge;
      
      ClipPolyEdge (csVector2* ClipPoly, csVector2* ClipData, size_t edge) :
	ClipPoly (ClipPoly), ClipData (ClipData), edge (edge)
      {
      }
      
      bool Inside (float px, float py)
      {  
	return (px - ClipPoly [edge].x) * ClipData [edge].y -
	  (py - ClipPoly [edge].y) * ClipData [edge].x >= 0;
      }
      void Intersect (float px, float py, float cx, float cy,
	double& t, float& tx, float& ty)
      {
	/* Check if and where edges intersects */
	/* t = - (n * (a - c)) / (n * (b - a)) */
	double denom = ((cx - px) * ClipData [edge].y -
			(cy - py) * ClipData [edge].x);
	t = (denom == 0) ? 1.0 :
	    ((py - ClipPoly [edge].y) * ClipData [edge].x
	   - (px - ClipPoly [edge].x) * ClipData [edge].y)
	   / denom;
	
	if (t <= 0)
	{ tx = px; ty = py; }
	else if (t >= 1)
	{ tx = cx; ty = cy; }
	else
	{
	  tx = px + t * (cx - px);
	  ty = py + t * (cy - py);
	}
      }
    };
  public:
    PolyClipper (const StatusOutput& statOut,
      csVector2 *InP, size_t InV, csVector2 *OutP,
      size_t ClipPolyVertices, csVector2* ClipPoly, csVector2* ClipData) : 
      statOut (statOut), InP (InP), InV (InV), OutP (OutP), OutV (0),
      ClipPoly (ClipPoly), ClipData (ClipData), 
      ClipPolyVertices (ClipPolyVertices)
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
      csVector2 *OutP = (ClipPolyVertices & 1) ? this->OutP : TempPoly;

      if (ClipPolyVertices & 1)
	statOut.Flip();

      uint8 Clipped = CS_CLIP_INSIDE;
      
      for (size_t edge = 0; edge < ClipPolyVertices; edge++)
      {
	EdgeClipper<ClipPolyEdge, StatusOutput> edgeClip (
	  ClipPolyEdge (ClipPoly, ClipData, edge), 
	  statOut, InP, InV, OutP, OutV);
	Clipped = csMin (Clipped, edgeClip.ClipEdge());

	// Switch input/output polys: now we're going to clip
	// the polygon we just created against the next clipper edge
	InV = OutV;
	InP = OutP;
	OutP = (OutP == TempPoly) ? this->OutP : TempPoly;
	statOut.Flip();
      }
      
      return Clipped;
    }
    size_t GetOutputCount() { return OutV; }
  };
 
} // namespace CrystalSpace

#endif // __CS_CSGEOM_POLYCLIP_H__
