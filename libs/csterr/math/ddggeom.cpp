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
/** Project a point onto a plane defined by its normal.
 * Return point in this.
 * QUAKE II q_shared.c(120)
 *  UNTESTED.
 */
/** inv_denom = 1.0F / DotProduct( n, n ); 
  d = DotProduct( n, p ) * inv_denom; 
  dst[0..2] = p[0..2] - d * inv_denom * n[0..2];
*/
ddgVector3 ddgPlane3::project(ddgVector3 *p0)
{
	float inv_denom = 1.0F / n.dot(&n);
	ddgVector3 pp(n);
	ddgVector3 p1;
	if (d)
		p1.set(0,0,-1*c()/d); // Some point on the plane.
//	else
//		p1.set(0,-1*b()/d,0); <- DIVIDE BY ZERO?

	pp.multiply(-inv_denom * n.dot(p0) * inv_denom);
	pp.add(&p1);
    return pp;
}

/// Return distance between point and plane.
/// Not useful except for that the sign indicates which side the point is on.
float ddgPlane3::distToPoint(const ddgVector3* q)
{
	// Get point on the plane, let x = 0, y  = 0.
	ddgVector3 pt;
	if (n[2] != 0)
		pt = ddgVector3(0,0, d / n[2]);
	else
		pt = ddgVector3(0,d/n[1],0);

	ddgVector3 pq(q);
	pq -= pt;
	return pq.dot(&n)/n.size();

}
/// Intersect a plane with a line (defined by 2 points) return point of intersection.
bool ddgPlane3::intersectPlaneWithLine(const ddgVector3* l1, const ddgVector3* l2, ddgVector3 *pi)
{
/*    If the plane is defined as:

    a*x + b*y + c*z + d = 0

and the line is defined as:

    x = x1 + (x2 - x1)*t = x1 + i*t
    y = y1 + (y2 - y1)*t = y1 + j*t
    z = z1 + (z2 - z1)*t = z1 + k*t

Then just substitute these into the plane equation. You end up
with:

    t = - (a*x1 + b*y1 + c*z1 + d)/(a*i + b*j + c*k)

When the denominator is zero, the line is contained in the plane 
if the numerator is also zero (the point at t=0 satisfies the
plane equation), otherwise the line is parallel to the plane.
*/
    ddgVector3 dl((*l2)-(*l1));
    float n = (l1->v[0]*a() + l1->v[1]*b() + l1->v[2]*c() + d);
    float dv = (a()*dl[0]  + b()*dl[1]  + c()*dl[2]);
    if (!dv && !n) // Line lies in the plane (and thus parallel).
        return true;
    if (dv && !n) // Line parallel to plane.
        return false;
    // Line intersects with the plane.
    float t = -1 * n / dv;
    dl *= t;
    *pi = l1 + dl;
    return true;
}
/// Intersect a point with plane.  Returns if point is on the plane
bool ddgPlane3::intersectPointPlane( const ddgVector3* )
{
    bool hit = false;
    return hit;
}

/// Intersect a triangle with line.  Returns if point is 
bool ddgTriangle3::intersectTriangleWithLine( const ddgVector3* l1, const ddgVector3* l2, ddgVector3 *pi)
{
    ddgPlane3 pl(v[0],v[1],v[2]);
    return pl.intersectPlaneWithLine(l1,l2,pi);
}
/** Intersect a point with triangle.  Returns if point is inside the triangle.
 *  point is assumed to lie in plane of triangle.
 */
bool ddgTriangle3::intersectPointTriangle( const ddgVector3* pt)
{
    // Must be able to optimize this...
    ddgVector3 d1(v[0]-*pt);
    ddgVector3 d2(v[1]-*pt);
    ddgVector3 d3(v[2]-*pt);
    ddgVector3 d12(v[0]-v[1]);
    ddgVector3 d13(v[0]-v[2]);
    ddgVector3 d23(v[1]-v[2]);

    float sd1 = d1.sizesq();
    float sd2 = d2.sizesq();
    float sd3 = d3.sizesq();
    float sd12 = d12.sizesq();
    float sd13 = d13.sizesq();
    float sd23 = d23.sizesq();

    float sdd1 = (sd12 < sd13) ? sd12 : sd13;
    float sdd2 = (sd12 < sd23) ? sd12 : sd23;
    float sdd3 = (sd13 < sd23) ? sd13 : sd23;
    if (sd1 <= sdd1 && sd2 <= sdd2 && sd3 <= sdd3)
        return true;
    return false;
}


/** Polygon clipping routine.
 *  The Sutherland-Hodgeman Clipping Algorithm
 *  Plane a set of clipping planes in world space.
 */

/*
Polygon3* Polygon3::clip (Plane3* p, bool rotated)
 {	// Early out: All points on one side of plane 
	 double dist[25];
	 bool allin=true, allout=true;
	 for (int i=0; icoord();
		 if (rotated) dist[i]=p->A()*c->rx()+p->B()*c->ry()+p->C()*c->rz()-p->D();
				 else dist[i]=p->A()*c->x() +p->B()*c->y() +p->C()*c->z() -p->D();
		 if (dist[i]<-EPSILON) allin=false;
		 if (dist[i]>=EPSILON) allout=false;
	 }	
	 if (allin) return this; else if (allout) return 0;
	 // Clip a polygon against a plane
	 Vector3 *cv[10], *v1=vertex(0);
	 double dist2, dist1=dist[0];
	 bool clipped=false, inside=(dist1>=0);
	 int curv=0;
	 for (i=1; i<=vertices(); i++)
	 {	 fVertex* v2=vertex(i % vertices());
		 dist2=dist[i % vertices()];
		 // Sutherland-hodgeman clipping
		 if (inside && (dist2>=0.0f)) cv[curv++]=v2;	// Both in
		 else if ((!inside) && (dist2>=EPSILON))		// Coming in
		 {	 clipped=inside=true;
			 fVertex* t=tempv[tempverts++]=newVert ();
			 double d=dist1/(dist1-dist2);
			 if (rotated)
			 {	 t->rx((float)(v1->rx()+(v2->rx()-v1->rx())*d));
				 t->ry((float)(v1->ry()+(v2->ry()-v1->ry())*d));
				 t->rz((float)(v1->rz()+(v2->rz()-v1->rz())*d));
				 t->coord ()->processed (1);
			 } else
			 {	 t->x((float)(v1->x()+(v2->x()-v1->x())*d));
				 t->y((float)(v1->y()+(v2->y()-v1->y())*d));
				 t->z((float)(v1->z()+(v2->z()-v1->z())*d));
				 t->coord ()->processed (0);
			 }
			 t->u((float)(v1->u()+(v2->u()-v1->u())*d));
			 t->v((float)(v1->v()+(v2->v()-v1->v())*d));
			 cv[curv++]=t;
			 cv[curv++]=v2;
		 } else if (inside && (dist2<-EPSILON))		// Going out
		 {	 clipped=true;
			 inside=false;
			 fVertex* t=tempv[tempverts++]=newVert ();
			 double d=dist1/(dist1-dist2);
			 if (rotated)
			 {	 t->rx((float)(v1->rx()+(v2->rx()-v1->rx())*d));
				 t->ry((float)(v1->ry()+(v2->ry()-v1->ry())*d));
				 t->rz((float)(v1->rz()+(v2->rz()-v1->rz())*d));
				 t->coord ()->processed (1);
			 } else
			 {	 t->x((float)(v1->x()+(v2->x()-v1->x())*d));
				 t->y((float)(v1->y()+(v2->y()-v1->y())*d));
				 t->z((float)(v1->z()+(v2->z()-v1->z())*d));
				 t->coord ()->processed (0);
			 }
			 t->u((float)(v1->u()+(v2->u()-v1->u())*d));
			 t->v((float)(v1->v()+(v2->v()-v1->v())*d));
			 cv[curv++]=t;
		 } clipped=true;								// Both out
		 v1=v2;
		 dist1=dist2;
	 }
	 if (!clipped) return this;
	 // Construct clipped polygon
	 if (curv<3) return 0;
	 fPolygon* poly=newPoly (curv);
	 poly->type (m_type|TEMP);
	 poly->m_plane=m_plane;
	 poly->texture (m_text);
	 for (i=0; ivertex(i, cv[i]);
		 if (rotated) cv[i]->coord()->perspective();
	 }
	 return poly;
 }
*/
