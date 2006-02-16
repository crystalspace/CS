/*
  Copyright (C) 2004-2006 by Frank Richter
	    (C) 2004-2006 by Jorrit Tyberghein

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
#if defined(CS_REF_TRACKER) && !defined(CS_REF_TRACKER_EXTENSIVE)
  // Performance hack
  #undef CS_REF_TRACKER
#endif
#include <ctype.h>

#include "csgeom/math.h"
#include "csutil/set.h"
#include "csutil/sysfunc.h"
#include "csutil/util.h"
#include "csutil/xmltiny.h"
#include "cstool/vfsdirchange.h"
#include "imap/services.h"
#include "ivaria/reporter.h"
#include "iutil/vfs.h"
#include "iutil/document.h"

#include "docwrap.h"
#include "tokenhelper.h"
#include "xmlshader.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{

void csWrappedDocumentNode::ConditionTree::RecursiveAdd (
  csConditionID condition, Node* node, NodeStackEntry& newCurrent)
{
  Variables trueVals;
  Variables falseVals;
  Logic3 r;
  if (condition == csCondAlwaysTrue)
    r.state = Logic3::Truth;
  else if (condition == csCondAlwaysFalse)
    r.state = Logic3::Lie;
  else
    r = evaluator.CheckConditionResults (condition, 
      node->values, trueVals, falseVals);

  if (node->condition == Node::csCondUnknown)
  {
    switch (r.state)
    {
      case Logic3::Truth:
        newCurrent.branches[0].Push (node);
        break;
      case Logic3::Lie:
        newCurrent.branches[1].Push (node);
        break;
      case Logic3::Uncertain:
        {
          node->condition = condition;
          for (int b = 0; b < 2; b++)
          {
            Node* nn = new Node (node);
            nn->values = (b == 0) ? trueVals : falseVals;
            node->branches[b] = nn;
            if ((b == 0) && (r.state == Logic3::Lie)) continue;
            if ((b == 1) && (r.state == Logic3::Truth)) continue;
            newCurrent.branches[b].Push (nn);
          }
        }
        break;
    }
  }
  else
  {
    if (r.state != Logic3::Lie)
      RecursiveAdd (condition, node->branches[0], newCurrent);
    if (r.state != Logic3::Truth)
      RecursiveAdd (condition, node->branches[1], newCurrent);
  }
}

Logic3 csWrappedDocumentNode::ConditionTree::Descend (csConditionID condition)
{
  const NodeStackEntry& current = 
    nodeStack[nodeStack.GetSize()-1];

  NodeStackEntry newCurrent;

  const csArray<Node*>& currentNodes = current.branches[currentBranch];
  for (size_t i = 0; i < currentNodes.GetSize(); i++)
  {
    RecursiveAdd (condition, currentNodes[i], newCurrent);
  }

  nodeStack.Push (newCurrent);
  branchStack.Push (currentBranch);
  currentBranch = 0;

  Logic3 r;
  if (newCurrent.branches[0].IsEmpty()
    && !newCurrent.branches[1].IsEmpty())
    r.state = Logic3::Lie;
  else if (!newCurrent.branches[0].IsEmpty()
    && newCurrent.branches[1].IsEmpty())
    r.state = Logic3::Truth;

  return r;
}

void csWrappedDocumentNode::ConditionTree::SwitchBranch ()
{
  CS_ASSERT(currentBranch == 0);
  currentBranch = 1;
}

void csWrappedDocumentNode::ConditionTree::Ascend (int num)
{
  CS_ASSERT_MSG("Either too many Ascend()s or too few Descend()s", 
    nodeStack.GetSize() > 1);
  while (num-- > 0)
  {
    nodeStack.Pop();
    currentBranch = branchStack.Pop();
  }
}

void csWrappedDocumentNode::ConditionTree::ToResolver (
  iConditionResolver* resolver, Node* node, csConditionNode* parent)
{
  if (node->condition == Node::csCondUnknown) return;

  csConditionNode* trueNode;
  csConditionNode* falseNode;

  resolver->AddNode (parent, node->condition, trueNode, falseNode);
  if (node->branches[0] != 0)
    ToResolver (resolver, node->branches[0], trueNode);
  if (node->branches[1] != 0)
    ToResolver (resolver, node->branches[1], falseNode);
}

void csWrappedDocumentNode::ConditionTree::ToResolver (
  iConditionResolver* resolver)
{
  if (root->branches[0] != 0)
  {
    ToResolver (resolver, root, 0);
  }
}

//---------------------------------------------------------------------------

CS_LEAKGUARD_IMPLEMENT(csWrappedDocumentNode);

csWrappedDocumentNode::csWrappedDocumentNode (csWrappedDocumentNodeFactory* shared_fact,
					      iDocumentNode* wrapped_node,
					      iConditionResolver* res,
                                              csConditionEvaluator& evaluator)
  : scfImplementationType (this), wrappedNode (wrapped_node), resolver (res),
    objreg (shared_fact->plugin->objectreg), shared (shared_fact)
{
  CS_ASSERT (resolver);
  globalState.AttachNew (new GlobalProcessingState (evaluator));

  ProcessWrappedNode ();
  globalState->condTree.ToResolver (resolver);

  globalState = 0;
}

csWrappedDocumentNode::csWrappedDocumentNode (iDocumentNode* wrapped_node,
					      csWrappedDocumentNode* parent,
					      csWrappedDocumentNodeFactory* shared_fact, 
					      GlobalProcessingState* global_state)
  : scfImplementationType (this), wrappedNode (wrapped_node), 
    resolver (parent->resolver), objreg (shared_fact->plugin->objectreg), 
    shared (shared_fact), globalState (global_state)
{
  ProcessWrappedNode ();

  globalState = 0;
}

csWrappedDocumentNode::~csWrappedDocumentNode ()
{
}

struct ReplacedEntity
{
  const char* entity;
  char replacement;
};

static const ReplacedEntity entities[] = {
  {"&lt;", '<'},
  {"&gt;", '>'},
  {0, 0}
};

static const char* ReplaceEntities (const char* str, csString& scratch)
{
  const ReplacedEntity* entity = entities;
  while (entity->entity != 0)
  {
    const char* entPos;
    if ((entPos = strstr (str, entity->entity)) != 0)
    {
      size_t offset = entPos - str;
      if (scratch.GetData() == 0)
      {
	scratch.Replace (str);
	str = scratch.GetData ();
      }
      scratch.DeleteAt (offset, strlen (entity->entity));
      scratch.Insert (offset, entity->replacement);
    }
    else
      entity++;
  }
  return str;
}

struct WrapperStackEntry
{
  csWrappedDocumentNode::WrappedChild* child;

  WrapperStackEntry () : child (0) {}
};

struct csWrappedDocumentNode::NodeProcessingState
{
  csArray<WrapperStackEntry> wrapperStack;
  WrapperStackEntry currentWrapper;
  csRef<iDocumentNodeIterator> iter;

  bool templActive;
  Template templ;
  uint templNestCount;
  csString templateName;

  NodeProcessingState() : templActive(false) {}
};

static const int syntaxErrorSeverity = CS_REPORTER_SEVERITY_WARNING;

void csWrappedDocumentNode::ParseCondition (WrapperStackEntry& newWrapper, 
					    const char* cond,
					    size_t condLen, 
					    iDocumentNode* node)
{
  newWrapper.child = new WrappedChild;
  const char* result = resolver->ParseCondition (cond,
    condLen, newWrapper.child->condition);
  if (result)
  {
    csString condStr;
    condStr.Append (cond, condLen);
    Report (syntaxErrorSeverity, node,
      "Error parsing condition '%s': %s", condStr.GetData(),
      result);
    newWrapper.child->condition = csCondAlwaysFalse;
  }
  shared->DumpCondition (newWrapper.child->condition,
    cond, condLen);
}

void csWrappedDocumentNode::CreateElseWrapper (NodeProcessingState* state, 
					       WrapperStackEntry& elseWrapper)
{
  WrapperStackEntry oldCurrentWrapper = state->currentWrapper;
  state->currentWrapper = state->wrapperStack.Pop ();
  elseWrapper = oldCurrentWrapper;
  elseWrapper.child = new WrappedChild;
  elseWrapper.child->condition = oldCurrentWrapper.child->condition;
  elseWrapper.child->conditionValue = false;
}

void csWrappedDocumentNode::ProcessInclude (const csString& filename, 
					    NodeProcessingState* state, 
					    iDocumentNode* node)
{
  csRef<iVFS> vfs = CS_QUERY_REGISTRY (objreg, iVFS);
  CS_ASSERT (vfs.IsValid ());
  csRef<iFile> include = vfs->Open (filename, VFS_FILE_READ);
  if (!include.IsValid ())
  {
    Report (syntaxErrorSeverity, node,
      "could not open '%s'", filename.GetData ());
  }
  else
  {
    csRef<iDocumentSystem> docsys (
      CS_QUERY_REGISTRY(objreg, iDocumentSystem));
    if (docsys == 0)
      docsys.AttachNew (new csTinyDocumentSystem ());

    csRef<iDocument> includeDoc = docsys->CreateDocument ();
    const char* err = includeDoc->Parse (include, false);
    if (err != 0)
    {
      Report (syntaxErrorSeverity, node,
	"error parsing '%s': %s", filename.GetData (), err);
    }
    else
    {
      csRef<iDocumentNode> rootNode = includeDoc->GetRoot ();
      csRef<iDocumentNode> includeNode = rootNode->GetNode ("include");
      if (!includeNode)
      {
	Report (syntaxErrorSeverity, rootNode,
	  "%s: no <include> node", filename.GetData ());
	return;
      }
      csVfsDirectoryChanger dirChange (vfs);
      dirChange.ChangeTo (filename);

      csRef<iDocumentNodeIterator> it = includeNode->GetNodes ();
      while (it->HasNext ())
      {
	csRef<iDocumentNode> child = it->Next ();
	ProcessSingleWrappedNode (state, child);
      }
    }
  }
}

void csWrappedDocumentNode::ProcessTemplate (iDocumentNode* templNode, 
					     NodeProcessingState* state)
{
  csRefArray<iDocumentNode>& templNodes = state->templ.nodes;
  csRef<iDocumentNode> node = templNode;
  if (node->GetType() == CS_NODE_UNKNOWN)
  {
    csString replaceScratch;
    const char* nodeValue = ReplaceEntities (node->GetValue(),
      replaceScratch);
    if ((nodeValue != 0) && (*nodeValue == '?') && 
      (*(nodeValue + strlen (nodeValue) - 1) == '?'))
    {
      const char* valStart = nodeValue + 1;

      while (*valStart == ' ') valStart++;
      CS_ASSERT (*valStart != 0);
      size_t valLen = strlen (valStart) - 1;
      if (valLen != 0)
      {
	while (*(valStart + valLen - 1) == ' ') valLen--;
	const char* space = strchr (valStart, ' ');
	/* The rightmost spaces were skipped and don't interest us
	    any more. */
	if (space >= valStart + valLen) space = 0;
	size_t cmdLen;
	if (space != 0)
	{
	  cmdLen = space - valStart;
	}
	else
	{
	  cmdLen = valLen;
	}
	csString tokenStr; tokenStr.Replace (valStart, cmdLen);
	csStringID tokenID = shared->pitokens.Request (tokenStr);
	switch (tokenID)
	{
	  case csWrappedDocumentNodeFactory::PITOKEN_ENDTEMPLATE:
            if (shared->plugin->do_verbose)
            {
              Report (CS_REPORTER_SEVERITY_WARNING, node,
                "Deprecated syntax, please use 'Endtemplate'");
            }
            // Fall through
	  case csWrappedDocumentNodeFactory::PITOKEN_ENDTEMPLATE_NEW:
	    state->templNestCount--;
	    if (state->templNestCount != 0)
	      templNodes.Push (node);
	    break;
	  case csWrappedDocumentNodeFactory::PITOKEN_TEMPLATE:
            if (shared->plugin->do_verbose)
            {
              Report (CS_REPORTER_SEVERITY_WARNING, node,
                "Deprecated syntax, please use 'Template'");
            }
            // Fall through
	  case csWrappedDocumentNodeFactory::PITOKEN_TEMPLATE_NEW:
	    state->templNestCount++;
	    // Fall through
	  default:
	    {
	      Template* templ;
	      if ((state->templNestCount == 1)
		&& (templ = globalState->templates.GetElementPointer (tokenStr)))
	      {
		csArray<csString> templArgs;
		csString pStr (valStart + cmdLen, valLen - cmdLen);
		pStr.Trim();
		ParseTemplateArguments (pStr, templArgs);
		InvokeTemplate (templ, templArgs, templNodes);
	      }
	      else
		templNodes.Push (node);
	    }
	    break;
	}
      }
    }
  }
  else
    templNodes.Push (node);

  if (state->templNestCount == 0)
  {
    globalState->templates.PutUnique (state->templateName, state->templ);
    state->templActive = false;
  }
}

bool csWrappedDocumentNode::InvokeTemplate (Template* templ,
					    const csArray<csString>& params,
					    csRefArray<iDocumentNode>& templatedNodes)
{
  if (!templ) return false;

  size_t i;
  Substitutions paramSubst;
  for (i = 0; i < csMin (params.Length(), templ->paramMap.Length()); i++)
  {
    paramSubst.Put (templ->paramMap[i], params[i]);
  }

  for (i = 0; i < templ->nodes.Length(); i++)
  {
    csRef<iDocumentNode> newNode = 
      shared->replacerFactory.CreateWrapper (templ->nodes.Get (i), 0,
      paramSubst);
    templatedNodes.Push (newNode);
  }
  return true;
}

bool csWrappedDocumentNode::InvokeTemplate (const char* name, 
					    iDocumentNode* node,
					    NodeProcessingState* state, 
					    const csArray<csString>& params)
{
  Template* templNodes = 
    globalState->templates.GetElementPointer (name);

  csRefArray<iDocumentNode> nodes;

  if (!InvokeTemplate (templNodes, params, nodes))
    return false;

  size_t i;
  for (i = 0; i < nodes.Length(); i++)
  {
    ProcessSingleWrappedNode (state, nodes[i]);
  }
  ValidateTemplateEnd (node, state);
  return true;
}

void csWrappedDocumentNode::ValidateTemplateEnd (iDocumentNode* node, 
						 NodeProcessingState* state)
{
  if ((state->templActive) && (state->templNestCount != 0))
  {
    Report (syntaxErrorSeverity, node,
      "'Template' without 'Endtemplate'");
  }
}

void csWrappedDocumentNode::ParseTemplateArguments (const char* str, 
						    csArray<csString>& strings)
{
  if (!str) return;

  csString currentStr;

  while (*str != 0)
  {
    currentStr.Empty();
    while ((*str != 0) && isspace (*str)) str++;
    if (*str == '"')
    {
      while (*str != 0)
      {
	if (*str == '\\')
	{
	  str++;
	  currentStr << *str;
	  str++;
	}
	else if (*str == '"')
	{
	  str++;
	  break;
	}
	else
	{
	  currentStr << *str;
	  str++;
	}
      }
    }
    else
    {
      while ((*str != 0) && !isspace (*str)) 
      {
	currentStr << *str;
	str++;
      }
    }
    if (!currentStr.IsEmpty()) strings.Push (currentStr);
  }
}

void csWrappedDocumentNode::ProcessSingleWrappedNode (
  NodeProcessingState* state, iDocumentNode* node)
{
  CS_ASSERT(globalState);

  if (state->templActive)
  {
    ProcessTemplate (node, state);
    return;
  }

  csArray<WrapperStackEntry>& wrapperStack = state->wrapperStack;
  WrapperStackEntry& currentWrapper = state->currentWrapper;
  bool handled = false;
  if (node->GetType() == CS_NODE_UNKNOWN)
  {
    csString replaceScratch;
    const char* nodeValue = ReplaceEntities (node->GetValue(),
      replaceScratch);
    if ((nodeValue != 0) && (*nodeValue == '?') && 
      (*(nodeValue + strlen (nodeValue) - 1) == '?'))
    {
      const char* valStart = nodeValue + 1;
      if ((*valStart == '!') || (*valStart == '#'))
      {
	/* Discard PIs beginning with ! and #. This allows comments, e.g.
	 * <?! some comment ?>
	 * The difference to XML comments is that the PI comments do not
	 * appear in the final document after processing, hence are useful
	 * if some PIs themselves are to be commented, but it is undesireable
	 * to have an XML comment in the result. */
	return;
      }

      while (*valStart == ' ') valStart++;
      CS_ASSERT (*valStart != 0);
      size_t valLen = strlen (valStart) - 1;
      if (valLen == 0)
      {
	Report (syntaxErrorSeverity, node,
	  "Empty processing instruction");
      }
      else
      {
	while (*(valStart + valLen - 1) == ' ') valLen--;
	const char* space = strchr (valStart, ' ');
	/* The rightmost spaces were skipped and don't interest us
	    any more. */
	if (space >= valStart + valLen) space = 0;
	size_t cmdLen;
	if (space != 0)
	{
	  cmdLen = space - valStart;
	}
	else
	{
	  cmdLen = valLen;
	}

	csString tokenStr; tokenStr.Replace (valStart, cmdLen);
	csStringID tokenID = shared->pitokens.Request (tokenStr);
	switch (tokenID)
	{
	  case csWrappedDocumentNodeFactory::PITOKEN_IF:
	    {
	      WrapperStackEntry newWrapper;
	      ParseCondition (newWrapper, space + 1, valLen - cmdLen - 1, 
		node);
          
              const csConditionID condition = newWrapper.child->condition;
              Logic3 r = globalState->condTree.Descend (condition);
              if (r.state == Logic3::Truth)
                newWrapper.child->condition = csCondAlwaysTrue;
              else if (r.state == Logic3::Lie)
                newWrapper.child->condition = csCondAlwaysFalse;
              globalState->ascendStack.Push (1);

	      currentWrapper.child->childrenWrappers.Push (newWrapper.child);
	      wrapperStack.Push (currentWrapper);
	      currentWrapper = newWrapper;
	    }
	    break;
	  case csWrappedDocumentNodeFactory::PITOKEN_ENDIF:
	    {
	      bool okay = true;
	      if (space != 0)
	      {
		Report (syntaxErrorSeverity, node,
		  "'endif' has parameters");
		okay = false;
	      }
	      if (okay && (wrapperStack.Length() == 0))
	      {
		Report (syntaxErrorSeverity, node,
		  "'endif' without 'if' or 'elsif'");
		okay = false;
	      }
	      if (okay)
              {
                const int ascendNum = globalState->ascendStack.Pop ();
                globalState->condTree.Ascend (ascendNum);
		currentWrapper = wrapperStack.Pop ();
              }
	    }
	    break;
	  case csWrappedDocumentNodeFactory::PITOKEN_ELSE:
	    {
	      bool okay = true;
	      if (space != 0)
	      {
		Report (syntaxErrorSeverity, node,
		  "'else' has parameters");
		okay = false;
	      }
	      if (okay && (wrapperStack.Length() == 0))
	      {
		Report (syntaxErrorSeverity, node,
		  "'else' without 'if' or 'elsif'");
		okay = false;
	      }
              if (globalState->condTree.GetBranch() != 0)
	      {
	        Report (syntaxErrorSeverity, node,
	          "Double 'else'");
	        okay = false;
	      }
	      if (okay)
	      {
		WrapperStackEntry newWrapper;
		CreateElseWrapper (state, newWrapper);

		currentWrapper.child->childrenWrappers.Push (newWrapper.child);
		wrapperStack.Push (currentWrapper);
		currentWrapper = newWrapper;

                globalState->condTree.SwitchBranch ();
	      }
	    }
	    break;
	  case csWrappedDocumentNodeFactory::PITOKEN_ELSIF:
	    {
	      bool okay = true;
	      if (wrapperStack.Length() == 0)
	      {
		Report (syntaxErrorSeverity, node,
		  "'elsif' without 'if' or 'elsif'");
		okay = false;
	      }
	      if (okay && (globalState->condTree.GetBranch() != 0))
	      {
		Report (syntaxErrorSeverity, node,
		  "Double 'else'");
		okay = false;
	      }
	      if (okay)
	      {
		WrapperStackEntry elseWrapper;
		CreateElseWrapper (state, elseWrapper);

		currentWrapper.child->childrenWrappers.Push (elseWrapper.child);

		WrapperStackEntry newWrapper;
		ParseCondition (newWrapper, space + 1, valLen - cmdLen - 1,
		  node);
          
                globalState->condTree.SwitchBranch ();
                const csConditionID condition = newWrapper.child->condition;
                Logic3 r = globalState->condTree.Descend (condition);
                globalState->ascendStack[globalState->ascendStack.GetSize()-1]++;
                if (r.state == Logic3::Truth)
                  newWrapper.child->condition = csCondAlwaysTrue;
                else if (r.state == Logic3::Lie)
                  newWrapper.child->condition = csCondAlwaysFalse;

		elseWrapper.child->childrenWrappers.Push (newWrapper.child);
		wrapperStack.Push (elseWrapper);
		currentWrapper = newWrapper;
	      }
	    }
	    break;
	  case csWrappedDocumentNodeFactory::PITOKEN_INCLUDE:
            if (shared->plugin->do_verbose)
            {
              Report (CS_REPORTER_SEVERITY_WARNING, node,
                "Deprecated syntax, please use 'Include'");
            }
            // Fall through
	  case csWrappedDocumentNodeFactory::PITOKEN_INCLUDE_NEW:
	    {
	      bool okay = true;
	      csString filename;
	      const char* space = strchr (valStart, ' ');
	      /* The rightmost spaces were skipped and don't interest us
	       * any more. */
	      if (space != 0)
	      {
		filename.Replace (space + 1, valLen - cmdLen - 1);
		filename.Trim ();
	      }
	      if ((space == 0) || (filename.IsEmpty ()))
	      {
		Report (syntaxErrorSeverity, node,
		  "'include' without filename");
		okay = false;
	      }
	      if (okay)
	      {
                //if (currentWrapper.skip) break;
		ProcessInclude (filename, state, node);
	      }
	    }
	    break;
	  case csWrappedDocumentNodeFactory::PITOKEN_TEMPLATE:
            if (shared->plugin->do_verbose)
            {
              Report (CS_REPORTER_SEVERITY_WARNING, node,
                "Deprecated syntax, please use 'Template'");
            }
            // Fall through
	  case csWrappedDocumentNodeFactory::PITOKEN_TEMPLATE_NEW:
	    {
	      bool okay = true;
	      csString templateName;
	      Template newTempl;
	      templateName.Replace (space + 1, valLen - cmdLen - 1);
	      templateName.RTrim();
	      size_t templateEnd = templateName.FindFirst (' ');
	      if (okay && (templateEnd != (size_t)-1))
	      {
		// Parse template parameter names
		csArray<csString> paramNames;
		ParseTemplateArguments (
		  templateName.GetData() + templateEnd + 1, paramNames);

		csSet<csString> dupeCheck;
		for (size_t i = 0; i < paramNames.Length(); i++)
		{
		  if (dupeCheck.Contains (paramNames[i]))
		  {
		    Report (syntaxErrorSeverity, node,
		      "Duplicate template parameter '%s'", 
		      paramNames[i].GetData());
		    okay = false;
		  }
		  newTempl.paramMap.Push (paramNames[i]);
		  dupeCheck.Add (paramNames[i]);
		}

		templateName.Truncate (templateEnd);
	      }
	      if (okay && templateName.IsEmpty())
	      {
		Report (syntaxErrorSeverity, node,
		  "'template' without name");
		okay = false;
	      }
	      if (okay 
		&& (shared->pitokens.Request (templateName) != csInvalidStringID))
	      {
		Report (syntaxErrorSeverity, node,
		  "Reserved template name '%s'", templateName.GetData());
		okay = false;
	      }
	      if (okay)
	      {
		state->templateName = templateName;
		state->templ = newTempl;
		state->templActive = true;
		state->templNestCount = 1;
	      }
	    }
	    break;
	  case csWrappedDocumentNodeFactory::PITOKEN_ENDTEMPLATE:
            if (shared->plugin->do_verbose)
            {
              Report (CS_REPORTER_SEVERITY_WARNING, node,
                "Deprecated syntax, please use 'Endtemplate'");
            }
            // Fall through
	  case csWrappedDocumentNodeFactory::PITOKEN_ENDTEMPLATE_NEW:
	    {
	      Report (syntaxErrorSeverity, node,
		"'Endtemplate' without 'Template'");
	    }
	    break;
	  default:
	    {
	      csArray<csString> params;
	      if (space != 0)
	      {
		csString pStr (space + 1, valLen - cmdLen - 1);
		ParseTemplateArguments (pStr, params);
	      }
	      if (!InvokeTemplate (tokenStr, node, state, params))
	      {
		Report (syntaxErrorSeverity, node,
		  "Unknown command '%s'", tokenStr.GetData());
	      }
	    }
	}

	handled = true;
      }
    }
  }
  if (!handled
    && !(((currentWrapper.child->condition == csCondAlwaysFalse)
        && (currentWrapper.child->conditionValue == true))
      || ((currentWrapper.child->condition == csCondAlwaysTrue)
        && (currentWrapper.child->conditionValue == false))))
  {
    WrappedChild* newWrapper = new WrappedChild;
    newWrapper->childNode.AttachNew (new csWrappedDocumentNode (node,
      this, shared, globalState));
    currentWrapper.child->childrenWrappers.Push (newWrapper);
  }
}

void csWrappedDocumentNode::ProcessWrappedNode (NodeProcessingState* state, 
						iDocumentNode* wrappedNode)
{
  if ((wrappedNode->GetType() == CS_NODE_ELEMENT)
    || (wrappedNode->GetType () == CS_NODE_DOCUMENT))
  {
    state->iter = wrappedNode->GetNodes ();
    while (state->iter->HasNext ())
    {
      csRef<iDocumentNode> node = state->iter->Next();
      ProcessSingleWrappedNode (state, node);
    }
    ValidateTemplateEnd (wrappedNode, state);
  }
}

void csWrappedDocumentNode::ProcessWrappedNode ()
{
  NodeProcessingState state;
  state.currentWrapper.child = new WrappedChild;
  wrappedChildren.Push (state.currentWrapper.child);
  ProcessWrappedNode (&state, wrappedNode);
}

void csWrappedDocumentNode::Report (int severity, iDocumentNode* node, 
				    const char* msg, ...)
{
  static const char* messageID = 
    "crystalspace.graphics3d.shadercompiler.xmlshader";

  va_list args;
  va_start (args, msg);

  csRef<iSyntaxService> synsrv = CS_QUERY_REGISTRY (objreg, iSyntaxService);
  if (synsrv.IsValid ())
  {
    csString str;
    str.FormatV (msg, args);
    synsrv->Report (messageID, 
      severity, node, "%s", str.GetData ());
  }
  else
  {
    csReportV (objreg, severity, messageID, msg, args);
  }
  va_end (args);
}

void csWrappedDocumentNode::AppendNodeText (WrapperWalker& walker, 
					    csString& text)
{
  while (walker.HasNext ())
  {
    iDocumentNode* node = walker.Peek ();
    if (node->GetType () != CS_NODE_TEXT)
      break;

    text.Append (node->GetValue ());

    walker.Next ();
  }
}

bool csWrappedDocumentNode::Equals (iDocumentNode* other)
{
  return wrappedNode->Equals (((csWrappedDocumentNode*)other)->wrappedNode);
}

const char* csWrappedDocumentNode::GetValue ()
{
  return wrappedNode->GetValue();
}

csRef<iDocumentNodeIterator> csWrappedDocumentNode::GetNodes ()
{
  csWrappedDocumentNodeIterator* iter = shared->iterPool.Alloc ();
  iter->SetData (this, 0);
  return csPtr<iDocumentNodeIterator> (iter);
}

csRef<iDocumentNodeIterator> csWrappedDocumentNode::GetNodes (
  const char* value)
{
  csWrappedDocumentNodeIterator* iter = shared->iterPool.Alloc ();
  iter->SetData (this, value);
  return csPtr<iDocumentNodeIterator> (iter);
}

csRef<iDocumentNode> csWrappedDocumentNode::GetNode (const char* value)
{
  WrapperWalker walker (wrappedChildren, resolver);
  while (walker.HasNext ())
  {
    iDocumentNode* node = walker.Next ();
    if (strcmp (node->GetValue (), value) == 0)
      return node;
  }

  return 0;
}

const char* csWrappedDocumentNode::GetContentsValue ()
{
  /* Note: it is tempting to reuse 'contents' here if not empty; however,
   * since the value may be different depending on the current resolver
   * and its state, the contents need to be reassembled every time.
   */
  contents.Clear();
  WrapperWalker walker (wrappedChildren, resolver);
  while (walker.HasNext ())
  {
    iDocumentNode* node = walker.Next ();
    if (node->GetType() == CS_NODE_TEXT)
    {
      contents.Append (node->GetValue ());
      AppendNodeText (walker, contents);
      return contents;
    }
  }
  return 0;
}

csRef<iDocumentAttributeIterator> csWrappedDocumentNode::GetAttributes ()
{
  return wrappedNode->GetAttributes ();
}

csRef<iDocumentAttribute> csWrappedDocumentNode::GetAttribute (const char* name)
{
  return wrappedNode->GetAttribute (name);
}

const char* csWrappedDocumentNode::GetAttributeValue (const char* name)
{
  return wrappedNode->GetAttributeValue (name);
}

int csWrappedDocumentNode::GetAttributeValueAsInt (const char* name)
{
  return wrappedNode->GetAttributeValueAsInt (name);
}

float csWrappedDocumentNode::GetAttributeValueAsFloat (const char* name)
{
  return wrappedNode->GetAttributeValueAsFloat (name);
}

bool csWrappedDocumentNode::GetAttributeValueAsBool (const char* name, 
  bool defaultvalue)
{
  return wrappedNode->GetAttributeValueAsBool (name, defaultvalue);
}

//---------------------------------------------------------------------------

csWrappedDocumentNode::WrapperWalker::WrapperWalker (
  csPDelArray<WrappedChild>& wrappedChildren, iConditionResolver* resolver)
{
  SetData (wrappedChildren, resolver);
}
    
csWrappedDocumentNode::WrapperWalker::WrapperWalker ()
{
}

void csWrappedDocumentNode::WrapperWalker::SetData (
  csPDelArray<WrappedChild>& wrappedChildren, iConditionResolver* resolver)
{
  currentPos = &posStack.GetExtend (0);
  currentPos->currentIndex = 0;
  currentPos->currentWrappers = &wrappedChildren;
  WrapperWalker::resolver = resolver;

  SeekNext();
}

void csWrappedDocumentNode::WrapperWalker::SeekNext()
{
  next = 0;

  while (!next.IsValid () && (currentPos != 0))
  {
    if (currentPos->currentIndex < currentPos->currentWrappers->Length ())
    {
      csWrappedDocumentNode::WrappedChild& wrapper = 
	*(currentPos->currentWrappers->Get (currentPos->currentIndex));
      currentPos->currentIndex++;
      if (wrapper.childNode.IsValid ())
      {
	next = wrapper.childNode;
      }
      else
      {
        if ((wrapper.condition == csCondAlwaysTrue)
          || (resolver->Evaluate (wrapper.condition) == wrapper.conditionValue))
	{
	  currentPos = &posStack.GetExtend (posStack.Length ());
	  currentPos->currentIndex = 0;
	  currentPos->currentWrappers = &wrapper.childrenWrappers;
	}
      }
    }
    else
    {
      posStack.Pop ();
      size_t psl = posStack.Length();
      currentPos = (psl > 0) ? &posStack[psl - 1] : 0;
    }
  }
}

bool csWrappedDocumentNode::WrapperWalker::HasNext ()
{
  return next.IsValid();
}

iDocumentNode* csWrappedDocumentNode::WrapperWalker::Peek ()
{
  return next;
}

iDocumentNode* csWrappedDocumentNode::WrapperWalker::Next ()
{
  iDocumentNode* ret = next;
  SeekNext();
  return ret;
}

//---------------------------------------------------------------------------

csTextNodeWrapper::csTextNodeWrapper (Pool* /*pool*/)
  : scfImplementationType (this)
{  
}

csTextNodeWrapper::~csTextNodeWrapper ()
{
  delete[] nodeText;
}

void csTextNodeWrapper::SetData (iDocumentNode* realMe, const char* text)
{
  csTextNodeWrapper::realMe = realMe;
  nodeText = csStrNew (text);
}

//---------------------------------------------------------------------------

// hack: work around problems caused by #defining 'new'
#if defined(CS_EXTENSIVE_MEMDEBUG) || defined(CS_MEMORY_TRACKER)
# undef new
#endif

SCF_IMPLEMENT_IBASE_POOLED(csWrappedDocumentNodeIterator)
  SCF_IMPLEMENTS_INTERFACE(iDocumentNodeIterator)
SCF_IMPLEMENT_IBASE_END

CS_LEAKGUARD_IMPLEMENT(csWrappedDocumentNodeIterator);

csWrappedDocumentNodeIterator::csWrappedDocumentNodeIterator (Pool* pool)
{
  SCF_CONSTRUCT_IBASE_POOLED(pool);

  filter = 0;
}

csWrappedDocumentNodeIterator::~csWrappedDocumentNodeIterator ()
{
  delete[] filter; 
  SCF_DESTRUCT_IBASE();
}

void csWrappedDocumentNodeIterator::SetData (csWrappedDocumentNode* node, 
					     const char* filter)
{
  delete[] csWrappedDocumentNodeIterator::filter;
  csWrappedDocumentNodeIterator::filter = csStrNew (filter);
  parentNode = node;

  walker.SetData (parentNode->wrappedChildren, parentNode->resolver);

  SeekNext();
}

void csWrappedDocumentNodeIterator::SeekNext()
{
  next = 0;
  csRef<iDocumentNode> node;
  while (walker.HasNext ())
  {
    node = walker.Next ();
    if ((filter == 0) || (strcmp (node->GetValue (), filter) == 0))
    {
      next = node;
      break;
    }
  }
  if (next.IsValid () && (next->GetType () == CS_NODE_TEXT))
  {
    csString str;
    str.Append (next->GetValue ());
    csWrappedDocumentNode::AppendNodeText (walker, str);
    //csTextNodeWrapper* textNode = parentNode->shared->textNodePool.Alloc (); //@@FIX
    csTextNodeWrapper *textNode = new csTextNodeWrapper (0);
    textNode->SetData (next, str);
    next.AttachNew (textNode);
  }
}

bool csWrappedDocumentNodeIterator::HasNext ()
{
  return next.IsValid();
}

csRef<iDocumentNode> csWrappedDocumentNodeIterator::Next ()
{
  csRef<iDocumentNode> ret = next;
  SeekNext();
  return ret;
}

//---------------------------------------------------------------------------

csWrappedDocumentNodeFactory::csWrappedDocumentNodeFactory (
  csXMLShaderCompiler* plugin) : plugin (plugin)
{
  InitTokenTable (pitokens);
  pitokens.Register ("Template", PITOKEN_TEMPLATE_NEW);
  pitokens.Register ("Endtemplate", PITOKEN_ENDTEMPLATE_NEW);
  pitokens.Register ("Include", PITOKEN_INCLUDE_NEW);
}

void csWrappedDocumentNodeFactory::DumpCondition (size_t id, 
						  const char* condStr, 
						  size_t condLen)
{
  if (currentOut)
  {
    currentOut->AppendFmt ("condition %zu = '", id);
    currentOut->Append (condStr, condLen);
    currentOut->Append ("'\n");
  }
}

csWrappedDocumentNode* csWrappedDocumentNodeFactory::CreateWrapper (
  iDocumentNode* wrappedNode, iConditionResolver* resolver,
  csConditionEvaluator& evaluator, csString* dumpOut)
{
  currentOut = dumpOut;
  return new csWrappedDocumentNode (this, wrappedNode, resolver, evaluator);
}

}
CS_PLUGIN_NAMESPACE_END(XMLShader)
