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

#include "csutil/ref.h"
#include "csutil/scf.h"
#include "ivideo/render3d.h"

class csVector3;
struct iString;
struct iDataBuffer;
struct iDocumentNode;

struct iShaderManager;
struct iShaderRenderInterface;
struct iShader;
struct iShaderVariable;
struct iShaderTechnique;
struct iShaderPass;
struct iShaderProgram;
struct iShaderProgramPlugin;


SCF_VERSION (iShaderManager, 0,0,1);
/**
 * A manager for all shaders. Will only be one at a given time
 */
struct iShaderManager : iBase
{
  /// Create a empty shader
  virtual csPtr<iShader> CreateShader() = 0;
  /// Get a shader by name
  virtual iShader* GetShader(const char* name) = 0;

  /// Create variable
  virtual csPtr<iShaderVariable> CreateVariable(const char* name) = 0;
  /// Add a variable to this context
  virtual bool AddVariable(iShaderVariable* variable) = 0;
  /// Get variable
  virtual iShaderVariable* GetVariable(const char* string) = 0;
  /// Get all variable stringnames added to this context (used when creatingthem)
  virtual csBasicVector GetAllVariableNames() = 0; 

  /// Create a shaderprogram
  virtual csPtr<iShaderProgram> CreateShaderProgram(const char* type) = 0;
};

SCF_VERSION (iShaderRenderInterface, 0,0,1);
struct iShaderRenderInterface : iBase
{
  /// Get a implementationspecific object
  virtual csSome GetObject(const char* name) = 0;
};

SCF_VERSION (iShader, 0,0,1);
/**
 * Specific shader. Can/will be either render-specific or general
 */
struct iShader : iBase
{
  /// Sets a stream-mapping
  virtual void MapStream( int mapped_id, const char* streamname) = 0;

  /// Set this shader's name
  virtual void SetName( const char* name ) = 0;
  /// Retrieve name of shader
  virtual const char* GetName() = 0;

  /// Create a new technique
  virtual csPtr<iShaderTechnique> CreateTechnique() = 0;
  /// Get number of techniques
  virtual int GetTechniqueCount() = 0;
  /// Retrieve a technique
  virtual iShaderTechnique* GetTechnique( int technique ) = 0;
  /// Retrieve the best technique in this shader
  virtual iShaderTechnique* GetBestTechnique() = 0;

  /// Add a variable to this context
  virtual bool AddVariable(iShaderVariable* variable) = 0;
  /// Get variable
  virtual iShaderVariable* GetVariable(const char* string) = 0;
  /// Get all variable stringnames in this context (used when creatingthem)
  virtual csBasicVector GetAllVariableNames() = 0; 

  /// Check if valid (normaly a shader is valid if there is at least one valid technique)
  virtual bool IsValid() = 0;

  /// Loads a shader from buffer
  virtual bool Load(iDataBuffer* program) = 0;

  /// Loads from a document-node
  virtual bool Load(iDocumentNode* node) = 0;

  /// Prepares the shader for usage. Must be called before the shader is assigned to a material
  virtual bool Prepare() = 0;
};

SCF_VERSION (iShaderVariable, 0,0,1);
struct iShaderVariable : iBase
{
  enum VariableType
  {
    INT = 1,
    STRING,
    VECTOR1,
    VECTOR3,
    VECTOR4
  };

  virtual VariableType GetType() = 0;
  virtual void SetType(VariableType) = 0;

  virtual const char* GetName() = 0;
  virtual bool GetValue(int& value) = 0;
  virtual bool GetValue(float& value) = 0;
  virtual bool GetValue(iString* value) = 0;
  virtual bool GetValue(csVector3& value) = 0;
//  virtual bool GetValue(csVector4* value) = 0;
  virtual bool SetValue(int value) = 0;
  virtual bool SetValue(float value) = 0;
  virtual bool SetValue(iString* value) = 0;
  virtual bool SetValue(csVector3 value) = 0;
//  virtual bool SetValue(csVector4* value) = 0;
};

SCF_VERSION (iShaderTechnique, 0,0,1);
/**
 * One specific technique used by shader.
 */
struct iShaderTechnique : iBase
{
  /// Sets a stream-mapping
  virtual void MapStream( int mapped_id, const char* streamname) = 0;

  /* Get technique priority. If there are several valid techniques
   * use the one with highest priority
   */
  virtual int GetPriority() = 0;

  /// Set technique priority.
  virtual void SetPriority(int priority) = 0;

  /// Create a pass
  virtual csPtr<iShaderPass> CreatePass() = 0;
  /// Get number of passes
  virtual int GetPassCount() = 0;
  /// Retrieve a pass
  virtual iShaderPass* GetPass( int pass ) = 0;

  /// Check if valid
  virtual bool IsValid() = 0;

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
struct iShaderPass : iBase
{
  /// Get vertex-program
  virtual iShaderProgram* GetVertexProgram() = 0;

  /// Set vertex-program
  virtual void SetVertexProgram(iShaderProgram* program) = 0;

  /// Get fragment-program
  virtual iShaderProgram* GetFragmentProgram() = 0;

  /// Set fragment-program
  virtual void SetFragmentProgram(iShaderProgram* program) = 0;

  /// Check if valid
  virtual bool IsValid() = 0;

  /// Activate the whole pass for the indicated mesh (which might be NULL)
  virtual void Activate(csRenderMesh* mesh) = 0;

  /// Deactivate the whole pass
  virtual void Deactivate() = 0;

  /// Add a variable to this context
  virtual bool AddVariable(iShaderVariable* variable) = 0;
  /// Get variable
  virtual iShaderVariable* GetVariable(const char* string) = 0;
  /// Get all variable stringnames added to this context (used when creatingthem)
  virtual csBasicVector GetAllVariableNames() = 0; 

  /// Loads pass from buffer
  virtual bool Load(iDataBuffer* program) = 0;

  /// Loads from a document-node
  virtual bool Load(iDocumentNode* node) = 0;

  /// Prepares the pass for usage.
  virtual bool Prepare() = 0;
};

SCF_VERSION (iShaderProgram, 0,0,1);
/** 
 * A shader-program is either a vertexprogram, fragmentprogram or any
 * other type of "program" utilizied by shader.
 */
struct iShaderProgram : iBase
{
  /// Get a programid for the current program
  virtual csPtr<iString> GetProgramID() = 0;

  /// Sets this program to be the one used when rendering
  virtual void Activate(iShaderPass* current, csRenderMesh* mesh) = 0;

  /// Deactivate program so that it's not used in next rendering
  virtual void Deactivate(iShaderPass* current) = 0;

  /* Propertybag - get property, return false if no such property found
   * Which properties there is is implementation specific
   */
  virtual bool GetProperty(const char* name, iString* string) = 0;
  virtual bool GetProperty(const char* name, int* string) = 0;
  virtual bool GetProperty(const char* name, csVector3* string) = 0;
//  virtual bool GetProperty(const char* name, csVector4* string) = 0;

  /* Propertybag - set property.
   * Which properties there is is implementation specific
   */
  virtual bool SetProperty(const char* name, iString* string) = 0;
  virtual bool SetProperty(const char* name, int* string) = 0;
  virtual bool SetProperty(const char* name, csVector3* string) = 0;
//  virtual bool SetProperty(const char* name, csVector4* string) = 0;

  /// Add a variable to this context
  virtual bool AddVariable(iShaderVariable* variable) = 0;
  /// Get variable
  virtual iShaderVariable* GetVariable(const char* string) = 0;
  /// Get all variable stringnames added to this context (used when creatingthem)
  virtual csBasicVector GetAllVariableNames() = 0; 

  /// Check if valid
  virtual bool IsValid() = 0;

  /// Loads shaderprogram from buffer
  virtual bool Load(iDataBuffer* program) = 0;

  /// Loads from a document-node
  virtual bool Load(iDocumentNode* node) = 0;

  /// Prepares the shaderprogram for usage. Must be called before the shader is assigned to a material
  virtual bool Prepare() = 0;
};

SCF_VERSION(iShaderProgramPlugin, 0,0,1);
struct iShaderProgramPlugin : iBase
{
  virtual csPtr<iShaderProgram> CreateProgram() = 0  ;
  virtual bool SupportType(const char* type) = 0;
  virtual void Open() = 0;
};

#endif // __CS_IVIDEO_SHADER_H__
