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

#include "cssysdef.h"

#include "csgfx/shadervararrayhelper.h"

namespace CS
{
  namespace Graphics
  {
    template<typename T>
    struct OptionalPointerWrapper
    {
      T* p;
      
      OptionalPointerWrapper (T* p) : p (p) { }
      
      void Set (const T& val)
      {
        if (p) *p = val;
      }
    };
  
    template<typename IndicesSource>
    static csShaderVariable* GetArrayItemWorker (csShaderVariable* outer, 
      const IndicesSource& indices, 
      ShaderVarArrayHelper::MissingAction missingAction, 
      ShaderVarArrayHelper::Error* _errorStatus, size_t* _errorLevel)
    {
      OptionalPointerWrapper<ShaderVarArrayHelper::Error> errorStatus (
        _errorStatus);
      OptionalPointerWrapper<size_t> errorLevel (_errorLevel);
      
      CS_ASSERT_MSG ("Outer shader variable cannot be 0", outer != 0);
      
      errorStatus.Set (ShaderVarArrayHelper::errSuccess);
      csShaderVariable* current = outer;
      for (size_t n = 0; n < indices.GetIndexNum (); n++)
      {
        errorLevel.Set (n);
        if (current->GetType () != csShaderVariable::ARRAY)
        {
          if ((current->GetType() == csShaderVariable::UNKNOWN)
            && (missingAction == ShaderVarArrayHelper::maCreate))
          {
            /* Only makes sense to set to an array when maCreate is set,
             * maCreateNoEnlarge it would fail on the array size check
             * anyway */
            current->SetType (csShaderVariable::ARRAY);
          }
          else
          {
            errorStatus.Set (ShaderVarArrayHelper::errNoArray);
            return _errorStatus ? current : 0;
          }
        }
        
        const size_t index = indices.GetIndexValue (n);
        if (current->GetArraySize() <= index)
        {
          if (missingAction == ShaderVarArrayHelper::maCreate)
          {
            current->SetArraySize (index+1);
          }
          else
          {
            errorStatus.Set (ShaderVarArrayHelper::errArrayToSmall);
            return _errorStatus ? current : 0;
          }
        }
        csShaderVariable* next = current->GetArrayElement (index);
        if (next == 0)
        {
          if (missingAction != ShaderVarArrayHelper::maFail)
          {
            next = new csShaderVariable ();
            current->SetArrayElement (index, next);
          }
          else
          {
            errorStatus.Set (ShaderVarArrayHelper::errNoEntry);
            return _errorStatus ? current : 0;
          }
        }
        current = next;
      }
      return current;
    }
  
    csShaderVariable* ShaderVarArrayHelper::GetArrayItem (
      csShaderVariable* outer, const ShaderVarNameParser& indices, 
      MissingAction missingAction, Error* errorStatus,
      size_t* errorLevel)
    {
      return GetArrayItemWorker (outer, indices, missingAction, errorStatus,
        errorLevel);
    }
    
    struct IndicesArraySource
    {
      const size_t* indices;
      size_t indexNum;
      
      IndicesArraySource (const size_t* indices, size_t indexNum)
        : indices (indices), indexNum (indexNum) { }
        
      size_t GetIndexNum() const { return indexNum; }
      size_t GetIndexValue (size_t index) const
      { 
        CS_ASSERT(index < indexNum);
        return indices[index]; 
      }
    };
    
    csShaderVariable* ShaderVarArrayHelper::GetArrayItem (
      csShaderVariable* outer, const size_t* indices, size_t indexNum, 
      MissingAction missingAction, Error* errorStatus,
      size_t* errorLevel)
    {
      IndicesArraySource indicesSource (indices, indexNum);
      return GetArrayItemWorker (outer, indicesSource, missingAction, 
        errorStatus, errorLevel);
    }
    
  } // namespace Graphics
} // namespace CS
