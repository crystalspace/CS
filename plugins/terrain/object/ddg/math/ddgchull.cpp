/*
    Copyright (C) 1997, 1998, 1999, 2000 by Alex Pfaffe
	(Digital Dawn Graphics Inc)
  
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
#include "math/ddggeom.h"
#include "math/ddgchull.h"
#include "math/ddgbbox.h"
/*
Return Values:
0 = Outside
1 = Intersecting
2 = Inside
 This method would benefit from SSE since we can perform 4 dot products in one go and we may need
  as many as 12 in the worst case.
*/
ddgInside ddgCHull3::clip(ddgBBox3 *bbox, bool occluder )
{
	ddgAssert(noPlanes > 0 && noPlanes < 20);
	static ddgVector3 minPt(0,0,0), maxPt(0,0,0);

	ddgInside visState = ddgIN;  // Assume all points are inside.
	int i = noPlanes;

	while (i)   // For each plane.
	{
		i--;
		ddgAsserts(planes[i].n[0]||planes[i].n[1]||planes[i].n[2],"Zero size plane normal!");
		for (int j = 0; j < 3; j++)  // For each dimension.
		{
			if (planes[i].n[j] >= 0.0f)
			{
				minPt.v[j] = bbox->min[j];
				maxPt.v[j] = bbox->max[j];
			}
			else
			{
				minPt.v[j] = bbox->max[j];
				maxPt.v[j] = bbox->min[j];
			}
		}
		// If there is any plane for which the min point is outside, then the box is completely outside.
	   	if (planes[i].isPointAbovePlane(&minPt) > 0.0f)	// MinPt is on outside.
		{
			visState = ddgOUT;
			i = 0;
		}
		else if (planes[i].isPointAbovePlane(&maxPt) >= 0.0f) // MaxPt is on outside (and min was on inside).
			visState = ddgPART;
	}
	// If the bbox has not been excluded, test any sub trees.
	if (ddgOUT != visState && subhulls)
		{
		subhulls->clip(bbox,!occluder);
		}
	return visState;
}

ddgInside ddgCHull3::clip(ddgTriangle3 *tri, bool occluder )
{
	ddgAssert(noPlanes > 0 && noPlanes < 20);
	static ddgVector3 minPt(0,0,0), maxPt(0,0,0);

	ddgInside visState = ddgIN;  // Assume all points are inside.
	int i = noPlanes, j;
	int out = 0;			// Number of points which are out w.r.t. a given plane.
	int totalin = 0;
	while (i)   // For each plane.
	{
		i--;
		out = 0;
		ddgAsserts(planes[i].n[0]||planes[i].n[1]||planes[i].n[2],"Zero size plane normal!");
		// Check each point.
		for (j = 0; j < 3; j++)
			{
			if (planes[i].isPointAbovePlane(&(tri->v[j])) >= 0.0f)
				out++;
			else
				totalin++;
			}
		if (out == 3)
		{
			visState = ddgOUT;
			i = 0;
		}
	}
	if (totalin < noPlanes * 3)
		visState = ddgPART;
	// If the bbox has not been excluded, test any sub trees.
	if (ddgOUT != visState && subhulls)
		{
		subhulls->clip(tri,!occluder);
		}
	return visState;
}

ddgInside ddgCHull3::clip(ddgPrism3 *prism, bool occluder )
{
	ddgAssert(noPlanes > 0 && noPlanes < 20);
	static ddgVector3 minPt(0,0,0), maxPt(0,0,0);

	ddgInside visState = ddgIN;  // Assume all points are inside.
	int i = noPlanes, j,k;
	int out = 0;			// Number of points which are out w.r.t. a given plane.
	int totalin = 0;
	while (i)   // For each plane.
	{
		i--;
		out = 0;
		ddgAsserts(planes[i].n[0]||planes[i].n[1]||planes[i].n[2],"Zero size plane normal!");
		// Check each point.
		for (k = 0; k < 2; k++)
		{
			for (j = 0; j < 3; j++)
				{
				if (planes[i].isPointAbovePlane(&(prism->t[k].v[j])) >= 0.0f)
					out++;
				else
					totalin++;
				}
			if (out == 3)
			{
				visState = ddgOUT;
				i = 0;
			}
		}
	}
	if (totalin < noPlanes * 6)
		visState = ddgPART;
	// If the bbox has not been excluded, test any sub trees.
	if (ddgOUT != visState && subhulls)
		{
		subhulls->clip(prism,!occluder);
		}
	return visState;
}


static ddgInside ddgInsideState[3] = {ddgOUT, ddgPART, ddgIN};
ddgInside ddgCHullSet::clip(ddgBBox3 *bbox, bool occluder)
{
	ddgInside visState = ddgOUT, tmp;
	int i = 0;
	while (i < _noVolumes)
	{
		tmp = _hulls[i].clip(bbox,occluder);
		// If we are entirely within a box we can exit.
		if ( ddgIN == tmp )
		{
			visState = tmp;
			break;
		}
		// At least we are partially inside some volume.
		if ( ddgPART == tmp)
		{
			visState = ddgPART;
		}
		// If ddgOUT we just try the next volume.
		i++;
	}
	return ddgInsideState[occluder ? visState : ddgInside(2 - visState)];
}

ddgInside ddgCHullSet::clip(ddgTriangle3 *tri, bool occluder)
{
	ddgInside visState = ddgOUT, tmp;
	int i = 0;
	while (i < _noVolumes)
	{
		tmp = _hulls[i].clip(tri,occluder);
		// If we are entirely within a box we can exit.
		if ( ddgIN == tmp )
		{
			visState = tmp;
			break;
		}
		// At least we are partially inside some volume.
		if ( ddgPART == tmp)
		{
			visState = ddgPART;
		}
		// If ddgOUT we just try the next volume.
		i++;
	}
	return ddgInsideState[occluder ? visState : ddgInside(2 - visState)];
}


ddgInside ddgCHullSet::clip(ddgPrism3 *prism, bool occluder)
{
	ddgInside visState = ddgOUT, tmp;
	int i = 0;
	while (i < _noVolumes)
	{
		tmp = _hulls[i].clip(prism,occluder);
		// If we are entirely within a box we can exit.
		if ( ddgIN == tmp )
		{
			visState = tmp;
			break;
		}
		// At least we are partially inside some volume.
		if ( ddgPART == tmp)
		{
			visState = ddgPART;
		}
		// If ddgOUT we just try the next volume.
		i++;
	}
	return ddgInsideState[occluder ? visState : ddgInside(2 - visState)];
}

ddgCHullSet::ddgCHullSet( int nv ) : _noVolumes(nv)
{
	_hulls = new ddgCHull3[_noVolumes];
	ddgMemorySet(ddgCHull3,_noVolumes);
}

ddgCHullSet::~ddgCHullSet(void)
{
	ddgMemoryFree(ddgCHull3,_noVolumes);

	delete [] _hulls;
	_noVolumes = 0;
}


ddgInside ddgCHull2::clip( ddgRect2 *rect, bool occluder)
{
	(void)occluder;
	ddgAssert(noLines > 0 && noLines < 20);
	static ddgVector2 minPt(0,0), maxPt(0,0);

	ddgInside visState = ddgIN;  // Assume all points are inside.
	int i = noLines;

	while (i)   // For each plane.
	{
		i--;
		ddgAsserts(lines[i].n[0]||lines[i].n[1],"Zero size plane normal!");
		for (int j = 0; j < 2; j++)  // For each dimension.
		{
			if (lines[i].n[j] >= 0.0f)
			{
				minPt.v[j] = rect->min[j];
				maxPt.v[j] = rect->max[j];
			}
			else
			{
				minPt.v[j] = rect->max[j];
				maxPt.v[j] = rect->min[j];
			}
		}
		// If there is any plane for which the min point is outside, then the box is completely outside.
	   	if (lines[i].isPointAbovePlane(&minPt) > 0.0f)	// MinPt is on outside.
		{
			visState = ddgOUT;
			i = 0;
		}
		else if (lines[i].isPointAbovePlane(&maxPt) >= 0.0f) // MaxPt is on outside (and min was on inside).
			visState = ddgPART;
	}
	return visState;
}
