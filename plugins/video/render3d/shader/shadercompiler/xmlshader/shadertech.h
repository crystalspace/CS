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

  struct ShaderPass : public CS::Memory::CustomAllocated
  {
    //mix and alpha mode
    uint mixMode;
    csAlphaMode alphaMode;
    csZBufMode zMode;
    bool overrideZmode;
    bool flipCulling;
    bool zoffset;

    ShaderPass () : zoffset (false)
    { 
      mixMode = CS_FX_MESH;
      overrideZmode = false;
      flipCulling = false;
      //setup default mappings
      for (unsigned int i=0; i < CS_VATTRIB_SPECIFIC_NUM; i++)
        defaultMappings[i] = CS_BUFFER_NONE;

      defaultMappings[CS_VATTRIB_POSITION] = CS_BUFFER_POSITION;
    }

    // buffer mappings
    // default mapping, index is csVertexAttrib (16 first), value is
    // csRenderBufferName
    csRenderBufferName defaultMappings[CS_VATTRIB_SPECIFIC_NUM];
    csArray<CS::ShaderVarStringID> custommapping_id;
    csDirtyAccessArray<csVertexAttrib> custommapping_attrib;
    csArray<csRenderBufferName> custommapping_buffer;

    // texture mappings
    struct TextureMapping
    {
      CS::ShaderVarStringID id;
      csDirtyAccessArray<size_t, csArrayElementHandler<size_t>,
	CS::Memory::LocalBufferAllocator<size_t, 2,
	  CS::Memory::AllocatorMalloc, true> > indices;
      int textureUnit;
      CS::Graphics::TextureComparisonMode texCompare;
    };
    csArray<TextureMapping> textures;

    // programs
    csRef<iShaderProgram> vp;
    csRef<iShaderProgram> fp;
    csRef<iShaderProgram> vproc;

    // writemasks
    bool wmRed, wmGreen, wmBlue, wmAlpha;

    csXMLShaderTech* owner;
  };

  //variable context
  csShaderVariableContext svcontext;

  //keep this so we can reset in deactivate
  bool orig_wmRed, orig_wmGreen, orig_wmBlue, orig_wmAlpha;
  csZBufMode oldZmode;

  //Array of passes
  ShaderPass* passes;
  size_t passesCount;

  size_t currentPass;

  csXMLShader* parent;
  const csStringHash& xmltokens;
  bool do_verbose;
  csString fail_reason;

  // load one pass, return false if it fails
  bool LoadPass (iDocumentNode *node, ShaderPass* pass, size_t variant);
  // load a shaderdefinition block
  //bool LoadSVBlock (iDocumentNode *node, iShaderVariableContext *context);
  // load a shaderprogram
  csPtr<iShaderProgram> LoadProgram (iShaderDestinationResolver* resolve,
  	iDocumentNode *node, ShaderPass* pass, size_t variant);
  // Set reason for failure.
  void SetFailReason (const char* reason, ...) CS_GNUC_PRINTF (2, 3);

  int GetPassNumber (ShaderPass* pass);
public:
  CS_LEAKGUARD_DECLARE (csXMLShaderTech);

  csXMLShaderTech (csXMLShader* parent);
  ~csXMLShaderTech();

  size_t GetNumberOfPasses()
  { return passesCount; }
  bool ActivatePass (size_t number);
  bool SetupPass  (const CS::Graphics::RenderMesh *mesh,
    CS::Graphics::RenderMeshModes& modes,
    const csShaderVariableStack& stack);
  bool TeardownPass();
  bool DeactivatePass();
  void GetUsedShaderVars (csBitArray& bits) const;

  bool Load (iLoaderContext* ldr_context, iDocumentNode* node,
      iDocumentNode* parentSV, size_t variant);

  const char* GetFailReason()
  { return fail_reason.GetData(); }
};

}
CS_PLUGIN_NAMESPACE_END(XMLShader)

#endif // __CS_SHADERTECH_H__
