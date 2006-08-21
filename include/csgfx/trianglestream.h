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
 */

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
    TriangleIndicesStream ()
    {
    }

    /**
     * Begin triangulation of an index buffer.
     * \param index Pointer to start of the indices.
     * \param indexEnd Pointer to end of the indices.
     * \param stride Distance between index elements in bytes.
     * \param meshType Mesh type of the index data.
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

    /// Returns whether a triangle is available.
    bool HasNextTri() const
    {
      return (index < indexEnd);
    }
    /// Fetches the next triangle from the buffer.
    bool NextTriangle (T& a, T& b, T& c)
    {
      if (index < indexEnd)
      {
	switch (meshtype)
	{
	case CS_MESHTYPE_TRIANGLES:
	  {
            a = GetIndex (0);
            b = GetIndex (1);
            c = GetIndex (2);
	    index += 3*csRenderBufferComponentSizes[compType];
            return true;
	  }
	  break;
	case CS_MESHTYPE_TRIANGLESTRIP:
	  {
	    const uint cur = GetNextIndex();
            a = old1;
            b = old2;
            c = cur;
            if (stripFlag)
	      old2 = cur;
            else
              old1 = cur;
            stripFlag = !stripFlag;
            return true;
	  }
	  break;
	case CS_MESHTYPE_TRIANGLEFAN:
	  {
	    const uint cur = GetNextIndex();
            a = old2;
            b = old1;
            c = cur;
	    old1 = cur;
            return true;
	  }
          break;
	case CS_MESHTYPE_QUADS:
	  {
	    if (quadPart == 0)
            {
              a = GetIndex (0);
              b = GetIndex (1);
              c = GetIndex (2);
            }
	    else
	    {
              a = GetIndex (0);
              b = GetIndex (2);
              c = GetIndex (3);
	      index += 4*csRenderBufferComponentSizes[compType];
	    }
	    quadPart ^= 1;
            return true;
	  }
	  break;
	default:
	  ;
	}
      }
      return false;
    }
  };
  
} // namespace CS

#endif // __CS_TRIANGLESTREAM_H__
