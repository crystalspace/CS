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

#include "cssysdef.h"
#include <ctype.h>

// For builtin shader consts:
#include "iengine/light.h"
#include "imap/services.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"
#include "iutil/verbositymanager.h"
#include "ivaria/keyval.h"
#include "ivaria/reporter.h"

#include "csutil/cfgacc.h"

#include "docwrap.h"
#include "shader.h"
#include "xmlshader.h"

CS_IMPLEMENT_PLUGIN

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{

CS_LEAKGUARD_IMPLEMENT (csXMLShaderCompiler);

//---------------------------------------------------------------------------

SCF_IMPLEMENT_FACTORY (csXMLShaderCompiler)

csXMLShaderCompiler::csXMLShaderCompiler(iBase* parent) : 
  scfImplementationType (this, parent)
{
  wrapperFact = 0;
  InitTokenTable (xmltokens);

  // Set up builtin constants
#define BUILTIN_CONSTANT(Type, Value)					    \
  condConstants.AddConstant (#Value, (Type)Value);
#include "condconstbuiltin.inc"
#undef BUILTIN_CONSTANT
}

csXMLShaderCompiler::~csXMLShaderCompiler()
{
  delete wrapperFact;
}

void csXMLShaderCompiler::Report (int severity, const char* msg, ...)
{
  va_list args;
  va_start (args, msg);
  csReportV (objectreg, severity, 
    "crystalspace.graphics3d.shadercompiler.xmlshader", msg, args);
  va_end (args);
}

bool csXMLShaderCompiler::Initialize (iObjectRegistry* object_reg)
{
  objectreg = object_reg;

  wrapperFact = new csWrappedDocumentNodeFactory (this);

  csRef<iPluginManager> plugin_mgr = CS_QUERY_REGISTRY (
      object_reg, iPluginManager);

  strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
    object_reg, "crystalspace.shared.stringset", iStringSet);

  g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  vfs = CS_QUERY_REGISTRY (object_reg, iVFS);
  
  CS_QUERY_REGISTRY_PLUGIN(synldr, object_reg,
    "crystalspace.syntax.loader.service.text", iSyntaxService);
  if (!synldr)
    return false;

  csRef<iVerbosityManager> verbosemgr (
    CS_QUERY_REGISTRY (object_reg, iVerbosityManager));
  if (verbosemgr) 
    do_verbose = verbosemgr->Enabled ("renderer.shader");
  else
    do_verbose = false;
    
  csConfigAccess config (object_reg);
  doDumpXML = config->GetBool ("Video.XMLShader.DumpVariantXML");
  doDumpConds = config->GetBool ("Video.XMLShader.DumpConditions");

  return true;
}

csPtr<iShader> csXMLShaderCompiler::CompileShader (iDocumentNode *templ,
		int forcepriority)
{
  if (!templ) return 0;

  if (!ValidateTemplate (templ))
    return 0;
  
  // Create a shader. The actual loading happens later.
  csRef<csXMLShader> shader;
  shader.AttachNew (new csXMLShader (this, templ, forcepriority));
  shader->SetName (templ->GetAttributeValue ("name"));
  shader->SetDescription (templ->GetAttributeValue ("description"));
  if (do_verbose)
  {
    csString str;
    shader->DumpStats (str);
    Report(CS_REPORTER_SEVERITY_NOTIFY, 
      "Shader %s: %s", shader->GetName (), str.GetData ());
  }

  csRef<iDocumentNodeIterator> tagIt = templ->GetNodes ("key");
  while (tagIt->HasNext ())
  {
    iKeyValuePair *keyvalue = 0;
    synldr->ParseKey (tagIt->Next (), keyvalue);
    if (keyvalue)
      shader->QueryObject ()->ObjAdd (keyvalue->QueryObject ());
  }

  csRef<iShader> ishader (shader);
  return csPtr<iShader> (ishader);
}

class csShaderPriorityList : public iShaderPriorityList
{
public:
  csArray<int> priorities;
  csShaderPriorityList ()
  {
    SCF_CONSTRUCT_IBASE (0);
  }
  virtual ~csShaderPriorityList ()
  {
    SCF_DESTRUCT_IBASE ();
  }

  SCF_DECLARE_IBASE;
  virtual size_t GetCount () const { return priorities.Length (); }
  virtual int GetPriority (size_t idx) const { return priorities[idx]; }
};

SCF_IMPLEMENT_IBASE (csShaderPriorityList)
  SCF_IMPLEMENTS_INTERFACE (iShaderPriorityList)
SCF_IMPLEMENT_IBASE_END

csPtr<iShaderPriorityList> csXMLShaderCompiler::GetPriorities (
	iDocumentNode* templ)
{
  csRef<iShaderManager> shadermgr = 
    CS_QUERY_REGISTRY (objectreg, iShaderManager);
  CS_ASSERT (shadermgr); // Should be present - loads us, after all

  csShaderPriorityList* list = new csShaderPriorityList ();

  /* @@@ A bit awkward, almost the same code as in 
     csXMLShader::ScanForTechniques */
  csRef<iDocumentNodeIterator> it = templ->GetNodes();

  // Read in the techniques.
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () == CS_NODE_ELEMENT &&
      xmltokens.Request (child->GetValue ()) == XMLTOKEN_TECHNIQUE)
    {
      //save it
      int p = child->GetAttributeValueAsInt ("priority");
      // Compute the tag's priorities.
      csRef<iDocumentNodeIterator> tagIt = child->GetNodes ("tag");
      while (tagIt->HasNext ())
      {
	csRef<iDocumentNode> tag = tagIt->Next ();
	csStringID tagID = strings->Request (tag->GetContentsValue ());

	csShaderTagPresence presence;
	int priority;
	shadermgr->GetTagOptions (tagID, presence, priority);
	if (presence == TagNeutral)
	{
	  p += priority;
	}
      }
      list->priorities.InsertSorted (p);
    }
  }

  return csPtr<iShaderPriorityList> (list);
}

bool csXMLShaderCompiler::ValidateTemplate(iDocumentNode *templ)
{
  if (!IsTemplateToCompiler(templ))
    return false;

  /*@@TODO: Validation without accual compile. should check correct xml
  syntax, and that we have at least one techqniue which can load. Also check
  that we have valid texturemapping and buffermapping*/

  return true;
}

bool csXMLShaderCompiler::IsTemplateToCompiler(iDocumentNode *templ)
{
  //Check that we got an element
  if (templ->GetType() != CS_NODE_ELEMENT) return false;

  //With name shader  (<shader>....</shader>
  if (xmltokens.Request (templ->GetValue())!=XMLTOKEN_SHADER) return false;

  //Check the type-string in <shader>
  const char* shaderName = templ->GetAttributeValue ("name");
  const char* shaderType = templ->GetAttributeValue ("compiler");
  // Prefer "compiler" about (somewhat ambiguous) "type"
  if (shaderType == 0) shaderType = templ->GetAttributeValue ("type");
  if ((shaderType == 0) || (xmltokens.Request (shaderType) != 
    XMLTOKEN_XMLSHADER))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, 
      "Type of shader '%s' is not 'xmlshader', but '%s'",
      shaderName, shaderType);
    return false;
  }

  //Check that we have children, no children == not a template to this one at
  //least.
  if (!templ->GetNodes()->HasNext()) return false;

  //Ok, passed check. We will try to validate it
  return true;
}

}
CS_PLUGIN_NAMESPACE_END(XMLShader)
