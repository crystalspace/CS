/*
    Copyright (C) 2007-2008 by Scott Johnson

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

    typedef csDirtyAccessArray< size_t > csVertexSet;
    
/** @class csEarClipper A data structure for clipping polygon ears
  *
  * A polygon ear is defined as three consecutive ears of a polygon, for
  * which no other vertices are within the triangle formed by these three
  * vertices.  A ear is clipped by connecting the two end vertices (on either
  * side of the ear vertex), and removing the ear vertex from the vertices
  * left to be clipped.  Ear clipping is used to triangulate polygons.
  *
  * @note This ear clipper requires a planar polygon that is located in the 
  * x-y plane.  If this is not the case, you will need to use the 3D
  * triangulator class to map the polygon to a planar polygon and then 
  * rotate it to coincide with the x-y plane.
  */

    class CS_CRYSTALSPACE_EXPORT csEarClipper
    {
      private:

        /// The polygon we are currently in the process of clipping ears from
        csContour3 clipPoly;

        /// The original indices of the vertices in the polygon
        csDirtyAccessArray<size_t> originalIndices;

        /// An array indicating whether the specified vertex is reflex or convex
        csDirtyAccessArray<bool> isVertexReflex;

        /// An array indicating which vertices are ears
        csDirtyAccessArray<size_t> ears;

        /// A function to classify polygon vertices as either convex or reflex
        void ClassifyVertices();

        /// A function which determines if a specified vertex is convex
        bool IsConvex(int x);

      public:

        /** @brief Constructor
          *
          * @param polygon A 2D planar polygon within the x-y plane.  See the note at the 
          * top of the class description for more information.
          */
        csEarClipper(csContour3 polygon);
        ~csEarClipper() {}

        /// Returns whether the ear clipper is finished or not
        bool IsFinished();

        /// Returns the index of the vertex clipped
        csVertexSet ClipEar();

        /// Returns the original index of a vertex
        size_t GetOriginalIndex(size_t at) { return originalIndices[at]; }

    }; // End class csEarClipper

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


