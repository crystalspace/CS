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

#ifndef __CS_CSGFX_SHADERVARARRAYHELPER_H__
#define __CS_CSGFX_SHADERVARARRAYHELPER_H__

#include "ivideo/shader/shader.h"
#include "csgfx/shadervarnameparser.h"

/**\file
 * Helper to retrieve a shader variable from multiple nested shader variable
 * arrays.
 */

namespace CS
{
  namespace Graphics
  {
    class CS_CRYSTALSPACE_EXPORT ShaderVarArrayHelper
    {
    public:
      /** 
       * Action to take when a nested shader variable is not present
       */
      enum MissingAction
      {
        /// Create all missing shader variables, enlarge arrays if necessary
        maCreate,
        /// If an array item is 0, create it, but never enlarge arrays
        maCreateNoEnlarge,
        /// Fail when a variable is missing
        maFail
      };
      /// Error status
      enum Error
      {
        /// The desired variable was retrieved
        errSuccess,
        /// The variable at the error level is not an array
        errNoArray,
        /// The variable at the error level is too small
        errArrayToSmall,
        /// There was no valid shader variable at the requested index
        errNoEntry
      };
    
      /**
       * Retrieve a shader variable from multiple nested shader variable
       * arrays.
       * \param outer The outmost shader variable - the lookup of level 0
       *   is done in it. Cannot be 0.
       * \param indices Indices into arrays and sub-arrays.
       * \param errorStatus Optionally indicates whether the retrieval was
       *   successful.
       * \param errorLevel Optionally indicates the nesting level at which
       *   the error occured. 0 mean the level is at the top level; 1 means
       *   the level is one below the top level; and so on.
       * \return If no error occured, the nested variable that corresponds to
       *   the given indices. If an error occured, the return value depends on
       *   whether \a errorStatus is a valid pointer or not: if not, an error 
       *   is indicated by returning 0. If \a errorStatus is a valid pointer,
       *   errors are indicated by that, and the return value is the innermost
       *   shader variable that could be retrieved.
       */
      static csShaderVariable* GetArrayItem (csShaderVariable* outer,
        const ShaderVarNameParser& indices, MissingAction missingAction,
        Error* errorStatus = 0, size_t* errorLevel = 0);
    
      /**
       * Retrieve a shader variable from multiple nested shader variable
       * arrays.
       * \param outer The outmost shader variable - the lookup of level 0
       *   is done in it. Cannot be 0.
       * \param indices Pointer to indices into arrays and sub-arrays.
       * \param indexNum Number of items in \a indices.
       * \param errorStatus Optionally indicates whether the retrieval was
       *   successful.
       * \param errorLevel Optionally indicates the nesting level at which
       *   the error occured. 0 mean the level is at the top level; 1 means
       *   the level is one below the top level; and so on.
       * \return If no error occured, the nested variable that corresponds to
       *   the given indices. If an error occured, the return value depends on
       *   whether \a errorStatus is a valid pointer or not: if not, an error 
       *   is indicated by returning 0. If \a errorStatus is a valid pointer,
       *   errors are indicated by that, and the return value is the innermost
       *   shader variable that could be retrieved.
       */
      static csShaderVariable* GetArrayItem (csShaderVariable* outer,
        const size_t* indices, size_t indexNum, MissingAction missingAction,
        Error* errorStatus = 0, size_t* errorLevel = 0);
    };
  } // namespace Graphics
} // namespace CS

#endif // __CS_CSGFX_SHADERVARARRAYHELPER_H__
