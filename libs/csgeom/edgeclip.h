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
    This include file contains the logic that clips the polygon pointed by
    InP to the output array pointed by OutP against a single edge of clipper
    polygon. This file is used both for clipping against a polygon
    (csPolygonClipper) and against a box (csBoxClipper). 
*/

#ifndef __CS_CSGEOM_EDGECLIP_H__
#define __CS_CSGEOM_EDGECLIP_H__

namespace CrystalSpace
{
  
  template <typename ClipLogic, typename StatusOutput>
  class EdgeClipper
  {
    ClipLogic clip;
    StatusOutput statOut;

    // The currently clipped polygon
    csVector2 *InP;
    // Number of input vertices
    size_t InV;
    // The current output polygon
    csVector2 *OutP;
    // Number of output vertices
    size_t& OutV;
  public:
    EdgeClipper (const ClipLogic& clip, const StatusOutput& statOut,
      csVector2 *InP, size_t InV, csVector2 *OutP, size_t& OutV) : 
      clip (clip), statOut (statOut), InP (InP), InV (InV), OutP (OutP), 
      OutV (OutV)
    {
    }
    uint8 ClipEdge()
    {
      bool Clipped = false;
      // Current input vertex, vertex % num_vertices
      size_t vert, realvert;
      
      // First ("previous") point x/y
      float px = InP [0].x, py = InP [0].y;
      // The "inside/outside polygon" flag for previous vertex
      bool prevVertexInside = clip.Inside (px, py);
    
      // Empty output polygon
      OutV = 0;
      // A convex polygon cannot be intersected by a line more than twice
      int IntersectionCount = 0;
      for (vert = 1; vert <= InV; vert++)
      {
	// Is this the last vertex being considered?
	bool LastVertex = (vert == InV);
	// Second ("current") point x/y
	float cx, cy;
	realvert = LastVertex ? 0 : vert;
	CS_ASSERT (realvert < InV);
	cx = InP [realvert].x;
	cy = InP [realvert].y;
    
	// If starting vertex is visible, put it into output array
	if (prevVertexInside)
	  if ((!OutV
	    || ABS (px - OutP [OutV - 1].x) > EPSILON
	    || ABS (py - OutP [OutV - 1].y) > EPSILON)
	   && (!LastVertex
	    || ABS (px - OutP [0].x) > EPSILON
	    || ABS (py - OutP [0].y) > EPSILON))
	{
	  CS_ASSERT (OutV >= 0 && OutV < MAX_OUTPUT_VERTICES);
	  OutP [OutV].x = px;
	  OutP [OutV].y = py;
	  statOut.Copy (OutV, vert - 1);
	  if (++OutV >= MAX_OUTPUT_VERTICES)
	    break;
	} /* endif */
    
	// The "inside/outside polygon" flag for current vertex
	bool curVertexInside = clip.Inside (cx, cy);
    
	// If vertices are on different sides of edge,
	// look where we're intersecting
	if (prevVertexInside != curVertexInside)
	{
	  // Set the "input has been clipped" flag
	  Clipped = true;
    
	  // Check if and where edges intersects
	  double t;
	  float tx, ty;
    
	  clip.Intersect (px, py, cx, cy, t, tx, ty);

	  if ((!OutV
	    || ABS (tx - OutP [OutV - 1].x) > EPSILON
	    || ABS (ty - OutP [OutV - 1].y) > EPSILON)
	   && (!LastVertex
	    || ABS (tx - OutP [0].x) > EPSILON
	    || ABS (ty - OutP [0].y) > EPSILON))
	  {
	    CS_ASSERT (OutV >= 0 && OutV < MAX_OUTPUT_VERTICES);
	    OutP [OutV].x = tx;
	    OutP [OutV].y = ty;
	    /*
	     * Four cases are possible here:
	     *
	     * (*) prev vertex: CS_VERTEX_ORIGINAL
	     *     curr vertex: CS_VERTEX_ORIGINAL
	     *       -> status_v = prev_vertex.index
	     *       -> status_t = t
	     *
	     * (*) prev vertex: CS_VERTEX_ORIGINAL
	     *     curr vertex: CS_VERTEX_ONEDGE
	     *       -> status_v = prev_vertex.index
	     *       -> status_t = t * curr_vertex.status_t
	     *
	     * (*) prev vertex: CS_VERTEX_ONEDGE
	     *     curr vertex: CS_VERTEX_ORIGINAL
	     *       -> status_type = CS_VERTEX_INSIDE
	     *
	     * (*) prev vertex: CS_VERTEX_ONEDGE
	     *     curr vertex: CS_VERTEX_ONEDGE
	     *       -> status_type = CS_VERTEX_INSIDE
	     */
	    if (statOut.GetType (vert - 1) == CS_VERTEX_ORIGINAL)
	    {
	      if (statOut.GetType (realvert) == CS_VERTEX_ORIGINAL)
	      {
		if ((ABS(tx - px) > EPSILON)
		  && (ABS(ty - py) > EPSILON))
		  statOut.OnEdge (OutV, vert - 1, t);
		else
		  statOut.Copy (OutV, vert - 1);
	      }
	      else
	      {
		// Current vertex is on edge, it cannot be CS_VERTEX_INSIDE
		// because it is connected with at least one CS_VERTEX_ORIGINAL
		const double newPos = t * statOut.GetPos (realvert);
		statOut.OnEdge (OutV, vert - 1, newPos);
	      }
	    }
	    else
	    {
	      statOut.Inside (OutV);
	    }
	    if (++OutV >= MAX_OUTPUT_VERTICES)
	      break;
	  } /* endif */
    
	  if (++IntersectionCount >= 2)
	  {
	    // Drop out, after adding all vertices left in input polygon
	    if (curVertexInside && !LastVertex)
	    {
	      if (ABS (InP [vert].x - OutP [OutV - 1].x) < EPSILON
	       && ABS (InP [vert].y - OutP [OutV - 1].y) < EPSILON)
		vert++;
	      size_t count = InV - vert;
	      if (OutV + count > MAX_OUTPUT_VERTICES)
		count = MAX_OUTPUT_VERTICES - OutV;
	      memcpy (&OutP [OutV], &InP [vert], count * sizeof (OutP [0]));
	      statOut.Copy (OutV, count, vert);
	      OutV += count;
	      if (OutV >= MAX_OUTPUT_VERTICES)
		break;
	    } /* endif */
	    break;
	  } /* endif */
	} /* endif */
    
	px = cx; py = cy;
	prevVertexInside = curVertexInside;
      } /* endfor */
    
      // If polygon is wiped out, break
      if (OutV < 3)
      {
	OutV = 0;
	return CS_CLIP_OUTSIDE;
      }
    
      return Clipped ? CS_CLIP_CLIPPED : CS_CLIP_INSIDE;
    }
  };
  
} // namespace CrystalSpace

#endif // __CS_CSGEOM_EDGECLIP_H__
