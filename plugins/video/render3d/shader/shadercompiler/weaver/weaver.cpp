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

#include "cssysdef.h"
#include <ctype.h>

#include "imap/services.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"
#include "iutil/verbositymanager.h"
#include "ivaria/keyval.h"
#include "ivaria/reporter.h"

#include "csutil/cfgacc.h"
#include "csutil/csendian.h"
#include "csutil/memfile.h"
#include "csutil/parasiticdatabuffer.h"
#include "csutil/xmltiny.h"

#include "weaver.h"
#include "shader.h"

CS_IMPLEMENT_PLUGIN

CS_PLUGIN_NAMESPACE_BEGIN(ShaderWeaver)
{

CS_LEAKGUARD_IMPLEMENT (WeaverCompiler);

//---------------------------------------------------------------------------

SCF_IMPLEMENT_FACTORY (WeaverCompiler)

WeaverCompiler::WeaverCompiler(iBase* parent) : 
  scfImplementationType (this, parent)
{
  InitTokenTable (xmltokens);
}

WeaverCompiler::~WeaverCompiler()
{
}

void WeaverCompiler::Report (int severity, const char* msg, ...) const
{
  va_list args;
  va_start (args, msg);
  csReportV (objectreg, severity, 
    "crystalspace.graphics3d.shadercompiler.weaver", msg, args);
  va_end (args);
}

void WeaverCompiler::Report (int severity, iDocumentNode* node, 
			     const char* msg, ...) const
{
  va_list args;
  va_start (args, msg);
  csString str;
  str.FormatV (msg, args);
  va_end (args);
  
  synldr->Report ("crystalspace.graphics3d.shadercompiler.weaver",
    severity, node, "%s", str.GetData());
}

csPtr<iDocumentNode> WeaverCompiler::LoadDocumentFromFile (
  const char* filename, iDocumentNode* node) const
{
  // @@@ TODO: Make thread safe
  csRef<iFile> file = vfs->Open (filename, VFS_FILE_READ);
  if (!file)
  {
    Report (CS_REPORTER_SEVERITY_WARNING, node,
      "Unable to open file '%s'", filename);
    return 0;
  }
  csRef<iDocumentSystem> docsys (
    csQueryRegistry<iDocumentSystem> (objectreg));
  if (docsys == 0) docsys = xmlDocSys;

  csRef<iDocument> doc = docsys->CreateDocument ();
  const char* err = doc->Parse (file);
  if (err != 0)
  {
    Report (CS_REPORTER_SEVERITY_WARNING, node,
      "Unable to parse file '%s': %s", filename, err);
    return 0;
  }

  return csPtr<iDocumentNode> (doc->GetRoot ());
}

csRef<iDocumentNode> WeaverCompiler::CreateAutoNode (csDocumentNodeType type) const
{
  // @@@ TODO: Mutex for thread safety
  if (!autoDocRoot.IsValid ())
  {
    csRef<iDocument> autoDoc = xmlDocSys->CreateDocument ();
    csRef<iDocumentNode> root = autoDoc->CreateRoot ();
    autoDocRoot = root->CreateNodeBefore (CS_NODE_ELEMENT);
    autoDocRoot->SetValue ("(auto)");
  }
  return autoDocRoot->CreateNodeBefore (type);
}

bool WeaverCompiler::Initialize (iObjectRegistry* object_reg)
{
  objectreg = object_reg;

  csRef<iPluginManager> plugin_mgr = 
      csQueryRegistry<iPluginManager> (object_reg);
      
  xmlshader = csLoadPluginCheck<iShaderCompiler> (plugin_mgr,
    "crystalspace.graphics3d.shadercompiler.xmlshader");
  if (!xmlshader.IsValid())
    return false;

  strings = csQueryRegistryTagInterface<iStringSet> (
    object_reg, "crystalspace.shared.stringset");
  svstrings = csQueryRegistryTagInterface<iShaderVarStringSet> (
    object_reg, "crystalspace.shader.variablenameset");

  g3d = csQueryRegistry<iGraphics3D> (object_reg);
  vfs = csQueryRegistry<iVFS> (object_reg);
  
  synldr = csQueryRegistryOrLoad<iSyntaxService> (object_reg,
    "crystalspace.syntax.loader.service.text");
  if (!synldr)
    return false;
  
  binDocSys = csLoadPluginCheck<iDocumentSystem> (plugin_mgr,
    "crystalspace.documentsystem.binary");
  xmlDocSys.AttachNew (new csTinyDocumentSystem);
  
  csRef<iVerbosityManager> verbosemgr (
    csQueryRegistry<iVerbosityManager> (object_reg));
  if (verbosemgr) 
    do_verbose = verbosemgr->Enabled ("renderer.shader");
  else
    do_verbose = false;
    
  csConfigAccess config (object_reg);
  doDumpWeaved = config->GetBool ("Video.ShaderWeaver.DumpWeavedXML");
  annotateCombined = config->GetBool ("Video.ShaderWeaver.AnnotateOutput");
    
  return true;
}

csPtr<iShader> WeaverCompiler::CompileShader (
    	iLoaderContext* ldr_context, iDocumentNode *templ,
	int forcepriority)
{
  const char* shaderName = templ->GetAttributeValue ("name");
  csRef<WeaverShader> shader;

  csTicks startTime = 0, endTime = 0;
  // Create a shader. The actual loading happens later.
  if (do_verbose) startTime = csGetTicks();
  shader.AttachNew (new WeaverShader (this));
  bool loadRet = shader->Load (ldr_context, templ, forcepriority);
  autoDocRoot.Invalidate ();
  if (!loadRet)
    return 0;
  if (do_verbose) 
  {
    endTime = csGetTicks();
  
    csString str;
    //shader->DumpStats (str);
    Report(CS_REPORTER_SEVERITY_NOTIFY, 
      "Shader %s: %s weaved in %u ms", shaderName, str.GetData (),
      endTime - startTime);
  }
  shader->SetName (shaderName);
  //shader->SetDescription (templ->GetAttributeValue ("description"));

  csRef<iDocumentNodeIterator> tagIt = templ->GetNodes ("key");
  while (tagIt->HasNext ())
  {
    // @@@ FIXME: also keeps "editoronly" keys
    csRef<iKeyValuePair> keyvalue = synldr->ParseKey (tagIt->Next ());
    if (keyvalue)
      shader->QueryObject ()->ObjAdd (keyvalue->QueryObject ());
  }

  csRef<iShader> ishader (shader);
  return csPtr<iShader> (ishader);
}

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

csPtr<iShaderPriorityList> WeaverCompiler::GetPriorities (
	iDocumentNode* templ)
{
  csShaderPriorityList* list = new csShaderPriorityList ();

  return csPtr<iShaderPriorityList> (list);
}

bool WeaverCompiler::ValidateTemplate(iDocumentNode *templ)
{
  if (!IsTemplateToCompiler(templ))
    return false;

  /*@@TODO: Validation without accual compile. should check correct xml
  syntax, and that we have at least one techqniue which can load. Also check
  that we have valid texturemapping and buffermapping*/

  return true;
}

bool WeaverCompiler::IsTemplateToCompiler(iDocumentNode *templ)
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
    XMLTOKEN_SHADERWEAVER))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, 
      "Type of shader '%s' is not 'shaderweaver', but '%s'",
      shaderName, shaderType);
    return false;
  }

  //Check that we have children, no children == not a template to this one at
  //least.
  if (!templ->GetNodes()->HasNext()) return false;

  //Ok, passed check. We will try to validate it
  return true;
}

bool WeaverCompiler::PrecacheShader (iDocumentNode* templ,
                                     iHierarchicalCache* cache)
{
  const char* shaderName = templ->GetAttributeValue ("name");
  csRef<WeaverShader> shader;

  csTicks startTime = 0, endTime = 0;
  // Create a shader. The actual loading happens later.
  if (do_verbose) startTime = csGetTicks();
  shader.AttachNew (new WeaverShader (this));
  bool loadRet = shader->Precache (templ, cache);
  autoDocRoot.Invalidate ();
  if (!loadRet)
    return false;
  if (do_verbose) 
  {
    endTime = csGetTicks();
  
    csString str;
    //shader->DumpStats (str);
    Report(CS_REPORTER_SEVERITY_NOTIFY, 
      "Shader %s: %s weaved in %u ms", shaderName, str.GetData (),
      endTime - startTime);
  }

  return true;
}

}
CS_PLUGIN_NAMESPACE_END(ShaderWeaver)
