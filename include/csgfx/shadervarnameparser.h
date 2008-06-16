/*
    Copyright (C) 2008 by Frank Richter

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

#ifndef __CS_CSGFX_SHADERVARNAMEPARSER_H__
#define __CS_CSGFX_SHADERVARNAMEPARSER_H__

#include "csutil/array.h"
#include "csutil/csstring.h"

/**\file
 * Helper to extract the actual shader var name and array indices
 * given a shader var identifier containing both.
 */

namespace CS
{
  namespace Graphics
  {
    /**
     * Helper class to extract the actual shader var name and array indices
     * given a shader var identifier containing both.
     */
    class CS_CRYSTALSPACE_EXPORT ShaderVarNameParser
    {
      csString name;
      csArray<size_t, csArrayElementHandler<size_t>,
        CS::Memory::LocalBufferAllocator<size_t, 2,
          CS::Memory::AllocatorMalloc, true> > indices;
      size_t errorPos;
    public:
      /// Set up with a shader var identifier.
      ShaderVarNameParser (const char* identifier);
      
      /**
       * Return the location of a parse error in the identifier
       * (or <tt>(size_t)~0</tt> if none occured)
       */
      size_t GetErrorPosition () const { return errorPos; }
      
      /// Get the name part of the shader var.
      const char* GetShaderVarName () const { return name; }
      
      /// Get how many indices there are.
      size_t GetIndexNum() const { return indices.GetSize(); }
      /// Get the nth index value. Index 0 is leftmost.
      size_t GetIndexValue (size_t index) const { return indices[index]; }
      
      /// Fill some array with the indices
      template<typename ArrayType>
      void FillArrayWithIndices (ArrayType& arr) const
      {
        for (size_t i = 0; i < indices.GetSize(); i++)
          arr.Push (indices[i]);
      }
    };
  } // namespace Graphics
} // namespace CS

#endif // __CS_CSGFX_SHADERVARNAMEPARSER_H__
