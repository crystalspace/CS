/*
    Copyright (C) 2005 by Jorrit Tyberghein

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

/* Triangle/triangle intersection test routine,
 * by Tomas Moller, 1997.
 * See article "A Fast Triangle-Triangle Intersection Test",
 * Journal of Graphics Tools, 2(2), 1997
 * updated: 2001-06-20 (added line of intersection)
 *
 * 2005-04-15 ported to CS by Andrew Dai
 */

#include "cssysdef.h"
#include <math.h>

#include "csgeom/math3d.h"
#include "csgeom/vector3.h"
#include "csgeom/segment.h"
#include "csgeom/math.h"

#define FABS(x) (fabsf(x))        /* implement as is fastest on your machine */

/* if USE_EPSILON_TEST is true then we do a check: 
         if |dv|<SMALL_EPSILON then dv=0.0;
   else no check is done (which is less robust)
*/
#define USE_EPSILON_TEST TRUE  

/* some macros */

/* sort so that a<=b */
#define SORT(a,b)       \
             if(a>b)    \
             {          \
               float c; \
               c=a;     \
               a=b;     \
               b=c;     \
             }

/* this edge to edge test is based on Franlin Antonio's gem:
   "Faster Line Segment Intersection", in Graphics Gems III,
   pp. 199-202 */ 
#define EDGE_EDGE_TEST(V0,U0,U1)                      \
  Bx=U0[i0]-U1[i0];                                   \
  By=U0[i1]-U1[i1];                                   \
  Cx=V0[i0]-U0[i0];                                   \
  Cy=V0[i1]-U0[i1];                                   \
  f=Ay*Bx-Ax*By;                                      \
  d=By*Cx-Bx*Cy;                                      \
  if((f>0 && d>=0 && d<=f) || (f<0 && d<=0 && d>=f))  \
  {                                                   \
    e=Ax*Cy-Ay*Cx;                                    \
    if(f>0)                                           \
    {                                                 \
    if(e>=0 && e<=f) return true;                   \
    }                                                 \
    else                                              \
    {                                                 \
      if(e<=0 && e>=f) return true;                   \
    }                                                 \
  }                                

#define EDGE_AGAINST_TRI_EDGES(V0,V1,U0,U1,U2) \
{                                              \
  float Ax,Ay,Bx,By,Cx,Cy,e,d,f;               \
  Ax=V1[i0]-V0[i0];                            \
  Ay=V1[i1]-V0[i1];                            \
  /* test edge U0,U1 against V0,V1 */          \
  EDGE_EDGE_TEST(V0,U0,U1);                    \
  /* test edge U1,U2 against V0,V1 */          \
  EDGE_EDGE_TEST(V0,U1,U2);                    \
  /* test edge U2,U1 against V0,V1 */          \
  EDGE_EDGE_TEST(V0,U2,U0);                    \
}

#define POINT_IN_TRI(V0,U0,U1,U2)           \
{                                           \
  float a,b,c,d0,d1,d2;                     \
  /* is T1 completly inside T2? */          \
  /* check if V0 is inside tri(U0,U1,U2) */ \
  a=U1[i1]-U0[i1];                          \
  b=-(U1[i0]-U0[i0]);                       \
  c=-a*U0[i0]-b*U0[i1];                     \
  d0=a*V0[i0]+b*V0[i1]+c;                   \
                                            \
  a=U2[i1]-U1[i1];                          \
  b=-(U2[i0]-U1[i0]);                       \
  c=-a*U1[i0]-b*U1[i1];                     \
  d1=a*V0[i0]+b*V0[i1]+c;                   \
                                            \
  a=U0[i1]-U2[i1];                          \
  b=-(U0[i0]-U2[i0]);                       \
  c=-a*U2[i0]-b*U2[i1];                     \
  d2=a*V0[i0]+b*V0[i1]+c;                   \
  if(d0*d1>0.0)                             \
  {                                         \
    if(d0*d2>0.0) return true;              \
  }                                         \
}

static bool coplanar_tri_tri(const csVector3& N, const csVector3 tri1[3],
                             const csVector3 tri2[3])
{
   float A[3];
   short i0,i1;
   /* first project onto an axis-aligned plane, that maximizes the area */
   /* of the triangles, compute indices: i0,i1. */
   A[0]=fabs(N[0]);
   A[1]=fabs(N[1]);
   A[2]=fabs(N[2]);
   if(A[0]>A[1])
   {
      if(A[0]>A[2])  
      {
          i0=1;      /* A[0] is greatest */
          i1=2;
      }
      else
      {
          i0=0;      /* A[2] is greatest */
          i1=1;
      }
   }
   else   /* A[0]<=A[1] */
   {
      if(A[2]>A[1])
      {
          i0=0;      /* A[2] is greatest */
          i1=1;                                           
      }
      else
      {
          i0=0;      /* A[1] is greatest */
          i1=2;
      }
    }               
                
    /* test all edges of triangle 1 against the edges of triangle 2 */
    EDGE_AGAINST_TRI_EDGES(tri1[0],tri1[1],tri2[0],tri2[1],tri2[2]);
    EDGE_AGAINST_TRI_EDGES(tri1[1],tri1[2],tri2[0],tri2[1],tri2[2]);
    EDGE_AGAINST_TRI_EDGES(tri1[2],tri1[0],tri2[0],tri2[1],tri2[2]);
                
    /* finally, test if tri1 is totally contained in tri2 or vice versa */
    POINT_IN_TRI(tri1[0],tri2[0],tri2[1],tri2[2]);
    POINT_IN_TRI(tri2[0],tri1[0],tri1[1],tri1[2]);

    return false;
}

#define NEWCOMPUTE_INTERVALS(VV0,VV1,VV2,D0,D1,D2,D0D1,D0D2,A,B,C,X0,X1) \
{ \
        if(D0D1>0.0f) \
        { \
                /* here we know that D0D2<=0.0 */ \
            /* that is D0, D1 are on the same side, D2 on the other or on the plane */ \
                A=VV2; B=(VV0-VV2)*D2; C=(VV1-VV2)*D2; X0=D2-D0; X1=D2-D1; \
        } \
        else if(D0D2>0.0f)\
        { \
                /* here we know that d0d1<=0.0 */ \
            A=VV1; B=(VV0-VV1)*D1; C=(VV2-VV1)*D1; X0=D1-D0; X1=D1-D2; \
        } \
        else if(D1*D2>0.0f || D0!=0.0f) \
        { \
                /* here we know that d0d1<=0.0 or that D0!=0.0 */ \
                A=VV0; B=(VV1-VV0)*D0; C=(VV2-VV0)*D0; X0=D0-D1; X1=D0-D2; \
        } \
        else if(D1!=0.0f) \
        { \
                A=VV1; B=(VV0-VV1)*D1; C=(VV2-VV1)*D1; X0=D1-D0; X1=D1-D2; \
        } \
        else if(D2!=0.0f) \
        { \
                A=VV2; B=(VV0-VV2)*D2; C=(VV1-VV2)*D2; X0=D2-D0; X1=D2-D1; \
        } \
        else \
        { \
                /* triangles are coplanar */ \
                return coplanar_tri_tri(N1,tri1,tri2); \
        } \
}

/**
 * Tests if two triangles intersect (without divisions)
 *
 * @param tri1 Vertices of triangle 1
 * @param tri2 Vertices of triangle 2
 * @return true if the triangles intersect, otherwise false
 *
 */
bool csIntersect3::TriangleTriangle (const csVector3 tri1[3],
                                     const csVector3 tri2[3])
{
  /* compute plane equation of triangle(tri1) */
  csVector3 E1 = tri1[1] - tri1[0];
  csVector3 E2 = tri1[2] - tri1[0];
  csVector3 N1 = E1 % E2;
  float d1 =- (N1 * tri1[0]);
  /* plane equation 1: N1.X + d1 = 0 */

  /* put tri2 into plane equation 1 to compute signed distances to the plane*/
  float du0 = (N1 * tri2[0]) + d1;
  float du1 = (N1 * tri2[1]) + d1;
  float du2 = (N1 * tri2[2]) + d1;

  /* coplanarity robustness check */
#if USE_EPSILON_TEST==TRUE
  if (FABS (du0) < SMALL_EPSILON) du0 = 0.0f;
  if (FABS (du1) < SMALL_EPSILON) du1 = 0.0f;
  if (FABS (du2) < SMALL_EPSILON) du2 = 0.0f;
#endif

  float du0du1 = du0 * du1;
  float du0du2 = du0 * du2;
  if (du0du1 > 0.0f && du0du2 > 0.0f) /* same sign on all of them + not equal 0 ? */
    return false;                     /* no intersection occurs */

  /* compute plane of triangle (tri2) */
  E1 = tri2[1] - tri2[0];
  E2 = tri2[2] - tri2[0];
  csVector3 N2 = E1 % E2;
  float d2 =- (N2 * tri2[0]);
  /* plane equation 2: N2.X + d2 = 0 */

  /* put tri1 into plane equation 2 */
  float dv0 = (N2 * tri1[0]) + d2;
  float dv1 = (N2 * tri1[1]) + d2;
  float dv2 = (N2 * tri1[2]) + d2;

#if USE_EPSILON_TEST==TRUE
  if (FABS (dv0) < SMALL_EPSILON) dv0 = 0.0f;
  if (FABS (dv1) < SMALL_EPSILON) dv1 = 0.0f;
  if (FABS (dv2) < SMALL_EPSILON) dv2 = 0.0f;
#endif

  float dv0dv1 = dv0 * dv1;
  float dv0dv2 = dv0 * dv2;

  if (dv0dv1 > 0.0f && dv0dv2 > 0.0f) /* same sign on all of them + not equal 0 ? */
    return false;                     /* no intersection occurs */

  /* compute direction of intersection line */
  csVector3 D = N1 % N2;

  /* compute and index to the largest component of D */
  float max = (float) FABS (D[0]);
  short index = 0;
  float bb = (float) FABS (D[1]);
  float cc = (float) FABS (D[2]);
  if (bb > max) max = bb, index = 1;
  if (cc > max) max = cc, index = 2;

  /* this is the simplified projection onto L*/
  float vp0 = tri1[0][index];
  float vp1 = tri1[1][index];
  float vp2 = tri1[2][index];

  float up0 = tri2[0][index];
  float up1 = tri2[1][index];
  float up2 = tri2[2][index];

  float a, b, c, x0, x1;
  float d, e, f, y0, y1;
  /* compute interval for triangle 1 */
  NEWCOMPUTE_INTERVALS (vp0, vp1, vp2, dv0, dv1, dv2, dv0dv1, dv0dv2,
                          a, b, c, x0, x1);

  /* compute interval for triangle 2 */
  NEWCOMPUTE_INTERVALS (up0, up1, up2, du0, du1, du2, du0du1, du0du2,
                          d, e, f, y0, y1);

  float xx, yy, xxyy, tmp;
  float isect1[2], isect2[2];
  xx = x0 * x1;
  yy = y0 * y1;
  xxyy = xx * yy;

  tmp = a * xxyy;
  isect1[0] = tmp + b * x1 * yy;
  isect1[1] = tmp + c * x0 * yy;

  tmp = d * xxyy;
  isect2[0] = tmp + e * xx * y1;
  isect2[1] = tmp + f * xx * y0;

  SORT (isect1[0], isect1[1]);
  SORT (isect2[0], isect2[1]);

  if (isect1[1] < isect2[0] || isect2[1] < isect1[0]) return false;
  return true;
}

/* sort so that a<=b */
#define SORT2(a,b,smallest)       \
             if(a>b)       \
             {             \
               float c;    \
               c=a;        \
               a=b;        \
               b=c;        \
               smallest=1; \
             }             \
             else smallest=0;


static inline void isect2 (const csVector3 tri[3], float VV0, float VV1,
                           float VV2, float D0, float D1, float D2,
                           float *isect0, float *isect1, csVector3 isectline[2])
{
  float tmp = D0 / (D0 - D1);
  csVector3 diff;
  *isect0 = VV0 + (VV1 - VV0) * tmp;
  diff = tri[1] - tri[0];
  diff = diff * tmp;
  isectline[0] = diff + tri[0];
  tmp=D0 / (D0 - D2);
  *isect1 = VV0 + (VV2 - VV0) * tmp;
  diff = tri[2] - tri[0];
  diff = diff * tmp;
  isectline[1] = tri[0] + diff;
}


static inline bool compute_intervals_isectline (const csVector3 tri[3],
				       float VV0,float VV1,float VV2,float D0,float D1,float D2,
				       float D0D1,float D0D2,float *isect0,float *isect1,
				       csVector3 isectline[2])
{
  if (D0D1 > 0.0f)                                        
  {                                                    
    /* here we know that D0D2<=0.0 */                  
    /* that is D0, D1 are on the same side, D2 on the other or on the plane */
    csVector3 newTri[3] = {tri[2], tri[0], tri[1]};
    isect2(newTri,VV2,VV0,VV1,D2,D0,D1,isect0,isect1,isectline);
  } 
  else if (D0D2 > 0.0f)                                   
  {   
    csVector3 newTri[3] = {tri[1], tri[0], tri[2]};
    /* here we know that d0d1<=0.0 */             
    isect2 (newTri, VV1, VV0, VV2, D1, D0, D2, isect0, isect1, isectline);
  }                                                  
  else if (D1 * D2 > 0.0f || D0 != 0.0f)   
  {        
    csVector3 newTri[3] = {tri[0], tri[1], tri[2]};
    /* here we know that d0d1<=0.0 or that D0!=0.0 */
    isect2 (newTri, VV0, VV1, VV2, D0, D1, D2, isect0, isect1, isectline);   
  }                                                  
  else if (D1 != 0.0f)                                  
  {             
    csVector3 newTri[3] = {tri[1], tri[0], tri[2]};
    isect2 (newTri, VV1, VV0, VV2, D1, D0, D2, isect0, isect1, isectline); 
  }                                         
  else if (D2 != 0.0f)                                  
  {                  
    csVector3 newTri[3] = {tri[2], tri[0], tri[1]};
    isect2 (newTri, VV2, VV0, VV1, D2, D0, D1, isect0, isect1, isectline);     
  }                                                 
  else                                               
  {                                                   
    /* triangles are coplanar */    
    return true;
  }
  return false;
}

#define COMPUTE_INTERVALS_ISECTLINE(VERT0,VERT1,VERT2,VV0,VV1,VV2,D0,D1,D2,D0D1,D0D2,isect0,isect1,isectpoint0,isectpoint1) \
  if(D0D1>0.0f)                                         \
  {                                                     \
    /* here we know that D0D2<=0.0 */                   \
    /* that is D0, D1 are on the same side, D2 on the other or on the plane */ \
    isect2(VERT2,VERT0,VERT1,VV2,VV0,VV1,D2,D0,D1,&isect0,&isect1,isectpoint0,isectpoint1);          \
  }                                                     
#if 0
  else if(D0D2>0.0f)                                    \
  {                                                     \
    /* here we know that d0d1<=0.0 */                   \
    isect2(VERT1,VERT0,VERT2,VV1,VV0,VV2,D1,D0,D2,&isect0,&isect1,isectpoint0,isectpoint1);          \
  }                                                     \
  else if(D1*D2>0.0f || D0!=0.0f)                       \
  {                                                     \
    /* here we know that d0d1<=0.0 or that D0!=0.0 */   \
    isect2(VERT0,VERT1,VERT2,VV0,VV1,VV2,D0,D1,D2,&isect0,&isect1,isectpoint0,isectpoint1);          \
  }                                                     \
  else if(D1!=0.0f)                                     \
  {                                                     \
    isect2(VERT1,VERT0,VERT2,VV1,VV0,VV2,D1,D0,D2,&isect0,&isect1,isectpoint0,isectpoint1);          \
  }                                                     \
  else if(D2!=0.0f)                                     \
  {                                                     \
    isect2(VERT2,VERT0,VERT1,VV2,VV0,VV1,D2,D0,D1,&isect0,&isect1,isectpoint0,isectpoint1);          \
  }                                                     \
  else                                                  \
  {                                                     \
    /* triangles are coplanar */                        \
    coplanar=1;                                         \
    return coplanar_tri_tri(N1,V0,V1,V2,U0,U1,U2);      \
  }
#endif

#if 0
/**
 * Tests if two triangles intersect and compute the line of intersection
 * (if they are not coplanar).
 *
 * @param tri1 Vertices of triangle 1
 * @param tri2 Vertices of triangle 2
 * @param[out] isectline The line segment where they intersect
 * @param[out] coplanar Returns whether the triangles are coplanar
 * @return true if the triangles intersect, otherwise false
 *
 */
bool csIntersect3::TriangleTriangle (const csVector3 tri1[3],
				                             const csVector3 tri2[3],
				                             csSegment3& isectline, bool& coplanar)
{
  /* compute plane equation of triangle(tri1) */
  csVector3 E1 = tri1[1] - tri1[0];
  csVector3 E2 = tri1[2] - tri1[0];
  csVector3 N1 = E1 % E2;
  float d1 =- (N1 * tri1[0]);
  /* plane equation 1: N1.X + d1 = 0 */

  /* put tri2 into plane equation 1 to compute signed distances to the plane */
  float du0 = (N1 * tri2[0]) + d1;
  float du1 = (N1 * tri2[1]) + d1;
  float du2 = (N1 * tri2[2]) + d1;

  /* coplanarity robustness check */
#if USE_EPSILON_TEST==TRUE
  if (fabs (du0) < SMALL_EPSILON) du0 = 0.0f;
  if (fabs (du1) < SMALL_EPSILON) du1 = 0.0f;
  if (fabs (du2) < SMALL_EPSILON) du2 = 0.0f;
#endif

  float du0du1 = du0 * du1;
  float du0du2 = du0 * du2;

  if (du0du1 > 0.0f && du0du2 > 0.0f) /* same sign on all of them + not equal 0 ? */
    return false;                     /* no intersection occurs */

  /* compute plane of triangle (tri2) */
  E1 = tri2[1] - tri2[0];
  E2 = tri2[2] - tri2[0];
  csVector3 N2 = E1 % E2;
  float d2 =- (N2 * tri2[0]);
  /* plane equation 2: N2.X + d2 = 0 */

  /* put tri1 into plane equation 2 */
  float dv0 = (N2 * tri1[0]) + d2;
  float dv1 = (N2 * tri1[1]) + d2;
  float dv2 = (N2 * tri1[2]) + d2;

#if USE_EPSILON_TEST==TRUE
  if (fabs (dv0) < SMALL_EPSILON) dv0 = 0.0f;
  if (fabs (dv1) < SMALL_EPSILON) dv1 = 0.0f;
  if (fabs (dv2) < SMALL_EPSILON) dv2 = 0.0f;
#endif

  float dv0dv1 = dv0 * dv1;
  float dv0dv2 = dv0 * dv2;
        
  if (dv0dv1 > 0.0f && dv0dv2 > 0.0f) /* same sign on all of them + not equal 0 ? */
    return false;                     /* no intersection occurs */

  /* compute direction of intersection line */
  csVector3 D = N1 % N2;

  /* compute and index to the largest component of D */
  float max = fabs (D[0]);
  short index = 0;
  float b = fabs (D[1]);
  float c = fabs (D[2]);
  if (b > max) max = b, index = 1;
  if (c > max) max = c, index = 2;

  /* this is the simplified projection onto L*/
  float vp0 = tri1[0][index];
  float vp1 = tri1[1][index];
  float vp2 = tri1[2][index];
  
  float up0 = tri2[0][index];
  float up1 = tri2[1][index];
  float up2 = tri2[2][index];

  float isect1[2] = {0,0}, isect2[2] = {0,0};
  csVector3 isectpointA[2];
  csVector3 isectpointB[2];
  
  /* compute interval for triangle 1 */
  coplanar = compute_intervals_isectline (tri1, vp0, vp1, vp2, dv0, dv1, dv2,
				       dv0dv1, dv0dv2, &isect1[0], &isect1[1], isectpointA);
  if (coplanar)
    return coplanar_tri_tri (N1, tri1, tri2);     

  /* compute interval for triangle 2 */
  compute_intervals_isectline (tri2, up0, up1, up2, du0, du1, du2,
			      du0du1, du0du2, &isect2[0], &isect2[1], isectpointB);

  int smallest1, smallest2;
  SORT2 (isect1[0], isect1[1], smallest1);
  SORT2 (isect2[0], isect2[1], smallest2);

  if (isect1[1] < isect2[0] || isect2[1] < isect1[0])
    return false;

  /* at this point, we know that the triangles intersect */

  if (isect2[0] < isect1[0])
  {
    if (smallest1 == 0)
      isectline.SetStart (isectpointA[0]);
    else
      isectline.SetStart (isectpointA[1]);

    if (isect2[1] < isect1[1])
    {
      if (smallest2 == 0)
        isectline.SetEnd (isectpointB[1]);
      else
        isectline.SetEnd (isectpointB[0]);
    }
    else
    {
      if (smallest1 == 0)
        isectline.SetEnd(isectpointA[1]);
      else
        isectline.SetEnd (isectpointA[0]);
    }
  }
  else
  {
    if (smallest2 == 0)
      isectline.SetStart (isectpointB[0]);
    else
      isectline.SetStart (isectpointB[1]);

    if (isect2[1] > isect1[1])
    {
      if (smallest1 == 0)
        isectline.SetEnd (isectpointA[1]);
      else
        isectline.SetEnd (isectpointA[0]);    
    }
    else
    {
      if (smallest2 == 0)
        isectline.SetEnd (isectpointB[1]);
      else
        isectline.SetEnd (isectpointB[0]);
    }
  }
  return true;
}

#endif

//--- VERSION WHICH COMPUTES INTERSECTION LINE

enum SIGN_CLASS
{
  D0_DIFFERENT = 0,
  D1_DIFFERENT = 1,
  D2_DIFFERENT = 2,
  
  D_ALL_SAME = 3,
  D_COPLANAR = 4
};

/*
 Get which of the three distances that have a different sign than the others.
 Returns -1 if all have same sign, and are not 0
 */
static inline SIGN_CLASS GetSignClassification (float d0, float d1, float d2)
{
  const float d0d1 = d0*d1;
  const float d0d2 = d0*d2;
  
  // All distances are on same side of plane
  if (d0d1 > 0.0f && d0d2 > 0.0f)
    return D_ALL_SAME;
  
  if (d0d1 > 0.0f)
  {
    // d0 and d1 same sign, d2 must be different
    return D2_DIFFERENT;
  }
  else if (d0d2 > 0.0f)
  {
    // d0 and d2 same sign, d1 must be different
    return D1_DIFFERENT;
  }
  else if (d1*d2 > 0.0f || d0 != 0.0f)
  {
    // d1 and d2 same sign, d0 different
    // d1 or d2 0, d0 is not, d0 different
    return D0_DIFFERENT;
  }
  else if (d1 != 0.0f)
  {
    // d0 or d2 is 0, d1 is not, d1 different
    return D1_DIFFERENT;
  }
  else if (d2 != 0.0f)
  {
    // d0 or d1 is 0, d2 is not, d2 different
    return D2_DIFFERENT;
  }

  return D_COPLANAR;
}

// Compute intersection between two edges [0]-[1] [0]-[2] and the plane.
// This assumes that [0] is the vertex with different d-sign
static inline void ComputerInterval2DProj (const csVector3 tri[3],
                                           const float projV[3],
                                           const float d[3],
                                           csSegment3& isectSeg,
                                           float& t0, float &t1)
{
  // t_edge = dv1_0 / (dv1_0 - dv1_1), see (4) in Möller
  float t_edge = d[0] / (d[0] - d[1]);

  // Project onto axis
  t0 = projV[0] + (projV[1] - projV[0]) * t_edge;

  // Compute real point of intersection
  isectSeg.SetStart (tri[0] + (tri[1] - tri[0]) * t_edge);

  // REDO FOR OTHER EDGE

  t_edge = d[0] / (d[0] - d[2]);

  // Project onto axis
  t1 = projV[0] + (projV[2] - projV[0]) * t_edge;

  // Compute real point of intersection
  isectSeg.SetEnd (tri[0] + (tri[2] - tri[0]) * t_edge);

}

static inline void ComputeIntervals (const csVector3 tri[3], float d[3],
                                     SIGN_CLASS signs, size_t projDim,
                                     csSegment3& isectSeg,
                                     float& t0, float &t1)
{
  // Convert sign-class into an index
  size_t signIdx = (size_t)signs;
  size_t other1 = CS::Math::NextModulo3 (signIdx);
  size_t other2 = CS::Math::NextModulo3 (other1);

  // Setup new triangle
  const csVector3 newTri[3] = {tri[signIdx], 
                               tri[other1], 
                               tri[other2]};

  const float newD[3] = {d[signIdx], d[other1], d[other2]};

  const float projV[3] = {tri[signIdx][projDim], 
                          tri[other1][projDim],
                          tri[other2][projDim]};

  // Compute the intersection
  ComputerInterval2DProj (newTri, projV, newD, isectSeg, t0, t1);

  // Make sure t0 < t1, and keep track of endpoints to t values
  csSort (t0, t1, isectSeg.Start (), isectSeg.End ());
}

static inline size_t HandlePointsInTriangle (const csVector3 tri1[3], 
                                             const csVector3 tri2[2],
                                             size_t k, size_t l,
                                             csVector3& point0,
                                             csVector3& point1)
{
  size_t numP = 0;

  const float a0 =  tri2[1][l] - tri2[0][l];
  const float b0 =-(tri2[1][k] - tri2[0][k]);
  const float c0 =-a0*tri2[0][k] - b0*tri2[0][l];

  const float a1 =  tri2[2][l] - tri2[1][l];
  const float b1 =-(tri2[2][k] - tri2[1][k]);
  const float c1 =-a1*tri2[1][k] - b1*tri2[1][l];

  const float a2 =  tri2[2][l] - tri2[1][l];
  const float b2 =-(tri2[2][k] - tri2[1][k]);
  const float c2 =-a2*tri2[1][k] - b2*tri2[1][l];

  for (size_t idx = 0; idx < 3; ++idx)
  {
    const float d0 = a0*tri1[idx][k]+b0*tri1[idx][l]+c0;
    const float d1 = a1*tri1[idx][k]+b1*tri1[idx][l]+c1;
    const float d2 = a2*tri1[idx][k]+b2*tri1[idx][l]+c2;

    if(d0*d1 > 0.0f && d0*d2 > 0.0f)
    {
      //have a point
      (numP == 0 ? point0 : point1) = tri1[idx];
      numP++;
      if (numP == 2)
        break;
    }
  }  

  return numP;
}



static inline bool HandleCoplanar (const csPlane3& plane, 
                                   const csVector3 tri1[3], 
                                   const csVector3 tri2[2], 
                                   csSegment3& isectline)
{
  // All tests here run until we have two points
  csVector3 points[3];
  size_t numPoints = 0;

  // All computations are done in 2d projected system, get the two projection
  // axes to use
  size_t maxNormal = plane.norm.DominantAxis ();
  size_t k = CS::Math::NextModulo3 (maxNormal);
  size_t l = CS::Math::NextModulo3 (k);

  // Check verts against tris 
  for (size_t tidx = 0; tidx < 1; ++tidx)
  {
    numPoints += HandlePointsInTriangle ( (tidx ? tri1 : tri2),
      (tidx ? tri2 : tri1), k, l, points[0+numPoints], points[1+numPoints]);

    if (numPoints == 2)
    {
      isectline.Start () = points[0];
      isectline.End () = points[1];
      return true;
    }
  }
  

  // Check edges pairwise until we get our two points
  for (size_t oe = 0; oe < 3; ++oe)
  {
    size_t oe2 = CS::Math::NextModulo3 (oe);

    csVector2 A;

    A.x = tri1[oe2][k] - tri1[oe][k];
    A.y = tri1[oe2][l] - tri1[oe][l];
    for (size_t ie = 0; ie < 3; ++ie)
    {
      // Test edge [oe - oe+1] against [ie - ie+1]
      size_t ie2 = CS::Math::NextModulo3 (ie);

      csVector2 B, C;
      B.x = tri2[ie][k] - tri2[ie2][k];
      B.y = tri2[ie][l] - tri2[ie2][l];

      C.x = tri1[oe][k] - tri2[oe2][k];
      C.y = tri1[oe][l] - tri2[oe2][l];
     
      float f = A.y*B.x - A.x*B.y;
      float d = B.y*C.x - B.x*C.y;
      if ((f>0 && d>=0 && d<=f) ||
          (f<0 && d<=0 && d>=f))
      {
        float e = A.x*C.y - A.y*C.x;
        if ((f>0 && e>=0 && e<=f) ||
            (e<=0 && e>=f))
        {
          // Have intersection, compute point
          float t = d/f;
          csVector3 p = tri2[ie2] + t*(tri2[ie]-tri2[ie2]);
          points[numPoints] = p;
          numPoints++;

          if (numPoints == 2)
          {
            isectline.Start () = points[0];
            isectline.End () = points[1];
            return true;
          }
        }
        
      }
    }
  }

  if (numPoints == 1)
  {
    isectline.End () = isectline.Start () = points[0];
    return true;
  }

  return false;
}


/**
* Tests if two triangles intersect and compute the line of intersection
* (if they are not coplanar).
*
* @param tri1 Vertices of triangle 1
* @param tri2 Vertices of triangle 2
* @param[out] isectline The line segment where they intersect
* @param[out] coplanar Returns whether the triangles are coplanar
* @return true if the triangles intersect, otherwise false
*
*/

bool csIntersect3::TriangleTriangle(const csVector3 tri1[3],
                                    const csVector3 tri2[3],
                                    csSegment3& isectline, bool& coplanar)
{
  // Indexing.. _1, _u refers to triangle 1, _2, _v to triangle 2

  // compute plane equation of triangle(tri1)
  csPlane3 plane_1 (tri1[0], tri1[1], tri1[2]);

  // Compute from tri2 vertices to plane
  float d_2[3] = {plane_1.Classify (tri2[0]),
                  plane_1.Classify (tri2[1]), 
                  plane_1.Classify (tri2[2])}; 

  // Robustness check
  for (unsigned int i = 0; i < 3; ++i)
  {
    if (fabsf(d_2[i]) < SMALL_EPSILON) d_2[i] = 0.0f;
  }
  
  // Classify the signs
  SIGN_CLASS signClass_2 = GetSignClassification (d_2[0], d_2[1], d_2[2]);
  
  // All on same side, not equal to 0, no intersection
  if (signClass_2 == D_ALL_SAME)
    return false;

  
  // Redo computations for tri2

  // compute plane equation of triangle(tri2)
  csPlane3 plane_2 (tri2[0], tri2[1], tri2[2]);

  // Compute from tri2 vertices to plane
  float d_1[3] = {plane_2.Classify (tri1[0]),
                  plane_2.Classify (tri1[1]),
                  plane_2.Classify (tri1[2])};

  // Robustness check
  for (unsigned int i = 0; i < 3; ++i)
  {
    if (fabsf(d_1[i]) < SMALL_EPSILON) d_1[i] = 0.0f;
  }

  // Classify the signs
  SIGN_CLASS signClass_1 = GetSignClassification (d_1[0], d_1[1], d_1[2]);

  // All on same side, not equal to 0, no intersection
  if (signClass_1 == D_ALL_SAME)
    return false;

  //Check if they are coplanar (within epsilon above)
  if (signClass_2 == D_COPLANAR)
  {
    // Handle coplanar case
    coplanar = true;
    return HandleCoplanar (plane_1, tri1, tri2, isectline);
  }

  // Not coplanar, handle normal case

  // Not both on one side, compute direction of the intersection line
  csVector3 D = plane_1.norm % plane_2.norm;

  // Get largest component of D, which will be the axis we project onto
  // for computations
  size_t largestD = D.DominantAxis ();

  // Compute interval for triangle 1
  csSegment3 segment_1;
  float t_1[2];

  ComputeIntervals (tri1, d_1, signClass_1, largestD, segment_1, t_1[0], t_1[1]);

  // Compute interval for triangle 2
  csSegment3 segment_2;
  float t_2[2];

  ComputeIntervals (tri2, d_2, signClass_2, largestD, segment_2, t_2[0], t_2[1]);

  // Now, check if the intervals overlap
  if (t_1[1] < t_2[0] ||
      t_2[1] < t_1[0]) //if either end-point have higher value than the other start
    return false; //no intersection

  // Intersection line goes from the highest of t_1[0] and t_2[0] to lowest of
  // t_1[1] and t_2[1];  reuse some data-slots
  csSort (t_1[0], t_2[0], segment_1.Start (), segment_2.Start ());
  csSort (t_1[1], t_2[1], segment_1.End (), segment_2.End ());

  isectline.Set (segment_2.Start (), segment_1.End ());

  return true;
}
