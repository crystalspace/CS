/*
Copyright (C) 2008 by Julian Mautner

This application is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This application is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this application; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef _CLOUDCONTEXT_RM_H_
#define _CLOUDCONTEXT_RM_H_

#include <csutil/scf_implementation.h>
#include <csutil/ref.h>
#include <csutil/objreg.h>
#include <csgeom/polyclip.h>

#include "unshadowed.h"
#include "iengine/rendermanager.h"
#include "imesh/clouds.h"
#include "csplugincommon/rendermanager/renderview.h"

using namespace CS::RenderManager;
using namespace CS::Plugin::RMUnshadowed;

class csCloudsContexts
{
private:
  iObjectRegistry*              m_pObjectReg;
  RMUnshadowed*                 m_pRenderManager;
  const MultipleRenderLayer&    m_LayerConfig;
  RenderTree<>::PersistentData  m_TreePersistent;

public:
  csCloudsContexts(iObjectRegistry* pObjectReg, RMUnshadowed* pRenderManager, const MultipleRenderLayer& LayerConfig,
                   RenderTree<>::PersistentData TreePersistent)
    : m_LayerConfig(LayerConfig), m_pObjectReg(pObjectReg), m_pRenderManager(pRenderManager),
      m_TreePersistent(TreePersistent)
  {
  }
  ~csCloudsContexts()
  {
  }

  const bool SetupAllContexts(CS::RenderManager::RenderTree<>& RenderTree,
                              CS::RenderManager::RenderView& RView)
  {
    //First get the instance of the cloudsystem (if there is one)
    csRef<iCloudSystem> pCloudSystem = csQueryRegistry<iCloudSystem>(m_pObjectReg);
    //If there is none, abort
    if(!pCloudSystem.IsValid()) return false;

    //Iterate over all registred clouds
    for(UINT i = 0; i < pCloudSystem->GetCloudCount(); ++i)
    {
      csRef<iClouds> pCurrentCloud = pCloudSystem->GetCloud(i);
      /**
      Modify RenderView such that View Frustum is ok.
      */
      csRef<CS::RenderManager::RenderView> pNewRView;
      //pNewRView.AttachNew(new CS::RenderManager::RenderView());
#include "csutil/custom_new_disable.h"
      pNewRView.AttachNew(new (m_TreePersistent.renderViewPool) CS::RenderManager::RenderView());
#include "csutil/custom_new_enable.h"
      //Camera and Projectionmatrix
      csRef<iCustomMatrixCamera> pCamera = RView.GetEngine()->CreateCustomMatrixCamera();
      pCamera->SetProjectionMatrix(pCurrentCloud->GetOLVProjectionMatrix());
      pCamera->GetCamera()->SetTransform(pCurrentCloud->GetOLVCameraMatrix());
      pNewRView->SetCamera(pCamera->GetCamera());
      //Clipper
      csRef<iClipper2D> pBoxClipper;
      pBoxClipper.AttachNew(new csBoxClipper(0, 0, pCurrentCloud->GetOLVWidth(), pCurrentCloud->GetOLVHeight()));
      pNewRView->SetClipper(pBoxClipper);

      //================================================//

      for(UINT j = 0; j < pCurrentCloud->GetOLVSliceCount(); ++j)
      {
        iShaderManager* pShaderManager = m_pRenderManager->shaderManager;
        /**
        Create Context on RenderView for each of this slices.
        Set one slice of the OLV 3D-Texture as Rendertarget
        then setup shader and shaderSVs
        */
        CS::RenderManager::RenderTree<>::ContextNode* pContext = RenderTree.CreateContext(pNewRView);

        //================================================//

        //Setup ShaderSVs
        pContext->svArrays.Setup(m_LayerConfig.GetLayerCount(), 
                                 pShaderManager->GetSVNameStringset()->GetSize(),
                                 pContext->totalRenderMeshes);
        csShaderVariableStack& svStack = pShaderManager->GetShaderVariableStack();

        pContext->svArrays.SetupSVStack(svStack, 0, 0);

        pShaderManager->PushVariables(svStack);
        if(pContext->shadervars.IsValid()) pContext->shadervars->PushVariables(svStack);

        //Replicate
        pContext->svArrays.ReplicateSet(0, 0, 1);
        pContext->svArrays.ReplicateLayerZero();

        //================================================//

        //Setup Shader
        SetupStandardShader(*pContext, pShaderManager, m_LayerConfig);

        //================================================//

        //Set a 3D-Texture slice as rendertarget
        pContext->renderTargets[rtaColor0].texHandle  = pCurrentCloud->GetOLVTexture();
        pContext->renderTargets[rtaColor0].subtexture = j;
      }
    }

    return true;
  }
};

#endif // _CLOUDCONTEXT_RM_H_