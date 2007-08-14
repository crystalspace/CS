/*
    Copyright (C) 2007 by Marten Svanfeldt

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

#include "crystalspace.h"

#include "csutil/compositefunctor.h"

#include "rm_test1.h"

CS_IMPLEMENT_PLUGIN

using namespace CS::Plugin::RMTest1;
using namespace CS::RenderManager;

SCF_IMPLEMENT_FACTORY(RMTest1)

RMTest1::RMTest1 (iBase* parent)
: scfImplementationType (this, parent)
{

}

template<typename RenderTreeType>
class ContextSetup
{
public:
  ContextSetup (iShaderManager* shaderManager, iShader* defaultShader,
    csStringID defaultShaderName)
    : shaderManager (shaderManager), defaultShader (defaultShader), defaultShaderName (defaultShaderName)
  {

  }

  void operator() (RenderTreeType& renderTree, typename RenderTreeType::ContextNode* context, 
    iSector* sector, CS::RenderManager::RenderView* rview)
  {
    // Do the culling
    iVisibilityCuller* culler = sector->GetVisibilityCuller ();
    renderTree.Viscull (context, rview, culler);

    // Sort the mesh lists  
    {
      StandardMeshSorter<RenderTreeType> mySorter (rview->GetEngine (), rview->GetCamera ());
      renderTree.TraverseMeshNodes (mySorter, context);
    }

    // Setup the SV arrays
    // Push the default stuff
    SetupStandardSVs<RenderTreeType> (*context, shaderManager, sector);

    // Setup the material&mesh SVs
    {
      StandardSVSetup<RenderTreeType> svSetup (context->svArrays);
      renderTree.TraverseMeshNodes (svSetup, context);
    }

    // Setup shaders and tickets
    SetupStandarShaderAndTicket (renderTree, *context, shaderManager, 
      defaultShaderName, defaultShader);

    // Handle all portals
    for (size_t pc = 0; pc < context->allPortals.GetSize (); ++pc)
    {
      typename RenderTreeType::ContextNode::PortalHolder& holder = context->allPortals[pc];

      for (size_t pi = 0; pi < (size_t)holder.portalContainer->GetPortalCount (); ++pi)
      {
        iPortal* portal = holder.portalContainer->GetPortal (pi);

        if (IsSimplePortal (portal))
        {
          // Finish up the sector
          if (!portal->CompleteSector (rview))
            continue;

          // Setup a bounding box, in screen-space
          const csVector3* portalVerts = portal->GetWorldVertices ();
          int* indices = portal->GetVertexIndices ();
          size_t indexCount = portal->GetVertexIndicesCount ();

          csDirtyAccessArray<csVector2> portalVerts2d (64);
          const csOrthoTransform& camTrans = rview->GetCamera ()->GetTransform ();
          csBox2 screenBox;
          for (size_t i = 0; i < indexCount; ++i)
          {
            const csVector3 cameraVert = camTrans.Other2This (portalVerts[indices[i]]);
            const csVector2 screenVert = rview->GetCamera ()->Perspective (cameraVert);
            portalVerts2d.Push (screenVert);
            screenBox.AddBoundingVertex (screenVert);
          }

          size_t count = portalVerts2d.GetSize ();
          if ((rview->GetClipper ()->ClipInPlace (portalVerts2d.GetArray (), count, screenBox) == CS_CLIP_OUTSIDE) ||
            count == 0)
            continue;

          portalVerts2d.SetSize (count);
          
          // Setup simple portal
          rview->CreateRenderContext ();
          rview->SetLastPortal (portal);
          rview->SetPreviousSector (sector);
          csPolygonClipper newView (portalVerts2d.GetArray (), count);
          rview->SetClipper (&newView);

          typename RenderTreeType::ContextNode* portalCtx = renderTree.PrepareViscull (rview);

          // Setup the new context
          (*this)(renderTree, portalCtx, portal->GetSector (), rview);

          rview->RestoreRenderContext ();
        }
        else
        {
          // Setup heavy portal @@TODO
        }
      }
    }
  }


private:
  bool IsSimplePortal (iPortal* portal)
  {
    return true;
  }

  iShaderManager* shaderManager;
  iShader* defaultShader;
  csStringID defaultShaderName;
};

template<typename RenderTreeType>
class ContextRender
{
public:
  ContextRender (iGraphics3D* g3d, iShaderManager* shaderManager)
    : g3d (g3d), shaderManager (shaderManager)
  {}

  void operator() (typename RenderTreeType::ContextNode* node, RenderTreeType& tree)
  {
    SimpleRender<RenderTreeType> render (g3d, shaderManager->GetShaderVariableStack ());

    MeshNodeCB<SimpleRender<RenderTreeType> > cb (render, node, tree);
    node->meshNodes.TraverseInOrder (cb);
  }

private:
  template<typename Fn>
  struct MeshNodeCB
  {
    MeshNodeCB(Fn& meshNodeFunction, typename RenderTreeType::ContextNode* node, RenderTreeType& tree)
      : meshNodeFunction (meshNodeFunction), node (node), tree (tree)
    {}

    void operator() (const typename RenderTreeType::TreeTraitsType::MeshNodeKeyType& key, 
      typename RenderTreeType::MeshNode* meshNode)
    {
      meshNodeFunction (key, meshNode, *node, tree);
    }

    Fn& meshNodeFunction;
    typename RenderTreeType::ContextNode* node;
    RenderTreeType& tree;
  };

  iGraphics3D* g3d;
  iShaderManager* shaderManager;
};
  

bool RMTest1::RenderView (iView* view)
{
  // Setup a rendering view
  csRef<CS::RenderManager::RenderView> rview;
  rview.AttachNew (new(renderViewPool) CS::RenderManager::RenderView(view));
  view->GetEngine ()->UpdateNewFrame ();

  view->UpdateClipper ();

  iSector* startSector = rview->GetThisSector ();

  // Pre-setup culling graph
  RenderTreeType renderTree (treePersistent);
  RenderTreeType::ContextNode* mainContext = renderTree.PrepareViscull (rview);

  // Setup the main context
  {
    ContextSetup<RenderTreeType> contextSetup (shaderManager, defaultShader, defaultShaderName);
    contextSetup (renderTree, mainContext, startSector, rview);
  }

  // Render all contexts, back to front
  {
    iGraphics3D* g3d = view->GetContext ();

    BeginFinishDrawScope bd (g3d, view->GetEngine ()->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS);

    g3d->SetWorldToCamera (view->GetCamera ()->GetTransform ().GetInverse ());

    ContextRender<RenderTreeType> render (g3d, shaderManager);
    renderTree.TraverseContextsReverse (render);
    //renderTree.TraverseMeshNodes (render, mainContext);
  }

  return true;
}

bool RMTest1::Initialize(iObjectRegistry* objectReg)
{
  svNameStringSet = csQueryRegistryTagInterface<iStringSet> (objectReg,
    "crystalspace.shader.variablenameset");

  stringSet = csQueryRegistryTagInterface<iStringSet> (objectReg,
    "crystalspace.shared.stringset");

  shaderManager = csQueryRegistry<iShaderManager> (objectReg);

  defaultShaderName = stringSet->Request("standard");  
  defaultShader = shaderManager->GetShader ("std_lighting");
  return true;
}
