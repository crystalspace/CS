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
#include "csutil/bitarray.h"
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

// Hack: Work around problems caused by #defining 'new'.
#if defined(CS_EXTENSIVE_MEMDEBUG) || defined(CS_MEMORY_TRACKER)
# undef new
#endif
#include <new>

class ConditionTree
{
  struct Node
  {
    static const csConditionID csCondUnknown = (csConditionID)~2;    
    
    Node* parent;
    ConditionTree* owner;

    csConditionID condition;
    Node* branches[2];
    Variables values;
    MyBitArrayTemp conditionAffectedSVs;

    Node (Node* p, ConditionTree* owner) : parent (p), owner (owner),
      condition (csCondUnknown)
    {
      branches[0] = 0;
      branches[1] = 0;
    }
    ~Node ()
    {
      if (branches[0]) branches[0]->~Node();
      owner->nodeAlloc.Free (branches[0]);
      if (branches[1]) branches[1]->~Node();
      owner->nodeAlloc.Free (branches[1]);
    }
  };

  csFixedSizeAllocator<sizeof (Node), TempHeapAlloc> nodeAlloc;
  Node* root;
  int currentBranch;
  typedef csArray<Node*, csArrayElementHandler<Node*>, TempHeapAlloc> 
    NodeArray;
  struct NodeStackEntry
  {
    NodeArray branches[2];
  };

  csArray<NodeStackEntry, csArrayElementHandler<NodeStackEntry>, 
    TempHeapAlloc> nodeStack;
  csArray<int, csArrayElementHandler<int>, TempHeapAlloc> branchStack;

  void RecursiveAdd (csConditionID condition, Node* node, 
    NodeStackEntry& newCurrent, MyBitArrayTemp& affectedSVs);
  void ToResolver (iConditionResolver* resolver, Node* node,
    csConditionNode* parent);

  bool HasContainingCondition (Node* node, csConditionID containedCondition,
    csConditionID& condition);

  csConditionEvaluator& evaluator;
public:
  ConditionTree (csConditionEvaluator& evaluator) : nodeAlloc (256), evaluator (evaluator)
  {
    root = (Node*)nodeAlloc.Alloc();
    new (root) Node (0, this);
    currentBranch = 0;
    NodeStackEntry newPair;
    newPair.branches[0].Push (root);
    nodeStack.Push (newPair);
  }
  ~ConditionTree ()
  {
    root->~Node();
    nodeAlloc.Free (root);
  }

  Logic3 Descend (csConditionID condition);
  // Switch the current branch from "true" to "false"
  void SwitchBranch ();
  void Ascend (int num);
  int GetBranch() const { return currentBranch; }

  void ToResolver (iConditionResolver* resolver);
};


void ConditionTree::RecursiveAdd (csConditionID condition, Node* node, 
                                  NodeStackEntry& newCurrent, 
                                  MyBitArrayTemp& affectedSVs)
{
  /* Shortcut */
  if (node->condition == condition)
  {
    newCurrent.branches[0].Push (node->branches[0]);
    newCurrent.branches[1].Push (node->branches[1]);
    return;
  }

  Variables trueVals;
  Variables falseVals;
  Logic3 r;
  if (condition == csCondAlwaysTrue)
    r.state = Logic3::Truth;
  else if (condition == csCondAlwaysFalse)
    r.state = Logic3::Lie;
  else
  {
    bool isLeaf = node->condition == Node::csCondUnknown;
    bool doCheck = true;
    if (!isLeaf && (node->parent != 0))
    {
      /* Do a check if a condition result check is worthwhile.
       * For each node, the SVs which affect the node's and its
       * parents conditions are recorded; if the intersection of
       * that set with the set of SVs that affect the current
       * condition is empty, we don't need to do a check.
       *
       * Though, if the node is a leaf, always check since the
       * "true values" and "false values" are required.
       */
      MyBitArrayTemp& nodeAffectedSVs = node->parent->conditionAffectedSVs;
      if (affectedSVs.GetSize() != nodeAffectedSVs.GetSize())
      {
        size_t newSize = csMax (affectedSVs.GetSize(),
          nodeAffectedSVs.GetSize());
        affectedSVs.SetSize (newSize);
        nodeAffectedSVs.SetSize (newSize);
      }
      doCheck = !(affectedSVs & nodeAffectedSVs).AllBitsFalse();
    }
    if (doCheck)
    {
      if (isLeaf)
        r = evaluator.CheckConditionResults (condition, 
          node->values, trueVals, falseVals);
      else
        r = evaluator.CheckConditionResults (condition, 
          node->values);
    }
  }

  switch (r.state)
  {
    case Logic3::Truth:
      newCurrent.branches[0].Push (node);
      break;
    case Logic3::Lie:
      newCurrent.branches[1].Push (node);
      break;
    case Logic3::Uncertain:
      if (node->condition == Node::csCondUnknown)
      {
        csConditionID containerCondition;
        bool hasContainer = HasContainingCondition (node, condition,
          containerCondition);

        node->condition = condition;
        node->conditionAffectedSVs = affectedSVs;
        if (node->parent)
        {
          MyBitArrayTemp& nodeAffectedSVs = node->conditionAffectedSVs;
          MyBitArrayTemp& parentAffectedSVs = node->parent->conditionAffectedSVs;
          if (nodeAffectedSVs.GetSize() != parentAffectedSVs.GetSize())
          {
            size_t newSize = csMax (nodeAffectedSVs.GetSize(),
              parentAffectedSVs.GetSize());
            nodeAffectedSVs.SetSize (newSize);
            parentAffectedSVs.SetSize (newSize);
          }
          nodeAffectedSVs |= parentAffectedSVs;
        }
        for (int b = 0; b < 2; b++)
        {
          Node* nn = (Node*)nodeAlloc.Alloc();
          new (nn) Node (node, this);
          if (hasContainer)
          {
            /* If this condition is part of a "composite condition"
             * (|| or &&) up above in the tree, use the possible values
             * from running that containing condition with the possible
             * values from the contained condition. That way, in cases
             * like 'a' nested in 'a || b', a true 'a' will result in
             * a set of possible values of only true for 'b', allowing
             * the latter to be possibly folded away later.
             */
            Variables newTrueVals;
            Variables newFalseVals;
            if (b == 0)
            {
              evaluator.CheckConditionResults (containerCondition, 
                trueVals, newTrueVals, newFalseVals);
              nn->values = newTrueVals;
            }
            else
            {
              evaluator.CheckConditionResults (containerCondition, 
                falseVals, newTrueVals, newFalseVals);
              nn->values = newFalseVals;
            }
          }
          else
          {
            nn->values = (b == 0) ? trueVals : falseVals;
          }
          node->branches[b] = nn;
          newCurrent.branches[b].Push (nn);
        }
      } 
      else
      {
        RecursiveAdd (condition, node->branches[0], newCurrent, affectedSVs);
        RecursiveAdd (condition, node->branches[1], newCurrent, affectedSVs);
      }
      break;
  }
}

#if defined(CS_EXTENSIVE_MEMDEBUG) || defined(CS_MEMORY_TRACKER)
# define new CS_EXTENSIVE_MEMDEBUG_NEW
#endif

Logic3 ConditionTree::Descend (csConditionID condition)
{
  const NodeStackEntry& current = 
    nodeStack[nodeStack.GetSize()-1];

  NodeStackEntry newCurrent;

  MyBitArrayTemp affectedSVs;
  evaluator.GetUsedSVs (condition, affectedSVs);
  const NodeArray& currentNodes = current.branches[currentBranch];
  for (size_t i = 0; i < currentNodes.GetSize(); i++)
  {
    RecursiveAdd (condition, currentNodes[i], newCurrent, affectedSVs);
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

void ConditionTree::SwitchBranch ()
{
  CS_ASSERT(currentBranch == 0);
  currentBranch = 1;
}

void ConditionTree::Ascend (int num)
{
  CS_ASSERT_MSG("Either too many Ascend()s or too few Descend()s", 
    nodeStack.GetSize() > 1);
  while (num-- > 0)
  {
    nodeStack.Pop();
    currentBranch = branchStack.Pop();
  }
}

void ConditionTree::ToResolver (iConditionResolver* resolver, 
                                Node* node, csConditionNode* parent)
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

void ConditionTree::ToResolver (iConditionResolver* resolver)
{
  if (root->branches[0] != 0)
  {
    ToResolver (resolver, root, 0);
    resolver->FinishAdding();
  }
}

bool ConditionTree::HasContainingCondition (Node* node, 
                                            csConditionID containedCondition,
                                            csConditionID& condition)
{
  if (node->parent == 0) return false;
  condition = node->parent->condition;
  if (evaluator.IsConditionPartOf (containedCondition, condition)
    && (containedCondition != condition))
    return true;
  return HasContainingCondition (node->parent, containedCondition, condition);
}

//---------------------------------------------------------------------------

typedef csFixedSizeAllocator<sizeof (csWrappedDocumentNode::WrappedChild)> 
  WrappedChildAlloc;
CS_IMPLEMENT_STATIC_VAR (ChildAlloc, WrappedChildAlloc, (256));

void* csWrappedDocumentNode::WrappedChild::operator new (size_t n)
{
  return ChildAlloc()->Alloc (n);
}
void csWrappedDocumentNode::WrappedChild::operator delete (void* p)
{
  ChildAlloc()->Free (p);
}

//---------------------------------------------------------------------------

CS_LEAKGUARD_IMPLEMENT(csWrappedDocumentNode);

template<typename ConditionEval>
csWrappedDocumentNode::csWrappedDocumentNode (ConditionEval& eval,
                                              iDocumentNode* wrapped_node,
					      iConditionResolver* res,
					      csWrappedDocumentNodeFactory* shared_fact, 
					      GlobalProcessingState* global_state)
  : scfImplementationType (this), wrappedNode (wrapped_node), 
    resolver (res), objreg (shared_fact->plugin->objectreg), 
    shared (shared_fact), globalState (global_state)
{
  ProcessWrappedNode (eval);
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

static const char* ReplaceEntities (const char* str, TempString<>& scratch)
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

static bool SplitNodeValue (const char* nodeStr, TempString<>& command, 
                            TempString<>& args)
{
  TempString<> replaceScratch;
  const char* nodeValue = ReplaceEntities (nodeStr, replaceScratch);
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
      command.Replace (valStart, cmdLen);
      args.Replace (valStart + cmdLen, valLen - cmdLen);
      args.LTrim();
      return true;
    }
  }
  return false;
}

static void GetNextArg (const char*& p, TempString<>& arg)
{
  arg.Clear();
  if (p == 0) return;

  while (isspace (*p)) p++;

  if (*p == '"')
  {
    p++;
    while (*p && *p != '"')
    {
      if (*p == '\\')
      {
        p++;
        switch (*p)
        {
          case '"':
            arg << '"';
            p++;
            break;
          case '\\':
            arg << '\\';
            p++;
            break;
        }
      }
      else
        arg << *p++;
    }
    p++;
  }
  else
  {
    while (*p && !isspace (*p)) arg << *p++;
  }
}

struct WrapperStackEntry
{
  csWrappedDocumentNode::WrappedChild* child;

  WrapperStackEntry () : child (0) {}
};
typedef csArray<WrapperStackEntry, csArrayElementHandler<WrapperStackEntry>,
  TempHeapAlloc> WrapperStack;

struct csWrappedDocumentNode::NodeProcessingState
{
  WrapperStack wrapperStack;
  WrapperStackEntry currentWrapper;
  csRef<iDocumentNodeIterator> iter;

  bool templActive;
  Template templ;
  uint templNestLevel;
  TempString<> templateName;
  bool templWeak;

  bool generateActive;
  bool generateValid;
  uint generateNestLevel;
  TempString<> generateVar;
  int generateStart;
  int generateEnd;
  int generateStep;

  struct StaticIfState
  {
    bool processNodes;
    bool elseBranch;

    StaticIfState(bool processNodes) : processNodes (processNodes), 
      elseBranch (false) {}
  };
  csArray<StaticIfState, csArrayElementHandler<StaticIfState>,
    TempHeapAlloc> staticIfStateStack;
  csArray<uint, csArrayElementHandler<uint>, 
    TempHeapAlloc> staticIfNest;

  NodeProcessingState() : templActive (false), templWeak (false), 
    generateActive (false), generateValid (false) {}
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
    TempString<> condStr;
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

template<typename ConditionEval>
void csWrappedDocumentNode::ProcessInclude (ConditionEval& eval, 
                                            const TempString<>& filename,
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
    if (!docsys.IsValid())
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
	ProcessSingleWrappedNode (eval, state, child);
      }
    }
  }
}

template<typename ConditionEval>
bool csWrappedDocumentNode::ProcessTemplate (ConditionEval& eval, 
                                             iDocumentNode* templNode, 
					     NodeProcessingState* state)
{
  if (!(state->templActive || state->generateActive))
    return false;

  Template::Nodes& templNodes = state->templ.nodes;
  csRef<iDocumentNode> node = templNode;
  if (node->GetType() == CS_NODE_UNKNOWN)
  {
    TempString<> tokenStr, args; 
    if (SplitNodeValue (node->GetValue(), tokenStr, args))
    {
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
          if (!state->generateActive)
          {
	      state->templNestLevel--;
	      if (state->templNestLevel != 0)
	        templNodes.Push (node);
          }
          else
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
	case csWrappedDocumentNodeFactory::PITOKEN_TEMPLATEWEAK:
          if (!state->generateActive)
	      state->templNestLevel++;
	    // Fall through
	default:
	    {
	      Template* templ;
	      if ((!state->generateActive)
                && (state->templNestLevel == 1)
		&& (templ = globalState->templates.GetElementPointer (tokenStr)))
	      {
		Template::Params templArgs;
		ParseTemplateArguments (args, templArgs, false);
		InvokeTemplate (templ, templArgs, templNodes);
	      }
	      else
                // Avoid recursion when the template is later invoked
		if (tokenStr != state->templateName) templNodes.Push (node);
	    }
	    break;
	case csWrappedDocumentNodeFactory::PITOKEN_GENERATE:
          if (state->generateActive)
	      state->generateNestLevel++;
	    templNodes.Push (node);
          break;
        case csWrappedDocumentNodeFactory::PITOKEN_ENDGENERATE:
          {
            if (state->generateActive)
            {
	        state->generateNestLevel--;
	        if (state->generateNestLevel != 0)
	          templNodes.Push (node);
            }
            else
              templNodes.Push (node);
          }
          break;
      }
    }
  }
  else
    templNodes.Push (node);

  if (state->generateActive)
  {
    if (state->generateNestLevel == 0)
    {
      state->generateActive = false;
      int start = state->generateStart;
      int end = state->generateEnd;
      int step = state->generateStep;

      Template generateTempl (state->templ);
      generateTempl.paramMap.Push (state->generateVar);
      if (state->generateValid)
      {
        int v = start;
        while (true)
        {
          if (state->generateStep >= 0)
          {
            if (v > end) break;
          }
          else
          {
            if (v < end) break;
          }

          Template::Nodes templatedNodes;
          Template::Params params;
          TempString<> s; s << v;
          params.Push (s);
          InvokeTemplate (&generateTempl, params, templatedNodes);
          size_t i;
          for (i = 0; i < templatedNodes.Length(); i++)
          {
            ProcessSingleWrappedNode (eval, state, templatedNodes[i]);
          }

          v += step;
        }
      }
    }
  }
  else
  {
    if (state->templNestLevel == 0)
    {
      if (!state->templWeak || !globalState->templates.Contains (state->templateName))
        globalState->templates.PutUnique (state->templateName, state->templ);
      state->templActive = false;
    }
  }
  return true;
}

bool csWrappedDocumentNode::InvokeTemplate (Template* templ,
					    const Template::Params& params,
					    Template::Nodes& templatedNodes)
{
  if (!templ) return false;

  csRef<Substitutions> newSubst;
  {
    Substitutions paramSubst;
    for (size_t i = 0; i < csMin (params.Length(), templ->paramMap.Length()); i++)
    {
      paramSubst.Put (templ->paramMap[i], params[i]);
    }
    newSubst.AttachNew (new Substitutions (paramSubst));
  }

  for (size_t i = 0; i < templ->nodes.Length(); i++)
  {
    csRef<iDocumentNode> newNode = 
      shared->replacerFactory.CreateWrapper (templ->nodes.Get (i), 0,
      newSubst);
    templatedNodes.Push (newNode);
  }
  return true;
}

template<typename ConditionEval>
bool csWrappedDocumentNode::InvokeTemplate (ConditionEval& eval, 
                                            const char* name, 
                                            iDocumentNode* node,
					    NodeProcessingState* state, 
					    const Template::Params& params)
{
  Template* templNodes = 
    globalState->templates.GetElementPointer (name);

  Template::Nodes nodes;

  if (!InvokeTemplate (templNodes, params, nodes))
    return false;

  size_t i;
  for (i = 0; i < nodes.Length(); i++)
  {
    ProcessSingleWrappedNode (eval, state, nodes[i]);
  }
  return true;
}

void csWrappedDocumentNode::ValidateTemplateEnd (iDocumentNode* node, 
						 NodeProcessingState* state)
{
  if ((state->templActive) && (state->templNestLevel != 0))
  {
    Report (syntaxErrorSeverity, node,
      "'Template' without 'Endtemplate'");
  }
}

void csWrappedDocumentNode::ValidateGenerateEnd (iDocumentNode* node, 
						 NodeProcessingState* state)
{
  if ((state->generateActive) && (state->generateNestLevel != 0))
  {
    Report (syntaxErrorSeverity, node,
      "'Generate' without 'Endgenerate'");
  }
}

void csWrappedDocumentNode::ValidateStaticIfEnd (iDocumentNode* node, 
						 NodeProcessingState* state)
{
  if (!state->staticIfStateStack.IsEmpty())
  {
    Report (syntaxErrorSeverity, node,
      "'SIfDef' without 'SEndIf'");
  }
}

void csWrappedDocumentNode::ParseTemplateArguments (const char* str, 
						    Template::Params& strings,
                                                    bool omitEmpty)
{
  if (!str) return;

  TempString<> currentStr;
  while (*str != 0)
  {
    GetNextArg (str, currentStr);
    if (!omitEmpty || !currentStr.IsEmpty())
      strings.Push (currentStr);
  }
}

bool csWrappedDocumentNode::ProcessStaticIf (NodeProcessingState* state, 
                                             iDocumentNode* node)
{
  if (state->staticIfStateStack.IsEmpty()) return false;

  if (node->GetType() == CS_NODE_UNKNOWN)
  {
    TempString<> tokenStr, args; 
    if (SplitNodeValue (node->GetValue(), tokenStr, args))
    {
      csStringID tokenID = shared->pitokens.Request (tokenStr);
      switch (tokenID)
      {
        case csWrappedDocumentNodeFactory::PITOKEN_STATIC_IFDEF:
        case csWrappedDocumentNodeFactory::PITOKEN_STATIC_IFNDEF:
          ProcessStaticIfDef (state, node, args,
            tokenID == csWrappedDocumentNodeFactory::PITOKEN_STATIC_IFNDEF);
          return true;
        case csWrappedDocumentNodeFactory::PITOKEN_STATIC_ELSE:
          {
            NodeProcessingState::StaticIfState ifState = 
              state->staticIfStateStack.Pop();
            if (ifState.elseBranch)
            {
              Report (syntaxErrorSeverity, node,
                "Multiple 'SElse's in 'SIfDef'");
              ifState.processNodes = false;
            }
            else
            {
              bool lastState = 
                state->staticIfStateStack.IsEmpty() ? true : 
                state->staticIfStateStack.Top().processNodes;
              ifState.processNodes = !ifState.processNodes && lastState;
              ifState.elseBranch = true;
            }
            state->staticIfStateStack.Push (ifState);
            return true;
          }
        case csWrappedDocumentNodeFactory::PITOKEN_STATIC_ELSIFDEF:
        case csWrappedDocumentNodeFactory::PITOKEN_STATIC_ELSIFNDEF:
          {
            NodeProcessingState::StaticIfState ifState = 
              state->staticIfStateStack.Pop();
            if (ifState.elseBranch)
            {
              Report (syntaxErrorSeverity, node,
                "Multiple 'SElse's in 'SIfDef'");
              ifState.processNodes = false;
            }
            else
            {
              bool lastState = 
                state->staticIfStateStack.IsEmpty() ? true : 
                state->staticIfStateStack.Top().processNodes;
              ifState.processNodes = !ifState.processNodes && lastState;
              ifState.elseBranch = true;
            }
            ProcessStaticIfDef (state, node, args,
              tokenID == csWrappedDocumentNodeFactory::PITOKEN_STATIC_ELSIFNDEF);
            state->staticIfNest.Pop();
            state->staticIfNest.Top()++;
          }
          return true;
        case csWrappedDocumentNodeFactory::PITOKEN_STATIC_ENDIF:
          {
            uint nest = state->staticIfNest.Pop ();
            while (nest-- > 0) state->staticIfStateStack.Pop();
            return true;
          }
      }
    }
  }
  return !state->staticIfStateStack.Top().processNodes;
}

bool csWrappedDocumentNode::ProcessInstrTemplate (NodeProcessingState* state,
                                                  iDocumentNode* node, 
                                                  const TempString<>& args, bool weak)
{
  TempString<> templateName (args);
  Template newTempl;
  size_t templateEnd = templateName.FindFirst (' ');
  if (templateEnd != (size_t)-1)
  {
    // Parse template parameter names
    Template::Params paramNames;
    ParseTemplateArguments (templateName.GetData() + templateEnd + 1, 
      paramNames, true);

    csSet<TempString<>, TempHeapAlloc> dupeCheck;
    for (size_t i = 0; i < paramNames.Length(); i++)
    {
      if (dupeCheck.Contains (paramNames[i]))
      {
        Report (syntaxErrorSeverity, node,
                "Duplicate template parameter '%s'", 
	        paramNames[i].GetData());
        return false;
      }
      newTempl.paramMap.Push (paramNames[i]);
      dupeCheck.Add (paramNames[i]);
    }

    templateName.Truncate (templateEnd);
  }
  if (templateName.IsEmpty())
  {
    Report (syntaxErrorSeverity, node,
            "'template' without name");
    return false;
  }
  if (shared->pitokens.Request (templateName) != csInvalidStringID)
  {
    Report (syntaxErrorSeverity, node,
            "Reserved template name '%s'", templateName.GetData());
    return false;
  }
  state->templateName = templateName;
  state->templ = newTempl;
  state->templActive = true;
  state->templWeak = weak;
  state->templNestLevel = 1;
  return true;
}

bool csWrappedDocumentNode::ProcessDefine (NodeProcessingState* state, 
                                           iDocumentNode* node, 
                                           const TempString<>& args)
{
  TempString<> param;
  const char* p = args;
  GetNextArg (p, param);
  if (p) { while (*p && isspace (*p)) p++; }
  if (param.IsEmpty() || (*p != 0))
  {
    Report (syntaxErrorSeverity, node,
            "One parameter expected for 'Define'");
    return false;
  }
  globalState->defines.Add (param);
  return true;
}

bool csWrappedDocumentNode::ProcessUndef (NodeProcessingState* state, 
                                          iDocumentNode* node, 
                                          const TempString<>& args)
{
  TempString<> param;
  const char* p = args;
  GetNextArg (p, param);
  if (p) { while (*p && isspace (*p)) p++; }
  if (param.IsEmpty() || (*p != 0))
  {
    Report (syntaxErrorSeverity, node,
            "One parameter expected for 'Undef'");
    return false;
  }
  globalState->defines.Delete (param);
  return true;
}

bool csWrappedDocumentNode::ProcessStaticIfDef (NodeProcessingState* state, 
                                                iDocumentNode* node, 
                                                const TempString<>& args, bool invert)
{
  TempString<> param;
  const char* p = args;
  GetNextArg (p, param);
  if (p) { while (*p && isspace (*p)) p++; }
  if (param.IsEmpty() || (*p != 0))
  {
    Report (syntaxErrorSeverity, node,
            "One parameter expected for 'SIfDef'");
    return false;
  }
  bool lastState = 
    state->staticIfStateStack.IsEmpty() ? true : 
    state->staticIfStateStack.Top().processNodes;
  bool defineExists = globalState->defines.Contains (param);
  state->staticIfStateStack.Push (
    (invert ? !defineExists : defineExists) && lastState);
  state->staticIfNest.Push (1);
  return true;
}

template<typename ConditionEval>
void csWrappedDocumentNode::ProcessSingleWrappedNode (
  ConditionEval& eval, NodeProcessingState* state, iDocumentNode* node)
{
  CS_ASSERT(globalState);

  if (ProcessTemplate (eval, node, state)) return;
  if (ProcessStaticIf (state, node))
  {
    // Template invokation has precedence over dropping nodes...
    TempString<> tokenStr, args; 
    if (SplitNodeValue (node->GetValue(), tokenStr, args))
    {
      Template::Params params;
      if (!args.IsEmpty())
      {
        ParseTemplateArguments (args, params, false);
      }
      InvokeTemplate (eval, tokenStr, node, state, params);
    }
    return;
  }

  WrapperStack& wrapperStack = state->wrapperStack;
  WrapperStackEntry& currentWrapper = state->currentWrapper;
  bool handled = false;
  if (node->GetType() == CS_NODE_UNKNOWN)
  {
    TempString<> replaceScratch;
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

	TempString<> tokenStr; tokenStr.Replace (valStart, cmdLen);
	csStringID tokenID = shared->pitokens.Request (tokenStr);
	switch (tokenID)
	{
	  case csWrappedDocumentNodeFactory::PITOKEN_IF:
	    {
	      WrapperStackEntry newWrapper;
	      ParseCondition (newWrapper, space + 1, valLen - cmdLen - 1, 
		node);

              const csConditionID condition = newWrapper.child->condition;
              Logic3 r = eval.Descend (condition);
              // If the parent condition is always false, so is this
              if ((((currentWrapper.child->condition == csCondAlwaysFalse)
                && (currentWrapper.child->conditionValue == true))
                  || ((currentWrapper.child->condition == csCondAlwaysTrue)
                && (currentWrapper.child->conditionValue == false))))
              {
                r.state = Logic3::Lie;
              }
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
                eval.Ascend (ascendNum);
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
              if (eval.GetBranch() != 0)
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

                eval.SwitchBranch ();
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
              if (okay && (eval.GetBranch() != 0))
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
          
                eval.SwitchBranch ();
                const csConditionID condition = newWrapper.child->condition;
                Logic3 r = eval.Descend (condition);
                // If the parent condition is always false, so is this
                if ((((currentWrapper.child->condition == csCondAlwaysFalse)
                  && (currentWrapper.child->conditionValue == true))
                    || ((currentWrapper.child->condition == csCondAlwaysTrue)
                  && (currentWrapper.child->conditionValue == false))))
                {
                  r.state = Logic3::Lie;
                }
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
	      TempString<> filename;
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
		ProcessInclude (eval, filename, state, node);
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
	  case csWrappedDocumentNodeFactory::PITOKEN_TEMPLATEWEAK:
            {
              TempString<> args (valStart + cmdLen, valLen - cmdLen);
              args.LTrim();
              ProcessInstrTemplate (state, node, args, 
                tokenID == csWrappedDocumentNodeFactory::PITOKEN_TEMPLATEWEAK);
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
              // ProcessTemplate() would've handled it otherwise
	    }
	    break;
          case csWrappedDocumentNodeFactory::PITOKEN_GENERATE:
            {
	      bool okay = true;
              Template::Params args;
	      if (space != 0)
	      {
		TempString<> pStr (space + 1, valLen - cmdLen - 1);
		ParseTemplateArguments (pStr, args, false);
	      }
              if ((args.GetSize() < 3) || (args.GetSize() > 4))
              {
                okay = false;
	        Report (syntaxErrorSeverity, node,
		  "'Generate' expects 3 or 4 arguments, got %zu", args.GetSize());
              }
              if (okay)
              {
                state->generateVar = args[0];

                int start, end, step;
                char dummy;
                if (sscanf (args[1], "%d%c", &start, &dummy) != 1)
                {
	          Report (syntaxErrorSeverity, node,
		    "Argument '%s' is not an integer", args[1].GetData());
                  okay = false;
                }
                if (okay && sscanf (args[2], "%d%c", &end, &dummy) != 1)
                {
	          Report (syntaxErrorSeverity, node,
		    "Argument '%s' is not an integer", args[2].GetData());
                  okay = false;
                }
                if (okay)
                {
                  if (args.GetSize() == 4)
                  {
                    if (sscanf (args[3], "%d%c", &step, &dummy) != 1)
                    {
	              Report (syntaxErrorSeverity, node,
		        "Argument '%s' is not an integer", args[3].GetData());
                      okay = false;
                    }
                  }
                  else
                  {
                    step = (end < start) ? -1 : 1;
                  }
                }
                if (okay)
                {
                  state->generateActive = true;
		  state->generateNestLevel = 1;
                  if (((start > end) && (step >= 0))
                    || (end >= start) && (step <= 0))
                  {
	            Report (syntaxErrorSeverity, node,
		      "Can't reach end value %d starting from %d with step %d", 
                      end, start, step);
                    state->generateValid = false;
                  }
                  else
                  {
                    state->generateValid = true;
                    state->generateStart = start;
                    state->generateEnd = end;
                    state->generateStep = step;
                  }
                }
              }
            }
            break;
	  case csWrappedDocumentNodeFactory::PITOKEN_ENDGENERATE:
	    {
	      Report (syntaxErrorSeverity, node,
		"'Endgenerate' without 'Generate'");
              // ProcessTemplate() would've handled it otherwise
	    }
	    break;
	  case csWrappedDocumentNodeFactory::PITOKEN_DEFINE:
            {
              TempString<> args (valStart + cmdLen, valLen - cmdLen);
              args.LTrim();
              ProcessDefine (state, node, args);
            }
	    break;
	  case csWrappedDocumentNodeFactory::PITOKEN_UNDEF:
            {
              TempString<> args (valStart + cmdLen, valLen - cmdLen);
              args.LTrim();
              ProcessUndef (state, node, args);
            }
	    break;
	  case csWrappedDocumentNodeFactory::PITOKEN_STATIC_IFDEF:
	  case csWrappedDocumentNodeFactory::PITOKEN_STATIC_IFNDEF:
            {
              TempString<> args (valStart + cmdLen, valLen - cmdLen);
              args.LTrim();
              ProcessStaticIfDef (state, node, args,
                tokenID == csWrappedDocumentNodeFactory::PITOKEN_STATIC_IFNDEF);
            }
            break;
          case csWrappedDocumentNodeFactory::PITOKEN_STATIC_ELSIFDEF:
          case csWrappedDocumentNodeFactory::PITOKEN_STATIC_ELSIFNDEF:
          case csWrappedDocumentNodeFactory::PITOKEN_STATIC_ELSE:
          case csWrappedDocumentNodeFactory::PITOKEN_STATIC_ENDIF:
            {
	      Report (syntaxErrorSeverity, node,
		"'%s' without 'SIfDef'", shared->pitokens.Request (tokenID));
              // ProcessStaticIf() would've handled it otherwise
            }
            break;
	  default:
	    {
	      Template::Params params;
	      if (space != 0)
	      {
		TempString<> pStr (space + 1, valLen - cmdLen - 1);
		ParseTemplateArguments (pStr, params, false);
	      }
	      if (!InvokeTemplate (eval, tokenStr, node, state, params))
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
    newWrapper->childNode.AttachNew (new csWrappedDocumentNode (eval, node,
      resolver, shared, globalState));
    currentWrapper.child->childrenWrappers.Push (newWrapper);
  }
}

template<typename ConditionEval>
void csWrappedDocumentNode::ProcessWrappedNode (ConditionEval& eval, 
                                                NodeProcessingState* state, 
						iDocumentNode* wrappedNode)
{
  if ((wrappedNode->GetType() == CS_NODE_ELEMENT)
    || (wrappedNode->GetType() == CS_NODE_DOCUMENT))
  {
    state->iter = wrappedNode->GetNodes ();
    while (state->iter->HasNext ())
    {
      csRef<iDocumentNode> node = state->iter->Next();
      ProcessSingleWrappedNode (eval, state, node);
    }
    ValidateTemplateEnd (wrappedNode, state);
    ValidateGenerateEnd (wrappedNode, state);
    ValidateStaticIfEnd (wrappedNode, state);
  }
}

template<typename T>
void csWrappedDocumentNode::ProcessWrappedNode (T& eval)
{
  NodeProcessingState state;
  state.currentWrapper.child = new WrappedChild;
  wrappedChildren.Push (state.currentWrapper.child);
  ProcessWrappedNode (eval, &state, wrappedNode);
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

// Hack: Work around problems caused by #defining 'new'.
#if defined(CS_EXTENSIVE_MEMDEBUG) || defined(CS_MEMORY_TRACKER)
# undef new
#endif
#include <new>

csRef<iDocumentNodeIterator> csWrappedDocumentNode::GetNodes ()
{
  csWrappedDocumentNodeIterator* iter = 
    new (shared->iterPool) csWrappedDocumentNodeIterator (this, 0);
  return csPtr<iDocumentNodeIterator> (iter);
}

csRef<iDocumentNodeIterator> csWrappedDocumentNode::GetNodes (
  const char* value)
{
  csWrappedDocumentNodeIterator* iter = 
    new (shared->iterPool) csWrappedDocumentNodeIterator (this, value);
  return csPtr<iDocumentNodeIterator> (iter);
}

#if defined(CS_EXTENSIVE_MEMDEBUG) || defined(CS_MEMORY_TRACKER)
# define new CS_EXTENSIVE_MEMDEBUG_NEW
#endif

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

csTextNodeWrapper::csTextNodeWrapper (iDocumentNode* realMe, 
                                      const char* text) : 
  scfPooledImplementationType (this), realMe (realMe)
{  
  nodeText = csStrNew (text);
}

csTextNodeWrapper::~csTextNodeWrapper ()
{
  delete[] nodeText;
}

//---------------------------------------------------------------------------

CS_LEAKGUARD_IMPLEMENT(csWrappedDocumentNodeIterator);

csWrappedDocumentNodeIterator::csWrappedDocumentNodeIterator (
  csWrappedDocumentNode* node, const char* filter) : 
  scfPooledImplementationType (this), filter (filter), parentNode (node)
{
  walker.SetData (parentNode->wrappedChildren, parentNode->resolver);

  SeekNext();
}

csWrappedDocumentNodeIterator::~csWrappedDocumentNodeIterator ()
{
}

// Hack: Work around problems caused by #defining 'new'.
#if defined(CS_EXTENSIVE_MEMDEBUG) || defined(CS_MEMORY_TRACKER)
# undef new
#endif
#include <new>

void csWrappedDocumentNodeIterator::SeekNext()
{
  next = 0;
  csRef<iDocumentNode> node;
  while (walker.HasNext ())
  {
    node = walker.Next ();
    if ((filter.GetData() == 0) || (strcmp (node->GetValue (), filter) == 0))
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
    csTextNodeWrapper* textNode = 
      new (parentNode->shared->textWrapperPool) csTextNodeWrapper (next, str);
    next.AttachNew (textNode);
  }
}

#if defined(CS_EXTENSIVE_MEMDEBUG) || defined(CS_MEMORY_TRACKER)
# define new CS_EXTENSIVE_MEMDEBUG_NEW
#endif

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
  pitokens.Register ("TemplateWeak", PITOKEN_TEMPLATEWEAK);
  pitokens.Register ("Endtemplate", PITOKEN_ENDTEMPLATE_NEW);
  pitokens.Register ("Include", PITOKEN_INCLUDE_NEW);
  pitokens.Register ("Generate", PITOKEN_GENERATE);
  pitokens.Register ("Endgenerate", PITOKEN_ENDGENERATE);
  pitokens.Register ("Define", PITOKEN_DEFINE);
  pitokens.Register ("Undef", PITOKEN_UNDEF);
  pitokens.Register ("SIfDef", PITOKEN_STATIC_IFDEF);
  pitokens.Register ("SIfNDef", PITOKEN_STATIC_IFNDEF);
  pitokens.Register ("SElsIfDef", PITOKEN_STATIC_ELSIFDEF);
  pitokens.Register ("SElsIfNDef", PITOKEN_STATIC_ELSIFNDEF);
  pitokens.Register ("SElse", PITOKEN_STATIC_ELSE);
  pitokens.Register ("SEndIf", PITOKEN_STATIC_ENDIF);
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

struct EvalCondTree
{
  ConditionTree condTree;
  EvalCondTree (csConditionEvaluator& evaluator) : condTree (evaluator) {}

  Logic3 Descend (csConditionID condition)
  { return condTree.Descend (condition); }
  void SwitchBranch () { condTree.SwitchBranch (); }
  void Ascend (int num) { condTree.Ascend (num); }
  int GetBranch() const { return condTree.GetBranch(); }
};

csWrappedDocumentNode* csWrappedDocumentNodeFactory::CreateWrapper (
  iDocumentNode* wrappedNode, iConditionResolver* resolver,
  csConditionEvaluator& evaluator, const csRefArray<iDocumentNode>& extraNodes,
  csString* dumpOut)
{
  currentOut = dumpOut;

  csWrappedDocumentNode* node;
  {
    EvalCondTree eval (evaluator);
    for (size_t i = 0; i < extraNodes.GetSize(); i++)
    {
      csRef<csWrappedDocumentNode::GlobalProcessingState> globalState;
      globalState.AttachNew (csWrappedDocumentNode::GlobalProcessingState::Create ());
      delete new csWrappedDocumentNode (eval, extraNodes[i], resolver, this, 
        globalState);
    }
    csRef<csWrappedDocumentNode::GlobalProcessingState> globalState;
    globalState.AttachNew (csWrappedDocumentNode::GlobalProcessingState::Create ());
    node = new csWrappedDocumentNode (eval, wrappedNode, resolver, this, 
      globalState);
    eval.condTree.ToResolver (resolver);
    CS_ASSERT(globalState->GetRefCount() == 1);
  }
  evaluator.CompactMemory();
  TempHeap::Trim();
  return node;
}

struct EvalStatic
{
  iConditionResolver* resolver;
  int currentBranch;
  csArray<int, csArrayElementHandler<int>, TempHeapAlloc> branchStack;

  EvalStatic (iConditionResolver* resolver) : resolver (resolver),
    currentBranch(0) {}

  Logic3 Descend (csConditionID condition)
  { 
    branchStack.Push (currentBranch);
    return resolver->Evaluate (condition); 
  }
  void SwitchBranch ()
  {
    CS_ASSERT(currentBranch == 0);
    currentBranch = 1;
  }
  void Ascend (int num) 
  { 
    while (num-- > 0)
      branchStack.Pop ();
  }
  int GetBranch() const { return currentBranch; }
};

csWrappedDocumentNode* csWrappedDocumentNodeFactory::CreateWrapperStatic (
  iDocumentNode* wrappedNode, iConditionResolver* resolver, csString* dumpOut)
{
  currentOut = dumpOut;

  csWrappedDocumentNode* node;
  {
    csRef<csWrappedDocumentNode::GlobalProcessingState> globalState;
    globalState.AttachNew (csWrappedDocumentNode::GlobalProcessingState::Create ());
    EvalStatic eval (resolver);
    node = new csWrappedDocumentNode (eval, wrappedNode, resolver, this, 
      globalState);
    CS_ASSERT(globalState->GetRefCount() == 1);
  }
  return node;
}

}
CS_PLUGIN_NAMESPACE_END(XMLShader)
