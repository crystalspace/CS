/*
  Copyright (C) 2011 Alexandru - Teodor Voicu
      Imperial College London
      http://www3.imperial.ac.uk/

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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_SHADOW_OSM_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_SHADOW_OSM_H__

namespace CS
{
namespace RenderManager
{
  template<typename RenderTree, typename LayerConfigType>
  class ShadowOSM
  {
  public:
    struct PersistentData
    {
      void Initialize (iObjectRegistry* objectReg,
        RenderTreeBase::DebugPersistent& dbgPersist)
      {
      }
      
      void UpdateNewFrame ()
      {
      }
    };

    class ViewSetup
    {
    public:
      ViewSetup (PersistentData& persist, CS::RenderManager::RenderView* rview)
      {
        
      }
    };

    struct CachedLightData
    {
      void SetupFrame (RenderTree& tree, ShadowOSM& shadows, iLight* light){}

      uint GetSublightNum() const { return (uint)1; }

      void ClearFrameData(){}
    };

    typedef ViewSetup ShadowParameters;

    ShadowOSM (PersistentData& persist,
      const LayerConfigType& layerConfig,
      typename RenderTree::MeshNode* node, 
      ViewSetup& viewSetup) 
    {
    }

    csFlags GetLightFlagsMask () const { return csFlags (0); }

    static bool NeedFinalHandleLight() { return false; }

    void FinalHandleLight (iLight* light, CachedLightData& lightData) {}

    size_t GetLightLayerSpread() const { return 1; }

    uint HandleOneLight (typename RenderTree::MeshNode::SingleMesh& singleMesh,
      iLight* light, CachedLightData& lightData,
      csShaderVariableStack* lightStacks,
      uint lightNum, uint subLightNum) { return 1; }
  };
}
}

#endif // __CS_CSPLUGINCOMMON_RENDERMANAGER_SHADOW_OSM_H__