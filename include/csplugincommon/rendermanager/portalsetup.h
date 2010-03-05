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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_PORTALSETUP_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_PORTALSETUP_H__

/**\file
 * Render manager portal setup.
 */

#include "iengine/movable.h"
#include "iengine/portal.h"
#include "iengine/portalcontainer.h"
#include "iengine/sector.h"
#include "csgeom/math3d.h"
#include "csgeom/polyclip.h"
#include "csgfx/renderbuffer.h"
#include "csgfx/shadervarcontext.h"
#include "cstool/rbuflock.h"

#include "csplugincommon/rendermanager/renderview.h"
#include "csplugincommon/rendermanager/svsetup.h"
#include "csplugincommon/rendermanager/texturecache.h"

namespace CS
{
namespace RenderManager
{
  /**
   * Base class for StandardPortalSetup, containing types and
   * members which are independent of the template arguments that can be
   * provided to StandardPortalSetup.
   */
  class StandardPortalSetup_Base
  {
  public:
    /**
      * Data used by the helper that needs to persist over multiple frames.
      * Render managers must store an instance of this class and provide
      * it to the helper upon instantiation.
      */
    struct CS_CRYSTALSPACE_EXPORT PersistentData
    {

      /**
       * Cache structure for portal render buffers
       */
      struct PortalBuffers
      {
        csRef<iRenderBuffer> coordBuf;
        csRef<iRenderBuffer> tcBuf;
        csRef<iRenderBuffer> indexBuf;
        csRef<csRenderBufferHolder> holder;
      };

      /**
       * GenericCache-constraints for PortalBuffer caching
       */
      struct CS_CRYSTALSPACE_EXPORT PortalBufferConstraint
      {
        typedef size_t KeyType;

        static bool IsEqual (const PortalBuffers& b1,
                             const PortalBuffers& b2);
        static bool IsLargerEqual (const PortalBuffers& b1,
                                   const PortalBuffers& b2);
			     
        static bool IsEqual (const PortalBuffers& b1,
                            const KeyType& s2);
        static bool IsLargerEqual (const PortalBuffers& b1,
                                  const KeyType& s2);
        static bool IsLargerEqual (const KeyType& s1,
                                   const PortalBuffers& b2);
      };
      CS::Utility::GenericResourceCache<PortalBuffers, csTicks,
        PortalBufferConstraint> bufCache;

      /**
       * Cache-helper for box clipper caching
       */
      struct CS_CRYSTALSPACE_EXPORT csBoxClipperCached : public csBoxClipper
      {
        PersistentData* owningPersistentData;

        csBoxClipperCached (PersistentData* owningPersistentData,
          const csBox2& box) : csBoxClipper (box),
          owningPersistentData (owningPersistentData)
        { }

        void operator delete (void* p, void* q);
        void operator delete (void* p);
      };
      struct csBoxClipperCachedStore
      {
        uint bytes[(sizeof(csBoxClipperCached) + sizeof (uint) - 1)/sizeof(uint)];
	
	csBoxClipperCachedStore()
	{ 
	  // Avoid gcc complaining about uninitialised use
	  memset (bytes, 0, sizeof (bytes));
	}
      };
      CS::Utility::GenericResourceCache<csBoxClipperCachedStore, csTicks,
        CS::Utility::ResourceCache::SortingNone,
        CS::Utility::ResourceCache::ReuseConditionFlagged> boxClipperCache;

      void FreeCachedClipper (csBoxClipperCached* bcc);

      CS::ShaderVarStringID svNameTexPortal;
    #ifdef CS_DEBUG
      csFrameDataHolder<csStringBase> stringHolder;
    #endif

      TextureCache texCache;
      
      uint dbgDrawPortalOutlines;
      uint dbgDrawPortalPlanes;
      uint dbgShowPortalTextures;

      /// Construct helper
      PersistentData();

      /**
       * Initialize helper. Fetches various required values from objects in
       * the object registry.
       */
      void Initialize (iShaderManager* shmgr, iGraphics3D* g3d,
                       RenderTreeBase::DebugPersistent& dbgPersist);

      /**
       * Do per-frame house keeping - \b MUST be called every frame/
       * RenderView() execution.
       */
      void UpdateNewFrame ()
      {
        csTicks time = csGetTicks ();
        texCache.AdvanceFrame (time);
        bufCache.AdvanceTime (time);
        boxClipperCache.AdvanceTime (time);
      }
    };
  
    StandardPortalSetup_Base (PersistentData& persistentData)
      : persistentData (persistentData)
    {}
  protected:
    PersistentData& persistentData;
  };

  /**
   * Standard setup functor for portals.
   * Iterates over all portals in a context and sets up new contexts to
   * render the part of the scene "behind" the portal. Depending on the
   * settings of a portal, it is either rendered to the same target as the
   * context or a new texture (in which case the original context is
   * augmented with a mesh rendering that texture).
   *
   * Usage: instiate. Application after the visible meshes were determined,
   * but before mesh sorting.
   * Example:
   * \code
   *  // Type for portal setup
   *  typedef CS::RenderManager::StandardPortalSetup<RenderTreeType,
   *    ContextSetupType> PortalSetupType;
   *
   * // Assumes there is an argument 'PortalSetupType::ContextSetupData& portalSetupData'
   * // to the current method, taking data from the previous portal (if any)
   *
   * // Keep track of the portal recursions to avoid infinite portal recursions
   * if (recurseCount > 30) return;
   *
   * // Set up all portals
   * {
   *   recurseCount++;
   *   // Data needed to be passed between portal setup steps
   *   PortalSetupType portalSetup (rmanager->portalPersistent, *this);
   *   // Actual setup
   *   portalSetup (context, portalSetupData);
   *   recurseCount--;
   * }
   * \endcode
   *
   * The template parameter \a RenderTree gives the render tree type.
   * The parameter \a ContextSetup gives a class used to set up the contexts
   * for the rendering of the scene behind a portal. It must provide an
   * implementation of operator() (RenderTree::ContextNode& context,
   *   PortalSetupType::ContextSetupData& portalSetupData).
   *
   * \par Internal workings
   * The standard setup will classify portals into simple and heavy portals
   * respectively where simple portals can be rendered directly without clipping
   * while heavy portals requires render-to-texture.
   */
  template<typename RenderTreeType, typename ContextSetup>
  class StandardPortalSetup : public StandardPortalSetup_Base
  {
  public:
    typedef StandardPortalSetup<RenderTreeType, ContextSetup> ThisType;

    /**
     * Data that needs to be passed between portal setup steps by the
     * context setup function.
     */
    struct ContextSetupData
    {
      typename RenderTreeType::ContextNode* lastSimplePortalCtx;

      /// Construct, defaulting to no previous portal been rendered.
      ContextSetupData (typename RenderTreeType::ContextNode* last = 0)
        : lastSimplePortalCtx (last)
      {}
    };

    /// Constructor.
    StandardPortalSetup (PersistentData& persistentData, ContextSetup& cfun)
      : StandardPortalSetup_Base (persistentData), contextFunction (cfun)
    {}

    /**
     * Operator doing the actual work. Goes over the portals in the given
     * context and sets up rendering of behind the portals.
     */
    void operator() (typename RenderTreeType::ContextNode& context,
      ContextSetupData& setupData)
    {
      RenderView* rview = context.renderView;
      RenderTreeType& renderTree = context.owner;
      int screenW, screenH;
      if (!context.GetTargetDimensions (screenW, screenH))
      {
        screenW = rview->GetGraphics3D()->GetWidth();
        screenH = rview->GetGraphics3D()->GetHeight();
      }

      bool debugDraw =
	renderTree.IsDebugFlagEnabled (persistentData.dbgDrawPortalOutlines)
	|| renderTree.IsDebugFlagEnabled (persistentData.dbgDrawPortalPlanes);

      csDirtyAccessArray<csVector2> allPortalVerts2d (64);
      csDirtyAccessArray<csVector3> allPortalVerts3d (64);
      csDirtyAccessArray<size_t> allPortalVertsNums;
      // Handle all portals
      for (size_t pc = 0; pc < context.allPortals.GetSize (); ++pc)
      {
        typename RenderTreeType::ContextNode::PortalHolder& holder = context.allPortals[pc];
        const size_t portalCount = holder.portalContainer->GetPortalCount ();

        size_t allPortalVertices = holder.portalContainer->GetTotalVertexCount ();
        allPortalVerts2d.SetSize (allPortalVertices * 3);
        allPortalVerts3d.SetSize (allPortalVertices * 3);
        allPortalVertsNums.SetSize (portalCount);

        csVector2* portalVerts2d = allPortalVerts2d.GetArray();
        csVector3* portalVerts3d = allPortalVerts3d.GetArray();
        /* Get clipped screen space and camera space vertices */
        holder.portalContainer->ComputeScreenPolygons (rview,
          portalVerts2d, portalVerts3d,
          allPortalVerts2d.GetSize(), allPortalVertsNums.GetArray(),
          screenW, screenH);
	
        for (size_t pi = 0; pi < portalCount; ++pi)
        {
          iPortal* portal = holder.portalContainer->GetPortal (int (pi));
          const csFlags portalFlags (portal->GetFlags());

          // Finish up the sector
          if (!portal->CompleteSector (rview))
            continue;
	  
          size_t count = allPortalVertsNums[pi];
          if (count == 0) continue;
	  
	  iSector* sector = portal->GetSector ();
	  bool skipRec = sector->GetRecLevel() >= portal->GetMaximumSectorVisit();

	  if (debugDraw)
	  {
	    bool isSimple = IsSimplePortal (portalFlags);
	    if (renderTree.IsDebugFlagEnabled (persistentData.dbgDrawPortalOutlines))
	    {
	      for (size_t i = 0; i < count; i++)
	      {
		size_t next = (i+1)%count;
		csVector2 v1 (portalVerts2d[i]);
		csVector2 v2 (portalVerts2d[next]);
		v1.y = screenH - v1.y;
		v2.y = screenH - v2.y;
		renderTree.AddDebugLineScreen (v1, v2,
		  isSimple ? csRGBcolor (0, 255, int(skipRec) * 255)
				: csRGBcolor (255, 0, int (skipRec) * 255));
	      }
	    }
	    if (renderTree.IsDebugFlagEnabled (persistentData.dbgDrawPortalPlanes))
	    {
	      csVector3 guessedCenter (0);
	      const csVector3* pv = portal->GetWorldVertices();
	      const int* pvi = portal->GetVertexIndices();
	      size_t numOrgVerts = portal->GetVertexIndicesCount ();
	      for (size_t n = 0; n < numOrgVerts; n++)
	      {
		guessedCenter += pv[pvi[n]];
	      }
	      guessedCenter /= numOrgVerts;
	      csTransform identity;
	      renderTree.AddDebugPlane (portal->GetWorldPlane(), identity,
		isSimple ? csColor (0, 1, int (skipRec)) : csColor (1, 0, int (skipRec)),
		guessedCenter);
	    }
	  }
	  
	  if (!skipRec)
	  {
	    sector->IncRecLevel();
	    if (IsSimplePortal (portalFlags))
	    {
	      SetupSimplePortal (context, setupData, portal, sector,
		  portalVerts2d, count, screenW, screenH, holder);
	    }
	    else
	    {
	      SetupHeavyPortal (context, setupData, portal, sector,
		  portalVerts2d, portalVerts3d, count, screenW, screenH, holder);
	    }
	    sector->DecRecLevel();
	  }

	  portalVerts2d += count;
          portalVerts3d += count;
        }
      }
    }

  private:
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

    void SetupSimplePortal (
      typename RenderTreeType::ContextNode& context,
      ContextSetupData& setupData, iPortal* portal, iSector* sector,
      csVector2* portalVerts2d, size_t count,
      int screenW, int screenH,
      typename RenderTreeType::ContextNode::PortalHolder& holder)
    {
      RenderView* rview = context.renderView;
      RenderTreeType& renderTree = context.owner;
      const csFlags portalFlags (portal->GetFlags());

      // Setup simple portal
      rview->CreateRenderContext ();
      rview->SetLastPortal (portal);
      rview->SetPreviousSector (rview->GetThisSector ());
      rview->SetThisSector (sector);
      csPolygonClipper newView (portalVerts2d, count);
      rview->SetViewDimensions (screenW, screenH);
      rview->SetClipper (&newView);

      if (portalFlags.Check (CS_PORTAL_WARP))
      {
	iCamera *inewcam = rview->CreateNewCamera ();
        SetupWarp (inewcam, holder.meshWrapper->GetMovable(), portal);
      }
	
      typename RenderTreeType::ContextNode* portalCtx =
              renderTree.CreateContext (rview, setupData.lastSimplePortalCtx);
      setupData.lastSimplePortalCtx = portalCtx;

      // Copy the target from last portal
      for (int a = 0; a < rtaNumAttachments; a++)
        portalCtx->renderTargets[a] = context.renderTargets[a];
      portalCtx->perspectiveFixup = context.perspectiveFixup;

      // Setup the new context
      contextFunction (*portalCtx, setupData);

      rview->RestoreRenderContext ();
    }

    void SetupHeavyPortal (
      typename RenderTreeType::ContextNode& context,
      ContextSetupData& setupData, iPortal* portal, iSector* sector,
      csVector2* portalVerts2d, csVector3* portalVerts3d, size_t count,
      int screenW, int screenH,
      typename RenderTreeType::ContextNode::PortalHolder& holder)
    {
      RenderView* rview = context.renderView;
      RenderTreeType& renderTree = context.owner;
      const csFlags portalFlags (portal->GetFlags());

      // Setup a bounding box, in screen-space
      csBox2 screenBox;
      ComputeVector2BoundingBox (portalVerts2d, count, screenBox);
      
      // Obtain a texture handle for the portal to render to
      int sb_minX = int (screenBox.MinX());
      int sb_minY = int (screenBox.MinY());
      int txt_w = int (ceil (screenBox.MaxX() - screenBox.MinX()));
      int txt_h = int (ceil (screenBox.MaxY() - screenBox.MinY()));
      int real_w, real_h;
      csRef<iTextureHandle> tex = persistentData.texCache.QueryUnusedTexture (txt_w, txt_h,
		  real_w, real_h);
		  
      if (renderTree.IsDebugFlagEnabled (persistentData.dbgShowPortalTextures))
        renderTree.AddDebugTexture (tex, (float)real_w/(float)real_h);
		
      iCamera* cam = rview->GetCamera();
      // Create a new view
      csRef<CS::RenderManager::RenderView> newRenderView;
      csRef<iCustomMatrixCamera> newCam (rview->GetEngine()->CreateCustomMatrixCamera (cam));
      iCamera* inewcam = newCam->GetCamera();
#include "csutil/custom_new_disable.h"
      newRenderView.AttachNew (new (renderTree.GetPersistentData().renderViewPool) RenderView (
		  inewcam, 0, rview->GetGraphics3D(), rview->GetGraphics2D()));
#include "csutil/custom_new_enable.h"
      newRenderView->SetEngine (rview->GetEngine ());
	
      if (portalFlags.Check (CS_PORTAL_WARP))
      {
	SetupWarp (inewcam, holder.meshWrapper->GetMovable(), portal);
      }

      {
        /* Shift projection.
        
           What we want is to render the sector behind the portal, as seen
           from the current camera's viewpoint. The portal has a bounding
           rectangle. For reasons of efficiency we only want to render this
           rectangular region of the view. Also, the texture is only large
           enough to cover this rectangle, so the view needs to be offset
           to align the upper left corner of the portal rectangle with the
           upper left corner of the render target texture. This can be done
           by manipulating the projection matrix.
           
           The matrix 'projShift' below does:
           - Transform projection matrix from normalized space to screen
             space.
           - Translate the post-projection space so the upper left corner
             of the rectangle covering the the portal is in the upper left
             corner of the render target.
           - Transform projection matrix from (now) render target space to
             normalized space.
           (Each step can be represented as a matrix, and the matrix below
           is just the result of pre-concatenating these matrices.)
         */
      
	float irw = 1.0f/real_w;
	float irh = 1.0f/real_h;
	CS::Math::Matrix4 projShift (
	  screenW*irw, 0, 0, irw * (screenW-2*sb_minX) - 1,
	  0, screenH*irh, 0, irh * (screenH+2*((real_h - txt_h) - sb_minY)) - 1,
	  0, 0, 1, 0,
	  0, 0, 0, 1);
	
	newCam->SetProjectionMatrix (projShift * inewcam->GetProjectionMatrix());
      }
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
	csVector4 zToPostProject (0, 0, maxz, 1.0f);
	zToPostProject = cam->GetProjectionMatrix() * zToPostProject;
	
	const CS::Math::Matrix4& inverseProj (cam->GetInvProjectionMatrix());
	csVector2 p (portalVerts2d[maxc]);
	p.x += 1.5f;
	p.y += 1.5f;
	p.x /= 0.5f*screenW;
	p.y /= 0.5f*screenH;
	p.x -= 1;
	p.y -= 1;
	csVector4 p_proj (p.x*zToPostProject.w, p.y*zToPostProject.w,
	  zToPostProject.z, zToPostProject.w);
	csVector4 p_proj_inv = inverseProj * p_proj;
	csVector3 pMZ2 (p_proj_inv[0], p_proj_inv[1], p_proj_inv[2]);
	// - d = distance pMZ, pMZ2
	float d = sqrtf (csSquaredDist::PointPoint (portalVerts3d[maxc], pMZ2));
	// - Get portal target plane in target world space
	csVector3 portalDir;
	if (portalFlags.Check (CS_PORTAL_WARP))
	{
	  portalDir = portal->GetWarp().Other2ThisRelative (
	    portal->GetWorldPlane().Normal());
	}
	else
	  portalDir = portal->GetWorldPlane().Normal();
	/* - Offset target camera into portal direction in target sector,
	     amount of offset 'd' */
	csVector3 camorg (inewcam->GetTransform().GetOrigin());
	camorg += d * portalDir;
	inewcam->GetTransform().SetOrigin (camorg);
      }
	
      // Add a new context with the texture as the target
      // Setup simple portal
      newRenderView->SetLastPortal (portal);
      newRenderView->SetPreviousSector (rview->GetThisSector ());
      newRenderView->SetThisSector (sector);
      newRenderView->SetViewDimensions (real_w, real_h);
      /* @@@ FIXME Without the +1 pixels of the portal stay unchanged upon
       * rendering */
      csBox2 clipBox (0, real_h - (txt_h+1), txt_w+1, real_h);
      csRef<iClipper2D> newView;
      /* @@@ Consider PolyClipper?
	A box has an advantage when the portal tex is rendered
	distorted: texels from outside the portal area still have a
	good color. May not be the case with a (more exact) poly
	clipper. */
      typename ThisType::PersistentData::csBoxClipperCachedStore* bccstore;
      bccstore = persistentData.boxClipperCache.Query ();
      if (bccstore == 0)
      {
	typename ThisType::PersistentData::csBoxClipperCachedStore dummy;
	bccstore = persistentData.boxClipperCache.AddActive (dummy);
      }
#include "csutil/custom_new_disable.h"
      newView.AttachNew (new (bccstore) typename ThisType::PersistentData::csBoxClipperCached (
		  &persistentData, clipBox));
#include "csutil/custom_new_enable.h"
      newRenderView->SetClipper (newView);

      typename RenderTreeType::ContextNode* portalCtx =
		renderTree.CreateContext (newRenderView);
      portalCtx->renderTargets[rtaColor0].texHandle = tex;

      // Setup the new context
      ContextSetupData newSetup (portalCtx);
      contextFunction (*portalCtx, newSetup);

      // Synthesize a render mesh for the portal plane
      iMaterialWrapper* mat = portal->GetMaterial ();
      csRef<csShaderVariableContext> svc;
      svc.AttachNew (new csShaderVariableContext);
      csRef<csShaderVariable> svTexPortal =
		svc->GetVariableAdd (persistentData.svNameTexPortal);
      svTexPortal->SetValue (tex);

      typename ThisType::PersistentData::PortalBuffers* bufs =
	persistentData.bufCache.Query (count);
	
      if (bufs == 0)
      {
	typename ThisType::PersistentData::PortalBuffers newBufs;
	newBufs.coordBuf = csRenderBuffer::CreateRenderBuffer (count, CS_BUF_STREAM, CS_BUFCOMP_FLOAT, 3);
	newBufs.tcBuf = csRenderBuffer::CreateRenderBuffer (count, CS_BUF_STREAM, CS_BUFCOMP_FLOAT, 4);
	newBufs.indexBuf = csRenderBuffer::CreateIndexRenderBuffer (count, CS_BUF_STREAM,
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
	float xscale = (float)1.0f/(float)real_w;
	float yscale = (float)1.0f/(float)real_h;
	csRenderBufferLock<csVector4> tcoords (bufs->tcBuf);
	for (size_t c = 0; c < count; c++)
	{
	  float z = portalVerts3d[c].z;
	  const csVector2& p2 = portalVerts2d[c];
	  tcoords[c].Set ((p2.x - sb_minX) * xscale * z,
	    (txt_h - (p2.y - sb_minY)) * yscale * z, 0, z);
	}
      }
      {
	csRenderBufferLock<uint> indices (bufs->indexBuf);
	for (size_t c = 0; c < count; c++)
	  *indices++ = uint (c);
      }

      bool meshCreated;
      csRenderMesh* rm = renderTree.GetPersistentData().rmHolder.GetUnusedMesh (
		  meshCreated, rview->GetCurrentFrameNumber());
#ifdef CS_DEBUG
      bool created;
      csStringBase& nameStr = persistentData.stringHolder.GetUnusedData (
		created,  rview->GetCurrentFrameNumber());
      nameStr.Format ("[portal from %s to %s]",
		rview->GetThisSector()->QueryObject()->GetName(),
		sector->QueryObject()->GetName());
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

      CS::Graphics::RenderPriority renderPrio =
          holder.meshWrapper->GetRenderPriority ();
      context.AddRenderMesh (rm, renderPrio, sm);
    }

  };

} // namespace RenderManager
} // namespace CS

#endif // __CS_CSPLUGINCOMMON_RENDERMANAGER_CONTEXT_H__

