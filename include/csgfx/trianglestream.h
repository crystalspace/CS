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

#include "csgeom/tri.h"
#include "ivideo/graph3d.h"
#include "ivideo/rndbuf.h"

#ifdef CS_COMPILER_MSVC
#pragma warning(push)
/* Apparently, whatever return type MSVC sees first for GetIndex() below is
   taken as "the" return type, at least when it comes to return value precision
   checking: sometimes there is a lot of warning spam that assigning size_t to
   <something else> may lose precision. */
#pragma warning(disable:4267)
#endif

namespace CS
{
  
  /**
   * Helper class to extract triangles from an index buffer. Automatically
   * handles tristrips, quads etc. as well.
   */
  template<typename T>
  class TriangleIndicesStream
  {
  protected:
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
    /// Get element \a idx, based on \a index
    T GetIndex (size_t idx, const uint8* index) const
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
    TriangleIndicesStream () : old2(0), old1(0), buf (0) { }
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
			   size_t indexEnd = (size_t)~0) : old2(0), old1(0)
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
    void BeginTriangulate (const uint8* indexStart, const uint8* indexEnd,
      size_t stride, csRenderBufferComponentType compType,
      csRenderMeshType meshtype)
    {
      this->index = indexStart;
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
	  t.a = GetIndex (0, index);
	  t.b = GetIndex (1, index);
	  t.c = GetIndex (2, index);
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
	    t.a = GetIndex (0, index);
	    t.b = GetIndex (1, index);
	    t.c = GetIndex (2, index);
	  }
	  else
	  {
	    t.a = GetIndex (0, index);
	    t.b = GetIndex (2, index);
	    t.c = GetIndex (3, index);
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
    
    /**
     * Get the remaining components in the buffer.
     * Note that this number does't have to correspond with the remaining
     * number of triangles, especially when dealing with strips or fans.
     */
    size_t GetRemainingComponents() const
    {
      size_t size;
      switch (compType)
      {
        default:
          CS_ASSERT(false);
        case CS_BUFCOMP_BYTE:           size = sizeof (char); break;
        case CS_BUFCOMP_UNSIGNED_BYTE:  size = sizeof (unsigned char); break;
        case CS_BUFCOMP_SHORT:          size = sizeof (short); break;
        case CS_BUFCOMP_UNSIGNED_SHORT: size = sizeof (unsigned short); break;
        case CS_BUFCOMP_INT:            size = sizeof (int); break;
        case CS_BUFCOMP_UNSIGNED_INT:   size = sizeof (unsigned int); break;
        case CS_BUFCOMP_FLOAT:          size = sizeof (float); break;
        case CS_BUFCOMP_DOUBLE:         size = sizeof (double); break;
      }
      return (indexEnd - index) / size;
    }
  };
  
  /**
   * Extracts triangles like TriangleIndicesStream, but also provides random
   * access to individual triangles and can be resetted.
   * \remarks There is a slight overhead due to the housekeeping required for
   *   the random access. Hence for pure triangle streaming the "plain"
   *   TriangleIndicesStream is more appropriate.
   */
  template<typename T>
  class TriangleIndicesStreamRandom : public TriangleIndicesStream<T>
  {
  protected:
    const uint8* indexStart;
    size_t triangleNum;
    const uint8* streamIndex;
    size_t streamTriangleNum;
    
    // Update internal stream position from TriangleIndicesStream position
    void SwitchToInternalStreaming ()
    {
      if (streamIndex != 0) return;
      streamIndex = this->index;
      streamTriangleNum = triangleNum;
    }
    /* Restore TriangleIndicesStream with the position set before using
     * "internal" streaming */
    void SwitchToExternalStreaming ()
    {
      if (streamIndex == 0) return;
      this->index = streamIndex;
      triangleNum = streamTriangleNum;
      streamIndex = 0;
    }
  
    template<typename T2>
    void GetTriangleFastDefault (TriangleT<T2>& tri, size_t index) const
    {
      tri.a = T2 (this->GetIndex (index*3+0, indexStart));
      tri.b = T2 (this->GetIndex (index*3+1, indexStart));
      tri.c = T2 (this->GetIndex (index*3+2, indexStart));
    }
    void GetTriangleFast (TriangleT<char>& tri, size_t index) const
    {
      if (this->compType == CS_BUFCOMP_BYTE)
        memcpy (&tri, indexStart + (index*3), sizeof (char)*3);
      else
        GetTriangleFastDefault (tri, index);
    }
    void GetTriangleFast (TriangleT<unsigned char>& tri, size_t index) const
    {
      if (this->compType == CS_BUFCOMP_UNSIGNED_BYTE)
        memcpy (&tri, indexStart + (index*3), sizeof (unsigned char)*3);
      else
        GetTriangleFastDefault (tri, index);
    }
    void GetTriangleFast (TriangleT<short>& tri, size_t index) const
    {
      if (this->compType == CS_BUFCOMP_SHORT)
        memcpy (&tri, indexStart + (index*3), sizeof (short)*3);
      else
        GetTriangleFastDefault (tri, index);
    }
    void GetTriangleFast (TriangleT<unsigned short>& tri, size_t index) const
    {
      if (this->compType == CS_BUFCOMP_UNSIGNED_SHORT)
        memcpy (&tri, indexStart + (index*3), sizeof (unsigned short)*3);
      else
        GetTriangleFastDefault (tri, index);
    }
    void GetTriangleFast (TriangleT<int>& tri, size_t index) const
    {
      if (this->compType == CS_BUFCOMP_INT)
        memcpy (&tri, indexStart + (index*3), sizeof (int)*3);
      else
        GetTriangleFastDefault (tri, index);
    }
    void GetTriangleFast (TriangleT<unsigned int>& tri, size_t index) const
    {
      if (this->compType == CS_BUFCOMP_UNSIGNED_INT)
        memcpy (&tri, indexStart + (index*3), sizeof (unsigned int)*3);
      else
        GetTriangleFastDefault (tri, index);
    }
    void GetTriangleFast (TriangleT<float>& tri, size_t index) const
    {
      if (this->compType == CS_BUFCOMP_FLOAT)
        memcpy (&tri, indexStart + (index*3), sizeof (float)*3);
      else
        GetTriangleFastDefault (tri, index);
    }
    void GetTriangleFast (TriangleT<double>& tri, size_t index) const
    {
      if (this->compType == CS_BUFCOMP_DOUBLE)
        memcpy (&tri, indexStart + (index*3), sizeof (double)*3);
      else
        GetTriangleFastDefault (tri, index);
    }
    TriangleT<T> InternalNext ()
    {
      SwitchToInternalStreaming ();
      TriangleT<T> tri (TriangleIndicesStream<T>::Next ());
      ++triangleNum;
      return tri;
    }
public:
    /**
     * Construct uninitialized triangle stream.
     * \remarks Don't forget to call BeginTriangulate() before using 
     * HasNext() or Next()!
     */
    TriangleIndicesStreamRandom () : TriangleIndicesStream<T> () { }
    /**
     * Construct triangle stream with an index buffer.
     * \param indices Index buffer to triangulate.
     * \param meshtype Mesh type of the index data.
     * \param indexStart Element of the index buffer to start iterating at.
     * \param indexEnd Element of the index buffer to stop iterating at.
     *   (size_t)~0 means last element.
     */
    TriangleIndicesStreamRandom (iRenderBuffer* indices,
      csRenderMeshType meshtype, size_t indexStart = 0, 
      size_t indexEnd = (size_t)~0) : 
      TriangleIndicesStream<T> (indices, meshtype, indexStart, indexEnd)
    {
      streamIndex = this->indexStart = this->index;
      streamTriangleNum = triangleNum = 0;
    }
    ~TriangleIndicesStreamRandom()
    {
    }
    
    /**
     * Begin triangulation of an index buffer.
     * \param index Pointer to start of the indices.
     * \param indexEnd Pointer to end of the indices.
     * \param stride Distance between index elements in bytes.
     * \param compType Type of component contained in the data.
     * \param meshtype Mesh type of the index data.
     */
    void BeginTriangulate (const uint8* indexStart, const uint8* indexEnd,
      size_t stride, csRenderBufferComponentType compType,
      csRenderMeshType meshtype)
    {
      TriangleIndicesStream<T>::BeginTriangulate (indexStart, indexEnd, stride,
        compType, meshtype);
      streamIndex = this->indexStart = this->index;
      streamTriangleNum = triangleNum = 0;
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
      TriangleIndicesStream<T>::BeginTriangulate (indices, meshtype, 
        indexStart, indexEnd);
      streamIndex = this->indexStart = this->index;
      streamTriangleNum = triangleNum = 0;
    }
    
    /// Reset the stream to the start.
    void Reset()
    {
      this->index = indexStart;
      triangleNum = 0;
      streamIndex = 0;
    }

    /// Returns whether a triangle is available.
    bool HasNext() const
    {
      SwitchToExternalStreaming ();
      return TriangleIndicesStream<T>::HasNext ();
    }
    /// Fetches the next triangle from the buffer.
    TriangleT<T> Next ()
    {
      SwitchToExternalStreaming ();
      return TriangleIndicesStream<T>::Next ();
    }
    
    /// Get a specific triangle.
    TriangleT<T> operator[] (size_t index)
    {
      if (this->meshtype == CS_MESHTYPE_TRIANGLES)
      {
        // Simple triangles: direct access
        TriangleT<T> tri;
        GetTriangleFast (tri, index);
        return tri;
      }
      else
      {
        // Strips, fans...: need to iterate over all to find a specific tri
        if (index < triangleNum) Reset();
        while (index > triangleNum) InternalNext ();
        return InternalNext ();
      }
    }
  };
} // namespace CS

#ifdef CS_COMPILER_MSVC
#pragma warning(pop)
#endif

#endif // __CS_TRIANGLESTREAM_H__
