/*
    Copyright (C) 2009 by Jorrit Tyberghein

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

#ifndef __CS_MATERIALBUILDER_H__
#define __CS_MATERIALBUILDER_H__

/**\file
 * Material builder.
 */

#include "csextern.h"

struct iMaterialWrapper;
struct iTextureHandle;
struct iObjectRegistry;
class csVector4;
class csColor;

namespace CS
{
namespace Material
{

/**
 * Tools related to creating materials.
 */
class CS_CRYSTALSPACE_EXPORT MaterialBuilder
{
public:
  /**
   * Set up an already loaded material for parallax mapping with
   * previously loaded normalmap and heightmap textures.
   */
  static void SetupParallaxMaterial (iObjectRegistry* object_reg,
      iMaterialWrapper* material,
      iTextureHandle* normalmap, iTextureHandle* heightmap,
      const csVector4& specular);

  /**
   * Load a material, a normalmap texture, and a heightmap texture and
   * setup a parallax material from that. Returns 0 if the material could
   * not be created.
   * If there is already a material in the engine with the given 'matname'
   * then the material will not be loaded again.
   */
  static iMaterialWrapper* CreateParallaxMaterial (iObjectRegistry* object_reg,
      const char* matname, const char* matfile, const char* normalfile,
      const char* heightfile, const csVector4& specular);

  /**
   * Create a material from a color. The material will be composed of a single
   * texture with size 1x1 pixel of the given color.
   * If there is already a material in the engine with the given 'matname'
   * then the material will not be loaded again.
   */
  static iMaterialWrapper* CreateColorMaterial(iObjectRegistry* object_reg,
      const char* matname, csColor color);

};
} // namespace Material
} // namespace CS

/** @} */

#endif // __CS_MATERIALBUILDER_H__

