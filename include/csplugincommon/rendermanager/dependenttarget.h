/*
    Copyright (C) 2007 by Marten Svanfeldt
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

#include "iengine/rendermanager.h"
#include "csplugincommon/rendermanager/rendertree.h"
#include "csplugincommon/rendermanager/shadersetup.h"

namespace CS
{
namespace RenderManager
{
  
  /**
   * Traverse all the shader variables used in a certain render context.
   */
  template<typename Tree, typename Fn, typename BitArray = csBitArray>
  class TraverseAllUsedSVs
  {
    iShaderManager* shaderManager;
    Fn& function;
    BitArray& nameStorage;
    
    /**
     * The actual workhorse: for each mesh in the context construct the SV
     * stack. Then go over that stack, using the known used shader vars, and
     * call Fn.
     */
    struct FnMeshTraverser : 
      public NumberedMeshTraverser<Tree, FnMeshTraverser>
    {
      typedef NumberedMeshTraverser<Tree, FnMeshTraverser> BaseType;

      iShaderManager* shaderManager;
      Fn& function;

      FnMeshTraverser (iShaderManager* shaderManager, Fn& function, 
        BitArray& names) : BaseType (*this), shaderManager (shaderManager), 
        function (function), names (names)
      {
      }

      void operator() (const typename Tree::TreeTraitsType::MeshNodeKeyType& key,
        typename Tree::MeshNode* node, typename Tree::ContextNode& ctxNode, Tree& tree)
      {
        lastShader = 0;
        lastTicket = (size_t)~0;

        BaseType::operator() (key, node, ctxNode, tree);
      }

      void operator() (typename Tree::MeshNode* node,
        const typename Tree::MeshNode::SingleMesh& mesh, size_t index,
        typename Tree::ContextNode& ctxNode, const Tree& tree)
      {
        iShader* shader = ctxNode.shaderArray[index];
        if (!shader) return;
        size_t ticket = ctxNode.ticketArray[index];

        if ((shader != lastShader) || (ticket != lastTicket))
        {
          names.Clear();
          shader->GetUsedShaderVars (ticket, names);
        }

        csShaderVariableStack varStack;
        ctxNode.svArrays.SetupSVStck (varStack, index);

        size_t name = csMin (names.GetSize(), varStack.GetSize());
        while (name-- > 0)
        {
          if (names.IsBitSet (name))
          {
            csShaderVariable* sv = varStack[name];
            if (sv != 0) function (sv);
          }
        }
      }

      iShader* lastShader;
      size_t lastTicket;
      BitArray& names;
    };
  public:
    TraverseAllUsedSVs (Fn& function, iShaderManager* shaderManager,
      BitArray& nameStorage) : shaderManager (shaderManager), 
      function (function), nameStorage (nameStorage) {}

    void operator() (typename Tree::ContextNode* contextNode, Tree& tree)
    {
      FnMeshTraverser traverse (shaderManager, function, nameStorage);
      tree.TraverseMeshNodes (traverse, contextNode);
    }

  };

  template<typename Tree, typename Fn, typename BitArray = csBitArray>
  class TraverseAllTextures
  {
    Fn& function;
    iShaderManager* shaderManager;
    BitArray& names;

    struct FnShaderVarTraverser
    {
      Fn& function;

      FnShaderVarTraverser (Fn& function) : function (function)
      {
      }

      void operator() (csShaderVariable* sv)
      {
        if (sv->GetType() == csShaderVariable::TEXTURE)
        {
          iTextureHandle* texh;
          sv->GetValue (texh);
          function (texh);
        }
      }
    };
  public:
    TraverseAllTextures (Fn& function, iShaderManager* shaderManager, 
      BitArray& names) : function (function), shaderManager (shaderManager),
      names (names) {}

    void operator() (typename Tree::ContextNode* contextNode, Tree& tree)
    {
      FnShaderVarTraverser svTraverser (function);
      TraverseAllUsedSVs<Tree, FnShaderVarTraverser, BitArray> traverseSVs (
        svTraverser, shaderManager, names);
      traverseSVs (contextNode, tree);
    }
  };

  template<typename Tree>
  class DependentTargetManager
  {
    friend struct TextureTraverser;

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
    CS::RenderManager::RenderView::Pool renderViewPool;
    
    class HandledTargetsSet
    {
      csArray<iTextureHandle*> set;
    public:
      void SetCapacity (size_t n) { set.SetCapacity (n); }
      void Empty () { set.Empty(); }
      
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
    };
    
    int nestLevel;
    // Storage for used SV names
    csBitArray names;
    // Storage for handled targets
    HandledTargetsSet handledTargets;

    struct TextureTraverser
    {
      DependentTargetManager& parent;
      iShaderManager* shaderManager;
      HandledTargetsSet& handledTargets;
//      StandardContextSetup<Tree>& contextSetup;
      Tree& renderTree;

      TextureTraverser (DependentTargetManager& parent,
        iShaderManager* shaderManager,
        HandledTargetsSet& handledTargets,
        //StandardContextSetup<Tree>& contextSetup,
        Tree& renderTree) : parent (parent), 
        shaderManager (shaderManager), handledTargets (handledTargets),
        //contextSetup (contextSetup),
        renderTree (renderTree) {}

      void operator() (iTextureHandle* texh)
      {
        RenderTargetInfo* targetInfo;
        targetInfo = parent.targets.GetElementPointer (texh);
        if (targetInfo != 0)
        {
          HandleTarget (targetInfo, texh);
          return;
        }
        targetInfo = parent.oneTimeTargets.GetElementPointer (texh);
        if (targetInfo != 0)
        {
          HandleTarget (targetInfo, texh);
        }
      }
    private:
      void HandleTarget (const RenderTargetInfo* targetInfo,
			 iTextureHandle* texh)
      {
        size_t insertPos;
        if (handledTargets.Find (texh, insertPos)) return;
        handledTargets.Insert (insertPos, texh);
        
        typename RenderTargetInfo::ViewsHash::ConstGlobalIterator viewsIt (
          targetInfo->views.GetIterator());
        while (viewsIt.HasNext ())
        {
          int subtexture;
          const typename RenderTargetInfo::ViewInfo& viewInfo (
            viewsIt.Next (subtexture));
	  HandleView (viewInfo, texh, subtexture);
        }
      }
      void HandleView (const typename RenderTargetInfo::ViewInfo& viewInfo,
                       iTextureHandle* texh, int subtexture)
      {
	csRef<iView> targetView = viewInfo.view;
	if (!targetView.IsValid ()) return;

	// Setup a rendering view
	csRef<CS::RenderManager::RenderView> rview;

	typename Tree::ContextsContainer* targetContexts = 
	  renderTree.CreateContextContainer ();
	targetContexts->renderTarget = texh;
	targetContexts->subtexture = subtexture;
	targetContexts->view = targetView;

	rview.AttachNew (new (parent.renderViewPool) 
	  CS::RenderManager::RenderView (targetView));
	targetView->UpdateClipper ();
	typename Tree::ContextNode* context = 
	  renderTree.CreateContext (targetContexts, rview);

	//contextSetup (renderTree, context, targetContexts, rview->GetThisSector (), rview);

	TraverseAllTextures<Tree, TextureTraverser> 
	  traverseAllTextures (*this, shaderManager, parent.names);
	renderTree.TraverseContexts (targetContexts, traverseAllTextures);
      }
    };
  public:
    DependentTargetManager () : nestLevel (0) {}
  
    void RegisterRenderTarget (iTextureHandle* target, 
      iView* view, int subtexture = 0, uint flags = 0)
    {
      typename RenderTargetInfo::ViewInfo newView;
      newView.view = view;
      newView.flags = flags & ~iRenderManager::updateOnce;
      TargetsHash* targets;
      if (flags & iRenderManager::updateOnce)
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
        newInfo.views.Put (subtexture, newView);
        targets->PutUnique (target, newInfo);
      }
      else
      {
        targetInfo->views.Put (subtexture, newView);
      }
    }
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

    void AddDependentTargetsToTree (typename Tree::ContextsContainer* contexts,
      Tree& renderTree/*, StandardContextSetup<Tree>& contextSetup*/, 
      iShaderManager* shaderManager)
    {
      if (nestLevel == 0)
      {
        size_t numSVs = shaderManager->GetSVNameStringset()->GetSize();
        names.SetSize (numSVs);
        handledTargets.Empty();
        handledTargets.SetCapacity (targets.GetSize() + oneTimeTargets.GetSize());
      }
      
      nestLevel++;
 /*     TextureTraverser texTraverse (*this, shaderManager, handledTargets,
        contextSetup, renderTree);
      TraverseAllTextures<Tree, TextureTraverser> traverseAllTextures (texTraverse, 
        shaderManager, names);
      renderTree.TraverseContexts (contexts, traverseAllTextures);*/
      nestLevel--;
      
      if (nestLevel == 0) oneTimeTargets.DeleteAll ();
    }
  };

} // namespace RenderManager
} // namespace CS

#endif // __CS_CSPLUGINCOMMON_RENDERMANAGER_DEPENDENTTARGET_H__
