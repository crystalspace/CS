/*
    Copyright (C) 2005 by Jorrit Tyberghein

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

#ifndef __CS_VERTEXCOMPRESS_H__
#define __CS_VERTEXCOMPRESS_H__

/**\file
 * Vertex Compressor
 */

#include "csextern.h"

#include "csgeom/vector3.h"
#include "csgeom/vector2.h"
#include "csutil/array.h"
#include "csutil/cscolor.h"
#include "csutil/ref.h"

struct iRenderBuffer;

/// This structure is used by csVertexCompressor::Compress().
struct csCompressVertexInfo
{
  size_t orig_idx;
  int x, y, z;
  int u, v;
  int nx, ny, nz;
  int r, g, b, a;
  size_t new_idx;
  bool used;
};

/**
 * A vertex compressor.
 */
class CS_CRYSTALSPACE_EXPORT csVertexCompressor
{
public:
  /**
   * Compress an array of vertices (i.e. remove all duplicated
   * vertices). Returns an array of csCompressVertex which can be
   * used to map from the old index to the new one. 'new_count'
   * will be set to the new number of unique vertices (and 'new_vertices'
   * will be the new vertex table with that size). The size
   * of the returned array is 'num_vertices' though since it has
   * to be indexed with the original vertex array.
   * If this function returns 0 there is nothing to do (i.e. no duplicate
   * vertices). Otherwise you have to 'delete[]' the returned array.
   */
  static csCompressVertexInfo* Compress (csVector3* vertices,
  	csVector2* texels, csVector3* normals, csColor4* colors,
	size_t num_vertices, csVector3*& new_vertices,
	csVector2*& new_texels, csVector3*& new_normals,
	csColor4*& new_colors, size_t& new_count);

  /**
   * Compress an array of vertices (i.e. remove all duplicated
   * vertices). Returns an array of csCompressVertex which can be
   * used to map from the old index to the new one. The 'vertices'
   * table will be modified with the new compressed vertices.
   * If this function returns 0 there is nothing to do (i.e. no duplicate
   * vertices). Otherwise you have to 'delete[]' the returned array.
   */
  static csCompressVertexInfo* Compress (csArray<csVector3>& vertices,
  	csArray<csVector2>& texels, csArray<csVector3>& normals,
	csArray<csColor4>& colors);
	
  /**
   * Compress an array of vertices (i.e. remove all duplicated
   * vertices).
   * \param buffers Array of render buffers to compress.
   *   All buffers must have the same amount of elements.
   *   The pointed-to references will be replaced with the compressed
   *   buffers.
   * \param numBuffers Number of render buffers in \a buffers.
   * \param newCount Receives the new vertex count.
   * \returns An array of indices to map from an old index to a new one.
   *    The array size will be equal to old the number of vertices.
   *    It must be freed with delete[] after use.
   */
  static size_t* Compress (csRef<iRenderBuffer>* buffers, size_t numBuffers,
			   size_t& newCount);
};

/** @} */

#endif // __CS_VERTEXCOMPRESS_H__

