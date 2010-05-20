/*
  Copyright (C) 2003-2007 by Marten Svanfeldt
            (C) 2004-2007 by Frank Richter

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

#ifndef __WEAVER_H__
#define __WEAVER_H__

#include "iutil/comp.h"
#include "ivideo/shader/shader.h"

#include "csutil/threading/tls.h"
#include "csutil/weakref.h"
#include "csutil/scf_implementation.h"

struct iSyntaxService;
struct iLoaderContext;
struct iVFS;
struct iDocumentNode;
struct iJobQueue;

CS_PLUGIN_NAMESPACE_BEGIN(ShaderWeaver)
{

class WeaverCompiler : public scfImplementation2<WeaverCompiler,
						 iShaderCompiler, 
						 iComponent>
{
public:
  CS_LEAKGUARD_DECLARE (WeaverCompiler);

  WeaverCompiler(iBase* parent);

  virtual ~WeaverCompiler();

  virtual bool Initialize (iObjectRegistry* object_reg);

  /// Get a name identifying this compiler
  virtual const char* GetName() 
  { return "ShaderWeaver"; }

  /// Compile a template into a shader. Will return 0 if it fails
  virtual csPtr<iShader> CompileShader (
	iLoaderContext* ldr_context, iDocumentNode *templ,
	int forcepriority = -1);

  /// Validate if a template is a valid shader to this compiler
  virtual bool ValidateTemplate (iDocumentNode *templ);

  /// Check if template is parsable by this compiler
  virtual bool IsTemplateToCompiler (iDocumentNode *templ);

  /// Get a list of priorities for a given shader.
  virtual csPtr<iShaderPriorityList> GetPriorities (
		  iDocumentNode* templ);
		  
  bool PrecacheShader(iDocumentNode*, iHierarchicalCache*, bool);

  void Report (int severity, const char* msg, ...) const;
  void Report (int severity, iDocumentNode* node, const char* msg, ...) const;

  csPtr<iDocumentNode> LoadDocumentFromFile (const char* filename,
    iDocumentNode* node) const;
public:
  bool do_verbose;
  bool doDumpWeaved;
  bool annotateCombined;
  /// XML Token and management
  csStringHash xmltokens;

  csRef<iDocumentSystem> binDocSys;
  csRef<iDocumentSystem> xmlDocSys;

  //Standard vars
  iObjectRegistry* objectreg;
  csRef<iStringSet> strings;
  csRef<iShaderVarStringSet> svstrings;
  csWeakRef<iGraphics3D> g3d;
  csRef<iSyntaxService> synldr;
  csRef<iVFS> vfs;
  csRef<iShaderCompiler> xmlshader;
  csRef<iJobQueue> synthQueue;
#define CS_TOKEN_ITEM_FILE \
  "plugins/video/render3d/shader/shadercompiler/weaver/weaver.tok"
#include "cstool/tokenlist.h"

  /* When loading a snippet, sometimes document nodes have to be created.
     These are created from this "auto document".
   */
  CS::Threading::ThreadLocal<csRef<iDocumentNode> > autoDocRoot;
  csRef<iDocumentNode> CreateAutoNode (csDocumentNodeType type) const;

  /// Get the job queue used for shader technique synthesis
  iJobQueue* GetSynthQueue();
};

}
CS_PLUGIN_NAMESPACE_END(ShaderWeaver)

#endif // __WEAVER_H__
