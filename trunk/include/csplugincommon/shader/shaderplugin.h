/*
  Copyright (C) 2003 by Marten Svanfeldt

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CS_SHADERPLUGIN_H__
#define __CS_SHADERPLUGIN_H__

/**\file
 * Shader plugin related interfaces.
 */

#include "csutil/scf.h"
#include "csutil/strhash.h"

#include "ivideo/shader/shader.h"

struct iString;

/**\addtogroup plugincommon
 * @{ */

/// Mapping of a shader variable to some shader-specific destination
struct csShaderVarMapping
{
  /// Shader variable name
  CS::ShaderVarStringID name;
  /// Destination
  csString destination;
  csShaderVarMapping (CS::ShaderVarStringID n, const char* d) : name(n),
    destination(d) {}
};


/**
 * Interface to allow resolution of friendly destination names. Passed when a 
 * shader program is loaded, used to resolve unknown texture unit names etc.
 */
struct iShaderDestinationResolver : public virtual iBase
{
  SCF_INTERFACE(iShaderDestinationResolver, 0,0,1);
  /**
   * When the destination of a texture binding wasn't recognized, the FP
   * is asked whether it can provide a TU number for it.
   */
  virtual int ResolveTU (const char* binding) = 0;

  virtual csVertexAttrib ResolveBufferDestination (const char* binding) = 0;
};

/**
 * A helper for shaders that which to use the general plugins.
 * This is the main program plugin interface
 */
struct iShaderProgram : public virtual iBase
{
  SCF_INTERFACE(iShaderProgram, 7, 0, 0);
  /// Sets this program to be the one used when rendering
  virtual void Activate() = 0;

  /// Deactivate program so that it's not used in next rendering
  virtual void Deactivate() = 0;

  /// Setup states needed for proper operation of the shaderprogram
  virtual void SetupState (const CS::Graphics::RenderMesh* mesh, 
                           CS::Graphics::RenderMeshModes& modes,
                           const csShaderVariableStack& stack) = 0;

  /// Reset states to original
  virtual void ResetState () = 0;

  /// Loads from a document-node
  virtual bool Load (iShaderDestinationResolver* resolve, 
    iDocumentNode* node) = 0;

  /// Loads from raw text
  virtual bool Load (iShaderDestinationResolver* resolve, const char* program, 
    csArray<csShaderVarMapping>& mappings) = 0;

  /**
   * Compile a program.
   * If \a cacheTo is given, the shader program can store the compiled
   * program so it can later be restored using LoadFromCache().
   * \remark A program can expect that Compile() is only called once, and
   *   all calls after the first can fail.
   */
  virtual bool Compile (iHierarchicalCache* cacheTo,
    csRef<iString>* cacheTag = 0) = 0;
  
  /**
   * Request all shader variables used by a certain shader ticket.
   * \param ticket The ticket for which to retrieve the information.
   * \param bits Bit array with one bit for each shader variable set; if a 
   *   shader variable is used, the bit corresponding to the name of the
   *   variable is note set. Please note: first, the array passed in must 
   *   initially have enough bits for all possible shader variables, it will 
   *   not be resized - thus a good size would be the number of strings in the
   *   shader variable string set. Second, bits corresponding to unused
   *   shader variables will not be reset. It is the responsibility of the 
   *   caller to do so.
   */
  virtual void GetUsedShaderVars (csBitArray& bits) const = 0;
  
  enum CacheLoadResult
  {
    loadFail,
    loadSuccessShaderInvalid,
    loadSuccessShaderValid
  };
  /// Loads from a cache
  virtual CacheLoadResult LoadFromCache (iHierarchicalCache* cache,
    iBase* previous, iDocumentNode* programNode,
    csRef<iString>* failReason = 0, csRef<iString>* cacheTag = 0) = 0;
};

/**
 * Plugins which provide iShaderProgram should implement this as a factory
 * for iShaderProgram
 */
struct iShaderProgramPlugin : public virtual iBase
{
  SCF_INTERFACE(iShaderProgramPlugin,3,0,0);
  virtual csPtr<iShaderProgram> CreateProgram (const char* type) = 0;
  virtual bool SupportType (const char* type) = 0;
  
  virtual csPtr<iStringArray> QueryPrecacheTags (const char* type) = 0;
  /**
   * Warm the given cache with the program specified in \a node.
   * \a outObj can return an object which exposes iShaderDestinationResolver.
   */
  virtual bool Precache (const char* type, const char* tag,
    iBase* previous, iDocumentNode* node, 
    iHierarchicalCache* cacheTo, csRef<iBase>* outObj = 0) = 0;
};

/** @} */

#endif

