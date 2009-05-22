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
                                  size_t numLights)
    : persist (persist)
  {
    persist.lightTypeScratch.Empty();
    persist.lightTypeScratch.SetMinimalCapacity (numLights);
    persist.sublightNumMem.Empty();
    persist.putBackLights.Empty();
  }

  void LightingSorter::SetNumLights (size_t numLights)
  {
    persist.lightTypeScratch.SetMinimalCapacity (numLights);
  }
    
  void LightingSorter::AddLight (const csLightInfluence& influence,
                                 uint numSubLights,
                                 const csFlags& lightFlagsMask)
  {
    LightInfo iltp;
    iltp.light = influence.light;
    iltp.settings.type = influence.type;
    iltp.settings.lightFlags = influence.flags & lightFlagsMask;
    iltp.isStatic =
      influence.dynamicType != CS_LIGHT_DYNAMICTYPE_DYNAMIC;
    iltp.numSubLights = numSubLights;
    iltp.subLights =
      (uint*)persist.sublightNumMem.Alloc (numSubLights * sizeof (uint));
    for (uint l = 0; l < numSubLights; l++)
      iltp.subLights[l] = l;
    persist.lightTypeScratch.Push (iltp);
  }
    
  bool LightingSorter::GetNextLight (LightInfo& out)
  {
    csArray<LightInfo>& putBackLights = persist.putBackLights;
    
    size_t i = 0;
    if (i >= lightLimit + putBackLights.GetSize()) return false;
    if (i < putBackLights.GetSize())
    {
      size_t j = putBackLights.GetSize()-1-i;
      out = putBackLights[j];
      putBackLights.DeleteIndex (j);
    }
    else
    {
      out = persist.lightTypeScratch[i];
      persist.lightTypeScratch.DeleteIndex (i);
    }
    lightLimit = csMin (persist.lightTypeScratch.GetSize(), lightLimit);
    return true;
  }
  
  bool LightingSorter::GetNextLight (const LightSettings& settings, 
                                     LightInfo& out)
  {
    csArray<LightInfo>& putBackLights = persist.putBackLights;
    
    size_t i = 0;
    while (i < lightLimit + putBackLights.GetSize())
    {
      if (i < putBackLights.GetSize())
      {
        size_t j = putBackLights.GetSize()-1-i;
	if (putBackLights[j].settings == settings)
	  break;
      }
      else
      {
        size_t j = i-putBackLights.GetSize();
	if (persist.lightTypeScratch[j].settings == settings)
	  break;
      }
      i++;
    }
    if (i >= lightLimit + putBackLights.GetSize()) return false;
    if (i < putBackLights.GetSize())
    {
      size_t j = putBackLights.GetSize()-1-i;
      out = putBackLights[j];
      putBackLights.DeleteIndex (j);
    }
    else
    {
      out = persist.lightTypeScratch[i];
      persist.lightTypeScratch.DeleteIndex (i);
    }
    lightLimit = csMin (persist.lightTypeScratch.GetSize(), lightLimit);
    return true;
  }
  
  void LightingSorter::PutInFront (LightInfo* lights, size_t num)
  {
    csArray<LightInfo>& putBackLights = persist.putBackLights;
    while (num-- > 0)
    {
      putBackLights.Push (lights[num]);
    }
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
