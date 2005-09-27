#ifndef __CS_TRIANGULATE_POLY_H__
#define __CS_TRIANGULATE_POLY_H__

#include "csgeom/poly2d.h"
#include "csgeom/trimesh.h"
#include "csutil/dirtyaccessarray.h"

/*****************************************************************/
/** Static class to triangulate any contour/polygon efficiently **/
/** You should replace Vector2d with whatever your own Vector   **/
/** class might be.  Does not support polygons with holes.      **/
/** Uses STL vectors to represent a dynamic array of vertices.  **/
/** This code snippet was submitted to FlipCode.com by          **/
/** John W. Ratcliff (jratcliff@verant.com) on July 22, 2000    **/
/** I did not write the original code/algorithm for this        **/
/** this triangulator, in fact, I can't even remember where I   **/
/** found it in the first place.  However, I did rework it into **/
/** the following black-box static class so you can make easy   **/
/** use of it in your own code.  Simply replace Vector2d with   **/
/** whatever your own Vector implementation might be.           **/
/*****************************************************************/

// Typedef a vector of vertices which are used to represent
// a polygon/contour.
typedef csDirtyAccessArray< csVector2 > csContour2;

/** This triangulates a simple polygon.  It does not handle holes, but it is fast and efficient. */
class csTriangulate2
{
public:

  /** Triangulate a contour/polygon, places results in a triangle mesh, with the resulting vertices in result_vertices. */
  static bool Process(const csContour2 &contour,
                      csTriangleMesh &result, csContour2 &result_vertices);

  /** Compute area of a contour/polygon. */
  static float Area(const csContour2 &contour);

  /** Decide if point Px/Py is inside triangle defined by
   * (Ax,Ay) (Bx,By) (Cx,Cy) */
  static bool InsideTriangle(float Ax, float Ay,
                      float Bx, float By,
                      float Cx, float Cy,
                      float Px, float Py);


private:
  static bool Snip(const csContour2 &contour,int u,int v,int w,int n,int *V);

};

#endif

