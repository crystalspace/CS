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

#ifndef __MDLTOOL_H__
#define __MDLTOOL_H__

class csString;
class csIntArray;
struct iObject;
struct iModelData;
struct iModelDataObject;
struct iModelDataPolygon;

struct csModelDataVertexMap
{
  int VertexCount, NormalCount, ColorCount, TexelCount;
  int *Vertices, *Normals, *Colors, *Texels;
};

/// A set of utility functions to deal with model data components
struct csModelDataTools
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
   * about vertex colors, pass NULL for 'SpriteColors'. This has the effect that
   * (obviously) you don't get a list of mapped vertex colors. It also tells
   * this function that vertices with different colors may be merged.
   */
  static void BuildVertexArray (iModelDataPolygon* poly,
	csIntArray* SpriteVertices, csIntArray* SpriteNormals,
	csIntArray* SpriteColors, csIntArray* SpriteTexels,
	csIntArray* PolyVertices);
};

#endif // __MDLTOOL_H__
