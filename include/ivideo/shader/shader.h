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


#ifndef __SHADER_H__
#define __SHADER_H__

#include "csutil/ref.h"
#include "csutil/scf.h"

class csVector3;
struct iString;

struct iShaderManager;
struct iShaderRenderInterface;
struct iShader;
struct iShaderVariable;
struct iShaderTechnique;
struct iShaderPass;
struct iShaderProgram;

SCF_VERSION (iShaderManager, 0,0,1);

/**
 * A manager for all shaders. Will only be one at a given time
 */
struct iShaderManager : iBase
{
  /// Creates a shader from specified file
  virtual csPtr<iShader> CreateShader(const char* filename) = 0;
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

  /// Create a shaderpgoram from a shaderfile
  virtual csPtr<iShaderProgram> CreateShaderProgramFromFile(const char* programfile, const char* type) = 0;
  /// Create a shaderprogram from a string describing it
  virtual csPtr<iShaderProgram> CreateShaderProgramFromString(const char* programstring, const char* type) = 0;
};

SCF_VERSION (iShaderRenderInterface, 0,0,1);
struct iShaderRenderInterface : iBase
{
  /// Create a shaderprogram from a string describing it
  virtual csPtr<iShaderProgram> CreateShaderProgram(const char* programstring, void* parameters, const char* type) = 0;
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
};

SCF_VERSION (iShaderVariable, 0,0,1);
struct iShaderVariable : iBase
{
  virtual const char* GetName() = 0;
  virtual bool GetValue(int& value) = 0;
  virtual bool GetValue(iString* value) = 0;
  virtual bool GetValue(csVector3& value) = 0;
//  virtual bool GetValue(csVector4* value) = 0;
  virtual bool SetValue(int value) = 0;
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
};

SCF_VERSION (iShaderProgram, 0,0,1);
/** 
 * A shader-program is either a vertexprogram, fragmentprogram or any
 * other type of "program" utilizied by shader.
 */
struct iShaderProgram : iBase
{

  /// Sets this program to be the one used when rendering
  virtual void Activate() = 0;

  /// Deactivate program so that it's not used in next rendering
  virtual void Deactivate() = 0;

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
};

#endif //__SHADER_H__