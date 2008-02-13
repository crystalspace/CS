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
      persist.svKeeper.Empty();
      
      for (size_t i = 0; i < node->meshes.GetSize (); ++i)
      {
        typename RenderTree::MeshNode::SingleMesh& mesh = node->meshes[i];
        
	for (size_t layer = 0; layer < layerConfig.GetLayerCount (); ++layer)
        {
          const size_t numRelevantLights = layerConfig.GetMaxLights (layer);
          if (numRelevantLights == 0) continue;
          
          persist.ReserveInfluences (numRelevantLights);
          const size_t numLights = lightmgr->GetRelevantLights (node->owner.sector,
            mesh.bbox, persist.influences, numRelevantLights);
	
	  csShaderVariableStack localStack;
	  svArrays.SetupSVStack (localStack, layer, mesh.contextLocalId);

          csRef<csShaderVariable> lightNum = persist.svAlloc.Alloc();
          lightNum->SetName (persist.svNames.GetDefaultSVId (
              csLightShaderVarCache::varLightCount));
          lightNum->SetValue ((int)numLights);
          localStack[lightNum->GetName()] = lightNum;
          persist.svKeeper.Push (lightNum);

	  for (size_t l = numLights; l-- > 0; )
	  {
            const csRefArray<csShaderVariable>** thisLightSVs =
              persist.lightDataCache.GetElementPointer (persist.influences[l].light);
            if (thisLightSVs == 0)
            {
              thisLightSVs = &persist.lightDataCache.Put (
                persist.influences[l].light, 
                &(persist.influences[l].light->GetSVContext()->GetShaderVariables()));
            }

            MergeAsArrayItems (localStack, **thisLightSVs, l);
	  }
	}
      }
    }
    
    struct PersistentData
    {
      csLightInfluence* influences;
      size_t numInfluences;
      csLightShaderVarCache svNames;
      csHash<const csRefArray<csShaderVariable>*, csPtrKey<iLight> > lightDataCache;
      csShaderVarBlockAlloc<> svAlloc;
      /* A number of SVs have to be kept alive even past the expiration
       * of the actual step */
      csRefArray<csShaderVariable> svKeeper;
      
      PersistentData () : influences (0), numInfluences (0) {}
      ~PersistentData()
      {
        delete[] influences; 
        if (lcb.IsValid()) lcb->parent = 0;
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

      iLightCallback* GetLightCallback()
      {
        if (!lcb.IsValid()) lcb.AttachNew (new LightCallback (this));
        return lcb;
      }
    protected:
      class LightCallback : public scfImplementation1<LightCallback, 
                                                      iLightCallback>
      {
      public:
        PersistentData* parent;

        LightCallback (PersistentData* parent)
          : scfImplementation1<LightCallback, iLightCallback> (this),
            parent (parent) {}

	void OnColorChange (iLight* light, const csColor& newcolor) { }
	void OnPositionChange (iLight* light, const csVector3& newpos) { }
	void OnSectorChange (iLight* light, iSector* newsector) { }
	void OnRadiusChange (iLight* light, float newradius) { }
	void OnDestroy (iLight* light)
        {
          if (parent != 0)
          {
            parent->lightDataCache.DeleteAll (light);
          }
        }
	void OnAttenuationChange (iLight* light, int newatt) { }
      };
      csRef<LightCallback> lcb;
    };
  private:
    PersistentData& persist;
    iLightManager* lightmgr;
    SVArrayHolder& svArrays; 
    const LayerConfigType& layerConfig;
    
    csShaderVariable* GetNewSV (csShaderVariableStack& stack, CS::ShaderVarStringID name)
    {
      //if ((name < stack.GetSize()) && stack[name]) return stack[name];

      csRef<csShaderVariable> sv (persist.svAlloc.Alloc());
      sv->SetName (name);
      persist.svKeeper.Push (sv);
      if (name < stack.GetSize()) stack[name] = sv;
      return sv;
    }
    
    void MergeAsArrayItems (csShaderVariableStack& dst, 
      const csRefArray<csShaderVariable>& allVars, size_t index)
    {
      for (size_t v = 0; v < allVars.GetSize(); v++)
      {
        csShaderVariable* sv = allVars[v];
        CS::ShaderVarStringID name = sv->GetName();
        
        if (name >= dst.GetSize()) break;
        csShaderVariable*& dstVar = dst[name];

        if (dstVar == 0) dstVar = new csShaderVariable (name);
        if ((dstVar->GetType() != csShaderVariable::UNKNOWN)
          && (dstVar->GetType() != csShaderVariable::ARRAY)) continue;
        dstVar->SetArraySize (csMax (index+1, dstVar->GetArraySize()));
        dstVar->SetArrayElement (index, sv);
      }
    }
  };

}
}

#endif // __CS_CSPLUGINCOMMON_RENDERMANAGER_LIGHTSETUP_H__
