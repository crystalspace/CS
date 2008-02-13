/*
    Copyright (C) 2007-2008 by Marten Svanfeldt

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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_LIGHTSETUP_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_LIGHTSETUP_H__

#include "ivideo/shader/shader.h"

#include "csplugincommon/rendermanager/operations.h"
#include "csplugincommon/rendermanager/rendertree.h"

class csShaderVariable;

namespace CS
{
namespace RenderManager
{

  /**
   * For each mesh determine the array of affecting lights and generate shader
   * vars for it.
   * Should be done after StandardSVSetup.
   */
  template<typename RenderTree, typename LayerConfigType>
  class LightSetup
  {
  public:
    struct PersistentData;
    typedef csArray<iShader*> ShaderArrayType;

    LightSetup (PersistentData& persist, iLightManager* lightmgr,
      SVArrayHolder& svArrays, const LayerConfigType& layerConfig)
      : persist (persist), lightmgr (lightmgr), svArrays (svArrays),
        layerConfig (layerConfig)
    {
    }

    void operator() (typename RenderTree::MeshNode* node)
    {
      csReversibleTransform& camTransR = node->owner.cameraTransform;
      
      for (size_t i = 0; i < node->meshes.GetSize (); ++i)
      {
        typename RenderTree::MeshNode::SingleMesh& mesh = node->meshes[i];
        csReversibleTransform objT;
        mesh.svObjectToWorld->GetValue (objT);
        
	for (size_t layer = 0; layer < layerConfig.GetLayerCount (); ++layer)
        {
          const size_t numRelevantLights = layerConfig.GetMaxLights (layer);
          if (numRelevantLights == 0) continue;
          
          persist.ReserveInfluences (numRelevantLights);
          const size_t numLights = lightmgr->GetRelevantLights (node->owner.sector,
            mesh.bbox, persist.influences, numRelevantLights);
	
	  csShaderVariableStack lightSVs;
	  lightSVs.Setup (svArrays.GetNumSVNames ());
          csShaderVariable* lightNum =
            new csShaderVariable (persist.svNames.GetDefaultSVId (
              csLightShaderVarCache::varLightCount));
          lightNum->SetValue ((int)numLights);
          lightSVs[lightNum->GetName()] = lightNum;

	  for (size_t l = numLights; l-- > 0; )
	  {
            csShaderVariableStack thisLightSVs;
	    thisLightSVs.Setup (svArrays.GetNumSVNames ());
	    SetLightSVs (thisLightSVs, persist.influences[l].light, camTransR, objT);
            MergeAsArrayItems (lightSVs, thisLightSVs, l);
	  }
	  csShaderVariableStack localStack;
	  svArrays.SetupSVStack (localStack, layer, mesh.contextLocalId);
	  localStack.MergeBack (lightSVs);
	}
      }
    }
    
    struct PersistentData
    {
      csLightInfluence* influences;
      size_t numInfluences;
      csLightShaderVarCache svNames;
      
      PersistentData () : influences (0), numInfluences (0) {}
      ~PersistentData() { delete[] influences; }
      
      void Initialize (iShaderVarStringSet* strings)
      {
	svNames.SetStrings (strings);

        /* Generate light SV IDs - that way, space for them will be reserved
         * when shader stacks are set up */
	for (int p = 0; p < csLightShaderVarCache::_lightCount; p++)
	{
	  svNames.GetLightSVId (csLightShaderVarCache::LightProperty (p));
	}
      }
      
      void ReserveInfluences (size_t num)
      {
        if (num > numInfluences)
        {
          delete[] influences;
          influences = new csLightInfluence[num];
          numInfluences = num;
          // @@@ Shrink later?
        }
      }
    };
  private:
    PersistentData& persist;
    iLightManager* lightmgr;
    SVArrayHolder& svArrays; 
    const LayerConfigType& layerConfig;
    csRefArray<csShaderVariable> svKeeper;
    
    csShaderVariable* GetNewSV (csShaderVariableStack& stack, CS::ShaderVarStringID name)
    {
      csShaderVariable* sv = new csShaderVariable (name);
      svKeeper.Push (sv);
      if (name < stack.GetSize()) stack[name] = sv;
      return sv;
    }
    
    void SetLightSVs (csShaderVariableStack& stack, 
		      iLight* light, 
		      const csReversibleTransform& camTransR,
		      const csReversibleTransform &objT)
    {
      csLightShaderVarCache& lsvCache = persist.svNames;
    
      // @@@ FIXME: probably use arrays for light SVs
      // Light TF, this == object, other == world
      const csReversibleTransform& lightT = 
	light->GetMovable ()->GetFullTransform ();
    
      csRef<csShaderVariable> sv;
      sv = GetNewSV (stack, lsvCache.GetLightSVId (
	 csLightShaderVarCache::lightDiffuse));
      const csColor& color = light->GetColor ();
      sv->SetValue (csVector3 (color.red, color.green, color.blue));
    
      sv = GetNewSV (stack, lsvCache.GetLightSVId (
	  csLightShaderVarCache::lightSpecular));
      const csColor& specular = light->GetSpecularColor ();
      sv->SetValue (csVector3 (specular.red, specular.green, specular.blue));
    
    
      const csVector3& lightPosW = lightT.GetOrigin();
      sv = GetNewSV (stack, lsvCache.GetLightSVId (
	  csLightShaderVarCache::lightPositionCamera));
      sv->SetValue (lightPosW * camTransR);
    
      sv = GetNewSV (stack, lsvCache.GetLightSVId (
	  csLightShaderVarCache::lightPosition));
      sv->SetValue (objT.Other2This (lightPosW));
    
      sv = GetNewSV (stack, lsvCache.GetLightSVId (
	  csLightShaderVarCache::lightPositionWorld));
      sv->SetValue (lightPosW);
    
    
      //const csVector3 lightDirW = 
	//lightT.This2OtherRelative (light->GetDirection ());
      // @@@ Jorrit: check the following! Untested!
      const csVector3 lightDirW = 
	lightT.This2OtherRelative (csVector3 (0, 0, 1));
    
      if (!lightDirW.IsZero())
      {
	sv = GetNewSV (stack, lsvCache.GetLightSVId (
	    csLightShaderVarCache::lightDirectionWorld));
	sv->SetValue (lightDirW.Unit());
    
	sv = GetNewSV (stack, lsvCache.GetLightSVId (
	    csLightShaderVarCache::lightDirection));
	sv->SetValue (objT.Other2ThisRelative (lightDirW).Unit());
    
	sv = GetNewSV (stack, lsvCache.GetLightSVId (
	    csLightShaderVarCache::lightDirectionCamera));
	sv->SetValue (camTransR.Other2ThisRelative (lightDirW).Unit());
      }
    
    
      float falloffInner, falloffOuter;
      light->GetSpotLightFalloff (falloffInner, falloffOuter);
    
      sv = GetNewSV (stack, lsvCache.GetLightSVId (
	  csLightShaderVarCache::lightInnerFalloff));
      sv->SetValue (falloffInner);
    
      sv = GetNewSV (stack, lsvCache.GetLightSVId (
	  csLightShaderVarCache::lightOuterFalloff));
      sv->SetValue (falloffOuter);
    
    
      sv = GetNewSV (stack, lsvCache.GetLightSVId (
	  csLightShaderVarCache::lightAttenuation));
      csLightAttenuationMode attnMode = light->GetAttenuationMode ();
      if (attnMode == CS_ATTN_LINEAR)
      {
	float r = light->GetAttenuationConstants ().x;
	sv->SetValue (csVector3(r, 1/r, 0));
      }
      else
      {
	sv->SetValue (light->GetAttenuationConstants ());
      }
    
      sv = GetNewSV (stack, lsvCache.GetLightSVId (
	  csLightShaderVarCache::lightAttenuationMode));
      sv->SetValue ((int)attnMode);
    
      sv = GetNewSV (stack, lsvCache.GetLightSVId (
	  csLightShaderVarCache::lightType));
      sv->SetValue ((int)light->GetType());
    
      sv = GetNewSV (stack, lsvCache.GetLightSVId (
	  csLightShaderVarCache::lightAttenuationTex));
      //sv->SetAccessor (GetLightAccessor (light));
      /*if (!attTex.IsValid())
	attTex = GetAttenuationTexture (attnMode);
      sv->SetValue (attTex);*/
    }

    void MergeAsArrayItems (csShaderVariableStack& dst, 
      const csShaderVariableStack& src, size_t num)
    {
      for (size_t v = 0; v < src.GetSize(); v++)
      {
        csShaderVariable*& dstVar = dst[v];

        if (dstVar == 0) dstVar = new csShaderVariable (v);
        if ((dstVar->GetType() != csShaderVariable::UNKNOWN)
          && (dstVar->GetType() != csShaderVariable::ARRAY)) continue;
        dstVar->SetArraySize (csMax (num+1, dstVar->GetArraySize()));
        dstVar->SetArrayElement (num, src[v]);
      }
    }
  };

}
}

#endif // __CS_CSPLUGINCOMMON_RENDERMANAGER_LIGHTSETUP_H__
