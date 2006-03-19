/*
  Copyright (C) 2003-2006 by Marten Svanfeldt
		2005-2006 by Frank Richter

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

#ifndef __CS_SHADERTECH_H__
#define __CS_SHADERTECH_H__

#include "ivideo/graph3d.h"

#include "csgfx/shadervarcontext.h"
#include "csplugincommon/shader/shaderplugin.h"
#include "csutil/dirtyaccessarray.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{

class csXMLShaderCompiler;
class csXMLShader;

class csXMLShaderTech
{
private:
  friend class csXMLShader;

  struct shaderPass
  {
    //mix and alpha mode
    uint mixMode;
    csAlphaMode alphaMode;
    csZBufMode zMode;
    bool overrideZmode;
    bool flipCulling;

    shaderPass () 
    { 
      mixMode = CS_FX_MESH;
      overrideZmode = false;
      flipCulling = false;
      //setup default mappings
      for (unsigned int i=0; i < STREAMMAX; i++)
        defaultMappings[i] = CS_BUFFER_NONE;

      defaultMappings[CS_VATTRIB_POSITION] = CS_BUFFER_POSITION;
    }

    enum
    {
      STREAMMAX = 16,
      TEXTUREMAX = 16
    };

    // buffer mappings
    // default mapping, index is csVertexAttrib (16 first), value is
    // csRenderBufferName
    csRenderBufferName defaultMappings[STREAMMAX];
    csArray<csStringID> custommapping_id;
    csDirtyAccessArray<csVertexAttrib> custommapping_attrib;
    csDirtyAccessArray<csRef<csShaderVariable> > custommapping_variables;
    csArray<csRenderBufferName> custommapping_buffer;

    // texture mappings
    csStringID textureID[TEXTUREMAX];
    csRef<csShaderVariable> textureRef[TEXTUREMAX];
    csRef<csShaderVariable> autoAlphaTexRef;
    int textureCount;

    // programs
    csRef<iShaderProgram> vp;
    csRef<iShaderProgram> fp;
    csRef<iShaderProgram> vproc;

    // writemasks
    bool wmRed, wmGreen, wmBlue, wmAlpha;

    // variable context
    csShaderVariableContext svcontext;

    csXMLShaderTech* owner;
  };

  //variable context
  csShaderVariableContext svcontext;

  //optimization stuff
  static iRenderBuffer* last_buffers[shaderPass::STREAMMAX*2];
  static iRenderBuffer* clear_buffers[shaderPass::STREAMMAX*2];
  //static csVertexAttrib vertexattributes[shaderPass::STREAMMAX*2];
  static size_t lastBufferCount;

  static iTextureHandle* last_textures[shaderPass::TEXTUREMAX];
  static iTextureHandle* clear_textures[shaderPass::TEXTUREMAX];
  static int textureUnits[shaderPass::TEXTUREMAX];
  static size_t lastTexturesCount;

  //keep this so we can reset in deactivate
  bool orig_wmRed, orig_wmGreen, orig_wmBlue, orig_wmAlpha;
  csZBufMode oldZmode;

  //Array of passes
  shaderPass* passes;
  size_t passesCount;

  size_t currentPass;

  csXMLShader* parent;
  const csStringHash& xmltokens;
  bool do_verbose;
  csString fail_reason;

  // metadata
  csShaderMetadata metadata;

  // load one pass, return false if it fails
  bool LoadPass (iDocumentNode *node, shaderPass *pass);
  // load a shaderdefinition block
  //bool LoadSVBlock (iDocumentNode *node, iShaderVariableContext *context);
  // load a shaderprogram
  csPtr<iShaderProgram> LoadProgram (iShaderDestinationResolver* resolve,
  	iDocumentNode *node, shaderPass *pass);
  // Set reason for failure.
  void SetFailReason (const char* reason, ...) CS_GNUC_PRINTF (2, 3);

  int GetPassNumber (shaderPass* pass);
public:
  CS_LEAKGUARD_DECLARE (csXMLShaderTech);

  csXMLShaderTech (csXMLShader* parent);
  ~csXMLShaderTech();

  size_t GetNumberOfPasses()
  { return passesCount; }
  bool ActivatePass (size_t number);
  bool SetupPass  (const csRenderMesh *mesh,
    csRenderMeshModes& modes,
    const csShaderVarStack &stacks);
  bool TeardownPass();
  bool DeactivatePass();

  bool Load (iDocumentNode* node, iDocumentNode* parentSV);

  const char* GetFailReason()
  { return fail_reason.GetData(); }
};

}
CS_PLUGIN_NAMESPACE_END(XMLShader)

#endif // __CS_SHADERTECH_H__
