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

#include "csutil/weakref.h"
#include "csutil/csobject.h"
#include "csutil/leakguard.h"
#include "ivideo/material.h"
#include "ivideo/graph3d.h"
#include "ivideo/shader/shader.h"
#include "iutil/document.h"
#include "iutil/strset.h"
#include "iutil/objreg.h"
#include "../../common/shaderplugin.h"

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

    shaderPass () 
    { 
      mixMode = CS_FX_MESH;
      overrideZmode = false;
    }

    enum
    {
      STREAMMAX = 16,
      TEXTUREMAX = 16
    };

    //buffer mappings
    csStringID bufferID[STREAMMAX];
    csRef<csShaderVariable> bufferRef[TEXTUREMAX];
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

    //variable context
    csShaderVariableContext svcontext;

    csXMLShaderTech* owner;
  };

  //variable context
  csShaderVariableContext svcontext;

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
  csZBufMode oldZmode;

  //Array of passes
  shaderPass* passes;
  unsigned int passesCount;

  unsigned int currentPass;

  csXMLShader* parent;
  const csStringHash& xmltokens;
  bool do_verbose;
  csString fail_reason;

  // load one pass, return false if it fails
  bool LoadPass (iDocumentNode *node, shaderPass *pass);
  // load a shaderdefinition block
  bool LoadSVBlock (iDocumentNode *node, iShaderVariableContext *context);
  // load a shaderprogram
  csPtr<iShaderProgram> LoadProgram (iDocumentNode *node, shaderPass *pass);
  // Set reason for failure.
  void SetFailReason (const char* reason, ...);

  int GetPassNumber (shaderPass* pass);
public:
  CS_LEAKGUARD_DECLARE (csXMLShaderTech);

  csXMLShaderTech (csXMLShader* parent);
  ~csXMLShaderTech();

  int GetNumberOfPasses()
  { return passesCount; }
  bool ActivatePass (unsigned int number);
  bool SetupPass  (const csRenderMesh *mesh,
    csRenderMeshModes& modes,
    const csShaderVarStack &stacks);
  bool TeardownPass();
  bool DeactivatePass();

  bool Load (iDocumentNode* node, iDocumentNode* parentSV);

  const char* GetFailReason()
  { return fail_reason.GetData(); }
};

class csXMLShader : public iShader, public csObject
{
public:
  CS_LEAKGUARD_DECLARE (csXMLShader);

  SCF_DECLARE_IBASE_EXT (csObject);

  csXMLShader (csXMLShaderCompiler* compiler);
  virtual ~csXMLShader();

  virtual iObject* QueryObject () 
  { return (iObject*)(csObject*)this; }

  /// Get number of passes this shader have
  virtual int GetNumberOfPasses ()
  {
    return activeTech->GetNumberOfPasses();
  }

  /// Activate a pass for rendering
  virtual bool ActivatePass (unsigned int number)
  { return activeTech->ActivatePass (number); }

  /// Setup a pass.
  virtual bool SetupPass (const csRenderMesh *mesh,
    csRenderMeshModes& modes,
    const csShaderVarStack &stacks)
  { return activeTech->SetupPass (mesh, modes, stacks); }

  /**
  * Tear down current state, and prepare for a new mesh 
  * (for which SetupPass is called)
  */
  virtual bool TeardownPass ()
  { return activeTech->TeardownPass(); }

  /// Completly deactivate a pass
  virtual bool DeactivatePass ()
  { return activeTech->DeactivatePass(); }

  friend class csXMLShaderCompiler;

  //=================== iShaderVariableContext ================//

  /// Add a variable to this context
  void AddVariable (csShaderVariable *variable)
  { activeTech->svcontext.AddVariable (variable); }

  /// Get a named variable from this context
  csShaderVariable* GetVariable (csStringID name) const
  { return activeTech->svcontext.GetVariable (name); }

  /**
  * Push the variables of this context onto the variable stacks
  * supplied in the "stacks" argument
  */
  void PushVariables (csShaderVarStack &stacks) const
  { activeTech->svcontext.PushVariables (stacks); }

  /**
  * Pop the variables of this context off the variable stacks
  * supplied in the "stacks" argument
  */
  void PopVariables (csShaderVarStack &stacks) const
  { activeTech->svcontext.PopVariables (stacks); }

public:
  //Holders
  csXMLShaderCompiler* compiler;
  csXMLShaderTech* activeTech;
  csRef<iGraphics3D> g3d;
};

class csXMLShaderCompiler : public iShaderCompiler, public iComponent
{
public:
  CS_LEAKGUARD_DECLARE (csXMLShaderCompiler);

  SCF_DECLARE_IBASE;
  csXMLShaderCompiler(iBase* parent);

  virtual ~csXMLShaderCompiler();

  virtual bool Initialize (iObjectRegistry* object_reg);

  /// Get a name identifying this compiler
  virtual const char* GetName() 
  { return "XMLShader"; }

  /// Compile a template into a shader. Will return 0 if it fails
  virtual csPtr<iShader> CompileShader (iDocumentNode *templ,
		  int forcepriority = -1);

  /// Validate if a template is a valid shader to this compiler
  virtual bool ValidateTemplate (iDocumentNode *templ);

  /// Check if template is parsable by this compiler
  virtual bool IsTemplateToCompiler (iDocumentNode *templ);

  /// Get a list of priorities for a given shader.
  virtual csPtr<iShaderPriorityList> GetPriorities (
		  iDocumentNode* templ);

  void Report (int severity, const char* msg, ...);
private:

  // struct to hold all techniques, until we decide which to use
  struct TechniqueKeeper
  {
    TechniqueKeeper(iDocumentNode *n, unsigned int p) : 
      node(n), priority(p), tagPriority(0)
    {}
    csRef<iDocumentNode> node;
    unsigned int priority;
    int tagPriority;
  };

  // Scan all techniques in the document.
  void ScanForTechniques (iDocumentNode* templ,
	csArray<TechniqueKeeper>& techniquesTmp, int forcepriority);
  
  static int CompareTechniqueKeeper (TechniqueKeeper const&,
				     TechniqueKeeper const&);

public:
  bool do_verbose;
  /// XML Token and management
  csStringHash xmltokens;

  //Standard vars
  iObjectRegistry* objectreg;
  csRef<iStringSet> strings;
  csWeakRef<iGraphics3D> g3d;
  csRef<iSyntaxService> synldr;
  csWeakRef<iShaderManager> shadermgr;

#define CS_TOKEN_ITEM_FILE \
  "plugins/video/render3d/shader/shadercompiler/xmlshader/xmlshader.tok"
#include "cstool/tokenlist.h"
};

#endif
