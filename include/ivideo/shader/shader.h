/*
    Copyright (C) 2002 by Mårten Svanfeldt
                          Anders Stenberg

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


#ifndef __CS_IVIDEO_SHADER_H__
#define __CS_IVIDEO_SHADER_H__

#include "csgeom/vector4.h"
#include "csutil/ref.h"
#include "csutil/refarr.h"
#include "csutil/scf.h"
#include "csutil/strhash.h"
#include "ivideo/graph3d.h"
#include "ivideo/rendermesh.h"
#include "csgfx/shadervar.h"

struct iString;
struct iDataBuffer;
struct iDocumentNode;
struct iMaterial;

struct iShaderManager;
struct iShaderRenderInterface;
struct iShader;
struct iShaderTechnique;
struct iShaderPass;
struct iShaderProgram;
struct iShaderProgramPlugin;
struct iShaderRenderInterface;

SCF_VERSION (iShaderVariableContext, 0,1,0);

/**
 * This is a baseclass for all interfaces which provides shadervariables
 * both dynamically and static
 */
struct iShaderVariableContext : iBase
{
  /// Add a variable to this context
  virtual void AddVariable (csShaderVariable *variable) = 0;
  
  /// Get a named variable from this context
  virtual csShaderVariable* GetVariable (csStringID name) const = 0;

  /// Get a named variable from this context, and any context above/outer
  virtual csShaderVariable* GetVariableRecursive (csStringID name) const = 0;

  /// Fill a csShaderVariableList
  virtual void FillVariableList (csShaderVariableList *list) const = 0;
};

SCF_VERSION (iShaderManager, 0, 1, 0);

/**
 * A manager for all shaders. Will only be one at a given time
 */
struct iShaderManager : iShaderVariableContext
{
  /// Create an empty shader
  virtual csPtr<iShader> CreateShader() = 0;
  /// Get a shader by name
  virtual iShader* GetShader(const char* name) = 0;
  /// Returns all shaders that have been created
  virtual const csRefArray<iShader> &GetShaders ()  = 0;

  /// Create variable
  virtual csPtr<csShaderVariable> CreateVariable(csStringID name) const = 0;

  /// Create a shaderprogram
  virtual csPtr<iShaderProgram> CreateShaderProgram(const char* type) = 0;

  /// Prepare all created shaders
  virtual void PrepareShaders () = 0;
};

SCF_VERSION (iShaderRenderInterface, 0,0,1);

/// Document me!@@@
struct iShaderRenderInterface : iBase
{
  /// Get a implementationspecific object
  virtual void* GetPrivateObject(const char* name) = 0;
};

SCF_VERSION (iShader, 0,0,1);

/**
 * Specific shader. Can/will be either render-specific or general
 */
struct iShader : iShaderVariableContext
{
  /// Set this shader's name
  virtual void SetName(const char* name) = 0;
  /// Retrieve name of shader
  virtual const char* GetName() = 0;

  /// Create a new technique
  virtual csPtr<iShaderTechnique> CreateTechnique() = 0;
  /// Get number of techniques
  virtual int GetTechniqueCount() const = 0;
  /// Retrieve a technique
  virtual iShaderTechnique* GetTechnique(int technique) = 0;
  /// Retrieve the best technique in this shader
  virtual iShaderTechnique* GetBestTechnique() = 0;

  /**
   * Check if valid (normaly a shader is valid if there is at least one
   * valid technique)
   */
  virtual bool IsValid() const = 0;

  /// Loads a shader from buffer
  virtual bool Load(iDataBuffer* program) = 0;

  /// Loads from a document-node
  virtual bool Load(iDocumentNode* node) = 0;

  /**
   * Prepares the shader for usage. Must be called before the shader is
   * assigned to a material.
   */
  virtual bool Prepare() = 0;
};


SCF_VERSION (iShaderTechnique, 0,0,1);

/**
 * One specific technique used by shader.
 */
struct iShaderTechnique : iShaderVariableContext
{
  /**
   * Get technique priority. If there are several valid techniques
   * use the one with highest priority
   */
  virtual int GetPriority() const = 0;

  /// Set technique priority.
  virtual void SetPriority(int priority) = 0;

  /// Create a pass
  virtual csPtr<iShaderPass> CreatePass() = 0;
  /// Get number of passes
  virtual int GetPassCount() const = 0;
  /// Retrieve a pass
  virtual iShaderPass* GetPass( int pass ) = 0;

  /// Check if valid
  virtual bool IsValid() const = 0;

  /// Loads a technique from buffer
  virtual bool Load(iDataBuffer* program) = 0;

  /// Loads from a document-node
  virtual bool Load(iDocumentNode* node) = 0;

  /// Prepares the technique for usage.
  virtual bool Prepare() = 0;
};

SCF_VERSION (iShaderPass, 0,0,1);

/**
 * Description of a single pass in  a shader
 */
struct iShaderPass : iShaderVariableContext
{
  /// Add a stream mapping
  virtual void AddStreamMapping (csStringID name, csVertexAttrib attribute) = 0;
  /// Get stream mapping for a certain attribute
  virtual csStringID GetStreamMapping (csVertexAttrib attribute) const = 0;

  /// Add a texture mapping
  virtual void AddTextureMapping (csStringID name, int unit) = 0;
  /// Get texture mapping for a certain unit
  virtual csStringID GetTextureMapping (int unit) const = 0;

  /// Get mixmode override
  virtual uint GetMixmodeOverride () const = 0;

  /// Get vertex-program
  virtual iShaderProgram* GetVertexProgram() = 0;

  /// Set vertex-program
  virtual void SetVertexProgram(iShaderProgram* program) = 0;

  /// Get fragment-program
  virtual iShaderProgram* GetFragmentProgram() = 0;

  /// Set fragment-program
  virtual void SetFragmentProgram(iShaderProgram* program) = 0;

  /// Check if valid
  virtual bool IsValid() const = 0;

  /// Activate the whole pass for the indicated mesh (which might be 0)
  virtual void Activate(csRenderMesh* mesh) = 0;

  /// Deactivate the whole pass
  virtual void Deactivate() = 0;

  /// Setup states needed for proper operation of the shader
  virtual void SetupState (csRenderMesh* mesh,
    csArray<iShaderVariableContext*> &dynamicDomains) = 0;

  /// Reset states to original
  virtual void ResetState () = 0;

  /// Loads pass from buffer
  virtual bool Load(iDataBuffer* program) = 0;

  /// Loads from a document-node
  virtual bool Load(iDocumentNode* node) = 0;

  /// Prepares the pass for usage.
  virtual bool Prepare() = 0;
};

SCF_VERSION (iShaderProgram, 0,0,2);

/** 
 * A shader-program is either a vertexprogram, fragmentprogram or any
 * other type of "program" utilizied by shader.
 */
struct iShaderProgram : iShaderVariableContext
{
  /// Sets this program to be the one used when rendering
  virtual void Activate(csRenderMesh* mesh) = 0;

  /// Deactivate program so that it's not used in next rendering
  virtual void Deactivate() = 0;

  /// Setup states needed for proper operation of the shader
  virtual void SetupState (csRenderMesh* mesh, 
    csArray<iShaderVariableContext*> &dynamicDomains) = 0;

  /// Reset states to original
  virtual void ResetState () = 0;

  /// Check if valid
  virtual bool IsValid() = 0;

  /// Loads shaderprogram from buffer
  virtual bool Load(iDataBuffer* program) = 0;

  /// Loads from a document-node
  virtual bool Load(iDocumentNode* node) = 0;

  /**
   * Prepares the shaderprogram for usage.
   * Must be called before the shader is assigned to a material.
   */
  virtual bool Prepare(iShaderPass* pass) = 0;
};

SCF_VERSION(iShaderProgramPlugin, 0,0,1);

/// Document me!@@@
struct iShaderProgramPlugin : iBase
{
  virtual csPtr<iShaderProgram> CreateProgram(const char* type) = 0;
  virtual bool SupportType(const char* type) = 0;
  virtual void Open() = 0;
};

#endif // __CS_IVIDEO_SHADER_H__
