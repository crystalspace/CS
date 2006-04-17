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

#include "cssysdef.h"
#include <ctype.h>

#include "imap/services.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"
#include "ivideo/rendermesh.h"

#include "csutil/xmltiny.h"

#include "shader.h"
#include "xmlshader.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{

CS_LEAKGUARD_IMPLEMENT (csXMLShader);

//---------------------------------------------------------------------------

csShaderConditionResolver::csShaderConditionResolver (
  csXMLShaderCompiler* compiler) : rootNode (0), nextVariant (0),
  evaluator (compiler->strings, compiler->condConstants)
{
  SetEvalParams (0, 0);
}

csShaderConditionResolver::~csShaderConditionResolver ()
{
  delete rootNode;
}

const char* csShaderConditionResolver::SetLastError (const char* msg, ...)
{
  va_list args;
  va_start (args, msg);
  lastError.FormatV (msg, args);
  va_end (args);
  return lastError;
}

const char* csShaderConditionResolver::ParseCondition (const char* str, 
						       size_t len, 
						       csConditionID& result)
{
  CondOperation op;
  const char* err = ParseCondition (str, len, op);
  if (err) return err;
  result = evaluator.FindOptimizedCondition (op);
  return 0;
}

const char* csShaderConditionResolver::ParseCondition (const char* str, 
						       size_t len, 
						       CondOperation& result)
{
  csExpressionTokenList tokens;
  const char* err = tokenizer.Tokenize (str, len, tokens);
  if (err)
    return SetLastError ("Tokenization: %s", err);
  csExpression* newExpression;
  err = parser.Parse (tokens, newExpression);
  if (err)
  {
    delete newExpression;
    return SetLastError ("Parsing: %s", err);
  }

  err = evaluator.ProcessExpression (newExpression, result);
  delete newExpression;
  if (err)
    return SetLastError ("Processing: %s", err);

  return 0;
}

bool csShaderConditionResolver::Evaluate (csConditionID condition)
{
  const csRenderMeshModes* modes = csShaderConditionResolver::modes;
  const csShaderVarStack* stacks = csShaderConditionResolver::stacks;

  return evaluator.Evaluate (condition, modes ? *modes : csRenderMeshModes(),
    stacks ? *stacks : csShaderVarStack ());
}

csConditionNode* csShaderConditionResolver::NewNode (csConditionNode* parent)
{
  csConditionNode* newNode = new csConditionNode (parent);
  return newNode;
}

csConditionNode* csShaderConditionResolver::GetRoot ()
{
  if (rootNode == 0)
    rootNode = NewNode (0);
  return rootNode;
}

size_t csShaderConditionResolver::GetVariant (csConditionNode* node)
{
  MyBitArray bits (evaluator.GetNumConditions ());
  node->FillConditionArray (bits);
  size_t* var = variantIDs.GetElementPointer (bits);
  if (var)
    return *var;
  else
  {
    variantIDs.Put (bits, nextVariant);
    return nextVariant++;
  }
}

void csShaderConditionResolver::AddNode (csConditionNode* parent,
					 csConditionID condition, 
					 csConditionNode*& trueNode,
					 csConditionNode*& falseNode)
{
  if (rootNode == 0)
  {
    // This is the first condition, new node gets root
    CS_ASSERT_MSG ("No root but parent? Weird.", parent == 0);
    parent = GetRoot ();

    parent->condition = condition;

    parent->trueNode = trueNode = NewNode (parent);
    trueNode->variant = GetVariant (trueNode);

    parent->falseNode = falseNode = NewNode (parent);
    falseNode->variant = GetVariant (falseNode);
  }
  else
  {
    if (parent == 0)
      parent = GetRoot ();

    parent->trueNode = trueNode = NewNode (parent);
    parent->falseNode = falseNode = NewNode (parent);

    CS_ASSERT(parent->variant != csArrayItemNotFound);
    MyBitArray bits (evaluator.GetNumConditions ());
    parent->condition = condition;

    trueNode->variant = GetVariant (trueNode);

    falseNode->variant = parent->variant;
    falseNode->FillConditionArray (bits);
    variantIDs.PutUnique (bits, parent->variant);

    parent->variant = csArrayItemNotFound;
  }
}

void csShaderConditionResolver::FinishAdding ()
{
  variantIDs.Empty();
}

void csShaderConditionResolver::SetEvalParams (const csRenderMeshModes* modes,
					       const csShaderVarStack* stacks)
{
  csShaderConditionResolver::modes = modes;
  csShaderConditionResolver::stacks = stacks;
}

size_t csShaderConditionResolver::GetVariant ()
{
  const csRenderMeshModes& modes = *csShaderConditionResolver::modes;
  const csShaderVarStack& stacks = *csShaderConditionResolver::stacks;

  if (rootNode == 0)
  {
    return 0;
  }
  else
  {
    csConditionNode* currentRoot = 0;
    csConditionNode* nextRoot = rootNode;

    while (nextRoot != 0)
    {
      currentRoot = nextRoot;
      if (evaluator.Evaluate (currentRoot->condition, modes, stacks))
      {
	nextRoot = currentRoot->trueNode;
      }
      else
      {
	nextRoot = currentRoot->falseNode;
      }
    }
    CS_ASSERT (currentRoot != 0);
    return currentRoot->variant;
  }
}

void csShaderConditionResolver::DumpConditionTree (csString& out)
{
  if (rootNode == 0)
    return;

  out += "\n";
  DumpConditionNode (out, rootNode, 0);
}

static void Indent (csString& out, int n)
{
  while (n-- > 0)
    out += "| ";
}

void csShaderConditionResolver::DumpConditionNode (csString& out,
						   csConditionNode* node, 
						   int level)
{
  if (node == 0)
  {
    Indent (out, level);
    out.Append ("<none>\n");
  }
  else
  {
    if (node->variant != csArrayItemNotFound)
    {
      out.AppendFmt ("variant: %zu", node->variant);
    }
    else
    {
      out.Append ("\n");
      Indent (out, level);
      out.AppendFmt ("condition %zu = true: ", node->condition);
      DumpConditionNode (out, node->trueNode, level + 1);
      out.Append ("\n");
      Indent (out, level);
      out.AppendFmt ("condition %zu = false: ", node->condition);
      DumpConditionNode (out, node->falseNode, level + 1);
    }
  }
}

//---------------------------------------------------------------------------

csXMLShader::csXMLShader (csXMLShaderCompiler* compiler, 
			  iDocumentNode* source,
			  int forcepriority) : scfImplementationType (this)
{
  InitTokenTable (xmltokens);

  activeTech = 0;
  filename = 0;
  csXMLShader::compiler = compiler;
  g3d = compiler->g3d;
  csXMLShader::forcepriority = forcepriority;
  useFallbackContext = false;

  shadermgr = CS_QUERY_REGISTRY (compiler->objectreg, iShaderManager);
  CS_ASSERT (shadermgr); // Should be present - loads us, after all

  resolver = new csShaderConditionResolver (compiler);
  csRef<iDocumentNode> wrappedNode;
  if (compiler->doDumpConds)
  {
    csString tree;
    tree.SetGrowsBy (0);
    wrappedNode.AttachNew (compiler->wrapperFact->CreateWrapper (source, 
      resolver, resolver->evaluator, &tree));
    resolver->DumpConditionTree (tree);
    csString filename;
    filename.Format ("/tmp/shader/cond_%s.txt", source->GetAttributeValue ("name"));
    compiler->vfs->WriteFile (filename, tree.GetData(), tree.Length());
  }
  else
    wrappedNode.AttachNew (compiler->wrapperFact->CreateWrapper (source, 
    resolver, resolver->evaluator, 0));
  shaderSource = wrappedNode;
  vfsStartDir = csStrNew (compiler->vfs->GetCwd ());

  //Load global shadervars block
  csRef<iDocumentNode> varNode = shaderSource->GetNode(
    xmltokens.Request (csXMLShaderCompiler::XMLTOKEN_SHADERVARS));
 
  if (varNode)
    ParseGlobalSVs (varNode);

  csRef<iDocumentNode> fallbackNode = shaderSource->GetNode ("fallbackshader");
  if (fallbackNode.IsValid())
  {
    fallbackShader = compiler->synldr->ParseShaderRef (fallbackNode);
  }
}

csXMLShader::~csXMLShader ()
{
  for (size_t i = 0; i < variants.Length(); i++)
  {
    delete variants[i].tech;
  }

  delete[] filename;
  delete resolver;
  delete[] vfsStartDir;
  delete[] allShaderMeta.description;
}

void csXMLShader::SelfDestruct ()
{
  if (shadermgr)
    shadermgr->UnregisterShader ((iShader*)this);
}

int csXMLShader::CompareTechniqueKeeper (
  TechniqueKeeper const& t1, TechniqueKeeper const& t2)
{
  int v = t2.priority - t1.priority;
  if (v == 0) v = t2.tagPriority - t1.tagPriority;
  return v;
}

void csXMLShader::ScanForTechniques (iDocumentNode* templ,
		csArray<TechniqueKeeper>& techniquesTmp,
		int forcepriority)
{
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
      if ((forcepriority != -1) && (p != forcepriority)) continue;
      TechniqueKeeper keeper (child, p);
      // Compute the tag's priorities.
      csRef<iDocumentNodeIterator> tagIt = child->GetNodes ("tag");
      while (tagIt->HasNext ())
      {
	csRef<iDocumentNode> tag = tagIt->Next ();
	csStringID tagID = compiler->strings->Request (tag->GetContentsValue ());

	csShaderTagPresence presence;
	int priority;
	shadermgr->GetTagOptions (tagID, presence, priority);
	if (presence == TagNeutral)
	{
	  keeper.tagPriority += priority;
	}
      }
      techniquesTmp.Push (keeper);
    }
  }

  techniquesTmp.Sort (&CompareTechniqueKeeper);
}

/**
 * A wrapped class to have a csShaderVarStack synced to an
 * iShaderVariableContext*.
 */
class SVCWrapper : public scfImplementation1<SVCWrapper, 
					     iShaderVariableContext>
{
  csShaderVariableContext& wrappedSVC;
public:
  csShaderVarStack svStack;

  SVCWrapper (csShaderVariableContext& wrappedSVC) : 
    scfImplementationType (this), wrappedSVC (wrappedSVC)
  {
    wrappedSVC.PushVariables (svStack);
  }
  virtual ~SVCWrapper () { }
  virtual void AddVariable (csShaderVariable *variable)
  {
    wrappedSVC.AddVariable (variable);
  }
  virtual csShaderVariable* GetVariable (csStringID name) const
  { return wrappedSVC.GetVariable (name); }
  virtual const csRefArray<csShaderVariable>& GetShaderVariables () const
  { return wrappedSVC.GetShaderVariables (); }
  virtual void PushVariables (csShaderVarStack &stacks) const
  { wrappedSVC.PushVariables (stacks); }
  virtual bool IsEmpty() const
  { return wrappedSVC.IsEmpty(); }
  void ReplaceVariable (csShaderVariable *variable)
  { wrappedSVC.ReplaceVariable (variable); }
  void Clear ()
  { wrappedSVC.Clear(); }
};

void csXMLShader::ParseGlobalSVs (iDocumentNode* node)
{
  SVCWrapper wrapper (globalSVContext);
  resolver->ResetEvaluationCache();
  resolver->SetEvalParams (0, &wrapper.svStack);
  compiler->LoadSVBlock (node, &wrapper);
  resolver->SetEvalParams (0, 0);
}

static void CloneNode (iDocumentNode* from, iDocumentNode* to)
{
  to->SetValue (from->GetValue ());
  csRef<iDocumentNodeIterator> it = from->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    csRef<iDocumentNode> child_clone = to->CreateNodeBefore (
    	child->GetType (), 0);
    CloneNode (child, child_clone);
  }
  csRef<iDocumentAttributeIterator> atit = from->GetAttributes ();
  while (atit->HasNext ())
  {
    csRef<iDocumentAttribute> attr = atit->Next ();
    to->SetAttribute (attr->GetName (), attr->GetValue ());
  }
}

size_t csXMLShader::GetTicket (const csRenderMeshModes& modes, 
			       const csShaderVarStack& stacks)
{
  resolver->ResetEvaluationCache();
  resolver->SetEvalParams (&modes, &stacks);
  size_t vi = resolver->GetVariant ();

  if (vi != csArrayItemNotFound)
  {
    ShaderVariant& var = variants.GetExtend (vi);

    if (!var.prepared)
    {
      if (compiler->doDumpXML)
      {
	csRef<iDocumentSystem> docsys;
	docsys.AttachNew (new csTinyDocumentSystem);
	csRef<iDocument> newdoc = docsys->CreateDocument();
	CloneNode (shaderSource, newdoc->CreateRoot());
	newdoc->Write (compiler->vfs, csString().Format ("/tmp/shader/%s_%zu.xml",
	  GetName(), vi));
      }

      // So external files are found correctly
      compiler->vfs->PushDir ();
      compiler->vfs->ChDir (vfsStartDir);

      csArray<TechniqueKeeper> techniquesTmp;
      ScanForTechniques (shaderSource, techniquesTmp, forcepriority);
		      
      csArray<TechniqueKeeper>::Iterator techIt = techniquesTmp.GetIterator ();
      while (techIt.HasNext ())
      {
	TechniqueKeeper tk = techIt.Next();
	csXMLShaderTech* tech = new csXMLShaderTech (this);
	if (tech->Load (tk.node, shaderSource))
	{
	  if (compiler->do_verbose)
	    compiler->Report (CS_REPORTER_SEVERITY_NOTIFY,
	      "Shader '%s'<%zu>: Technique with priority %u succeeds!",
	      GetName(), vi, tk.priority);
	  var.tech = tech;
	  break;
	}
	else
	{
	  if (compiler->do_verbose)
	  {
	    compiler->Report (CS_REPORTER_SEVERITY_NOTIFY,
	      "Shader '%s'<%zu>: Technique with priority %u fails. Reason: %s.",
	      GetName(), vi, tk.priority, tech->GetFailReason());
	  }
	  delete tech;
	}
      }
      compiler->vfs->PopDir ();

      var.prepared = var.tech != 0;
    }
    if (var.tech == 0)
    {
      if (fallbackShader.IsValid())
      {
	if (compiler->do_verbose && !var.prepared)
	{
	  compiler->Report (CS_REPORTER_SEVERITY_NOTIFY,
	    "No technique validated for shader '%s'<%zu>: using fallback", 
	    GetName(), vi);
	}
	size_t fallbackTicket = fallbackShader->GetTicket (modes, stacks);
	if (fallbackTicket != csArrayItemNotFound)
	{
	  size_t vc = resolver->GetVariantCount();
	  if (vc == 0) vc = 1;
	  vi = fallbackTicket + vc;
	}
	else
	  vi = csArrayItemNotFound;
      }
      else if (!var.prepared && compiler->do_verbose)
	compiler->Report (CS_REPORTER_SEVERITY_WARNING,
	  "No technique validated for shader '%s'<%zu>", GetName(), vi);
      var.prepared = true;
    }
  }
  resolver->SetEvalParams (0, 0);
  return vi;
}

bool csXMLShader::ActivatePass (size_t ticket, size_t number)
{ 
  if (IsFallbackTicket (ticket))
  {
    useFallbackContext = true;
    return fallbackShader->ActivatePass (GetFallbackTicket (ticket), number);
  }

  CS_ASSERT_MSG ("ActivatePass() has already been called.",
    activeTech == 0);
  activeTech = (ticket != csArrayItemNotFound) ? variants[ticket].tech :
    0;
  return activeTech ? activeTech->ActivatePass (number) : false;
}

bool csXMLShader::DeactivatePass (size_t ticket)
{ 
  if (IsFallbackTicket (ticket))
  {
    useFallbackContext = false;
    return fallbackShader->DeactivatePass (GetFallbackTicket (ticket));
  }

  bool ret = activeTech ? activeTech->DeactivatePass() : false; 
  activeTech = 0;
  return ret;
}

void csXMLShader::DumpStats (csString& str)
{
  if (resolver->GetVariantCount () == 0)
    str.Replace ("unvarying");
  else
    str.Format ("%zu variations", resolver->GetVariantCount ());
}

csRef<iDocumentNode> csXMLShader::LoadProgramFile (const char* filename)
{
  csRef<iVFS> VFS = CS_QUERY_REGISTRY (compiler->objectreg, 
    iVFS);
  csRef<iFile> programFile = VFS->Open (filename, VFS_FILE_READ);
  if (!programFile)
  {
    compiler->Report (CS_REPORTER_SEVERITY_ERROR,
      "Unable to open shader program file '%s'", filename);
    return 0;
  }
  csRef<iDocumentSystem> docsys (
    CS_QUERY_REGISTRY (compiler->objectreg, iDocumentSystem));
  if (docsys == 0)
    docsys.AttachNew (new csTinyDocumentSystem ());

  csRef<iDocument> programDoc = docsys->CreateDocument ();
  const char* err = programDoc->Parse (programFile, true);
  if (err != 0)
  {
    compiler->Report (CS_REPORTER_SEVERITY_ERROR,
      "Unable to parse shader program file '%s': %s", filename, err);
    return 0;
  }

  csString dumpFN;
  if (compiler->doDumpConds || compiler->doDumpXML)
  {
    csString filenameClean (filename);
    for (size_t p = 0; p < filenameClean.Length(); p++)
    {
      if (!isalnum (filenameClean[p])) filenameClean[p] = '_';
    }
    dumpFN.Format ("%s_%s",
      GetName(), filenameClean.GetData());
  }

  csRef<iDocumentNode> programNode;
  if (compiler->doDumpConds)
  {
    csString tree;
    programNode.AttachNew (compiler->wrapperFact->CreateWrapperStatic (
      programDoc->GetRoot (), resolver, &tree));
    resolver->DumpConditionTree (tree);
    compiler->vfs->WriteFile (csString().Format ("/tmp/shader/%s.txt",
      dumpFN.GetData()), tree.GetData(), tree.Length());
  }
  else
    programNode.AttachNew (compiler->wrapperFact->CreateWrapperStatic (
      programDoc->GetRoot (), resolver, 0));

  if (compiler->doDumpXML)
  {
    csRef<iDocumentSystem> docsys;
    docsys.AttachNew (new csTinyDocumentSystem);
    csRef<iDocument> newdoc = docsys->CreateDocument();
    CloneNode (programNode, newdoc->CreateRoot());
    newdoc->Write (compiler->vfs, csString().Format ("/tmp/shader/%s.xml",
      dumpFN.GetData()));
  }
  return programNode;
}

}
CS_PLUGIN_NAMESPACE_END(XMLShader)
