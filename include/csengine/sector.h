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

#include "csutil/csobject.h"
#include "csutil/nobjvec.h"
#include "csengine/bsp.h"
#include "csgeom/math3d.h"
#include "csutil/cscolor.h"
#include "iengine/sector.h"
#include "ivideo/graph3d.h"
#include "iengine/mesh.h"
#include "iutil/objref.h"

class csEngine;
class csStatLight;
class csMeshWrapper;
class csPolygon3D;
class csCollection;
class csCamera;
class csDynLight;
class csPolygon2DQueue;
class csProgressPulse;
struct iGraphics3D;
struct iStatLight;
struct iVisibilityCuller;
struct iRenderView;
struct iMeshWrapper;
struct iFrustumView;


SCF_VERSION (csSector, 0, 0, 1);

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
   * of type csMeshWrapper*.
   */
  csVector meshes;

  /**
   * The same meshes above but each mesh in their own render priority
   * queue. This is a vector of vectors.
   */
  csVector mesh_priority_queues;

  /**
   * List of collections in this sector.
   */
  csVector collections;

  /**
   * List of references (portals?) to this sector.
   */
  csVector references;

  /**
   * All static and pseudo-dynamic lights in this sector.
   * This vector contains objects of type csStatLight*.
   */
  csNamedObjVector lights;

  /// Engine handle.
  csEngine* engine;

  /// Fog information.
  csFog fog;

  /**
   * This is a pointer to the csMeshWrapper which implements the visibility
   * culler.
   */
  csMeshWrapper* culler_mesh;

  /**
   * The visibility culler for this sector or NULL if none.
   * In future we should support more than one visibility culler probably.
   */
  iVisibilityCuller* culler;

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

  /**
   * Add a mesh to this sector and register it to the culler.
   */
  void AddMesh (csMeshWrapper* mesh);

  /**
   * Unlink a mesh from this sector and remove it from the culler.
   */
  void UnlinkMesh (csMeshWrapper* mesh);

  /**
   * Relink a mesh from this sector. This is mainly useful if
   * characterics of the mesh changed (like render priority) so
   * that the sector needs to know this.
   */
  void RelinkMesh (csMeshWrapper* mesh);

  /**
   * Get the number of meshes in this sector.
   */
  int GetMeshCount () const
  {
    return meshes.Length ();
  }

  /**
   * Get the specified mesh.
   */
  csMeshWrapper* GetMesh (int idx) const
  {
    return (csMeshWrapper*)meshes[idx];
  }

  /**
   * Find the given mesh by name.
   */
  csMeshWrapper* GetMesh (const char* name) const;

  //----------------------------------------------------------------------
  // Collection manipulation functions
  //----------------------------------------------------------------------

  /**
   * Add a collection to this sector.
   */
  void AddCollection (csCollection* col);

  /**
   * Unlink a collection from this sector.
   */
  void UnlinkCollection (csCollection* col);

  /**
   * Get the number of collections in this sector.
   */
  int GetCollectionCount () const
  {
    return collections.Length ();
  }

  /**
   * Get the specified collection.
   */
  csCollection* GetCollection (int idx) const
  {
    return (csCollection*)collections[idx];
  }

  /**
   * Find a collection with the given name.
   */
  csCollection* GetCollection (const char* name) const;

  //----------------------------------------------------------------------
  // Light manipulation functions
  //----------------------------------------------------------------------

  /**
   * Add a static or pseudo-dynamic light to this sector.
   */
  void AddLight (csStatLight* light);

  /**
   * Unlink a light from this sector.
   */
  void UnlinkLight (csStatLight* light);

  /**
   * Get the number of lights in this sector.
   */
  int GetLightCount () const
  {
    return lights.Length ();
  }

  /**
   * Get the specified light.
   */
  csStatLight* GetLight (int idx) const
  {
    return (csStatLight*)lights[idx];
  }

  /**
   * Find a light with the given name.
   */
  csStatLight* GetLight (const char* name) const
  {
    return (csStatLight*)lights.FindByName (name);
  }

  /**
   * Find a light with the given position and radius.
   */
  csStatLight* FindLight (float x, float y, float z, float dist) const;

  /**
   * Find the light with the given object id.
   */
  csStatLight* FindLight (unsigned long id) const;

  //----------------------------------------------------------------------
  // Visibility Stuff
  //----------------------------------------------------------------------

  /**
   * Get the mesh which implements the visibility culler.
   */
  csMeshWrapper* GetCullerMesh () const { return culler_mesh; }

  /**
   * Look for the mesh object and see if it implements iVisibilityCuller.
   * If so then use it for visibility culling in this sector.
   */
  void UseCuller (const char* meshname);

  /**
   * Get the visibility culler that is used for this sector.
   * NULL if none.
   */
  iVisibilityCuller* GetVisibilityCuller () const { return culler; }

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
  csPolygon3D* HitBeam (const csVector3& start, const csVector3& end,
  	csVector3& isect);

  /**
   * Follow a beam from start to end and return the first object
   * that is hit. Objects can be meshes, things, or sectors.
   * In case it is a thing or sector the csPolygon3D field will be
   * filled with the polygon that was hit.
   * If polygonPtr is null then the polygon will not be filled in.
   */
  csObject* HitBeam (const csVector3& start, const csVector3& end, csVector3& intersect,
  	csPolygon3D** polygonPtr);

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
   * Get a list of all objects which are visible in the given frustum.
   * Return an array to pointers to visible objects.
   * You must delete this array after you are ready using it.
   * @@@ When csThing becomes a mesh object then change rc to csMeshWrapper**
   */
  csObject** GetVisibleObjects (iFrustumView* lview, int& num_objects);

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
  csPolygon3D* IntersectSphere (csVector3& center, float radius,
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
   * to the last position that you can go to before hitting a wall.
   */
  csSector* FollowSegment (csReversibleTransform& t, csVector3& new_position, 
                          bool& mirror);

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
   * that it will also test for hits against things.
   */
  virtual csPolygon3D* IntersectSegment (const csVector3& start,
                                       const csVector3& end, csVector3& isect,
				       float* pr = NULL);

  /**
   * Calculate the bounding box of all objects in this sector.
   * This function is not very efficient as it will traverse all objects
   * in the sector one by one and compute a bounding box from that.
   */
  void CalculateSectorBBox (csBox3& bbox, bool do_meshes)
    const;

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

  //----------------------------------------------------------------------
  // Various
  //----------------------------------------------------------------------

  /// Get the engine for this sector.
  csEngine* GetEngine () const { return engine; }

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
    virtual int GetRecLevel () const { return scfParent->draw_busy; }
    virtual void SetVisibilityCuller (const char *Name)
    {
      scfParent->UseCuller (Name);
    }
    virtual iVisibilityCuller* GetVisibilityCuller () const
    {
      return scfParent->GetVisibilityCuller ();
    }

    virtual int GetMeshCount () const { return scfParent->GetMeshCount (); }
    virtual iMeshWrapper *GetMesh (int n) const;
    virtual void AddMesh (iMeshWrapper *pMesh);
    virtual iMeshWrapper *GetMesh (const char *name) const;
    virtual void UnlinkMesh (iMeshWrapper *pMesh)
    { scfParent->UnlinkMesh (pMesh->GetPrivateObject ()); }

    virtual int GetCollectionCount () const
    {
      return scfParent->GetCollectionCount ();
    }
    virtual iCollection* GetCollection (int n) const;
    virtual void AddCollection (iCollection* col);
    virtual iCollection* GetCollection (const char *name) const;
    virtual void UnlinkCollection (iCollection* col);

    virtual void AddLight (iStatLight *light);
    virtual int GetLightCount () const { return scfParent->GetLightCount (); }
    virtual iStatLight *GetLight (int n) const;
    virtual iStatLight *GetLight (const char* name) const;
    virtual iStatLight *FindLight (float x, float y, float z, float dist) const;
    virtual void ShineLights ()
    { scfParent->ShineLights (); }
    virtual void ShineLights (iMeshWrapper* mesh)
    { scfParent->ShineLights (mesh); }

    virtual void CalculateSectorBBox (csBox3& bbox, bool do_meshes) const
    { scfParent->CalculateSectorBBox (bbox, do_meshes); }

    virtual bool HasFog () const { return scfParent->HasFog (); }
    virtual csFog *GetFog () const { return &scfParent->fog; }
    virtual void SetFog (float density, const csColor& color)
    { scfParent->SetFog (density, color); }
    virtual void DisableFog ()
    { scfParent->DisableFog (); }
    virtual iPolygon3D* HitBeam (const csVector3& start, const csVector3& end,
  	csVector3& isect);
    virtual iObject* HitBeam (const csVector3& start, const csVector3& end,
  	csVector3& intersect, iPolygon3D** polygonPtr);
    virtual iSector* FollowSegment (csReversibleTransform& t,
  	csVector3& new_position, bool& mirror);
    virtual void Draw (iRenderView* rview)
    { scfParent->Draw (rview); }
  } scfiSector;
  friend struct eiSector;
};

CS_DECLARE_OBJECT_VECTOR (csSectorListHelper, iSector);

class csSectorList : public csSectorListHelper
{
public:
  SCF_DECLARE_IBASE;
  bool CleanupReferences;

  /// constructor
  csSectorList (bool CleanupReferences);

  /// override FreeItem
  virtual bool FreeItem (csSome Item);

  class SectorList : public iSectorList
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSectorList);
    virtual int GetSectorCount () const;
    virtual iSector *GetSector (int idx) const;
    virtual void AddSector (iSector *sec);
    virtual void RemoveSector (iSector *sec);
    virtual iSector *FindByName (const char *name) const;
  } scfiSectorList;
};

#endif // __CS_SECTOR_H__
