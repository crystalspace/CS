/*
    Copyright (C) 2001 by Martin Geisse <mgeisse@gmx.net>

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

#ifndef __CS_MDLTOOL_H__
#define __CS_MDLTOOL_H__

/**\file
 * Model data tools.
 */

#include "csextern.h"

#include "csutil/dirtyaccessarray.h"

class csString;
struct iObject;
struct iModelData;
struct iModelDataObject;
struct iModelDataPolygon;

/// Mapping table, used by csModelDataTools::CopyVerticesMapped().
struct csModelDataVertexMap
{
  size_t VertexCount, NormalCount, ColorCount, TexelCount;
  int *Vertices, *Normals, *Colors, *Texels;
};

/**
 * This class can be used to construct objects which use a single index for
 * vertex. normal, color and texel lists, from objects that don't.
 */
class CS_CRYSTALSPACE_EXPORT csSingleIndexVertexSet
{
private:
  // delete the lists upon destruction?
  bool Delete;
  // number of vertices. This is stored outside the lists because not all lists
  // may be available, making it hard to get the vertex count from the lists.
  size_t Count;
  // the lists
  csDirtyAccessArray<int> *Vertices, *Normals, *Colors, *Texels;

public:
  /**
   * Create a new set. You can chosse which lists to use. For example, if
   * you choose not to use a color list then colors will not be used when
   * searching for a vertex, and calls to GetColor() are not allowed.
   */
  csSingleIndexVertexSet (bool ver = true, bool nrm = true,
    bool col = true, bool tex = true);

  /**
   * Create a set from existing lists. If 0 is passed for a list then
   * that list will not be used. Note that this is different from adding
   * all elements of the given lists because the lists will be kept and
   * modified every time a vertex is added to the set.
   */
  csSingleIndexVertexSet (csDirtyAccessArray<int> *Vertices,
    csDirtyAccessArray<int> *Normals, csDirtyAccessArray<int> *Colors,
    csDirtyAccessArray<int> *Texels, bool DeleteLists);

  /// Destructor
  ~csSingleIndexVertexSet ();

  /**
   * Add a vertex to the set. This function accepts the four separate indices
   * and returns the single index. Certain elements are ignored if no lists
   * exist for them (if demanded so at construction of the set).
   */
  size_t Add (int Vertex, int Normal, int Color, int Texel);

  /**
   * Add several vertices at once.
   */
  void Add (int num, int *Vertices, int *Normal, int *Colors, int *Texels);

  /// Return the number of contained vertices
  size_t GetVertexCount () const;

  /// Get a vertex index
  int GetVertex (size_t n) const;
  /// Get a normal index
  int GetNormal (size_t n) const;
  /// Get a color index
  int GetColor (size_t n) const;
  /// Get a texel index
  int GetTexel (size_t n) const;
};

/// A set of utility functions to deal with model data components.
struct CS_CRYSTALSPACE_EXPORT csModelDataTools
{
  /**
   * Copy the contents of one object into antoher one. Both objects may
   * contain something. Note that overlapping vertices are NOT merged by
   * this function. There are two reasons: Firstly, vertex merging is
   * also useful without object merging and should therefore be done
   * separately. Secondly, vertex merging is a very complex task (as the
   * vertices to merge have to share position in every frame of every action)
   * and is thus done separately anyway. @@@ vertex merging is not yet
   * implemented.
   */
  static void MergeCopyObject (iModelDataObject *dest, iModelDataObject *src);

  /**
   * Copy the default vertices and actions of one object into another one.
   * All vertex, normal, color and texel indices are mapped as described in
   * the given mapping table.
   */
  static void CopyVerticesMapped (iModelDataObject *dest,
    iModelDataObject *src, const csModelDataVertexMap *map);

  /**
   * Merge all contained mesh objects in a scene into a single one. If
   * 'MultiTexture' is true then objects with different textures may be
   * merged.
   */
  static void MergeObjects (iModelData *Scene, bool MultiTexture);

  /**
   * Split objects in a scene using more than one material into
   * objects with only a single material.
   */
  static void SplitObjectsByMaterial (iModelData *Scene);

  /**
   * Print a description of an object into a string.
   */
  static void Describe (iObject *obj, csString &s);

  /**
   * Merge duplicate vertices, normals, colors and texels
   * in an object.
   */
  static void CompressVertices (iModelDataObject *Object);

  /**
   * Merge duplicate vertices, normals, colors and texels
   * in all objects of a model data scene.
   */
  static void CompressVertices (iModelData *Scene);

  /**
   * This functions helps to build structures that use a single index per
   * vertex for position, normal, color and texel instead of one index for
   * each list. CS's 3d sprites are an example for this. <p>
   *
   * The 'poly' parameter is expected to be a valid polygon. The Sprite...
   * lists should contain the (so far known) mapping of new indices to old
   * indices. In other words, these lists are adressed with the 'single index'
   * from the 3d sprite and return the 'multiple indices' used in the model
   * data components. In the process of converting several polygons with this
   * function, you should pass the same lists again and again. The lists will
   * (probably) be empty for the first polygon and be filled a bit more for
   * each polygon. Finally, after converting all polygons, the lists can be
   * used to convert the vertex frames. Finally, the 'PolyVertices' parameter
   * should be an empty list that will be filled with the 'single indices'. <p>
   *
   * You can omit those lists you do not need. For example, if you don't care
   * about vertex colors, pass 0 for 'SpriteColors'. This has the effect that
   * (obviously) you don't get a list of mapped vertex colors. It also tells
   * this function that vertices with different colors may be merged.
   */
  static void BuildVertexArray (iModelDataPolygon* poly,
	csDirtyAccessArray<int>* SpriteVertices,
	csDirtyAccessArray<int>* SpriteNormals,
	csDirtyAccessArray<int>* SpriteColors,
	csDirtyAccessArray<int>* SpriteTexels,
	csDirtyAccessArray<int>* PolyVertices);
};

#endif // __CS_MDLTOOL_H__
