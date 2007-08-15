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

#include "csplugincommon/rendermanager/rendertree.h"
#include "csplugincommon/rendermanager/shadersetup.h"

namespace CS
{
namespace RenderManager
{
  
  /**
   * Traverse all the shader variables used in a certain render context.
   */
  template<typename Tree, typename Fn>
  class TraverseAllUsedSVs
  {
    iShaderManager* shaderManager;
    Fn& function;
    
    /**
     * The actual workhorse: for each mesh in the context construct the SV
     * stack. Then go over that stack, using the known used shader vars, and
     * call Fn.
     */
    struct FnMeshTraverser : 
      public NumberedMeshTraverser<Tree, FnMeshTraverser>
    {
      typedef NumberedMeshTraverser<Tree, FnMeshTraverser> BaseType;

      ShaderVariableNameArray& names;
      iShaderManager* shaderManager;
      Fn& function;
      csShaderVariableStack& varStack;

      FnMeshTraverser (ShaderVariableNameArray& names,
        iShaderManager* shaderManager, Fn& function) : BaseType (*this),
        names (names), shaderManager (shaderManager), 
        function (function),
        varStack (shaderManager->GetShaderVariableStack()) {}

      using BaseType::operator();

      void operator() (const typename Tree::MeshNode::SingleMesh& mesh, size_t index,
        typename Tree::ContextNode& ctxNode, const Tree& tree)
      {
        ctxNode.svArrays.SetupSVStck (varStack, index);

        for (size_t j = 0; j < names.GetSize(); j++)
        {
          csShaderVariable* sv = varStack[names[j]];
          if (sv == 0) continue;
          function (sv);
        }
      }
    };
  public:
    TraverseAllUsedSVs (Fn& function, iShaderManager* shaderManager) : 
      function (function), shaderManager (shaderManager) {}

    void operator() (typename Tree::ContextNode* contextNode, Tree& tree)
    {
      ShaderVariableNameArray names;
      GetAllUsedShaderVars<Tree> (*contextNode, shaderManager, names);

      FnMeshTraverser traverse (names, shaderManager, function);
      tree.TraverseMeshNodes (traverse, contextNode);
    }

  };

  template<typename Tree, typename Fn>
  class TraverseAllTextures
  {
    Fn& function;
    iShaderManager* shaderManager;

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
    TraverseAllTextures (Fn& function, iShaderManager* shaderManager) : 
      function (function), shaderManager (shaderManager) {}

    void operator() (typename Tree::ContextNode* contextNode, Tree& tree)
    {
      FnShaderVarTraverser svTraverser (function);
      TraverseAllUsedSVs<Tree, FnShaderVarTraverser> traverseSVs (svTraverser, 
        shaderManager);
      traverseSVs (contextNode, tree);
    }
  };

  template<typename Tree>
  class DependentTargetManager
  {
    friend struct TextureTraverser;

    csHash<csRef<iView>, csRef<iTextureHandle> > targets;
    CS::RenderManager::RenderView::Pool renderViewPool;

    typedef csSet<csPtrKey<iTextureHandle> > HandledTargetsSet;

    struct TextureTraverser
    {
      DependentTargetManager& parent;
      iShaderManager* shaderManager;
      HandledTargetsSet& handledTargets;
      ContextSetup<Tree>& contextSetup;
      Tree& renderTree;

      TextureTraverser (DependentTargetManager& parent,
        iShaderManager* shaderManager,
        HandledTargetsSet& handledTargets,
        ContextSetup<Tree>& contextSetup,
        Tree& renderTree) : parent (parent), 
        shaderManager (shaderManager), handledTargets (handledTargets),
        contextSetup (contextSetup),
        renderTree (renderTree) {}

      void operator() (iTextureHandle* texh)
      {
        csRef<iView> targetView = parent.targets.Get (texh, csRef<iView> ());
        if (targetView.IsValid () && !handledTargets.Contains (texh))
        {
          handledTargets.Add (texh);
          // Setup a rendering view
          csRef<CS::RenderManager::RenderView> rview;

          Tree::ContextsContainer* targetContexts = 
            renderTree.CreateContextContainer ();
          targetContexts->renderTarget = texh;
          targetContexts->view = targetView;

          rview.AttachNew (new (parent.renderViewPool) 
            CS::RenderManager::RenderView (targetView));
          targetView->UpdateClipper ();
          Tree::ContextNode* context = 
            renderTree.CreateContext (targetContexts, rview);
  
          contextSetup (renderTree, context, targetContexts, rview->GetThisSector (), rview);

          TraverseAllTextures<Tree, TextureTraverser> traverseAllTextures (
            *this, shaderManager);
          renderTree.TraverseContexts (targetContexts, traverseAllTextures);
          //traverseAllTextures (targetContexts, renderTree);
        }
        
      }
    };
  public:
    void RegisterRenderTarget (iTextureHandle* target, 
      iView* view)
    {
      targets.PutUnique (target, view);
    }
    void UnregisterRenderTarget (iTextureHandle* target)
    {
      targets.DeleteAll (target);
    }

    void AddDependentTargetsToTree (typename Tree::ContextsContainer* contexts,
      Tree& renderTree, ContextSetup<Tree>& contextSetup, 
      iShaderManager* shaderManager)
    {
      HandledTargetsSet handledTargets;
      TextureTraverser texTraverse (*this, shaderManager, handledTargets,
        contextSetup, renderTree);
      TraverseAllTextures<Tree, TextureTraverser> traverseAllTextures (texTraverse, 
        shaderManager);
      renderTree.TraverseContexts (contexts, traverseAllTextures);
      //traverseAllTextures (, renderTree);
    }
  };

} // namespace RenderManager
} // namespace CS

#endif // __CS_CSPLUGINCOMMON_RENDERMANAGER_DEPENDENTTARGET_H__
