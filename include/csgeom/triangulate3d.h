/*
    Copyright (C) 2007 by Scott Johnson

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

#include "csutil/dirtyaccessarray.h"
#include "csgeom/trimesh.h"
#include "csgeom/vector3.h"
#include "ivaria/reporter.h"
#include "csgeom/plane3.h"

#ifndef __CS_GEOM_TRIANGULATE_3D_H_
#define __CS_GEOM_TRIANGULATE_3D_H_

namespace CS
{
  namespace Geometry
  {
    typedef csDirtyAccessArray< csVector3 > csContour3;

    /** @class Triangulate3D 3D Triangulation Functions
    * @brief A collection of functions for 3D triangulation.
    * 
    * This only includes functions for planar triangulation of 3D surfaces.
    * That is, it does not triangulate a 3D object into tetrahedra, but
    * finds and triangulates the surfaces of a polygon in 3D.
    */
    class CS_CRYSTALSPACE_EXPORT Triangulate3D 
    {
    public:
      Triangulate3D() {};
      ~Triangulate3D() {};

      /** @brief Triangulate a 3D polygon.
      *
      * Triangulates a 3D polygon into a csTriangleMesh object.  The polygon
      * may contain holes.
      *
      * @returns true on success; false otherwise
      *
      * @param polygon A contour representing a counter-clockwise traversal 
      *                of the polygon's edge.
      * @param result The csTriangleMesh into which the resulting triangulation 
      *               should be placed.
      * @param report2 A reporter to which errors are sent.
      * 
      * @warning This function does not yet work correctly.  Do not use until 
      *			this message is removed.
      */
      static bool Process(csContour3& polygon, csTriangleMesh& result);

    private:

      static csContour3 MapToPlanar(const csContour3& poly, csVector3& normal);
      static bool FindVertexGroups(csContour3& poly, csArray<bool>& isReflex, csArray<size_t>& ears);
      static bool IsConvex(const csContour3& polygon, const int index);
      static bool IsContained(const csVector3& testVertex, const csVector3& a, const csVector3& b, const csVector3& c);
      static bool IsSameSide(const csVector3& p1, const csVector3& p2, const csVector3& a, const csVector3& b);
      //static bool Snip(csContour3& polygon, csArray<size_t>& ears, const size_t earPoint, csTriangleMesh& addTo);

    }; /* End class Triangulate3D */

  } // namespace Geometry
} // namespace CS

#endif // __CS_GEOM_TRIANGULATE_3D_H_


