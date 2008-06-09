/*
    Copyright (C) 2007-2008 by Marten Svanfeldt
	      (C) 2007 by Frank Richter

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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_DEPENDENTTARGET_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_DEPENDENTTARGET_H__

#include "csplugincommon/rendermanager/operations.h"
#include "csplugincommon/rendermanager/rendertree.h"
#include "csplugincommon/rendermanager/shadersetup.h"
#include "csplugincommon/rendermanager/svtraverse.h"
#include "csutil/fifo.h"
#include "csutil/set.h"

namespace CS
{
namespace RenderManager
{

  /**
   * 
   */
  template<typename RenderTree, typename TargetHandler>
  class DependentTargetManager
  {
  public:
    /**
     * 
     */
    struct TargetSettings
    {
      CS::ShaderVarStringID svName;
      iView* view;
      iTextureHandle* target;
      int targetSubTexture;      
    };

    DependentTargetManager (TargetHandler& targetHandler)
      : targetHandler (targetHandler)
    {}

    /**
     * 
     */
    void RegisterRenderTarget (iTextureHandle* target, 
      iView* view, int subtexture = 0, uint flags = 0)
    {
      typename RenderTargetInfo::ViewInfo newView;
      newView.view = view;
      newView.flags = flags & ~iRenderManagerTargets::updateOnce;
      TargetsHash* targets;
      if (flags & iRenderManagerTargets::updateOnce)
      {
        targets = &oneTimeTargets;
      }
      else
      {
        targets = &this->targets;
      }
      RenderTargetInfo* targetInfo = targets->GetElementPointer (target);
      if (targetInfo == 0)
      {
        RenderTargetInfo newInfo;
        newInfo.views.PutUnique (subtexture, newView);
        targets->PutUnique (target, newInfo);
      }
      else
      {
        targetInfo->views.PutUnique (subtexture, newView);
      }
    }

    /**
     * 
     */
    void UnregisterRenderTarget (iTextureHandle* target,
      int subtexture = 0)
    {

      RenderTargetInfo* targetInfo = targets.GetElementPointer (target);
      if (targetInfo != 0)
      {
        targetInfo->views.DeleteAll (subtexture);
        if (targetInfo->views.IsEmpty()) targets.DeleteAll (target);
        return;
      }
      targetInfo = oneTimeTargets.GetElementPointer (target);
      if (targetInfo != 0)
      {
        targetInfo->views.DeleteAll (subtexture);
        if (targetInfo->views.IsEmpty()) oneTimeTargets.DeleteAll (target);
      }
    }

    /**
     * 
     */
    void PrepareQueues (iShaderManager* shaderManager)
    {
      size_t numSVs = shaderManager->GetSVNameStringset()->GetSize();
      names.SetSize (numSVs);
      handledTargets.Empty();
      handledTargets.SetCapacity (targets.GetSize() + oneTimeTargets.GetSize());

      targetQueue.DeleteAll ();
    }

    /**
     * 
     */
    template<typename LayerConfigType>
    void EnqueueTargets (RenderTree& renderTree, iShaderManager* shaderManager, 
      const LayerConfigType& layerConfig,
      csSet<typename RenderTree::ContextNode*>& contextsTested)
    {
      // Setup callbacks for SVs and mesh nodes
      NewTargetFn newTarget (*this, renderTree);
      typedef TraverseUsedSVs<RenderTree, NewTargetFn> MeshTraverseType;
      MeshTraverseType svTraverser
        (newTarget, shaderManager->GetSVNameStringset ()->GetSize ());

      // Just traverse each context once
      Implementation::OnceOperationBlockRef<typename RenderTree::ContextNode*> 
        opBlock (contextsTested);

      Implementation::NoOperationBlock<typename RenderTree::MeshNode*> meshNoBlock;

      // Helper for traversing all contexts within tree
      Implementation::MeshContextTraverser<
        typename RenderTree::ContextNode,
        MeshTraverseType, 
        Implementation::NoOperationBlock<typename RenderTree::MeshNode*>
      > contextTraverse (svTraverser, meshNoBlock);

      // And do the iteration
      ForEachContext (renderTree, contextTraverse, opBlock);
    }

    /**
     * 
     */
    bool HaveMoreTargets() const
    {
      return targetQueue.GetSize () > 0;
    }

    /**
     * 
     */
    void GetNextTarget (TargetSettings& settings)
    {
      settings = targetQueue.PopTop ();
    }

    /**
     * 
     */
    void PostCleanupQueues ()
    {
      oneTimeTargets.DeleteAll ();
      targetQueue.DeleteAll ();
    }
  private:
    typedef DependentTargetManager<RenderTree, TargetHandler> DependentTargetManagerType;
    
    /* FIXME: better handle multiple views per target
	      'flags' per subtexture seems odd */
    struct RenderTargetInfo
    {
      struct ViewInfo
      {
        csRef<iView> view;
        uint flags;
      };
      typedef csHash<ViewInfo, int> ViewsHash;
      ViewsHash views;
    };
    typedef csHash<RenderTargetInfo, csRef<iTextureHandle> > TargetsHash;
    TargetsHash targets;
    TargetsHash oneTimeTargets;
    
    class HandledTargetsSet
    {
    public:
      void SetCapacity (size_t n) 
      {
        set.SetCapacity (n); 
      }

      void Empty () 
      {
        set.Empty(); 
      }
      
      void Insert (size_t index, iTextureHandle* texh)
      {
        set.Insert (index, texh);
      }

      bool Find (iTextureHandle* texh, size_t& candidate) const
      {
        return set.FindSortedKey (
          csArrayCmp<iTextureHandle*, iTextureHandle*> (texh), &candidate)
          != csArrayItemNotFound;
      }

    private:
      csArray<iTextureHandle*> set;
    };
    
    // 
    TargetHandler& targetHandler;

    // Storage for used SV names
    csBitArray names;
    // Storage for handled targets
    HandledTargetsSet handledTargets;

    // Queue of contexts to setup
    csFIFO<TargetSettings> targetQueue;

    struct NewTargetFn
    {
      NewTargetFn (DependentTargetManagerType& parent, RenderTree& renderTree)
        : parent (parent), renderTree (renderTree)
      {}

      void operator() (CS::ShaderVarStringID name, csShaderVariable* sv)
      {
        if (sv->GetType() != csShaderVariable::TEXTURE)
          return;
        
        iTextureHandle* textureHandle;
        sv->GetValue (textureHandle);

        iView* localView = 0;
        bool handleTarget = false;

        // Check any of the explicit targets
        RenderTargetInfo* targetInfo;
        targetInfo = parent.targets.GetElementPointer (textureHandle);
        handleTarget = (targetInfo != 0);
        targetInfo = parent.oneTimeTargets.GetElementPointer (textureHandle);
        handleTarget |= (targetInfo != 0);

        // Dispatch upwards
        if (!handleTarget)
        {
          handleTarget = parent.targetHandler.HandleTargetSetup (name, sv, 
            textureHandle, localView);          
        }

        if (handleTarget)
        {
          size_t insertPos;
          if (parent.handledTargets.Find (textureHandle, insertPos)) return;
          parent.handledTargets.Insert (insertPos, textureHandle);

          if (targetInfo)
          {
            typename RenderTargetInfo::ViewsHash::GlobalIterator viewsIt (
              targetInfo->views.GetIterator());

            while (viewsIt.HasNext ())
            {
              int subtexture;
              const typename RenderTargetInfo::ViewInfo& viewInfo (
                viewsIt.Next (subtexture));
              HandleView (viewInfo.view, textureHandle, subtexture, name);
            }
          }
          else
          {
            HandleView (localView, textureHandle, 0, name);
          }
        }
      }


      void HandleView (iView* targetView,
        iTextureHandle* texh, int subtexture, CS::ShaderVarStringID svName)
      {
        if (!targetView) return;

        targetView->UpdateClipper ();

        // Setup a target info struct
        TargetSettings settings;
        settings.svName = svName;
        settings.target = texh;
        settings.targetSubTexture = subtexture;
        settings.view = targetView;        

        parent.targetQueue.Push (settings);        
      }


      DependentTargetManager& parent;
      RenderTree& renderTree;
    };

    friend struct TargetTraverser;
  };

} // namespace RenderManager
} // namespace CS

#endif // __CS_CSPLUGINCOMMON_RENDERMANAGER_DEPENDENTTARGET_H__
