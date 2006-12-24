/*
    Copyright (C) 2006 by Jorrit Tyberghein
              (C) 2006 by Frank Richter

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

#ifndef __CS_TRIANGLESTREAM_H__
#define __CS_TRIANGLESTREAM_H__

/**\file
 * Triangle extraction from index buffers
 */

#include "ivideo/graph3d.h"
#include "ivideo/rndbuf.h"

namespace CS
{
  
  /**
   * Helper class to extract triangles from an index buffer. Automatically
   * handles tristrips, quads etc. as well.
   */
  template<typename T>
  class TriangleIndicesStream
  {
    size_t stride;
    const uint8* index;
    const uint8* indexEnd;
    T old2, old1;
    bool stripFlag;
    int quadPart;

    iRenderBuffer* buf;
    csRenderBufferComponentType compType;
    csRenderMeshType meshtype;

    /// Fetch the next index from the index buffer
    T GetNextIndex()
    {
      T r;
      switch (compType)
      {
        default:
          CS_ASSERT(false);
        case CS_BUFCOMP_BYTE:
          r = *(char*)index;
          break;
        case CS_BUFCOMP_UNSIGNED_BYTE:
          r = *(unsigned char*)index;
          break;
        case CS_BUFCOMP_SHORT:
          r = *(short*)index;
          break;
        case CS_BUFCOMP_UNSIGNED_SHORT:
          r = *(unsigned short*)index;
          break;
        case CS_BUFCOMP_INT:
          r = *(int*)index;
          break;
        case CS_BUFCOMP_UNSIGNED_INT:
          r = *(unsigned int*)index;
          break;
        case CS_BUFCOMP_FLOAT:
          r = uint (*(float*)index);
          break;
        case CS_BUFCOMP_DOUBLE:
          r = uint (*(double*)index);
          break;
      }
      index += stride;
      return r;
    }
    T GetIndex (size_t idx)
    {
      switch (compType)
      {
        default:
          CS_ASSERT(false);
        case CS_BUFCOMP_BYTE:
          return T (*(char*)(index+idx*stride));
        case CS_BUFCOMP_UNSIGNED_BYTE:
          return T (*(unsigned char*)(index+idx*stride));
        case CS_BUFCOMP_SHORT:
          return T (*(short*)(index+idx*stride));
        case CS_BUFCOMP_UNSIGNED_SHORT:
          return T (*(unsigned short*)(index+idx*stride));
        case CS_BUFCOMP_INT:
          return T (*(int*)(index+idx*stride));
        case CS_BUFCOMP_UNSIGNED_INT:
          return T (*(unsigned int*)(index+idx*stride));
        case CS_BUFCOMP_FLOAT:
          return T (*(float*)(index+idx*stride));
        case CS_BUFCOMP_DOUBLE:
          return T (*(double*)(index+idx*stride));
      }
      return 0;
    }
  public:
    /**
     * Construct uninitialized triangle stream.
     * \remark Don't forget to call BeginTriangulate() before using 
     * HasNext() or Next()!
     */
    TriangleIndicesStream () : buf (0) { }
    /**
     * Construct triangle stream with an index buffer.
     * \param indices Index buffer to triangulate.
     * \param meshtype Mesh type of the index data.
     * \param indexStart Element of the index buffer to start iterating at.
     * \param indexEnd Element of the index buffer to stop iterating at.
     *   (size_t)~0 means last element.
     */
    TriangleIndicesStream (iRenderBuffer* indices,
			   csRenderMeshType meshtype,
			   size_t indexStart = 0, 
			   size_t indexEnd = (size_t)~0)
    {
      BeginTriangulate (indices, meshtype, indexStart, indexEnd);
    }
    ~TriangleIndicesStream()
    {
      if (buf != 0) buf->Release ();
    }

    /**
     * Begin triangulation of an index buffer.
     * \param index Pointer to start of the indices.
     * \param indexEnd Pointer to end of the indices.
     * \param stride Distance between index elements in bytes.
     * \param compType Type of component contained in the data.
     * \param meshtype Mesh type of the index data.
     */
    void BeginTriangulate (const uint8* index, const uint8* indexEnd,
      size_t stride, csRenderBufferComponentType compType,
      csRenderMeshType meshtype)
    {
      this->index = index;
      this->indexEnd = indexEnd;
      this->stride = stride;
      stripFlag = false;
      quadPart = 0;
      this->compType = compType;
      this->meshtype = meshtype;

      switch (meshtype)
      {
      case CS_MESHTYPE_TRIANGLESTRIP:
      case CS_MESHTYPE_TRIANGLEFAN:
	{
	  old2 = GetNextIndex();
	  old1 = GetNextIndex();
	  break;
	}
      default:
	;
      }
    }
    
    /**
     * Begin triangulation of an index buffer.
     * \param indices Index buffer to triangulate.
     * \param meshtype Mesh type of the index data.
     * \param indexStart Element of the index buffer to start iterating at.
     * \param indexEnd Element of the index buffer to stop iterating at.
     *   (size_t)~0 means last element.
     */
    void BeginTriangulate (iRenderBuffer* indices,
			    csRenderMeshType meshtype,
			    size_t indexStart = 0, 
			    size_t indexEnd = (size_t)~0)
    {
      if (indexEnd == (size_t)~0) indexEnd = indices->GetElementCount();
      
      buf = indices;
      uint8* indexLock = (uint8*)buf->Lock (CS_BUF_LOCK_READ);
  
      size_t stride = indices->GetElementDistance();
      uint8* tri = indexLock + indexStart*stride;
      const uint8* triEnd = indexLock + indexEnd*stride;
      
      BeginTriangulate (tri, triEnd, stride, indices->GetComponentType(), 
	meshtype);
    }

    /// Returns whether a triangle is available.
    bool HasNext() const
    {
      return (index < indexEnd);
    }
    CS_DEPRECATED_METHOD_MSG("Use HasNext() instead")
    bool HasNextTri() const { return HasNext(); }
    /// Fetches the next triangle from the buffer.
    TriangleT<T> Next ()
    {
      CS_ASSERT (index < indexEnd);
      TriangleT<T> t;
      switch (meshtype)
      {
      case CS_MESHTYPE_TRIANGLES:
	{
	  t.a = GetIndex (0);
	  t.b = GetIndex (1);
	  t.c = GetIndex (2);
	  index += 3*csRenderBufferComponentSizes[compType];
	}
	break;
      case CS_MESHTYPE_TRIANGLESTRIP:
	{
	  const T cur = GetNextIndex();
	  t.a = old1;
	  t.b = old2;
	  t.c = cur;
	  if (stripFlag)
	    old2 = cur;
	  else
	    old1 = cur;
	  stripFlag = !stripFlag;
	}
	break;
      case CS_MESHTYPE_TRIANGLEFAN:
	{
	  const T cur = GetNextIndex();
	  t.a = old2;
	  t.b = old1;
	  t.c = cur;
	  old1 = cur;
	}
	break;
      case CS_MESHTYPE_QUADS:
	{
	  if (quadPart == 0)
	  {
	    t.a = GetIndex (0);
	    t.b = GetIndex (1);
	    t.c = GetIndex (2);
	  }
	  else
	  {
	    t.a = GetIndex (0);
	    t.b = GetIndex (2);
	    t.c = GetIndex (3);
	    index += 4*csRenderBufferComponentSizes[compType];
	  }
	  quadPart ^= 1;
	}
	break;
      default:
	CS_ASSERT_MSG("Unsupported mesh type", false);
	;
      }
      return t;
    }
    CS_DEPRECATED_METHOD_MSG("Use Next() instead")
    bool NextTriangle (T& a, T& b, T& c)
    {
      TriangleT<T> tri = Next ();
      a = tri.a; b = tri.b; c = tri.c;
      return true;
    }
  };
  
} // namespace CS

#endif // __CS_TRIANGLESTREAM_H__
