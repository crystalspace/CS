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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_CONTEXT_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_CONTEXT_H__

#include "iengine/sector.h"

#include "csplugincommon/rendermanager/render.h"
#include "csplugincommon/rendermanager/renderview.h"
#include "csplugincommon/rendermanager/standardsorter.h"
#include "csplugincommon/rendermanager/svsetup.h"
#include "csplugincommon/rendermanager/texturecache.h"
#include "csutil/sysfunc.h"

namespace CS
{
namespace RenderManager
{
  template<typename RenderTreeType, typename ContextSetup>
  class StandardPortalSetup
  {
  public:
    struct PersistentData
    {
      TextureCache texCache;
      csStringID svNameTexPortal;
      
      PersistentData() : texCache (csimg2D, "rgb8", 
        CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS | CS_TEXTURE_CLAMP,
        "target", TextureCache::tcachePowerOfTwo) {}
      
      void Initialize (iShaderManager* shmgr, iGraphics3D* g3d)
      {
        svNameTexPortal = 
          shmgr->GetSVNameStringset()->Request ("tex portal");
	texCache.SetG3D (g3d);
      }
      
      void UpdateNewFrame ()
      {
        texCache.AdvanceFrame (csGetTicks ());
      }
    };
  
    StandardPortalSetup (PersistentData& persistentData, ContextSetup& cfun)
      : persistentData (persistentData), contextFunction (cfun)
    {}

    void operator() (RenderTreeType& renderTree, 
      typename RenderTreeType::ContextNode* context, 
      typename RenderTreeType::ContextsContainer* container, 
      iSector* sector, CS::RenderManager::RenderView* rview)
    {
      csDirtyAccessArray<csVector2> allPortalVerts2d (64);
      csDirtyAccessArray<csVector3> allPortalVerts3d (64);
      csDirtyAccessArray<size_t> allPortalVertsNums;
      // Handle all portals
      for (size_t pc = 0; pc < context->allPortals.GetSize (); ++pc)
      {
        typename RenderTreeType::ContextNode::PortalHolder& holder = context->allPortals[pc];
        const size_t portalCount = holder.portalContainer->GetPortalCount ();

        size_t allPortalVertices = holder.portalContainer->GetTotalVertexCount ();
        allPortalVerts2d.SetSize (allPortalVertices * 3);
        allPortalVerts3d.SetSize (allPortalVertices * 3);
        allPortalVertsNums.SetSize (portalCount);

        /* Get clipped screen space and camera space vertices */
        holder.portalContainer->ComputeScreenPolygons (rview, 
          allPortalVerts2d.GetArray(), allPortalVerts3d.GetArray(), 
          allPortalVerts2d.GetSize(), allPortalVertsNums.GetArray());
        csVector2* portalVerts2d = allPortalVerts2d.GetArray();
        csVector3* portalVerts3d = allPortalVerts3d.GetArray();

        for (size_t pi = 0; pi < portalCount; ++pi)
        {
          iPortal* portal = holder.portalContainer->GetPortal (pi);
          const csFlags portalFlags (portal->GetFlags());

          if (IsSimplePortal (portalFlags))
          {
            // Finish up the sector
            if (!portal->CompleteSector (rview))
              continue;
              
            size_t count = allPortalVertsNums[pi];
            if (count == 0) continue;

            // Setup simple portal
            rview->CreateRenderContext ();
            rview->SetLastPortal (portal);
            rview->SetPreviousSector (sector);
            csPolygonClipper newView (portalVerts2d, count);
            rview->SetClipper (&newView);

	    if (portalFlags.Check (CS_PORTAL_WARP))
	    {
	      iCamera *inewcam = rview->CreateNewCamera ();
	      
	      const csReversibleTransform& movtrans =
		holder.meshWrapper->GetMovable()->GetFullTransform();
	      bool mirror = inewcam->IsMirrored ();
	      csReversibleTransform warp_wor;
	      portal->ObjectToWorld (movtrans, warp_wor);
	      portal->WarpSpace (warp_wor, inewcam->GetTransform (), mirror);
	      inewcam->SetMirrored (mirror);
	    }
	    
            typename RenderTreeType::ContextNode* portalCtx = 
              renderTree.CreateContext (container, rview);

            // Setup the new context
            contextFunction(renderTree, portalCtx, container, portal->GetSector (), rview);

            rview->RestoreRenderContext ();
            portalVerts2d += count;
            portalVerts3d += count;
          }
          else
          {
	    // Setup heavy portal
	    
	    // Finish up the sector
	    if (!portal->CompleteSector (rview))
	      continue;
  
            size_t count = allPortalVertsNums[pi];
            if (count == 0) continue;
	    
	    // Setup a bounding box, in screen-space
	    csBox2 screenBox;
	    ComputeVector2BoundingBox (portalVerts2d, count, screenBox);
	    
	    // Obtain a texture handle for the portal to render to
	    int txt_w = int (ceil (screenBox.MaxX() - screenBox.MinX()));
	    int txt_h = int (ceil (screenBox.MaxY() - screenBox.MinY()));
	    csRef<iTextureHandle> tex = 
	      persistentData.texCache.QueryUnusedTexture (txt_w, txt_h, 0);
	      
	    int real_w, real_h;
	    tex->GetRendererDimensions (real_w, real_h);
	          
	    iCamera* cam = rview->GetCamera();
	    // Create a new view
	    csRef<CS::RenderManager::RenderView> newRenderView;
	    newRenderView.AttachNew (
	      new (renderTree.GetPersistentData().renderViewPool) RenderView (
	        cam->Clone(), 0, rview->GetGraphics3D(), rview->GetGraphics2D()));
	    newRenderView->SetEngine (rview->GetEngine ());
	    
	    iCamera *inewcam = newRenderView->GetCamera();
	    if (portalFlags.Check (CS_PORTAL_WARP))
	    {
	      const csReversibleTransform& movtrans =
		holder.meshWrapper->GetMovable()->GetFullTransform();
	      bool mirror = inewcam->IsMirrored ();
	      csReversibleTransform warp_wor;
	      portal->ObjectToWorld (movtrans, warp_wor);
	      portal->WarpSpace (warp_wor, inewcam->GetTransform (), mirror);
	      inewcam->SetMirrored (mirror);
	    }
	    inewcam->SetPerspectiveCenter (cam->GetShiftX() - screenBox.MinX(),
	      real_h - (screenBox.MaxY() - cam->GetShiftY()));
	    
	    // Add a new context with the texture as the target
	    // Setup simple portal
	    newRenderView->CreateRenderContext ();
	    newRenderView->SetLastPortal (portal);
	    newRenderView->SetPreviousSector (sector);
	    csBox2 clipBox (0, real_h - txt_h, txt_w, txt_h);
            csBoxClipper newView (clipBox);
            newRenderView->SetClipper (&newView);

	    typename RenderTreeType::ContextsContainer* targetContexts = 
	      renderTree.CreateContextContainer ();
	    targetContexts->renderTarget = tex;
	    targetContexts->rview = newRenderView;
    
            typename RenderTreeType::ContextNode* portalCtx = 
              renderTree.CreateContext (targetContexts, newRenderView);
  
	    // Setup the new context
            contextFunction(renderTree, portalCtx, targetContexts, portal->GetSector (), rview);
  
	    newRenderView->RestoreRenderContext ();

	    // Synthesize a render mesh for the portal plane
	    iMaterialWrapper* mat = portal->GetMaterial ();
	    csRef<csShaderVariableContext> svc;
	    svc.AttachNew (new csShaderVariableContext);
	    csRef<csShaderVariable> svTexPortal =
	      svc->GetVariableAdd (persistentData.svNameTexPortal);
	    svTexPortal->SetValue (tex);
	    
	    csRef<iRenderBuffer> coordBuf = 
	      csRenderBuffer::CreateRenderBuffer (count, CS_BUF_STREAM, CS_BUFCOMP_FLOAT, 3);
	    {
	      csRenderBufferLock<csVector3> coords (coordBuf);
	      for (size_t c = 0; c < count; c++)
	      {
		coords[c].Set (portalVerts3d[c]);
	      }
	    }
	    csRef<iRenderBuffer> tcBuf =
	      csRenderBuffer::CreateRenderBuffer (count, CS_BUF_STREAM, CS_BUFCOMP_FLOAT, 4);
	    {
	      float xscale = 1.0f / real_w;
	      float yscale = 1.0f / real_h;
	      csRenderBufferLock<csVector4> tcoords (tcBuf);
	      for (size_t c = 0; c < count; c++)
	      {
	        float z = portalVerts3d[c].z;
		tcoords[c].Set ((portalVerts2d[c].x - screenBox.MinX()) * xscale * z, 
		  (screenBox.MaxY() - portalVerts2d[c].y) * yscale * z, 0, 
		  z);
	      }
	    }
	    csRef<iRenderBuffer> indexBuf =
	      csRenderBuffer::CreateIndexRenderBuffer (count, CS_BUF_STREAM, 
		CS_BUFCOMP_UNSIGNED_INT, 0, count-1);
	    {
	      csRenderBufferLock<uint> indices (indexBuf);
	      for (size_t c = 0; c < count; c++)
	      {
	        *indices++ = c;
	      }
	    }
	    
	    csRef<csRenderBufferHolder> buffers;
	    buffers.AttachNew (new csRenderBufferHolder);
	    buffers->SetRenderBuffer (CS_BUFFER_INDEX, indexBuf);
	    buffers->SetRenderBuffer (CS_BUFFER_POSITION, coordBuf);
	    buffers->SetRenderBuffer (CS_BUFFER_TEXCOORD0, tcBuf);
	    
	    bool meshCreated;
	    csRenderMesh* rm = 
	      renderTree.GetPersistentData().rmHolder.GetUnusedMesh (
	        meshCreated, rview->GetCurrentFrameNumber());
	    rm->db_mesh_name = "portal";
	    rm->material = mat;
	    rm->meshtype = CS_MESHTYPE_TRIANGLEFAN;
	    rm->buffers = buffers;
	    rm->z_buf_mode = CS_ZBUF_USE;
	    rm->object2world = rview->GetCamera()->GetTransform();
	    rm->indexstart = 0;
	    rm->indexend = count;
	    rm->mixmode = CS_MIXMODE_BLEND(ONE, ZERO);
	    rm->variablecontext = svc;
	    
	    typename RenderTreeType::MeshNode::SingleMesh sm;
	    sm.meshObjSVs = 0;
	    // @@@ Use REAL priority
	    renderTree.AddRenderMeshToContext (context, rm, 7, sm);
	    
	    portalVerts2d += count;
            portalVerts3d += count;
          }
        }
      }
    }

  private:
    PersistentData& persistentData;
    ContextSetup& contextFunction;

    bool IsSimplePortal (const csFlags& portalFlags)
    {
      return (portalFlags.Get() & (CS_PORTAL_CLIPDEST 
        | CS_PORTAL_CLIPSTRADDLING 
        | CS_PORTAL_ZFILL
	| CS_PORTAL_MIRROR
	| CS_PORTAL_FLOAT)) == 0;
    }
    
    void ComputeVector2BoundingBox (const csVector2* verts, size_t count, 
                                    csBox2& box)
    {
      if (count == 0)
      {
        box.StartBoundingBox ();
        return;
      }
      box.StartBoundingBox (verts[0]);
      for (size_t i = 1; i < count; i++)
        box.AddBoundingVertexSmart (verts[i]);
    }
  
  };


  
  template<typename RenderTreeType>
  class SetupRenderTarget
  {
  public:
    SetupRenderTarget (typename RenderTreeType::ContextsContainer* contexts,
      iGraphics3D* g3d)
    {
      g3d->SetRenderTarget (contexts->renderTarget, false,
        contexts->subtexture);
    }
  };
    
  template<typename RenderTreeType, typename LayerConfigType>
  class ContextRender
  {
  public:
    ContextRender (iShaderManager* shaderManager, 
      const LayerConfigType& layerConfig)
      : shaderManager (shaderManager), layerConfig (layerConfig)
    {
    }
  
    void operator() (typename RenderTreeType::ContextsContainer* contexts, 
      RenderTreeType& tree)
    {
      RenderView* rview = contexts->rview;
      iGraphics3D* g3d = rview->GetGraphics3D ();
      int drawFlags = rview->GetEngine ()->GetBeginDrawFlags ();
      iCamera* cam = rview->GetCamera();
      iClipper2D* clipper = rview->GetClipper ();
      
      drawFlags |= CSDRAW_3DGRAPHICS /*| CSDRAW_CLEARSCREEN*/;

      SetupRenderTarget<RenderTreeType> setupTarget (contexts, g3d);
      g3d->SetPerspectiveCenter (int (cam->GetShiftX ()), 
        int (cam->GetShiftY ()));
      g3d->SetPerspectiveAspect (cam->GetFOV ());
      g3d->SetClipper (clipper, CS_CLIPPER_TOPLEVEL);
      
      BeginFinishDrawScope bd (g3d, drawFlags);

      g3d->SetWorldToCamera (cam->GetTransform ().GetInverse ());

      for (size_t layer = 0; layer < layerConfig.GetLayerCount (); ++layer)
      {
        ContextCB cb (*this, g3d, layer);
        tree.TraverseContextsReverse (contexts, cb);
      }
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

    struct ContextCB
    {
      ContextRender& parent;
      iGraphics3D* g3d;
      size_t layer;

      ContextCB (ContextRender& parent, iGraphics3D* g3d, size_t layer) 
        : parent (parent), g3d (g3d), layer (layer)
      {}

      void operator() (typename RenderTreeType::ContextNode* node, 
        RenderTreeType& tree)
      {
        SimpleRender<RenderTreeType> render (g3d, 
          parent.shaderManager->GetShaderVariableStack (), layer);
    
        MeshNodeCB<SimpleRender<RenderTreeType> > cb (render, node, tree);
        node->meshNodes.TraverseInOrder (cb);
      }
    };

    iShaderManager* shaderManager;
    const LayerConfigType& layerConfig;
  };

} // namespace RenderManager
} // namespace CS

#endif // __CS_CSPLUGINCOMMON_RENDERMANAGER_CONTEXT_H__
