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

#ifndef __XMLSHADER_H__
#define __XMLSHADER_H__

#include "ivideo/material.h"
#include "ivideo/graph3d.h"
#include "ivideo/shader/shader.h"
#include "iutil/document.h"
#include "iutil/strset.h"
#include "iutil/objreg.h"
#include "../../common/shaderplugin.h"

class csXMLShaderCompiler;

class csXMLShader : public iShader
{
public:
  SCF_DECLARE_IBASE;

  csXMLShader (iGraphics3D* g3d);
  virtual ~csXMLShader();

  /// Retrieve name of shader
  virtual const char* GetName()
  {
    return name;
  }

  /// Get number of passes this shader have
  virtual int GetNumberOfPasses()
  {
    return passesCount;
  }

  /// Activate a pass for rendering
  virtual bool ActivatePass(unsigned int number);

  /// Setup a pass.
  virtual bool SetupPass(csRenderMesh *mesh,
    const csArray<iShaderVariableContext*> &dynamicDomains);

  /**
  * Tear down current state, and prepare for a new mesh 
  * (for which SetupPass is called)
  */
  virtual bool TeardownPass();

  /// Completly deactivate a pass
  virtual bool DeactivatePass();

  friend class csXMLShaderCompiler;

  /// iShaderVariableContext members
  /// Get a named variable from this context
  virtual csShaderVariable* GetVariable (csStringID name) const 
  {
    return staticVariables.GetVariable (name);
  }
  /// Get a named variable from this context, and any context above/outer
  virtual csShaderVariable* GetVariableRecursive (csStringID name) const
  {
    csShaderVariable* var;
    var=GetVariable (name);
    if(var) return var;
    return 0;
  }
  /// Fill a csShaderVariableList. Return number of variables filled
  virtual unsigned int FillVariableList (csShaderVariableProxyList *list) const
  {
    return staticVariables.FillVariableList (list);
  }

private:
  /// Add a variable to this context
  virtual void AddVariable (csShaderVariable *variable) {}

  //helpers for static and dynamic variables
  csShaderVariableContextHelper staticVariables;
  csShaderVariableProxyList dynamicVariables;

  struct shaderPass : public iShaderVariableContext
  {
    //mix and alpha mode
    uint mixMode;
    csAlphaMode alphaMode;

    SCF_DECLARE_IBASE;

    shaderPass () 
    { 
      SCF_CONSTRUCT_IBASE (0);
      mixMode = CS_FX_COPY;
    }

    virtual ~shaderPass () 
    { 
      SCF_DESTRUCT_IBASE();
    }

    /// Get a named variable from this context
    virtual csShaderVariable* GetVariable (csStringID name) const
    {
      return staticVariables.GetVariable (name);
    }
    /// Get a named variable from this context, and any context above/outer
    virtual csShaderVariable* GetVariableRecursive (csStringID name) const
    {
      csShaderVariable* var;
      var=GetVariable (name);
      if(var) return var;
      return owner->GetVariableRecursive (name);
    }
    /// Fill a csShaderVariableList. Return number of variables filled
    virtual unsigned int FillVariableList (
	csShaderVariableProxyList *list) const
    {
      return staticVariables.FillVariableList (list);
    }
    /// Add a variable to this context
    virtual void AddVariable (csShaderVariable *variable) {}

    static const int STREAMMAX = 16;
    static const int TEXTUREMAX = 16;

    //buffer mappings
    csStringID bufferID[STREAMMAX];
    csRef<csShaderVariable> bufferRef[STREAMMAX];
    csVertexAttrib vertexattributes[STREAMMAX];
    bool bufferGeneric[STREAMMAX];
    int bufferCount;

    //texture mappings
    csStringID textureID[TEXTUREMAX];
    csRef<csShaderVariable> textureRef[TEXTUREMAX];
    csRef<csShaderVariable> autoAlphaTexRef;
    //int textureUnits[TEXTUREMAX];
    int textureCount;

    //programs
    csRef<iShaderProgram> vp;
    csRef<iShaderProgram> fp;

    //writemasks
    bool wmRed, wmGreen, wmBlue, wmAlpha;

    //helpers for static and dynamic variables
    csShaderVariableContextHelper staticVariables;
    csShaderVariableProxyList dynamicVariables;

    csXMLShader *owner;
  };

  //optimization stuff
  static iRenderBuffer* last_buffers[shaderPass::STREAMMAX*2];
  static iRenderBuffer* clear_buffers[shaderPass::STREAMMAX*2];
  //static csVertexAttrib vertexattributes[shaderPass::STREAMMAX*2];
  static int lastBufferCount;

  static iTextureHandle* last_textures[shaderPass::TEXTUREMAX];
  static iTextureHandle* clear_textures[shaderPass::TEXTUREMAX];
  static int textureUnits[shaderPass::TEXTUREMAX];
  static int lastTexturesCount;

  //keep this so we can reset in deactivate
  bool orig_wmRed, orig_wmGreen, orig_wmBlue, orig_wmAlpha;

  //Array of passes
  shaderPass* passes;
  unsigned int passesCount;

  unsigned int currentPass;

  char* name;

  //Holders
  csRef<iGraphics3D> g3d;
public:
  int GetPassNumber (shaderPass* pass);
};

class csXMLShaderCompiler : public iShaderCompiler, public iComponent
{
public:
  SCF_DECLARE_IBASE;
  csXMLShaderCompiler(iBase* parent);

  virtual ~csXMLShaderCompiler();

  virtual bool Initialize (iObjectRegistry* object_reg);

  /// Get a name identifying this compiler
  virtual const char* GetName() 
  { return "XMLShader"; }

  /// Compile a template into a shader. Will return 0 if it fails
  virtual csPtr<iShader> CompileShader (iDocumentNode *templ);

  /// Validate if a template is a valid shader to this compiler
  virtual bool ValidateTemplate (iDocumentNode *templ);

  /// Check if template is parsable by this compiler
  virtual bool IsTemplateToCompiler (iDocumentNode *templ);

private:
  void Report (int severity, const char* msg, ...);

  // struct to hold all techniques, until we decide which to use
  struct techniqueKeeper
  {
    techniqueKeeper(iDocumentNode *n, unsigned int p) : node(n), priority(p)
    {}
    csRef<iDocumentNode> node;
    unsigned int priority;
  };

  static int CompareTechniqueKeeper(void const* item1, void const* item2);
  
  // load one technique, and create shader from it
  csPtr<csXMLShader> CompileTechnique (iDocumentNode *node, 
    const char* shaderName, iDocumentNode *parentSV = 0);

  // load one pass, return false if it fails
  bool LoadPass (iDocumentNode *node, csXMLShader::shaderPass *pass);

  // load a shaderdefinition block
  bool LoadSVBlock (iDocumentNode *node, csShaderVariableContextHelper*
    staticVariables, csShaderVariableProxyList *dynamicVariables) ;

  // load a shaderprogram
  csPtr<iShaderProgram> LoadProgram (iDocumentNode *node,
	csXMLShader::shaderPass *pass);

  // Set reason for failure.
  void SetFailReason (const char* reason);

  /// XML Token and management
  csStringHash xmltokens;
#define CS_TOKEN_ITEM_FILE \
  "video/render3d/shader/shadercompiler/xmlshader/xmlshader.tok"
#include "cstool/tokenlist.h"

  //Standard vars
  csRef<iObjectRegistry> objectreg;
  csRef<iStringSet> strings;
  csRef<iGraphics3D> g3d;
  csRef<iSyntaxService> synldr;

  bool do_verbose;
  char* fail_reason;
};

#endif
