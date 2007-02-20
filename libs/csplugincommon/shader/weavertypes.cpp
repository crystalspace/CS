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
          {"float", 
            {TypeInfo::Vector,  false, 1, TypeInfo::NoSemantics, TypeInfo::NoSpace}},
          {"float2", 
            {TypeInfo::Vector,  false, 2, TypeInfo::NoSemantics, TypeInfo::NoSpace}},
          {"float3", 
            {TypeInfo::Vector,  false, 3, TypeInfo::NoSemantics, TypeInfo::NoSpace}},
          {"float4", 
            {TypeInfo::Vector,  false, 4, TypeInfo::NoSemantics, TypeInfo::NoSpace}},
          {"normal",
            {TypeInfo::Vector,  false, 3, TypeInfo::Normal,      TypeInfo::NoSpace}},
          {"position4",
            {TypeInfo::Vector,  false, 4, TypeInfo::Position,    TypeInfo::NoSpace}},
          {"position4_object",
            {TypeInfo::Vector,  false, 4, TypeInfo::Position,    TypeInfo::Object}},
          {"position4_screen",
            {TypeInfo::Vector,  false, 4, TypeInfo::Position,    TypeInfo::Camera}},
          {"position4_world",
            {TypeInfo::Vector,  false, 4, TypeInfo::Position,    TypeInfo::World}},
          {"rgb",
            {TypeInfo::Vector,  false, 3, TypeInfo::Color,       TypeInfo::NoSpace}},
          {"rgba",
            {TypeInfo::Vector,  false, 4, TypeInfo::Color,       TypeInfo::NoSpace}},
          {"tex2d", 
            {TypeInfo::Sampler, false, 2, TypeInfo::NoSemantics, TypeInfo::NoSpace}},
          {"texcoord2", 
            {TypeInfo::Vector,  false, 2, TypeInfo::Texcoord,    TypeInfo::NoSpace}},
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
