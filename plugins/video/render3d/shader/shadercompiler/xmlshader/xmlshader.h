/*
  Copyright (C) 2003-2006 by Marten Svanfeldt
            (C) 2004-2006 by Frank Richter

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

#include "iutil/comp.h"
#include "ivideo/shader/shader.h"

#include "csutil/weakref.h"
#include "csutil/scf_implementation.h"

#include "cpi/condition.h"

struct iLoaderContext;
struct iSyntaxService;
struct iVFS;

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{

class csWrappedDocumentNodeFactory;
class csXMLShaderCompiler;
class csXMLShader;

class csXMLShaderCompiler : public scfImplementation2<csXMLShaderCompiler,
						      iShaderCompiler, 
						      iComponent>
{
public:
  CS_LEAKGUARD_DECLARE (csXMLShaderCompiler);

  csXMLShaderCompiler(iBase* parent);

  virtual ~csXMLShaderCompiler();

  virtual bool Initialize (iObjectRegistry* object_reg);

  /// Get a name identifying this compiler
  virtual const char* GetName() 
  { return "XMLShader"; }

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

  bool PrecacheShader(iDocumentNode*, iHierarchicalCache*);
  
  void Report (int severity, const char* msg, ...);
  void Report (int severity, iDocumentNode* node, const char* msg, ...);

  bool LoadSVBlock (iLoaderContext* ldr_context,
      iDocumentNode *node, iShaderVariableContext *context);
  
  csPtr<iDocumentNode> ReadNodeFromBuf (iDataBuffer* buf);
  csPtr<iDataBuffer> WriteNodeToBuf (iDocument* doc);
  csPtr<iDocument> CreateCachingDoc ();
public:
  bool do_verbose;
  bool doDumpXML;
  bool doDumpConds;
  bool doDumpValues;
  /// XML Token and management
  csStringHash xmltokens;
  bool debugInstrProcessing;

  //Standard vars
  iObjectRegistry* objectreg;
  csRef<iStringSet> strings;
  csRef<iShaderVarStringSet> stringsSvName;
  csWeakRef<iGraphics3D> g3d;
  csRef<iSyntaxService> synldr;
  csRef<iVFS> vfs;
  csWrappedDocumentNodeFactory* wrapperFact;
  /// Condition constants
  csConditionConstants condConstants;
  
  csRef<csConditionEvaluator> sharedEvaluator;
  
  csRef<iDocumentSystem> binDocSys;
  csRef<iDocumentSystem> xmlDocSys;
  
  CS::ShaderVarStringID string_mixmode_alpha;
  CS::ShaderVarStringID stringLightCount;

#define CS_TOKEN_ITEM_FILE \
  "plugins/video/render3d/shader/shadercompiler/xmlshader/xmlshader.tok"
#include "cstool/tokenlist.h"
};

class csShaderPriorityList : public scfImplementation1<csShaderPriorityList, 
                                                       iShaderPriorityList>
{
public:
  csArray<int> priorities;
  csShaderPriorityList () : scfImplementationType (this)
  {
  }
  virtual ~csShaderPriorityList ()
  {
  }

  virtual size_t GetCount () const { return priorities.GetSize (); }
  virtual int GetPriority (size_t idx) const { return priorities[idx]; }
};

}
CS_PLUGIN_NAMESPACE_END(XMLShader)

#endif
