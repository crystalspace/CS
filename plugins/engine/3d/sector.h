/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein
              (C) 2004 by Marten Svanfeldt

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

#ifndef __CS_SECTOR_H__
#define __CS_SECTOR_H__

#include "csgeom/math3d.h"
#include "csutil/csobject.h"
#include "csutil/nobjvec.h"
#include "csutil/refarr.h"
#include "csutil/cscolor.h"
#include "csutil/array.h"
#include "csutil/hash.h"
#include "plugins/engine/3d/light.h"
#include "plugins/engine/3d/meshobj.h"
#include "plugins/engine/3d/rdrprior.h"
#include "iengine/sector.h"
#include "ivideo/graph3d.h"
#include "csutil/array.h"
#include "ivideo/rndbuf.h"
#include "ivideo/shader/shader.h"
#include "iengine/viscull.h"
#include "iengine/portalcontainer.h"

#include "cstool/rendermeshlist.h"

class csEngine;
class csProgressPulse;
class csSector;
class csMeshWrapper;
class csKDTree;
struct iVisibilityCuller;
struct iRenderView;
struct iMeshWrapper;
struct iFrustumView;

/// A list of lights for a sector.
class csSectorLightList : public csLightList
{
private:
  csSector* sector;
  csKDTree* kdtree; // kdtree to help find lights faster.

public:
  /// constructor
  csSectorLightList ();
  /// destructor
  virtual ~csSectorLightList ();
  /// Set the sector.
  void SetSector (csSector* s) { sector = s; }

  /// Override PrepareLight
  virtual void PrepareLight (iLight* light);
  /// Override FreeLight
  virtual void FreeLight (iLight* item);

  /// Get the kdtree for this light list.
  csKDTree* GetLightKDTree () const { return kdtree; }
};

/// A list of meshes for a sector.
class csSectorMeshList : public csMeshList
{
private:
  csSector* sector;

public:
  /// constructor
  csSectorMeshList ();
  /// destructor
  virtual ~csSectorMeshList () { RemoveAll (); }
  /// Set the sector.
  void SetSector (csSector* sec) { sector = sec; }

  /// Override PrepareMesh
  virtual void PrepareMesh (iMeshWrapper* item);
  /// Override FreeMesh
  virtual void FreeMesh (iMeshWrapper* item);
};

SCF_VERSION (csSector, 0, 0, 2);

/**
 * A sector is a container for objects. It is one of
 * the base classes for the portal engine.
 */
class csSector : public csObject
{
private:
  /**
   * List of meshes in this sector. Note that meshes also
   * need to be in the engine list. This vector contains objects
   * of type iMeshWrapper*.
   */
  csSectorMeshList meshes;

  /**
   * List of camera meshes (meshes with CS_ENTITY_CAMERA flag set).
   */
  csArray<iMeshWrapper*> camera_meshes;

  /**
   * List of meshes that have portals that leave from this sector.
   */
  csSet<iMeshWrapper*> portal_meshes;

  /**
   * The same meshes above but each mesh in their own render priority
   * queue. This is a vector of vectors.
   */
  csRenderQueueSet RenderQueues;

  /**
   * List of sector callbacks.
   */
  csRefArray<iSectorCallback> sector_cb_vector;

  /**
   * List of sector mesh callbacks.
   */
  csRefArray<iSectorMeshCallback> sector_mesh_cb_vector;

  /**
   * All static and pseudo-dynamic lights in this sector.
   * This vector contains objects of type iLight*.
   */
  csSectorLightList lights;

  /**
   * This color stores the most recently set dynamic
   * ambient color.
   */
  csColor dynamic_ambient_color;

  /// Engine handle.
  csEngine* engine;

  /// Optional renderloop.
  iRenderLoop* renderloop;

  /// Fog information.
  csFog fog;

  /**
   * The visibility culler for this sector or 0 if none.
   * In future we should support more than one visibility culler probably.
   */
  csRef<iVisibilityCuller> culler;

  /// Caching of visible meshes
  struct visibleMeshCacheHolder
  {
    csRenderMeshList *meshList;

    // We consider visibility result to be the same if
    // the frame number and context id are the same.
    // The context_id is stored in csRenderContext and
    // is modified whenever a new csRenderContext is created.
    uint32 cachedFrameNumber;
    uint32 cached_context_id;

    visibleMeshCacheHolder() : meshList(0) {}
    ~visibleMeshCacheHolder()
    {
      //delete meshList;
    }
  };

  csArray<visibleMeshCacheHolder> visibleMeshCache;
  csPDelArray<csRenderMeshList> usedMeshLists;


private:
  /**
   * Destroy this sector. All things in this sector are also destroyed.
   * Meshes are unlinked from the sector but not removed because they
   * could be in other sectors.
   */
  virtual ~csSector ();

public:
  /**
  * Visibilty number for last VisTest call
  */
  uint32 current_visnr;

  /**
   * Configuration variable: number of allowed reflections for static lighting.
   * This option controls how many time a given sector may be visited by the
   * same beam of light. When this value is 1 it means that light is not
   * reflected.
   */
  static int cfg_reflections;

  /**
   * Option variable: do pseudo-radiosity?
   * When pseudo-radiosity is enabled every polygon behaves as if
   * it is a mirroring portal when lighting calculations are concerned.
   * This simulates radiosity because light reflects from every surface.
   * The number of reflections allowed is controlled by cfg_reflections.
   */
  static bool do_radiosity;

  /**
   * How many times are we busy drawing this sector (recursive).
   * This is an important variable as it indicates to
   * 'new_transformation' which set of camera vertices it should
   * use.
   */
  int draw_busy;

public:
  /**
   * Construct a sector. This sector will be completely empty.
   */
  csSector (csEngine*);

  /// Set the renderloop for this sector.
  void SetRenderLoop (iRenderLoop* rl) { renderloop = rl; }
  /// Get the renderloop for this sector (or 0 in case of default).
  iRenderLoop* GetRenderLoop () { return renderloop; }

  /**
   * Unlink all meshes from this sector. WARNING! This function may
   * cause virtual function calls to happen on this sector so don't
   * call it from the csSector destructor! It should only be called
   * from csSectorList::FreeSector()!.
   */
  void UnlinkObjects ();

  //----------------------------------------------------------------------
  // Mesh manipulation functions
  //----------------------------------------------------------------------

  iMeshList* GetMeshes ()
    { return &meshes; }

  /// Get render queues (for rendering priorities).
  csRenderQueueSet& GetRenderQueues () { return RenderQueues; }

  /**
   * Register a mesh and all children to the visibility culler.
   */
  void RegisterEntireMeshToCuller (iMeshWrapper* mesh);

  /**
   * Register a mesh (without children) to the visibility culler.
   */
  void RegisterMeshToCuller (iMeshWrapper* mesh);

  /**
   * Unregister a mesh (without children) from the visibility culler.
   */
  void UnregisterMeshToCuller (iMeshWrapper* mesh);

  /**
   * Prepare a mesh for rendering. This function is called for all meshes that
   * are added to the sector.
   */
  void PrepareMesh (iMeshWrapper* mesh);

  /**
   * Unprepare a mesh. This function is called for all meshes that
   * are removed from the sector.
   */
  void UnprepareMesh (iMeshWrapper* mesh);

  /**
   * Relink a mesh from this sector. This is mainly useful if
   * characterics of the mesh changed (like render priority) so
   * that the sector needs to know this.
   */
  void RelinkMesh (iMeshWrapper* mesh);

  //----------------------------------------------------------------------
  // Light manipulation functions
  //----------------------------------------------------------------------

  /**
   * Get the list of lights in this sector.
   */
  iLightList* GetLights ()
    { return &lights; }

  //----------------------------------------------------------------------
  // Callbacks
  //----------------------------------------------------------------------
  void SetSectorCallback (iSectorCallback* cb)
  {
    sector_cb_vector.Push (cb);
  }

  void RemoveSectorCallback (iSectorCallback* cb)
  {
    sector_cb_vector.Delete (cb);
  }

  int GetSectorCallbackCount () const
  {
    return (int)sector_cb_vector.Length ();
  }

  iSectorCallback* GetSectorCallback (int idx) const
  {
    return sector_cb_vector.Get (idx);
  }

  void AddSectorMeshCallback (iSectorMeshCallback* cb);
  void RemoveSectorMeshCallback (iSectorMeshCallback* cb);
  void FireNewMesh (iMeshWrapper* mesh);
  void FireRemoveMesh (iMeshWrapper* mesh);

  //----------------------------------------------------------------------
  // Visibility Stuff
  //----------------------------------------------------------------------

  /**
   * Use the given plugin as a visibility culler (should implement
   * iVisibilityCuller). Returns false if the culler could not be
   * loaded for some reason.
   */
  bool UseCullerPlugin (const char* plugname, iDocumentNode* culler_params = 0);

  /**
   * Get the visibility culler that is used for this sector.
   * 0 if none.
   */
  iVisibilityCuller* GetVisibilityCuller ();

  /**
   * Get a set of visible meshes for given camera. These will be cached for
   * a given frame and camera, but if the cached result isn't enough it will
   * be reculled. The returned pointer is valid as long as the sector exsist
   * (the sector will delete it)
   */
  csRenderMeshList* GetVisibleMeshes (iRenderView *);

  //----------------------------------------------------------------------
  // Drawing
  //----------------------------------------------------------------------

  /**
   * Prepare this sector for drawing.
   */
  void PrepareDraw (iRenderView* rview);

  /**
   * Draw the sector in the given view and with the given transformation.
   */
  void Draw (iRenderView* rview);

  //----------------------------------------------------------------------
  // Utility Functions
  //----------------------------------------------------------------------

  /**
   * Follow a beam from start to end and return the first polygon that
   * is hit. This function correctly traverse portals and space warping
   * portals. Normally the sector you call this on should be the sector
   * containing the 'start' point. 'isect' will be the intersection point
   * if a polygon is returned.
   */
  iMeshWrapper* HitBeamPortals (const csVector3& start, const csVector3& end,
    csVector3& isect, int* polygon_idx);

  /**
   * Follow a beam from start to end and return the first object
   * that is hit. For some meshes the polygonPtr field will be
   * filled with the polygon index that was hit.
   * If polygonPtr is null then the polygon will not be filled in.
   */
  iMeshWrapper* HitBeam (const csVector3& start, const csVector3& end,
    csVector3& intersect, int* polygonPtr, bool accurate = false);

  /**
   * Check visibility in a frustum way for all things and polygons in
   * this sector and possibly traverse through portals to other sectors.
   * This version doesn't init the 2D culler cube so it can be used
   * for recursing.
   */
  void RealCheckFrustum (iFrustumView* lview);

  /**
   * Check visibility in a frustum way for all things and polygons in
   * this sector and possibly traverse through portals to other sectors.
   */
  void CheckFrustum (iFrustumView* lview);

  /**
   * Follow a segment starting at this sector. If the segment intersects
   * with a polygon it will stop there unless the polygon is a portal in which
   * case it will recursively go to that sector (possibly applying warping
   * transformations) and continue there.<p>
   *
   * This routine will modify all the given parameters to reflect space warping.
   * These should be used as the new camera transformation when you decide to
   * really go to the new position.<p>
   *
   * This function returns the resulting sector and new_position will be set
   * to the last position that you can go to before hitting a wall.<p>
   *
   * If only_portals is true then only portals will be checked. This
   * means that intersection with normal polygons is not checked. This
   * is a lot faster but it does mean that you need to use another
   * collision detection system to test with walls.
   */
  iSector* FollowSegment (csReversibleTransform& t, csVector3& new_position,
                          bool& mirror, bool only_portals = false);

  /**
   * Intersect world-space segment with polygons of this sector. Return
   * polygon it intersects with (or 0) and the intersection point
   * in world coordinates.<p>
   *
   * If 'pr' != 0 it will also return a value between 0 and 1
   * indicating where on the 'start'-'end' vector the intersection
   * happened.<p>
   *
   * If 'only_portals' == true only portals are checked.<p>
   *
   * If 'mesh' != 0 the mesh will be filled in.
   */
  int IntersectSegment (const csVector3& start,
	const csVector3& end, csVector3& isect,
	float* pr = 0, bool only_portals = false,
	iMeshWrapper** p_mesh = 0);

  /**
   * Calculate the bounding box of all objects in this sector.
   * This function is not very efficient as it will traverse all objects
   * in the sector one by one and compute a bounding box from that.
   */
  void CalculateSectorBBox (csBox3& bbox, bool do_meshes) const;

  //------------------------------------------------
  // Everything for setting up the lighting system.
  //------------------------------------------------

  /**
   * The whole setup starts with csEngine::shine_lights calling
   * csSector::shine_lights for every sector in the engine.
   * This function will call csLight::shine_lightmaps for every
   * light in the sector.
   * csLight::shine_light will generate a view frustum from the
   * center of the light and use that to light all polygons that
   * are hit by the frustum.
   */
  void ShineLights (csProgressPulse* = 0);

  /// Version of shine_lights() which only affects one mesh object.
  void ShineLights (iMeshWrapper*, csProgressPulse* = 0);

  /// Sets dynamic ambient light for all things in the sector
  void SetDynamicAmbientLight(const csColor& color);

  /// Get the kdtree for the light list.
  csKDTree* GetLightKDTree () const { return lights.GetLightKDTree (); }

  //----------------------------------------------------------------------
  // Various
  //----------------------------------------------------------------------

  /// Get the engine for this sector.
  csEngine* GetEngine () const { return engine; }

  /// Return true if this has fog.
  bool HasFog () const { return fog.enabled; }

  /// Return fog structure.
  csFog& GetFog () { return fog; }

  /// Convenience function to set fog to some setting.
  void SetFog (float density, const csColor& color)
  {
    fog.enabled = true;
    fog.density = density;
    fog.red = color.red;
    fog.green = color.green;
    fog.blue = color.blue;
  }

  /// Disable fog.
  void DisableFog () { fog.enabled = false; }

  SCF_DECLARE_IBASE_EXT (csObject);

  //----------------------------------------------------------------------
  // Portal stuff.
  //----------------------------------------------------------------------
  const csSet<iMeshWrapper*>& GetPortalMeshes () const
  { return portal_meshes; }
  void RegisterPortalMesh (iMeshWrapper* mesh);
  void UnregisterPortalMesh (iMeshWrapper* mesh);

  //------------------------- iSector interface -------------------------------
  struct eiSector : public iSector
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSector);

    csSector *GetPrivateObject ()
      { return (csSector*)scfParent; }
    virtual iObject *QueryObject()
      { return scfParent; }
    virtual void SetRenderLoop (iRenderLoop* rl)
      { scfParent->SetRenderLoop (rl); }
    virtual iRenderLoop* GetRenderLoop ()
      { return scfParent->GetRenderLoop (); }
    virtual int GetRecLevel () const
      { return scfParent->draw_busy; }
    virtual void IncRecLevel ()
      { scfParent->draw_busy++; }
    virtual void DecRecLevel ()
      { scfParent->draw_busy--; }
    virtual bool SetVisibilityCullerPlugin (const char* name,
    	iDocumentNode* culler_params = 0)
    {
      return scfParent->UseCullerPlugin (name, culler_params);
    }
    virtual iVisibilityCuller* GetVisibilityCuller ()
      { return scfParent->GetVisibilityCuller (); }
    virtual iMeshList* GetMeshes ()
      { return scfParent->GetMeshes (); }
    virtual iLightList* GetLights ()
      { return scfParent->GetLights (); }
    virtual void ShineLights ()
      { scfParent->ShineLights (); }
    virtual void ShineLights (iMeshWrapper* mesh)
      { scfParent->ShineLights (mesh); }
    virtual void SetDynamicAmbientLight(const csColor& color)
      { scfParent->SetDynamicAmbientLight(color); }
    virtual csColor GetDynamicAmbientLight() const
      { return scfParent->dynamic_ambient_color; }
    virtual void CalculateSectorBBox (csBox3& bbox, bool do_meshes) const
      { scfParent->CalculateSectorBBox (bbox, do_meshes); }
    virtual bool HasFog () const
      { return scfParent->HasFog (); }
    virtual csFog *GetFog () const
      { return &scfParent->fog; }
    virtual void SetFog (float density, const csColor& color)
      { scfParent->SetFog (density, color); }
    virtual void DisableFog ()
      { scfParent->DisableFog (); }
    virtual iMeshWrapper* HitBeamPortals (const csVector3& start,
    	const csVector3& end, csVector3& isect, int* polygon_idx);
    virtual iMeshWrapper* HitBeam (const csVector3& start, const csVector3& end,
      csVector3& intersect, int* polygonPtr, bool accurate = false);
    virtual iSector* FollowSegment (csReversibleTransform& t,
      csVector3& new_position, bool& mirror, bool only_portals = false);
    virtual void Draw (iRenderView* rview)
      { scfParent->Draw (rview); }
    virtual void PrepareDraw (iRenderView* rview)
    { scfParent->PrepareDraw (rview); }
    virtual csRenderMeshList* GetVisibleMeshes (iRenderView *rview)
    { return scfParent->GetVisibleMeshes (rview); }
    virtual void SetSectorCallback (iSectorCallback* cb)
    {
      scfParent->SetSectorCallback (cb);
    }
    virtual void RemoveSectorCallback (iSectorCallback* cb)
    {
      scfParent->RemoveSectorCallback (cb);
    }
    virtual int GetSectorCallbackCount () const
    {
      return scfParent->GetSectorCallbackCount ();
    }
    virtual iSectorCallback* GetSectorCallback (int idx) const
    {
      return scfParent->GetSectorCallback (idx);
    }

    virtual void AddSectorMeshCallback (iSectorMeshCallback* cb)
    {
      scfParent->AddSectorMeshCallback (cb);
    }
    virtual void RemoveSectorMeshCallback (iSectorMeshCallback* cb)
    {
      scfParent->RemoveSectorMeshCallback (cb);
    }

    virtual void CheckFrustum (iFrustumView* lview)
    {
      scfParent->CheckFrustum (lview);
    }

    virtual const csSet<iMeshWrapper*>& GetPortalMeshes () const
    {
      return scfParent->GetPortalMeshes ();
    }
    virtual void RegisterPortalMesh (iMeshWrapper* mesh)
    {
      scfParent->RegisterPortalMesh (mesh);
    }
    virtual void UnregisterPortalMesh (iMeshWrapper* mesh)
    {
      scfParent->UnregisterPortalMesh (mesh);
    }
    virtual void UnlinkObjects ()
    {
      scfParent->UnlinkObjects ();
    }
    csSector* GetCsSector ()
    {
      return scfParent;
    }
  } scfiSector;
  friend struct eiSector;
};

/// List of 3D engine sectors.
class csSectorList : public iSectorList
{
private:
  csRefArrayObject<iSector> list;
  csHash<iSector*,csStrKey> sectors_hash;

public:
  SCF_DECLARE_IBASE;

  /// constructor
  csSectorList ();
  /// destructor
  virtual ~csSectorList ();

  /// Override FreeSector.
  virtual void FreeSector (iSector* item);

  virtual int GetCount () const { return (int)list.Length (); }
  virtual iSector *Get (int n) const { return list.Get (n); }
  virtual int Add (iSector *obj);
  virtual bool Remove (iSector *obj);
  virtual bool Remove (int n);
  virtual void RemoveAll ();
  virtual int Find (iSector *obj) const;
  virtual iSector *FindByName (const char *Name) const;
};

#endif // __CS_SECTOR_H__
