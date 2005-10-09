/*
  Copyright (C) 2005 by Marten Svanfeldt

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

#ifndef __CS_VPROC_VPROC_PROGRAM_H__
#define __CS_VPROC_VPROC_PROGRAM_H__

#include "csutil/bitarray.h"
#include "csplugincommon/shader/shaderplugin.h"
#include "csplugincommon/shader/shaderprogram.h"
#include "vproc_std.h"

class csVProcStandardProgram : public scfImplementationExt0<csVProcStandardProgram,
                                                            csShaderProgram>
{
public:
  CS_LEAKGUARD_DECLARE (csVProcStandardProgram);

  csVProcStandardProgram (csVProc_Std *plug);
  virtual ~csVProcStandardProgram ();

  ////////////////////////////////////////////////////////////////////
  //                      iShaderProgram
  ////////////////////////////////////////////////////////////////////

  /// Sets this program to be the one used when rendering
  virtual void Activate ();

  /// Deactivate program so that it's not used in next rendering
  virtual void Deactivate();

  /// Setup states needed for proper operation of the shader
  virtual void SetupState (const csRenderMesh* mesh,
                           csRenderMeshModes& modes,
                           const csShaderVarStack &stacks);

  /// Reset states to original
  virtual void ResetState ();

  /// Check if valid
  virtual bool IsValid() { return true; } 

  /// Loads from a document-node
  virtual bool Load(iShaderTUResolver* tuResolve, iDocumentNode* node);

  /// Loads from raw text
  virtual bool Load (iShaderTUResolver* tuResolve, const char* program, 
    csArray<csShaderVarMapping>& mappings);


  /// Compile a program
  virtual bool Compile();

  virtual int ResolveTextureBinding (const char* binding)
  { return -1; }

private:
  csVProc_Std *shaderPlugin;

  csStringHash tokens;
#define CS_TOKEN_ITEM_FILE \
  "plugins/video/render3d/shader/shaderplugins/vproc_std/vproc_program.tok"
#include "cstool/tokenlist.h"
#undef CS_TOKEN_ITEM_FILE

  //light calculation parameters
  enum LightMixmode
  {
    LIGHTMIXMODE_NONE = 0,
    LIGHTMIXMODE_ADD = 1,
    LIGHTMIXMODE_MUL = 2
  };
  LightMixmode lightMixMode;
  LightMixmode colorMixMode;

  ProgramParam finalLightFactor;
  size_t numLights;
  bool useAttenuation;

  struct BufferName
  {
    csRenderBufferName defaultName;
    csStringID userName;

    BufferName (csRenderBufferName name = CS_BUFFER_NONE) : 
    defaultName (name), userName (csInvalidStringID) {}
  };
  BufferName positionBuffer;
  BufferName normalBuffer;
  BufferName colorBuffer;

  csBitArray disableMask;

  bool ParseLightMixMode (iDocumentNode* child, LightMixmode& mixmode);
  bool ParseBufferName (iDocumentNode* child, BufferName& name);
  iRenderBuffer* GetBuffer (const BufferName& name,
    csRenderMeshModes& modes, const csShaderVarStack &stacks);
};

#endif //__CS_VPROC_VPROC_PROGRAM_H__

