/*
    Copyright (C) 2005-2006 by Jorrit Tyberghein

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

#ifndef __CS_MESHGEN_POSITIONMAP_H__
#define __CS_MESHGEN_POSITIONMAP_H__

#include "csgeom/vector4.h"
#include "csutil/array.h"
#include "csutil/randomgen.h"

class csBox2;

CS_PLUGIN_NAMESPACE_BEGIN(Engine)
{
  /**
   * A map of available positions.
   */
  class PositionMap
  {
  public:
    PositionMap(const csBox2& box);

    /**
     * Get a random available position. 
     * \param xpos X position.
     * \param zpos Z position.
     * \param radius The radius from the (x, z) coordinates to mark off as used.
     * \param minRadius The minimum radius used by all geometries.
     */
    bool GetRandomPosition(float& xpos, float& zpos, float& radius, float& minRadius);

  private:
    csArray<csVector4> freeAreas;
    csRandomGen posGen;
  };
}
CS_PLUGIN_NAMESPACE_END(Engine)

#endif // __CS_MESHGEN_POSITIONMAP_H__
