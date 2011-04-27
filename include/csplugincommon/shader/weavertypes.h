/*
  Copyright (C) 2007 by Frank Richter

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CS_CSPLUGINCOMMON_SHADER_WEAVERTYPES_H__
#define __CS_CSPLUGINCOMMON_SHADER_WEAVERTYPES_H__

#include "csutil/csstring.h"

/**\file
 */

/**\addtogroup plugincommon
 * @{ */

namespace CS
{
  namespace PluginCommon
  {
    namespace ShaderWeaver
    {
      struct TypeInfo
      {
        /// Base type (sampler or vector)
        enum 
        {
          /// Type is a (float) vector type
          Vector, 
          /// Type is a (bool) vector type
          VectorB, 
          /// Type is a (integer) vector type
          VectorI, 
          /// Type is a texture sampler type
          Sampler
        } baseType;
        /// For samplers: whether sampler is a cube map sampler
        bool samplerIsCube;
        /**
         * Dimensions: number of components for a vector or number of texture
         * dimensions for a sampler.
         */
        int dimensions;
        /// Semantics of value
        enum
        {
          /// No special semantics
          NoSemantics,
          /// Color
          Color,
          /// Texture coordinate
          Texcoord,
          /// Position
          Position,
          /// Direction (surface normal, tangent/bitangent, light direction...)
          Direction,
          /// Fragment depth value
          Depth,
          /**
           * Translation+scale values (first components are translation,
           * last components scale)
           */
          TranslateScale,
          /// Scale values
	  Scale
        } semantics;
        /// Space of direction/position
        enum
        {
          /// No special space
          NoSpace,
          /// Object space
          Object,
          /// World space
          World,
          /// Camera space
          Camera,
          /// Screen space (camera and projection)
          Screen,
          /// Tangent space
          Tangent
        } space;
        /// Whether the values should be normalized
        bool unit;

        bool operator==(const TypeInfo& other) const
        {
          return (baseType == other.baseType)
            && (samplerIsCube == other.samplerIsCube)
            && (dimensions == other.dimensions)
            && (semantics == other.semantics)
            && (space == other.space)
            && (unit == other.unit);
        }
      };
      
      CS_CRYSTALSPACE_EXPORT const TypeInfo* QueryTypeInfo (
        const char* type);
      CS_CRYSTALSPACE_EXPORT const char* QueryType (
        const TypeInfo& typeInfo);
        
      class CS_CRYSTALSPACE_EXPORT TypeInfoIterator
      {
        size_t n;
      public:
        TypeInfoIterator ();
        
        bool HasNext ();
        const TypeInfo* Next (csString& type);
      };
    } // namespace ShaderWeaver
  } // namespace PluginCommon
} // namespace CS

/** @} */

#endif // __CS_CSPLUGINCOMMON_SHADER_WEAVERTYPES_H__

