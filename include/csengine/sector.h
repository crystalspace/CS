/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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
#include "csutil/csvector.h"
#include "iutil/objref.h"
#include "csengine/light.h"
#include "csengine/meshobj.h"
#include "csengine/rdrprior.h"
#include "iengine/sector.h"
#include "ivideo/graph3d.h"


class csEngine;
class csProgressPulse;
class csSector;
class csStatLight;
class csMeshWrapper;
struct iPolygon3D;
struct iStatLight;
struct iVisibilityCuller;
struct iRenderView;
struct iMeshWrapper;
struct iFrustumView;

/// A list of lights for a sector.
class csSectorLightList : public csLightList
{
private:
  csSector* sector;

public:
  /// constructor
  csSectorLightList ();
  /// destructor
  ~csSectorLightList ();
  /// Set the sector.
  void SetSector (csSector* s) { sector = s; }

  /// Override PrepareItem
  virtual void PrepareItem (iLight* light);
  /// Override FreeItem
  virtual void FreeItem (iLight* item);
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
  ~csSectorMeshList ();
  /// Set the sector.
  void SetSector (csSector* sec) { sector = sec; }

  /// Override PrepareItem
  virtual void PrepareItem (iMeshWrapper* item);
  /// Override FreeItem
  virtual void FreeItem (iMeshWrapper* item);
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
   * The same meshes above but each mesh in their own render priority
   * queue. This is a vector of vectors.
   */
  csRenderQueueSet RenderQueues;

  /**
   * List of references (portals?) to this sector.
   */
  csVector references;

  /**
   * List of sector callbacks.
   */
  csRefArray<iSectorCallback> sector_cb_vector;

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

#ifndef CS_USE_NEW_RENDERER
  /// Fog information.
  csFog fog;
#endif // CS_USE_NEW_RENDERER

  /**
   * The visibility culler for this sector or NULL if none.
   * In future we should support more than one visibility culler probably.
   */
  csRef<iVisibilityCuller> culler;

private:
  /**
   * Destroy this sector. All things in this sector are also destroyed.
   * Meshes are unlinked from the sector but not removed because they
   * could be in other sectors.
   */
  virtual ~csSector ();

public:
  /**
   * Option variable: render portals?
   * If this variable is false portals are rendered as a solid polygon.
   */
  static bool do_portals;

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

  /**
   * This function MUST be called before the sector is deleted in order
   * to make sure that all references to the sector are cleaned up.
   */
  void CleanupReferences ();

  //----------------------------------------------------------------------
  // Mesh manipulation functions
  //----------------------------------------------------------------------

  iMeshList* GetMeshes ()
    { return &(meshes.scfiMeshList); }

  /**
   * Register a mesh and all children to the visibility culler.
   */
  void RegisterMeshToCuller (iMeshWrapper* mesh);

  /**
   * Unregister a mesh and all children from the visibility culler.
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
    { return &lights.scfiLightList; }

  //----------------------------------------------------------------------
  // Callbacks
  //----------------------------------------------------------------------
  void SetSectorCallback (iSectorCallback* cb)
  {
    sector_cb_vector.Push (cb);
  }

  void RemoveSectorCallback (iSectorCallback* cb)
  {
    int idx = sector_cb_vector.Find (cb);
    if (idx != -1)
      sector_cb_vector.Delete (idx);
  }

  int GetSectorCallbackCount () const
  {
    return sector_cb_vector.Length ();
  }
  
  iSectorCallback* GetSectorCallback (int idx) const
  {
    return sector_cb_vector.Get (idx);
  }

  //----------------------------------------------------------------------
  // Visibility Stuff
  //----------------------------------------------------------------------

  /**
   * Use the given plugin as a visibility culler (should implement
   * iVisibilityCuller). Returns false if the culler could not be
   * loaded for some reason.
   */
  bool UseCullerPlugin (const char* plugname);

  /**
   * Get the visibility culler that is used for this sector.
   * NULL if none.
   */
  iVisibilityCuller* GetVisibilityCuller ();

  //----------------------------------------------------------------------
  // Drawing
  //----------------------------------------------------------------------

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
  iPolygon3D* HitBeam (const csVector3& start, const csVector3& end,
  	csVector3& isect);

  /**
   * Follow a beam from start to end and return the first object
   * that is hit. In case it is a thing the iPolygon3D field will be
   * filled with the polygon that was hit.
   * If polygonPtr is null then the polygon will not be filled in.
   */
  csMeshWrapper* HitBeam (const csVector3& start, const csVector3& end,
  	csVector3& intersect, iPolygon3D** polygonPtr);

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
   * Intersects world-space sphere with polygons of this set. Return
   * polygon it hits with (or NULL) and the intersection point
   * in world coordinates. It will also return the polygon with the
   * closest hit (the most nearby polygon).
   * If 'pr' != NULL it will also return the distance where the
   * intersection happened.
   * Note. This function correctly accounts for portal polygons
   * and could thus return a polygon not belonging to this sector.
   */
  iPolygon3D* IntersectSphere (csVector3& center, float radius,
                               float* pr = NULL);

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
  csSector* FollowSegment (csReversibleTransform& t, csVector3& new_position,
                          bool& mirror, bool only_portals = false);

  /**
   * Intersect world-space segment with polygons of this sector. Return
   * polygon it intersects with (or NULL) and the intersection point
   * in world coordinates.<p>
   *
   * If 'pr' != NULL it will also return a value between 0 and 1
   * indicating where on the 'start'-'end' vector the intersection
   * happened.<p>
   *
   * This function is an extension of csPolygonSet::intersect_segment in
   * that it will also test for hits against things.<p>
   *
   * If 'only_portals' == true only portals are checked.<p>
   *
   * If 'mesh' != NULL the mesh will be filled in.
   */
  iPolygon3D* IntersectSegment (const csVector3& start,
	const csVector3& end, csVector3& isect,
	float* pr = NULL, bool only_portals = false,
	csMeshWrapper** p_mesh = NULL);

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
   * This function will call csStatLight::shine_lightmaps for every
   * light in the sector.
   * csStatLight::shine_light will generate a view frustum from the
   * center of the light and use that to light all polygons that
   * are hit by the frustum.
   */
  void ShineLights (csProgressPulse* = 0);

  /// Version of shine_lights() which only affects one mesh object.
  void ShineLights (iMeshWrapper*, csProgressPulse* = 0);

  /// Sets dynamic ambient light for all things in the sector
  void SetDynamicAmbientLight(const csColor& color);

  //----------------------------------------------------------------------
  // Various
  //----------------------------------------------------------------------

  /// Get the engine for this sector.
  csEngine* GetEngine () const { return engine; }

#ifndef CS_USE_NEW_RENDERER
  /// Return true if this has fog.
  bool HasFog () const { return fog.enabled; }

  /// Return fog structure.
  csFog& GetFog () { return fog; }

  /// Conveniance function to set fog to some setting.
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
#endif // CS_USE_NEW_RENDERER

  SCF_DECLARE_IBASE_EXT (csObject);

  //-------------------- iReferencedObject interface --------------------------
  struct ReferencedObject : public iReferencedObject
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSector);
    virtual void AddReference (iReference* ref);
    virtual void RemoveReference (iReference* ref);
  } scfiReferencedObject;
  friend struct ReferencedObject;

  //------------------------- iSector interface -------------------------------
  struct eiSector : public iSector
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSector);

    virtual csSector *GetPrivateObject ()
      { return (csSector*)scfParent; }
    virtual iObject *QueryObject()
      { return scfParent; }
    virtual int GetRecLevel () const
      { return scfParent->draw_busy; }
    virtual bool SetVisibilityCullerPlugin (const char *Name)
      { return scfParent->UseCullerPlugin (Name); }
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
#ifndef CS_USE_NEW_RENDERER
    virtual bool HasFog () const
      { return scfParent->HasFog (); }
    virtual csFog *GetFog () const
      { return &scfParent->fog; }
    virtual void SetFog (float density, const csColor& color)
      { scfParent->SetFog (density, color); }
    virtual void DisableFog ()
      { scfParent->DisableFog (); }
#endif // CS_USE_NEW_RENDERER
    virtual iPolygon3D* HitBeam (const csVector3& start, const csVector3& end,
  	csVector3& isect);
    virtual iMeshWrapper* HitBeam (const csVector3& start, const csVector3& end,
  	csVector3& intersect, iPolygon3D** polygonPtr);
    virtual iSector* FollowSegment (csReversibleTransform& t,
  	csVector3& new_position, bool& mirror, bool only_portals = false);
    virtual void Draw (iRenderView* rview)
      { scfParent->Draw (rview); }
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
    virtual void CheckFrustum (iFrustumView* lview)
    {
      scfParent->CheckFrustum (lview);
    }
  } scfiSector;
  friend struct eiSector;
};

/// List of 3D engine sectors.
class csSectorList : public csRefArrayObject<iSector>
{
public:
  SCF_DECLARE_IBASE;
  bool CleanupReferences;

  /// constructor
  csSectorList (bool CleanupReferences);
  /// destructor
  virtual ~csSectorList ();

  /// Override FreeItem.
  virtual void FreeItem (iSector* item);

  class SectorList : public iSectorList
  {
  public:
    SCF_DECLARE_EMBEDDED_IBASE (csSectorList);

    virtual int GetCount () const;
    virtual iSector *Get (int n) const;
    virtual int Add (iSector *obj);
    virtual bool Remove (iSector *obj);
    virtual bool Remove (int n);
    virtual void RemoveAll ();
    virtual int Find (iSector *obj) const;
    virtual iSector *FindByName (const char *Name) const;
  } scfiSectorList;
};

#endif // __CS_SECTOR_H__
