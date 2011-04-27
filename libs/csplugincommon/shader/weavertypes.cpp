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

#include "cssysdef.h"

#include "csplugincommon/shader/weavertypes.h"

namespace CS
{
  namespace PluginCommon
  {
    namespace ShaderWeaver
    {
      namespace
      {
        struct TypeInfoMap
        {
          const char* typeName;
          TypeInfo typeInfo;
        };
        static const TypeInfoMap typeInfoMap[] = {
          {"bool", 
            {TypeInfo::VectorB, false, 1, TypeInfo::NoSemantics, TypeInfo::NoSpace, false}},
          {"depth", 
            {TypeInfo::Vector,  false, 1, TypeInfo::Depth,       TypeInfo::NoSpace, false}},
          {"direction",
            {TypeInfo::Vector,  false, 3, TypeInfo::Direction,   TypeInfo::NoSpace, false}},
          {"direction_camera",
            {TypeInfo::Vector,  false, 3, TypeInfo::Direction,   TypeInfo::Camera,  false}},
          {"direction_object",
            {TypeInfo::Vector,  false, 3, TypeInfo::Direction,   TypeInfo::Object,  false}},
          {"direction_screen",
            {TypeInfo::Vector,  false, 3, TypeInfo::Direction,   TypeInfo::Screen,  false}},
          {"direction_tangent",
            {TypeInfo::Vector,  false, 3, TypeInfo::Direction,   TypeInfo::Tangent, false}},
          {"direction_world",
            {TypeInfo::Vector,  false, 3, TypeInfo::Direction,   TypeInfo::World,   false}},
          {"float", 
            {TypeInfo::Vector,  false, 1, TypeInfo::NoSemantics, TypeInfo::NoSpace, false}},
          {"float2", 
            {TypeInfo::Vector,  false, 2, TypeInfo::NoSemantics, TypeInfo::NoSpace, false}},
          {"float3", 
            {TypeInfo::Vector,  false, 3, TypeInfo::NoSemantics, TypeInfo::NoSpace, false}},
          {"float4", 
            {TypeInfo::Vector,  false, 4, TypeInfo::NoSemantics, TypeInfo::NoSpace, false}},
          {"int", 
            {TypeInfo::VectorI, false, 1, TypeInfo::NoSemantics, TypeInfo::NoSpace, false}},
          {"normal",
            {TypeInfo::Vector,  false, 3, TypeInfo::Direction,   TypeInfo::NoSpace, true}},
          {"normal_camera",
            {TypeInfo::Vector,  false, 3, TypeInfo::Direction,   TypeInfo::Camera,  true}},
          {"normal_object",
            {TypeInfo::Vector,  false, 3, TypeInfo::Direction,   TypeInfo::Object,  true}},
          {"normal_screen",
            {TypeInfo::Vector,  false, 3, TypeInfo::Direction,   TypeInfo::Screen,  true}},
          {"normal_tangent",
            {TypeInfo::Vector,  false, 3, TypeInfo::Direction,   TypeInfo::Tangent, true}},
          {"normal_world",
            {TypeInfo::Vector,  false, 3, TypeInfo::Direction,   TypeInfo::World,   true}},
          {"position4",
            {TypeInfo::Vector,  false, 4, TypeInfo::Position,    TypeInfo::NoSpace, false}},
          {"position4_camera",
            {TypeInfo::Vector,  false, 4, TypeInfo::Position,    TypeInfo::Camera,  false}},
          {"position4_object",
            {TypeInfo::Vector,  false, 4, TypeInfo::Position,    TypeInfo::Object,  false}},
          {"position4_screen",
            {TypeInfo::Vector,  false, 4, TypeInfo::Position,    TypeInfo::Screen,  false}},
          {"position4_world",
            {TypeInfo::Vector,  false, 4, TypeInfo::Position,    TypeInfo::World,   false}},
          {"rgb",
            {TypeInfo::Vector,  false, 3, TypeInfo::Color,       TypeInfo::NoSpace, false}},
          {"rgba",
            {TypeInfo::Vector,  false, 4, TypeInfo::Color,       TypeInfo::NoSpace, false}},
          {"scale2", 
            {TypeInfo::Vector,  false, 2, TypeInfo::Scale,       TypeInfo::NoSpace, false}},
          {"tex2d", 
            {TypeInfo::Sampler, false, 2, TypeInfo::NoSemantics, TypeInfo::NoSpace, false}},
          {"texcoord2", 
            {TypeInfo::Vector,  false, 2, TypeInfo::Texcoord,    TypeInfo::NoSpace, false}},
          {"texcoord3", 
            {TypeInfo::Vector,  false, 3, TypeInfo::Texcoord,    TypeInfo::NoSpace, false}},
          {"texcoord4", 
            {TypeInfo::Vector,  false, 4, TypeInfo::Texcoord,    TypeInfo::NoSpace, false}},
          {"texcube", 
            {TypeInfo::Sampler, true,  3, TypeInfo::NoSemantics, TypeInfo::NoSpace, false}},
          {"translateScale2", 
            {TypeInfo::Vector,  false, 4, TypeInfo::TranslateScale, TypeInfo::NoSpace, false}},
	};
	static const size_t numTypeInfos = 
	  sizeof (typeInfoMap) / sizeof (typeInfoMap[0]);
      }
      
      const TypeInfo* QueryTypeInfo (const char* type)
      {
	size_t l = 0, r = numTypeInfos;
	while (l < r)
	{
	  size_t m = (l+r) / 2;
	  int i = strcmp (typeInfoMap[m].typeName, type);
	  if (i == 0) return &typeInfoMap[m].typeInfo;
	  if (i < 0)
	    l = m + 1;
	  else
	    r = m;
	}
        return 0;
      }

      const char* QueryType (const TypeInfo& typeInfo)
      {
        for (size_t i = 0; i < numTypeInfos; i++)
        {
          if (typeInfoMap[i].typeInfo == typeInfo) 
            return typeInfoMap[i].typeName;
        }
        return 0;
      }
    
      TypeInfoIterator::TypeInfoIterator () : n(0) {}
      
      bool TypeInfoIterator::HasNext ()
      {
        return n < numTypeInfos;
      }
      
      const TypeInfo* TypeInfoIterator::Next (csString& type)
      {
        const TypeInfo* ret = &typeInfoMap[n].typeInfo;
        type = typeInfoMap[n].typeName;
        n++;
        return ret;
      }
    
    } // namespace ShaderWeaver
  } // namespace PluginCommon
} // namespace CS
