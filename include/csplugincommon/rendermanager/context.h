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
      struct PortalBuffers
      {
        csRef<iRenderBuffer> coordBuf;
        csRef<iRenderBuffer> tcBuf;
        csRef<iRenderBuffer> indexBuf;
        csRef<csRenderBufferHolder> holder;
      };
      struct PortalBufferConstraint
      {
        typedef size_t KeyType;

        static int IsLargerEqual (const PortalBuffers& b1, 
                                  const PortalBuffers& b2)
        {
          size_t s1 = b1.coordBuf->GetElementCount ();
          size_t s2 = b1.coordBuf->GetElementCount ();
          
          if (s1 > s2) return true;
          return false;
        }
      
        static int IsEqual (const PortalBuffers& b1, 
                            const PortalBuffers& b2)
        {
          size_t s1 = b1.coordBuf->GetElementCount ();
          size_t s2 = b1.coordBuf->GetElementCount ();
          
          if (s1 == s2) return true;
          return false;
        }
      
        static int IsLargerEqual(const PortalBuffers& b1, 
                                 const KeyType& s2)
        {
          size_t s1 = b1.coordBuf->GetElementCount ();
          
          if (s1 > s2) return true;
          return false;
        }
      
        static int IsEqual(const PortalBuffers& b1, 
                           const KeyType& s2)
        {
          size_t s1 = b1.coordBuf->GetElementCount ();
          
          if (s1 == s2) return true;
          return false;
        }
      
      };
      CS::Utility::GenericResourceCache<PortalBuffers, csTicks,
        PortalBufferConstraint> bufCache;

      struct csBoxClipperCached : public csBoxClipper
      {
        PersistentData* owningPersistentData;

        csBoxClipperCached (PersistentData* owningPersistentData,
          const csBox2& box) : csBoxClipper (box), 
          owningPersistentData (owningPersistentData)
        { }

        void operator delete (void* p, void* q)
        {
          csBoxClipperCached* bcc = reinterpret_cast<csBoxClipperCached*> (p);
          bcc->owningPersistentData->FreeCachedClipper (bcc);
        }
        void operator delete (void* p)
        {
          csBoxClipperCached* bcc = reinterpret_cast<csBoxClipperCached*> (p);
          bcc->owningPersistentData->FreeCachedClipper (bcc);
        }
      };
      struct csBoxClipperCachedStore
      {
        uint32 bytes[sizeof(csBoxClipperCached)/sizeof(uint32)];
      };
      CS::Utility::GenericResourceCache<csBoxClipperCachedStore, csTicks,
        CS::Utility::ResourceCache::SortingNone,
        CS::Utility::ResourceCache::ReuseConditionFlagged> boxClipperCache;

      void FreeCachedClipper (csBoxClipperCached* bcc)
      {
        CS::Utility::ResourceCache::ReuseConditionFlagged::StoredAuxiliaryInfo* 
          reuseAux = boxClipperCache.GetReuseAuxiliary (
            reinterpret_cast<csBoxClipperCachedStore*> (bcc));
        reuseAux->reusable = true;
      }

      csStringID svNameTexPortal;
    #ifdef CS_DEBUG
      csFrameDataHolder<csStringBase> stringHolder;
    #endif
      
      PersistentData() : texCache (csimg2D, "rgb8", 
        CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS | CS_TEXTURE_CLAMP,
        "target", TextureCache::tcachePowerOfTwo)
      {
        bufCache.agedPurgeInterval = 5000;
        bufCache.purgeAge = 10000;
        boxClipperCache.agedPurgeInterval = 5000;
        boxClipperCache.purgeAge = 10000;
      }
      
      void Initialize (iShaderManager* shmgr, iGraphics3D* g3d)
      {
        svNameTexPortal = 
          shmgr->GetSVNameStringset()->Request ("tex portal");
	texCache.SetG3D (g3d);
      }
      
      void UpdateNewFrame ()
      {
        csTicks time = csGetTicks ();
        texCache.AdvanceFrame (time);
        bufCache.AdvanceTime (time);
        boxClipperCache.AdvanceTime (time);
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
        CS::Graphics::RenderPriority renderPrio = 
          holder.meshWrapper->GetRenderPriority ();

        size_t allPortalVertices = holder.portalContainer->GetTotalVertexCount ();
        allPortalVerts2d.SetSize (allPortalVertices * 3);
        allPortalVerts3d.SetSize (allPortalVertices * 3);
        allPortalVertsNums.SetSize (portalCount);

        csVector2* portalVerts2d = allPortalVerts2d.GetArray();
        csVector3* portalVerts3d = allPortalVerts3d.GetArray();
        /* Get clipped screen space and camera space vertices */
        holder.portalContainer->ComputeScreenPolygons (rview, 
          portalVerts2d, portalVerts3d, 
          allPortalVerts2d.GetSize(), allPortalVertsNums.GetArray());

        for (size_t pi = 0; pi < portalCount; ++pi)
        {
          iPortal* portal = holder.portalContainer->GetPortal (int (pi));
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
              SetupWarp (inewcam, holder.meshWrapper->GetMovable(), portal);
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
            int sb_minX = int (screenBox.MinX());
            int sb_maxY = int (screenBox.MaxY());
	    int txt_w = int (ceil (screenBox.MaxX() - screenBox.MinX()));
	    int txt_h = int (ceil (screenBox.MaxY() - screenBox.MinY()));
	    int real_w, real_h;
	    csRef<iTextureHandle> tex = 
	      persistentData.texCache.QueryUnusedTexture (txt_w, txt_h, 0,
                real_w, real_h);
	          
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
              SetupWarp (inewcam, holder.meshWrapper->GetMovable(), portal);
	    }
	    inewcam->SetPerspectiveCenter (cam->GetShiftX() - sb_minX,
	      real_h - (sb_maxY - cam->GetShiftY()));
            /* Visible cracks can occur on portal borders when the geometry
               behind the portal is supposed to fit seamlessly into geometry
               before the portal since the rendering of the target geometry
               may not exactly line up with the portal area on the portal 
               texture.
               To reduce that effect the camera position in the target sector
               is somewhat fudged to move slightly into the target so that 
               the rendered target sector geometry extends beyond the portal 
               texture area. */
            {
              // - Find portal point with largest Z (pMZ)
              float maxz = 0;
              size_t maxc = 0;
	      for (size_t c = 0; c < count; c++)
	      {
                float z = portalVerts3d[c].z;
                if (z > maxz) 
                {
                  maxz = z;
                  maxc = c;
                }
	      }
              // - Find inverse perspective point of pMZ plus some offset (pMZ2)
              csVector2 p (portalVerts2d[maxc]);
              p.x += 1.5f;
              p.y += 1.5f;
              csVector3 pMZ2 = cam->InvPerspective (p, maxz);
              // - d = distance pMZ, pMZ2
              float d = sqrtf (csSquaredDist::PointPoint (portalVerts3d[maxc], pMZ2));
              // - Get portal target plane in target world space
              csReversibleTransform warp;
	      if (portalFlags.Check (CS_PORTAL_WARP))
                warp = portal->GetWarp();
              csVector3 portalDir (warp.Other2ThisRelative (portal->GetWorldPlane().Normal()));
              /* - Offset target camera into portal direction in target sector,
                   amount of offset 'd' */
              csVector3 camorg (inewcam->GetTransform().GetOrigin());
              camorg += d * portalDir;
              inewcam->GetTransform().SetOrigin (camorg);
            }
	    
	    // Add a new context with the texture as the target
	    // Setup simple portal
	    newRenderView->SetLastPortal (portal);
	    newRenderView->SetPreviousSector (sector);
	    csBox2 clipBox (0, real_h - txt_h, txt_w, real_h);
            csRef<iClipper2D> newView;
            /* @@@ Consider PolyClipper?
               A box has an advantage when the portal tex is rendered 
               distorted: texels from outside the portal area still have a
               good color. May not be the case with a (more exact) poly 
               clipper. */
            PersistentData::csBoxClipperCachedStore* bccstore =
              persistentData.boxClipperCache.Query ();
            if (bccstore == 0)
            {
              PersistentData::csBoxClipperCachedStore dummy;
              bccstore = persistentData.boxClipperCache.AddActive (dummy);
            }
            newView.AttachNew (
              new (bccstore) PersistentData::csBoxClipperCached (
                &persistentData, clipBox));
            newRenderView->SetClipper (newView);

	    typename RenderTreeType::ContextsContainer* targetContexts = 
	      renderTree.CreateContextContainer ();
	    targetContexts->renderTarget = tex;
	    targetContexts->rview = newRenderView;
    
            typename RenderTreeType::ContextNode* portalCtx = 
              renderTree.CreateContext (targetContexts, newRenderView);
  
	    // Setup the new context
            contextFunction(renderTree, portalCtx, targetContexts, 
              portal->GetSector (), newRenderView);
  
	    // Synthesize a render mesh for the portal plane
	    iMaterialWrapper* mat = portal->GetMaterial ();
	    csRef<csShaderVariableContext> svc;
	    svc.AttachNew (new csShaderVariableContext);
	    csRef<csShaderVariable> svTexPortal =
	      svc->GetVariableAdd (persistentData.svNameTexPortal);
	    svTexPortal->SetValue (tex);

            PersistentData::PortalBuffers* bufs = 
              persistentData.bufCache.Query (count);
            if (bufs == 0)
            {
              PersistentData::PortalBuffers newBufs;
	      newBufs.coordBuf = 
	        csRenderBuffer::CreateRenderBuffer (count, CS_BUF_STREAM, CS_BUFCOMP_FLOAT, 3);
	      newBufs.tcBuf =
	        csRenderBuffer::CreateRenderBuffer (count, CS_BUF_STREAM, CS_BUFCOMP_FLOAT, 4);
	      newBufs.indexBuf =
	        csRenderBuffer::CreateIndexRenderBuffer (count, CS_BUF_STREAM, 
		  CS_BUFCOMP_UNSIGNED_INT, 0, count-1);
	      newBufs.holder.AttachNew (new csRenderBufferHolder);
              newBufs.holder->SetRenderBuffer (CS_BUFFER_INDEX, newBufs.indexBuf);
	      newBufs.holder->SetRenderBuffer (CS_BUFFER_POSITION, newBufs.coordBuf);
	      newBufs.holder->SetRenderBuffer (CS_BUFFER_TEXCOORD0, newBufs.tcBuf);
              bufs = persistentData.bufCache.AddActive (newBufs);
            }
	    
	    {
	      csRenderBufferLock<csVector3> coords (bufs->coordBuf);
	      for (size_t c = 0; c < count; c++)
	      {
		coords[c].Set (portalVerts3d[c]);
	      }
	    }
	    {
	      float xscale = 1.0f / real_w;
	      float yscale = 1.0f / real_h;
	      csRenderBufferLock<csVector4> tcoords (bufs->tcBuf);
	      for (size_t c = 0; c < count; c++)
	      {
	        float z = portalVerts3d[c].z;
		tcoords[c].Set ((portalVerts2d[c].x - sb_minX) * xscale * z, 
		  (sb_maxY - portalVerts2d[c].y) * yscale * z, 0, 
		  z);
	      }
	    }
	    {
	      csRenderBufferLock<uint> indices (bufs->indexBuf);
	      for (size_t c = 0; c < count; c++)
	      {
	        *indices++ = uint (c);
	      }
	    }
    
	    bool meshCreated;
	    csRenderMesh* rm = 
	      renderTree.GetPersistentData().rmHolder.GetUnusedMesh (
	        meshCreated, rview->GetCurrentFrameNumber());
          #ifdef CS_DEBUG
            bool created;
            csStringBase& nameStr = persistentData.stringHolder.GetUnusedData (
              created,  rview->GetCurrentFrameNumber());
            nameStr.Format ("[portal from %s to %s]",
              rview->GetThisSector()->QueryObject()->GetName(),
              portal->GetSector()->QueryObject()->GetName());
	    rm->db_mesh_name = nameStr;
          #else
	    rm->db_mesh_name = "[portal]";
          #endif
	    rm->material = mat;
	    rm->meshtype = CS_MESHTYPE_TRIANGLEFAN;
	    rm->buffers = bufs->holder;
	    rm->z_buf_mode = CS_ZBUF_USE;
	    rm->object2world = rview->GetCamera()->GetTransform();
	    rm->indexstart = 0;
	    rm->indexend = uint (count);
	    rm->mixmode = CS_MIXMODE_BLEND(ONE, ZERO);
	    rm->variablecontext = svc;
	    
	    typename RenderTreeType::MeshNode::SingleMesh sm;
	    sm.meshObjSVs = 0;
	    renderTree.AddRenderMeshToContext (context, rm, renderPrio, sm);
	    
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

    void SetupWarp (iCamera* inewcam, iMovable* movable, iPortal* portal)
    {
      const csReversibleTransform& movtrans = movable->GetFullTransform();
      bool mirror = inewcam->IsMirrored ();
      csReversibleTransform warp_wor;
      portal->ObjectToWorld (movtrans, warp_wor);
      portal->WarpSpace (warp_wor, inewcam->GetTransform (), mirror);
      inewcam->SetMirrored (mirror);
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
