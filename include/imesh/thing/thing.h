/*
    Copyright (C) 1998-2003 by Jorrit Tyberghein

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

#ifndef __CS_IMESH_THING_H__
#define __CS_IMESH_THING_H__

#include "csutil/scf.h"
#include "csutil/flags.h"

class csVector2;
class csVector3;
class csMatrix3;
struct iSector;
struct iPolygon3D;
struct iPolygon3DStatic;
struct iGraphics3D;
struct iFrustumView;
struct iMaterialWrapper;
struct iMaterialList;
struct iMovable;

/**
 * If CS_THING_FASTMESH is set then this thing will be drawn using
 * the faster DrawPolygonMesh.
 */
#define CS_THING_FASTMESH 2

/**
 * If CS_THING_NOCOMPRESS is set then vertices of this thing will not
 * be compressed. By default the vertex table is compressed before the
 * thing is used for the first time (this means that duplicate vertices
 * are removed).
 */
#define CS_THING_NOCOMPRESS 4

/**
 * The following flags affect movement options for a thing. See
 * SetMovingOption() for more info.
 */
#define CS_THING_MOVE_NEVER 0
#define CS_THING_MOVE_OCCASIONAL 2


SCF_VERSION (iThingFactoryState, 0, 1, 0);

/**
 * This is the state interface to access the internals of a thing
 * mesh factory.
 */
struct iThingFactoryState : public iBase
{
  /// @@@ UGLY
  virtual void* GetPrivateObject () = 0;

  /**
   * Compress the vertex table so that all nearly identical vertices
   * are compressed. The polygons in the set are automatically adapted.
   * This function can be called at any time in the creation of the object
   * and it can be called multiple time but it normally only makes sense
   * to call this function after you have finished adding all polygons
   * and all vertices.<p>
   * Note that calling this function will make the camera vertex array
   * invalid.
   */
  virtual void CompressVertices () = 0;

  /// Query number of polygons in this thing.
  virtual int GetPolygonCount () = 0;
  /// Get a polygon from set by his index.
  virtual iPolygon3DStatic *GetPolygon (int idx) = 0;
  /// Get a polygon from set by name.
  virtual iPolygon3DStatic *GetPolygon (const char* name) = 0;
  /// Create a new polygon and return a pointer to it.
  virtual iPolygon3DStatic *CreatePolygon (const char *iName = 0) = 0;
  /// Find the index for a polygon. Returns -1 if polygon cannot be found.
  virtual int FindPolygonIndex (iPolygon3DStatic* polygon) const = 0;
  /// Delete a polygon given an index.
  virtual void RemovePolygon (int idx) = 0;
  /// Delete all polygons.
  virtual void RemovePolygons () = 0;

  /// Query number of vertices in set
  virtual int GetVertexCount () const = 0;
  /// Get the given vertex coordinates in object space
  virtual const csVector3 &GetVertex (int idx) const = 0;
  /// Get the vertex coordinates in object space
  virtual const csVector3* GetVertices () const = 0;
  /// Create a vertex given his object-space coords and return his index
  virtual int CreateVertex (const csVector3& vt) = 0;
  /// Set the object space vertices for a given vertex.
  virtual void SetVertex (int idx, const csVector3& vt) = 0;
  /**
   * Delete a vertex. Warning this will invalidate all polygons
   * that use vertices after this vertex because their vertex indices
   * will no longer be ok.
   */
  virtual void DeleteVertex (int idx) = 0;
  /**
   * Delete a range of vertices (inclusive). Warning this will invalidate
   * all polygons that use vertices after these vertices because their
   * vertex indices will no longer be ok. This function does
   * bounds-checking so an easy way to delete all vertices is
   * DeleteVertices(0,1000000000).
   */
  virtual void DeleteVertices (int from, int to) = 0;

  /// Set thing flags (see CS_THING_... values above)
  virtual csFlags& GetFlags () = 0;

  /**
   * Sets the smoothing flag.
   */
  virtual void SetSmoothingFlag (bool smoothing) = 0;

  /**
   * Gets the smoothing flag.
   */
  virtual bool GetSmoothingFlag () = 0;
  
  /**
   * Gets the normals.
   */
  virtual csVector3* GetNormals () = 0;

  /**
   * Get cosinus factor.
   */
  virtual float GetCosinusFactor () const = 0;
  /**
   * Set cosinus factor. This cosinus factor controls how lighting affects
   * the polygons relative to the angle. If no value is set here then the
   * default is used.
   */
  virtual void SetCosinusFactor (float cosfact) = 0;
};

SCF_VERSION (iThingState, 0, 6, 1);

/**
 * This is the state interface to access the internals of a thing
 * mesh object.
 */
struct iThingState : public iBase
{
  /// @@@ UGLY
  virtual void* GetPrivateObject () = 0;

  /// Get the factory.
  virtual iThingFactoryState* GetFactory () = 0;

  /// Get a polygon from set by his index.
  virtual iPolygon3D *GetPolygon (int idx) = 0;
  /// Get a polygon from set by name.
  virtual iPolygon3D *GetPolygon (const char* name) = 0;
  /// Find the index for a polygon. Returns -1 if polygon cannot be found.
  virtual int FindPolygonIndex (iPolygon3D* polygon) const = 0;

  /// Get the given vertex coordinates in world space
  virtual const csVector3 &GetVertexW (int idx) const = 0;
  /// Get the vertex coordinates in world space
  virtual const csVector3* GetVerticesW () const = 0;
  /// Get the given vertex coordinates in camera space
  virtual const csVector3 &GetVertexC (int idx) const = 0;
  /// Get the vertex coordinates in camera space
  virtual const csVector3* GetVerticesC () const = 0;

  /**
   * Get the moving option.
   */
  virtual int GetMovingOption () const = 0;

  /**
   * Control how this thing will be moved.
   * There are currently two options.
   * <ul>
   *   <li>CS_THING_MOVE_NEVER: this option is set for a thing that cannot
   *       move at all. In this case the movable will be ignored and only
   *       hard transforms can be used to move a thing with this flag. This
   *       setting is both efficient for memory (object space coordinates are
   *       equal to world space coordinates so only one array is kept) and
   *       render speed (only the camera transform is needed). This option
   *       is very useful for static geometry like walls.
   *       This option is default.
   *   <li>CS_THING_MOVE_OCCASIONAL: this option is set for a thing that
   *       is movable but doesn't move all the time usually. Setting this
   *       option means that the world space vertices will be cached (taking
   *       up more memory that way) but the coordinates will be recalculated
   *       only at rendertime (and cached at that time). This option has
   *       the same speed efficiency as MOVE_NEVER when the object doesn't
   *       move but more memory is used as all the vertices are duplicated.
   *       Use this option for geometry that is not too big (in number of
   *       vertices) and only moves occasionally like doors of elevators.
   * </ul>
   * <p>
   * Note: it is no longer needed to manually set this option. By default
   * things will use CS_THING_MOVE_NEVER and they will automatically switch
   * to the slightly less efficient CS_THING_MOVE_OCCASIONAL if needed.
   */
  virtual void SetMovingOption (int opt) = 0;

  /**
   * Prepare the thing to be ready for use. Normally this doesn't have
   * to be called as the engine will call this function automatically
   * as soon as the object is rendered. However, to avoid the (sometimes long)
   * setup time for an object while walking around an application can choose
   * to call this function manually in order to increase load time but
   * decrease the time need to setup things later.
   */
  virtual void Prepare () = 0;

  /** Reset the prepare flag so that this Thing can be re-prepared.
   * Among other things this will allow cached lightmaps to be
   * recalculated.
   */
  virtual void Unprepare () = 0;

  /**
   * Scan all polygons and replace the given material with a new material.
   * Note that the new material MUST have the same size as the old material!
   * If 'newmat' == 0 then the default from the factory will be used
   * again. Note that 'oldmat' will always be compared from the factory
   * and not from the current material the polygon has!
   */
  virtual void ReplaceMaterial (iMaterialWrapper* oldmat,
  	iMaterialWrapper* newmat) = 0;

  /**
   * Clear all replaced materials (i.e. reset to default materials from
   * factory).
   */
  virtual void ClearReplacedMaterials () = 0;
};

SCF_VERSION (iThingEnvironment, 0, 3, 0);

/**
 * This interface is implemented by the iObjectType for things.
 * Using this interface you can access some global information for things.
 */
struct iThingEnvironment : public iBase
{
  /**
   * Reset the thing environment (clear all stuff related to things).
   */
  virtual void Clear () = 0;

  /// Return the current lightmap cell size
  virtual int GetLightmapCellSize () const = 0;
  /// Set lightmap cell size
  virtual void SetLightmapCellSize (int Size) = 0;
  /// Return default lightmap cell size
  virtual int GetDefaultLightmapCellSize () const = 0;
};

#endif // __CS_IMESH_THING_H__
