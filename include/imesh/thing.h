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
class csPlane3;
struct iSector;
struct iGraphics3D;
struct iFrustumView;
struct iMaterialWrapper;
struct iMaterialList;
struct iMovable;
struct iMeshObject;
struct iMeshObjectFactory;
struct iThingState;
struct iThingFactoryState;
struct iRenderBuffer;

/**
 * A range structure for specifing polygon ranges.
 */
struct csPolygonRange
{
  int start, end;
  csPolygonRange (int start, int end)
  {
    csPolygonRange::start = start;
    csPolygonRange::end = end;
  }
  void Set (int start, int end)
  {
    csPolygonRange::start = start;
    csPolygonRange::end = end;
  }
  void Set (int idx)
  {
    csPolygonRange::start = idx;
    csPolygonRange::end = idx;
  }
};

/** \name Polygon flags
 * @{ */
/**
 * If CS_POLY_LIGHTING is set for a polygon then the polygon will be lit.
 * It is set by default.
 */
#define CS_POLY_LIGHTING 0x00000001

/**
 * If this flag is set then this polygon is used for collision detection.
 */
#define CS_POLY_COLLDET	0x00000002

/**
 * If this flag is set then this polygon is used for visibility culling.
 */
#define CS_POLY_VISCULL	0x00000004
/** @} */


/** \name Polygon ranges
 * @{ */
/**
 * A general range of polygons (inclusive).
 */
#define CS_POLYRANGE(s1,s2) csPolygonRange (s1, s2)
/**
 * A single polygon.
 */
#define CS_POLYRANGE_SINGLE(idx) csPolygonRange (idx, idx)
/**
 * The last created polygon or series of polygons (in case it was a box).
 */
#define CS_POLYRANGE_LAST csPolygonRange (-1, -1)
/**
 * All polygons.
 */
#define CS_POLYRANGE_ALL csPolygonRange (0, 2000000000)
/** @} */

/**
 * Last created polygon index (used where a single polygon index is required).
 */
#define CS_POLYINDEX_LAST -1

/** \name Thing flags
 * @{ */
/**
 * If CS_THING_NOCOMPRESS is set then vertices of this thing factory will not
 * be compressed. By default the vertex table is compressed before the
 * thing is used for the first time (this means that duplicate vertices
 * are removed). This is a flag for iMeshObjectFactory.
 */
#define CS_THING_NOCOMPRESS 0x00010000
/** @} */

/** \name Move option flags
 * @{ */
/**
 * The following flags affect movement options for a thing. See
 * SetMovingOption() for more info.
 */
#define CS_THING_MOVE_NEVER 0
#define CS_THING_MOVE_OCCASIONAL 2
/** @} */

SCF_VERSION (iPolygonHandle, 0, 0, 1);

/**
 * This is an interface that can be used to represent a polygon in
 * situations where a SCF object is required. Create an instance of
 * this object using iThingFactoryState->CreatePolygonHandle() or
 * iThingState->CreatePolygonHandle(). Note that this handle will make
 * sure that the returned pointers are cleared if the thing or thing
 * factory happens to be removed.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>iThingState::CreatePolygonHandle()
 *   <li>iThingFactoryState::CreatePolygonHandle()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>iThingState
 *   <li>iThingFactoryState
 *   </ul>
 */
struct iPolygonHandle : public iBase
{
  /**
   * Get the factory state for this polygon. Or 0 if the factory is
   * removed.
   */
  virtual iThingFactoryState* GetThingFactoryState () const = 0;

  /**
   * Get the mesh object factory for this polygon. Or 0 if the factory
   * is removed.
   */
  virtual iMeshObjectFactory* GetMeshObjectFactory () const = 0;

  /**
   * Get the instance of this polygon. This can be 0 if this polygon handle
   * was created from a factory or if the instance was removed.
   */
  virtual iThingState* GetThingState () const = 0;

  /**
   * Get the mesh object of this polygon. This can be 0 if this polygon handle
   * was created from a factory or if the instance was removed.
   */
  virtual iMeshObject* GetMeshObject () const = 0;

  /**
   * Get the polygon index which this polygon handle represents.
   */
  virtual int GetIndex () const = 0;
};


SCF_VERSION (iThingFactoryState, 0, 2, 1);

/**
 * This is the state interface to access the internals of a thing
 * mesh factory.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>Thing mesh object plugin (crystalspace.mesh.object.thing)
 *   <li>iMeshObjectType::NewFactory()
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>SCF_QUERY_INTERFACE() on iMeshFactoryWrapper::GetMeshObjectFactory()
 *   <li>iThingState::GetFactory()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>Thing Factory Loader plugin (crystalspace.mesh.loader.factory.thing)
 *   </ul>
 */
struct iThingFactoryState : public iBase
{
  /**
   * Compress the vertex table so that all nearly identical vertices
   * are compressed. The polygons in the set are automatically adapted.
   * This function can be called at any time in the creation of the object
   * and it can be called multiple time but it normally only makes sense
   * to call this function after you have finished adding all polygons
   * and all vertices.
   */
  virtual void CompressVertices () = 0;

  /// Query number of polygons in this thing.
  virtual int GetPolygonCount () = 0;
  /// Delete a polygon given an index.
  virtual void RemovePolygon (int idx) = 0;
  /// Delete all polygons.
  virtual void RemovePolygons () = 0;

  /// Find a polygon index with a name.
  virtual int FindPolygonByName (const char* name) = 0;

  /**
   * Add an empty polygon.
   * \return the index of the created polygon.
   */
  virtual int AddEmptyPolygon () = 0;

  /**
   * Add a triangle.
   * <p>
   * By default the texture mapping is set so that the texture
   * is aligned on the u-axis with the 'v1'-'v2' vector and the scale is set
   * so that the texture tiles once for every unit (i.e. if you have the
   * vertices v1 and v2 are 5 units separated from each other then the texture
   * will repeat exactly five times between v1 and v2).
   * \return the index of the created polygon.
   */
  virtual int AddTriangle (const csVector3& v1, const csVector3& v2,
  	const csVector3& v3) = 0;
  /**
   * Add a quad. Note that quads are the most optimal kind of polygon
   * for a thing so you should try to use these as much as possible.
   * <p>
   * By default the texture mapping is set so that the texture
   * is aligned on the u-axis with the 'v1'-'v2' vector and the scale is set
   * so that the texture tiles once for every unit (i.e. if you have the
   * vertices v1 and v2 are 5 units separated from each other then the texture
   * will repeat exactly five times between v1 and v2).
   * \return the index of the created polygon.
   */
  virtual int AddQuad (const csVector3& v1, const csVector3& v2,
  	const csVector3& v3, const csVector3& v4) = 0;

  /**
   * Add a general polygon.
   * <p>
   * By default the texture mapping is set so that the texture
   * is aligned on the u-axis with the 'v1'-'v2' vector and the scale is set
   * so that the texture tiles once for every unit (i.e. if you have the
   * vertices v1 and v2 are 5 units separated from each other then the texture
   * will repeat exactly five times between v1 and v2).
   * \return the index of the created polygon.
   */
  virtual int AddPolygon (csVector3* vertices, int num) = 0;

  /**
   * Add a general polygon using vertex indices.
   * <p>
   * By default the texture mapping is set so that the texture
   * is aligned on the u-axis with the 'v1'-'v2' vector and the scale is set
   * so that the texture tiles once for every unit (i.e. if you have the
   * vertices v1 and v2 are 5 units separated from each other then the texture
   * will repeat exactly five times between v1 and v2).
   * \return the index of the created polygon.
   */
  virtual int AddPolygon (int num, ...) = 0;

  /**
   * Add a box that can be seen from the outside. This will add six polygons.
   * <p>
   * By default the texture mapping is set so that the texture
   * is aligned on the u-axis with the 'v1'-'v2' vector and the scale is set
   * so that the texture tiles once for every unit (i.e. if you have the
   * vertices v1 and v2 are 5 units separated from each other then the texture
   * will repeat exactly five times between v1 and v2).
   * \return the index of the first created polygon.
   */
  virtual int AddOutsideBox (const csVector3& bmin, const csVector3& bmax) = 0;

  /**
   * Add a box that can be seen from the inside. This will add six polygons.
   * <p>
   * By default the texture mapping is set so that the texture
   * is aligned on the u-axis with the 'v1'-'v2' vector and the scale is set
   * so that the texture tiles once for every unit (i.e. if you have the
   * vertices v1 and v2 are 5 units separated from each other then the texture
   * will repeat exactly five times between v1 and v2).
   * \return the index of the first created polygon.
   */
  virtual int AddInsideBox (const csVector3& bmin, const csVector3& bmax) = 0;

  /**
   * Set the name of all polygons in the given range.
   * \param range is one of the #CS_POLYRANGE defines to specify a polygon
   * range.
   * \param name the name to be assigned to the indicated polygons.
   */
  virtual void SetPolygonName (const csPolygonRange& range,
  	const char* name) = 0;

  /**
   * Get the name of the specified polygon.
   * \param polygon_idx is a polygon index or #CS_POLYINDEX_LAST for last
   * created polygon.
   */
  virtual const char* GetPolygonName (int polygon_idx) = 0;

  /**
   * Create a polygon handle that can be used to refer to some polygon.
   * This can be useful in situations where an SCF handle is required
   * to be able to reference a polygon. The thing will not keep a reference
   * to this handle so you are fully responsible for it after calling
   * this function. The polygon handle created here will not have a mesh
   * object or thing state set since it is created from the factory.
   * \param polygon_idx is a polygon index or #CS_POLYINDEX_LAST for last
   * created polygon.
   */
  virtual csPtr<iPolygonHandle> CreatePolygonHandle (int polygon_idx) = 0;

  /**
   * Set the material of all polygons in the given range.
   * \param range is one of the #CS_POLYRANGE defines to specify a polygon
   * range.
   * \param material the material to be assigned to the indicated polygons.
   */
  virtual void SetPolygonMaterial (const csPolygonRange& range,
  	iMaterialWrapper* material) = 0;

  /**
   * Get the material for the specified polygon.
   * \param polygon_idx is a polygon index or #CS_POLYINDEX_LAST for last
   * created polygon.
   */
  virtual iMaterialWrapper* GetPolygonMaterial (int polygon_idx) = 0;

  /**
   * Add a vertex to all polygons in the given range.
   * \param range is one of the #CS_POLYRANGE defines to specify a polygon
   * range.
   * \param vt the vertex index to be added to the polygons.
   */
  virtual void AddPolygonVertex (const csPolygonRange& range,
  	const csVector3& vt) = 0;

  /**
   * Add a vertex index to all polygons in the given range.
   * \param range is one of the #CS_POLYRANGE defines to specify a polygon
   * range.
   * \param vt the vertex index to be added to the polygons.
   */
  virtual void AddPolygonVertex (const csPolygonRange& range, int vt) = 0;

  /**
   * Set the given polygon index table for all polygons in the given range.
   * It is more optimal to call this routine as opposed to calling
   * AddPolygonVertex() all the time.
   * \param range is one of the #CS_POLYRANGE defines to specify a polygon
   * \param num number of entires in `indices'.
   * \param indices polygon index table to be assigned to range.
   * range.
   */
  virtual void SetPolygonVertexIndices (const csPolygonRange& range,
  	int num, int* indices) = 0;

  /**
   * Get number of vertices for polygon.
   * \param polygon_idx is a polygon index or #CS_POLYINDEX_LAST for last
   * created polygon.
   */
  virtual int GetPolygonVertexCount (int polygon_idx) = 0;

  /**
   * Get a vertex from a polygon.
   * \param polygon_idx is a polygon index or #CS_POLYINDEX_LAST for last
   * created polygon.
   * \param vertex_idx the desired vertex.
   */
  virtual const csVector3& GetPolygonVertex (int polygon_idx,
  	int vertex_idx) = 0;

  /**
   * Get table with vertex indices from polygon.
   * \param polygon_idx is a polygon index or #CS_POLYINDEX_LAST for last
   * created polygon.
   */
  virtual int* GetPolygonVertexIndices (int polygon_idx) = 0;

  /**
   * Set texture mapping of all polygons in the given range to use the
   * transform.
   * This function is usually not used by application code as it is complicated
   * to specify texture mapping like this. It is recommended to use one
   * of the other texture mapping routines.
   * \param range is one of the #CS_POLYRANGE defines to specify a polygon
   * \param m the matrix which defines the mapping.
   * \param v the vector which defines the mapping.
   * range.
   * \return false in case the texture mapping was invalid. In that case
   * a default texture mapping will be set on the polygon.
   */
  virtual bool SetPolygonTextureMapping (const csPolygonRange& range,
  	const csMatrix3& m, const csVector3& v) = 0;

  /**
   * Set texture mapping of all polygons in the given range to use the
   * given uv coordinates for the first three vertices of every polygon.
   * \param range is one of the #CS_POLYRANGE defines to specify a polygon
   * range.
   * \param uv1 uv coordinates for vertex 1.
   * \param uv2 uv coordinates for vertex 2.
   * \param uv3 uv coordinates for vertex 3.
   * \return false in case the texture mapping was invalid. In that case
   * a default texture mapping will be set on the polygon.
   */
  virtual bool SetPolygonTextureMapping (const csPolygonRange& range,
  	const csVector2& uv1, const csVector2& uv2, const csVector2& uv3) = 0;

  /**
   * Set texture mapping of all polygons in the given range to use the
   * given uv coordinates for the specified three vertices of every polygon.
   * Note that this function is only useful for polygon ranges that are on
   * the same plane.
   * \param range is one of the #CS_POLYRANGE defines to specify a polygon
   * range.
   * \param p1 vertex 1.
   * \param uv1 uv coordinates for vertex 1.
   * \param p2 vertex 2.
   * \param uv2 uv coordinates for vertex 2.
   * \param p3 vertex 3.
   * \param uv3 uv coordinates for vertex 3.
   * \return false in case the texture mapping was invalid. In that case
   * a default texture mapping will be set on the polygon.
   */
  virtual bool SetPolygonTextureMapping (const csPolygonRange& range,
  	const csVector3& p1, const csVector2& uv1,
  	const csVector3& p2, const csVector2& uv2,
  	const csVector3& p3, const csVector2& uv3) = 0;

  /**
   * Set texture mapping of all polygons in the given range to use the
   * texture mapping as specified by two vertices on the polygon. The
   * first vertex is seen as the origin and the second as the u-axis of the
   * texture space coordinate system. The v-axis is calculated on the plane
   * of the polygons and orthogonal to the given u-axis. The length of the
   * u-axis and the v-axis is given as the 'len' parameter.
   * <p>
   * For example, if 'len' is equal to 2 this means that texture will be
   * tiled exactly two times between vertex 'v_orig' and 'v'.
   * \param range is one of the #CS_POLYRANGE defines to specify a polygon
   * range.
   * \param v_orig origin.
   * \param v u-axis in texture space coordinate system.
   * \param len length of u-axis.
   * \return false in case the texture mapping was invalid. In that case
   * a default texture mapping will be set on the polygon.
   */
  virtual bool SetPolygonTextureMapping (const csPolygonRange& range,
  	const csVector3& v_orig, const csVector3& v, float len) = 0;

  /**
   * Set texture mapping of all polygons in the given range to use the
   * texture mapping as specified by two vertices on the polygon. The
   * first vertex is seen as the origin, the second as the u-axis of the
   * texture space coordinate system, and the third as the v-axis.
   * The length of the u-axis and the v-axis is given with the 'len1' and
   * 'len2' parameters.
   * <p>
   * For example, if 'len1' is equal to 2 this means that texture will be
   * tiled exactly two times between vertex 'v_orig' and 'v1'.
   * \param range is one of the #CS_POLYRANGE defines to specify a polygon
   * range.
   * \param v_orig origin.
   * \param v1 u-axis in texture space coordinate system.
   * \param len1 length of u-axis.
   * \param v2 v-axis in texture space coordinate system.
   * \param len2 length of v-axis.
   * \return false in case the texture mapping was invalid. In that case
   * a default texture mapping will be set on the polygon.
   */
  virtual bool SetPolygonTextureMapping (const csPolygonRange& range,
  	const csVector3& v_orig,
	const csVector3& v1, float len1,
	const csVector3& v2, float len2) = 0;

  /**
   * Set texture mapping of all polygons in the given range to use the
   * texture mapping as specified by the two first vertices on the polygon. The
   * first vertex is seen as the origin and the second as the u-axis of the
   * texture space coordinate system. The v-axis is calculated on the plane
   * of the polygons and orthogonal to the given u-axis. The length of the
   * u-axis and the v-axis is given as the 'len' parameter.
   * <p>
   * For example, if 'len1' is equal to 2 this means that texture will be
   * tiled exactly two times between the two first vertices.
   * \param range is one of the #CS_POLYRANGE defines to specify a polygon
   * range.
   * \param len length of the u-axis and the v-axis is given as the 'len1'
   * parameter.
   * \return false in case the texture mapping was invalid. In that case
   * a default texture mapping will be set on the polygon.
   */
  virtual bool SetPolygonTextureMapping (const csPolygonRange& range,
  	float len) = 0;

  /**
   * Get the texture space information for the specified polygon.
   * \param polygon_idx is a polygon index or #CS_POLYINDEX_LAST for last
   * created polygon.
   * \param m the matrix which will receive the mapping.
   * \param v the vector which will receive the mapping.
   */
  virtual void GetPolygonTextureMapping (int polygon_idx,
  	csMatrix3& m, csVector3& v) = 0;

  /**
   * Enable or disable texture mapping for the range of polygons.
   * \param range is one of the #CS_POLYRANGE defines to specify a polygon
   * range.
   * \param enabled boolean flag indicating if mapping should be enabled or
   * disabled.
   */
  virtual void SetPolygonTextureMappingEnabled (const csPolygonRange& range,
  	bool enabled) = 0;

  /**
   * Check if texture mapping is enabled for the specified polygon.
   * \param polygon_idx is a polygon index or #CS_POLYINDEX_LAST for last
   * created polygon.
   */
  virtual bool IsPolygonTextureMappingEnabled (int polygon_idx) const = 0;

  /**
   * Set the given flags to all polygons in the range.
   * \param range is one of the #CS_POLYRANGE defines to specify a polygon
   * range.
   * \param flags a mask of the polygon flags, such as #CS_POLY_LIGHTING,
   * #CS_POLY_COLLDET, etc.
   */
  virtual void SetPolygonFlags (const csPolygonRange& range, uint32 flags) = 0;

  /**
   * Set the given flags to all polygons in the range.
   * \param range is one of the #CS_POLYRANGE defines to specify a polygon
   * range.
   * \param mask a mask of the polygon flags, such as #CS_POLY_LIGHTING,
   * #CS_POLY_COLLDET, etc. Only the indicated bits from `flags' will be
   * assigned.
   * \param flags The set of bits to assign to the corresponding flags
   * indicated by `mask'.
   */
  virtual void SetPolygonFlags (const csPolygonRange& range, uint32 mask,
  	uint32 flags) = 0;

  /**
   * Reset the given flags to all polygons in the range.
   * \param range is one of the #CS_POLYRANGE defines to specify a polygon
   * range.
   * \param flags a mask of the polygon flags, such as #CS_POLY_LIGHTING,
   * #CS_POLY_COLLDET, etc.
   */
  virtual void ResetPolygonFlags (const csPolygonRange& range,
  	uint32 flags) = 0;

  /**
   * Get the flags of the specified polygon.
   * \param polygon_idx is a polygon index or #CS_POLYINDEX_LAST for last
   * created polygon.
   */
  virtual csFlags& GetPolygonFlags (int polygon_idx) = 0;

  /**
   * Get object space plane of the specified polygon.
   * \param polygon_idx is a polygon index or #CS_POLYINDEX_LAST for last
   * created polygon.
   */
  virtual const csPlane3& GetPolygonObjectPlane (int polygon_idx) = 0;

  /**
   * Return true if this polygon or the texture it uses is transparent.
   * \param polygon_idx is a polygon index or #CS_POLYINDEX_LAST for last
   * created polygon.
   */
  virtual bool IsPolygonTransparent (int polygon_idx) = 0;

  /**
   * Return true if an object space point is on (or very nearly on)
   * the given polygon.
   * \param polygon_idx is a polygon index or #CS_POLYINDEX_LAST for last
   * created polygon.
   * \param v the point in object space to test against the polygon.
   */
  virtual bool PointOnPolygon (int polygon_idx, const csVector3& v) = 0;

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

  virtual bool AddPolygonRenderBuffer (int polygon_idx, const char* name,
    iRenderBuffer* buffer) = 0;

  /// Set mix mode.
  virtual void SetMixMode (uint mode) = 0;
  /// Get mix mode.
  virtual uint GetMixMode () const = 0;
};

SCF_VERSION (iThingState, 0, 7, 0);

/**
 * This is the state interface to access the internals of a thing
 * mesh object.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>Thing mesh object plugin (crystalspace.mesh.object.thing)
 *   <li>iMeshObjectFactory::NewInstance()
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>SCF_QUERY_INTERFACE() on iMeshWrapper::GetMeshObject()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>Thing Loader plugin (crystalspace.mesh.loader.thing)
 *   </ul>
 */
struct iThingState : public iBase
{
  /// Get the factory.
  virtual iThingFactoryState* GetFactory () = 0;

  /// Get the given vertex coordinates in world space
  virtual const csVector3 &GetVertexW (int idx) const = 0;
  /// Get the vertex coordinates in world space
  virtual const csVector3* GetVerticesW () const = 0;

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

  /// Set mix mode.
  virtual void SetMixMode (uint mode) = 0;
  /// Get mix mode.
  virtual uint GetMixMode () const = 0;

  /**
   * Create a polygon handle that can be used to refer to some polygon.
   * This can be useful in situations where an SCF handle is required
   * to be able to reference a polygon. The thing will not keep a reference
   * to this handle so you are fully responsible for it after calling
   * this function.
   * \param polygon_idx is a polygon index. #CS_POLYINDEX_LAST is NOT
   * supported here!
   */
  virtual csPtr<iPolygonHandle> CreatePolygonHandle (int polygon_idx) = 0;

  /**
   * Get world space plane of the specified polygon.
   * \param polygon_idx is a polygon index. #CS_POLYINDEX_LAST is NOT
   * supported here!
   */
  virtual const csPlane3& GetPolygonWorldPlane (int polygon_idx) = 0;
};

SCF_VERSION (iThingEnvironment, 0, 3, 0);

/**
 * This interface is implemented by the iObjectType for things.
 * Using this interface you can access some global information for things.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>Thing mesh plugin.
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>SCF_QUERY_INTERFACE on thing mesh plugin (iMeshObjectType).
 *   </ul>
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
