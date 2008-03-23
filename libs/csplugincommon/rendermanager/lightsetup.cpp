/*
    Copyright (C) 2008 by Frank Richter

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

#include "cssysdef.h"

#include "csplugincommon/rendermanager/lightsetup.h"

namespace CS
{
namespace RenderManager
{
  LightingSorter::LightingSorter (PersistentData& persist,
				  csLightInfluence* influenceLights, 
                                  size_t numLights)
    : persist (persist)
  {
      // Sort lights by type
    persist.lightTypeScratch.Empty();
    for (size_t l = 0; l < numLights; l++)
    {
      IndexLightTypePair iltp;
      iltp.light = influenceLights[l].light;
      iltp.type = influenceLights[l].light->GetType();
      iltp.isStatic =
        influenceLights[l].light->GetDynamicType() != CS_LIGHT_DYNAMICTYPE_DYNAMIC;
      persist.lightTypeScratch.Push (iltp);
    }
    persist.lightTypeScratch.Sort();
  }
  
  size_t LightingSorter::RemoveStatic ()
  {
    for (size_t l = 0; l < persist.lightTypeScratch.GetSize();)
    {
      if (!persist.lightTypeScratch[l].isStatic)
        l++;
      else
        persist.lightTypeScratch.DeleteIndex (l);
    }
    return persist.lightTypeScratch.GetSize();
  }
  
  //-------------------------------------------------------------------------
  
  bool LightingVariablesHelper::MergeAsArrayItem (csShaderVariableStack& dst, 
			                          csShaderVariable* sv, 
			                          size_t index)
  {
    CS::ShaderVarStringID name = sv->GetName();
    
    if (name >= dst.GetSize()) return false;
    csShaderVariable*& dstVar = dst[name];

    if (dstVar == 0) dstVar = CreateTempSV (name);
    if ((dstVar->GetType() != csShaderVariable::UNKNOWN)
	  && (dstVar->GetType() != csShaderVariable::ARRAY)) return true;
    dstVar->SetArraySize (csMax (index+1, dstVar->GetArraySize()));
    dstVar->SetArrayElement (index, sv);
    return true;
  }

  void LightingVariablesHelper::MergeAsArrayItems (csShaderVariableStack& dst,
    const csRefArray<csShaderVariable>& allVars, size_t index)
  {
    for (size_t v = 0; v < allVars.GetSize(); v++)
    {
      if (!MergeAsArrayItem (dst, allVars[v], index)) break;
    }
  }

  csShaderVariable* LightingVariablesHelper::CreateTempSV (
    CS::ShaderVarStringID name)
  {
    csRef<csShaderVariable> var = persist.svAlloc.Alloc();
    var->SetName (name);
    persist.svKeeper.Push (var);
    return var;
  }

  csShaderVariable* LightingVariablesHelper::CreateVarOnStack (CS::ShaderVarStringID name,
    csShaderVariableStack& stack)
  {
    csShaderVariable* var = CreateTempSV (name);
    stack[name] = var;
    return var;
  }
  
}
}
