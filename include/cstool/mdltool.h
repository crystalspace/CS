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
   * contain something.
   */
  static void MergeCopyObject (iModelDataObject *dest, iModelDataObject *src);

  /**
   * Copy the default vertices and actions of one object into another one.
   * All vertex, normal, color and texel indices are mapped as described in
   * the given mapping table.
   */
  static void CopyVerticesMapped (iModelDataObject *dest,
    iModelDataObject *src, csModelDataVertexMap *map);

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
};

#endif // __MDLTOOL_H__
