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
#include <ctype.h>

#include "csgeom/math.h"
#include "csutil/bitarray.h"
#include "csutil/csendian.h"
#include "csutil/documenthelper.h"
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
#include "../foreignnodes.h"
#include "../xmlshader.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{

#include "csutil/custom_new_disable.h"

class ConditionTree
{
  /// Indices for node branches
  enum
  {
    /// Index for 'condition is true' branch
    bTrue = 0,
    /// Index for 'condition is false' branch
    bFalse = 1
  };

  struct Node
  {
    Node* parent;
    
    csConditionID condition;
    Node* branches[2];
    Variables values;
    MyBitArrayTemp conditionAffectedSVs;
    MyBitArrayTemp conditionResults[2];
    MyBitArrayTemp conditionResultsSet;

    Node (Node* p, int parentBranch) : parent (p), condition (csCondUnknown)
    {
      if (p != 0)
      {
	conditionResults[bTrue] = conditionResults[bFalse] =
	  p->conditionResults[parentBranch];
	conditionResultsSet = p->conditionResultsSet;
      }
      branches[bTrue] = 0;
      branches[bFalse] = 0;
    }
    
    void SetConditionResult (int _branches, csConditionID cond, bool val)
    {
      for (int b = 0; b < 2; b++)
      {
	if (!(_branches & (1 << b))) continue;
	if (conditionResults[b].GetSize() <= cond)
	  conditionResults[b].SetSize (cond+1);
	if (val)
	  conditionResults[b].SetBit (cond);
	
	if (branches[b]) branches[b]->SetConditionResult (0x3, cond, val);
      }
      if (conditionResultsSet.GetSize() <= cond)
	conditionResultsSet.SetSize (cond+1);
      conditionResultsSet.SetBit (cond);
    }

    void MergeConditionResults (const Node* other, int otherBranch)
    {
      for (size_t c = 0; c < other->conditionResultsSet.GetSize(); c++)
      {
	if (!other->conditionResultsSet[c]) continue;
	if ((c >= conditionResultsSet.GetSize())
	    || !conditionResultsSet.IsBitSet (c))
	{
	  SetConditionResult (0x3, c,
	    other->conditionResults[otherBranch].IsBitSet (c));
	}
      }
    }
  };

  csFixedSizeAllocator<sizeof (Node), TempHeapAlloc> nodeAlloc;
  Node* root;
  int currentBranch;
  
  typedef csArray<Node*, csArrayElementHandler<Node*>, TempHeapAlloc,
    CS::Container::ArrayCapacityExponential<> > NodeArray;
  struct NodeStackEntry
  {
    NodeArray branches[2];
  };
  csBlockAllocator<NodeStackEntry, TempHeapAlloc,
    csBlockAllocatorDisposeDelete<NodeStackEntry> > nodeStackEntryAlloc;
    
  csArray<NodeStackEntry*, csArrayElementHandler<NodeStackEntry*>,
    TempHeapAlloc> nodeStack;
  csArray<int, csArrayElementHandler<int>, TempHeapAlloc> branchStack;
  
  csConditionID cheapshotCondition;
  Variables cheapshotTrueVals;
  Variables cheapshotFalseVals;
  MyBitArrayTemp affectedSVs;
  
  struct CommitNode
  {
    Node* owner;
    int branch;
    Node* newNode;

    csConditionID oldCondition;
    MyBitArrayTemp oldConditionAffectedSVs;
    MyBitArrayTemp oldConditionResults[2];
    MyBitArrayTemp oldConditionResultsSet;
  };
  typedef csArray<CommitNode> CommitArray;
  csArray<CommitArray> commitArrays;

  void RecursiveAdd (csConditionID condition, Node* node, 
    NodeStackEntry& newCurrent, MyBitArrayTemp& affectedSVs,
    CommitArray& commitNodes);
  void ToResolver (iConditionResolver* resolver, Node* node,
    csConditionNode* parent);

  bool HasContainingCondition (Node* node, csConditionID containedCondition,
    csConditionID& condition, int& branch);

  void ClearCommitArray (CommitArray& ca);
  void RecursiveFree (Node* node);
  
  csConditionEvaluator& evaluator;
  void DumpNode (csString& out, const Node* node, int level);
public:
  ConditionTree (csConditionEvaluator& evaluator) : nodeAlloc (256), evaluator (evaluator)
  {
    root = (Node*)nodeAlloc.Alloc();
    new (root) Node (0, 0);
    currentBranch = bTrue;
    NodeStackEntry* newPair = nodeStackEntryAlloc.Alloc();
    newPair->branches[bTrue].Push (root);
    nodeStack.Push (newPair);
    cheapshotCondition = csCondUnknown;
  }
  ~ConditionTree ()
  {
    RecursiveFree (root);
  }
  
  void PresetConditions (const MyBitArrayTemp& presetCondResults);

  Logic3 Descend (csConditionID condition);
  // Switch the current branch from "true" to "false"
  void SwitchBranch ();
  void Ascend (int num);
  int GetBranch() const { return currentBranch; }
  void Commit ();

  void ToResolver (iConditionResolver* resolver);

  void Dump (csString& out);
};

void ConditionTree::RecursiveAdd (csConditionID condition, Node* node, 
                                  NodeStackEntry& newCurrent, 
                                  MyBitArrayTemp& affectedSVs,
                                  CommitArray& commitNodes)
{
  /* Shortcut */
  if (node->condition == condition)
  {
    newCurrent.branches[bTrue].Push (node->branches[bTrue]);
    newCurrent.branches[bFalse].Push (node->branches[bFalse]);
    return;
  }

  Logic3 r;
  bool isLeaf = node->condition == csCondUnknown;
  bool doCheck = true;
  if (node->parent != 0)
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
  if (isLeaf)
  {
    Variables trueVals;
    Variables falseVals;
    if (doCheck)
    {
      r = evaluator.CheckConditionResults (condition, 
	node->values, trueVals, falseVals);
    }
    else
    {
      if (cheapshotCondition != condition)
      {
        Variables v;
	r = evaluator.CheckConditionResults (condition, 
	  v, cheapshotTrueVals, cheapshotFalseVals);
	cheapshotCondition = condition;
      }
      trueVals = node->values & cheapshotTrueVals;
      falseVals = node->values & cheapshotFalseVals;
    }
  
    switch (r.state)
    {
      case Logic3::Truth:
        node->SetConditionResult (0x3, condition, true);
	newCurrent.branches[bTrue].Push (node);
	break;
      case Logic3::Lie:
        node->SetConditionResult (0x3, condition, false);
	newCurrent.branches[bFalse].Push (node);
	break;
      case Logic3::Uncertain:
	{
	  csConditionID containerCondition;
	  int containingBranch;
	  bool hasContainer = HasContainingCondition (node, condition,
	    containerCondition, containingBranch);

	  CommitNode commitNode;
  
	  commitNode.oldCondition = node->condition;
	  commitNode.oldConditionAffectedSVs = node->conditionAffectedSVs;
	  commitNode.oldConditionResults[0] = node->conditionResults[0];
	  commitNode.oldConditionResults[1] = node->conditionResults[1];
	  commitNode.oldConditionResultsSet = node->conditionResultsSet;
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
	      affectedSVs.SetSize (newSize);
	    }
	    nodeAffectedSVs |= parentAffectedSVs;
	  }
	  for (int b = 0; b < 2; b++)
	  {
	    Node* nn = (Node*)nodeAlloc.Alloc();
	    new (nn) Node (node, b);
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
	      if (b == bTrue)
	      {
		evaluator.CheckConditionResults (containerCondition, 
		  trueVals, newTrueVals, newFalseVals);
	      }
	      else
	      {
		evaluator.CheckConditionResults (containerCondition, 
		  falseVals, newTrueVals, newFalseVals);
	      }
	      /* Pick the results for the branch of the containing condition
	      * the contained one appears in. */
	      if (containingBranch == bTrue)
		nn->values = newTrueVals;
	      else
		nn->values = newFalseVals;
	    }
	    else
	    {
	      nn->values = (b == bTrue) ? trueVals : falseVals;
	    }
	    commitNode.owner = node;
	    commitNode.branch = b;
	    commitNode.newNode = nn;
	    commitNodes.Push (commitNode);
	    newCurrent.branches[b].Push (nn);
	  }
	} 
	break;
    }
  }
  else
  {
    if (doCheck)
    {
      r = evaluator.CheckConditionResults (condition, 
	node->values);
    }
  
    switch (r.state)
    {
      case Logic3::Truth:
        node->SetConditionResult (0x3, condition, true);
	newCurrent.branches[bTrue].Push (node);
	break;
      case Logic3::Lie:
        node->SetConditionResult (0x3, condition, false);
	newCurrent.branches[bFalse].Push (node);
	break;
      case Logic3::Uncertain:
	RecursiveAdd (condition, node->branches[0], newCurrent, affectedSVs,
	  commitNodes);
	RecursiveAdd (condition, node->branches[1], newCurrent, affectedSVs,
	  commitNodes);
	break;
    }
  }
}

void ConditionTree::PresetConditions (const MyBitArrayTemp& presetCondResults)
{
  CS_ASSERT(root->condition == csCondUnknown);
  
  size_t numCond = presetCondResults.GetSize()/2;
  for (size_t c = 0; c < numCond; c++)
  {
    int condBits = (presetCondResults[2*c] ? 2 : 0) | (presetCondResults[2*c+1] ? 1 : 0);
    if ((condBits == 2) || (condBits == 1))
    {
      Variables trueVals;
      Variables falseVals;
      
      evaluator.CheckConditionResults (c, root->values, trueVals, falseVals);
      root->values = (condBits == 2) ? trueVals : falseVals;
    }
  }
}

Logic3 ConditionTree::Descend (csConditionID condition)
{
  bool conditionReserved = (condition == csCondAlwaysTrue)
    || (condition == csCondAlwaysFalse);
  
  const NodeStackEntry& current = *(nodeStack[nodeStack.GetSize()-1]);
  NodeStackEntry* newCurrent = nodeStackEntryAlloc.Alloc();
  
  CommitArray ca;
  const NodeArray& currentNodes = current.branches[currentBranch];
  if (conditionReserved)
  {
    switch (condition)
    {
      case csCondAlwaysTrue:
        newCurrent->branches[bTrue] = currentNodes;
	break;
      case csCondAlwaysFalse:
        newCurrent->branches[bFalse] = currentNodes;
	break;
    }
  }
  else
  {
    affectedSVs.Clear();
    evaluator.GetUsedSVs (condition, affectedSVs);
    for (size_t i = 0; i < currentNodes.GetSize(); i++)
    {
      RecursiveAdd (condition, currentNodes[i], *newCurrent, affectedSVs, ca);
    }
  }
  commitArrays.Push (ca);
    
  nodeStack.Push (newCurrent);
  branchStack.Push (currentBranch);
  currentBranch = bTrue;

  Logic3 r;
  if (newCurrent->branches[bTrue].IsEmpty()
    && !newCurrent->branches[bFalse].IsEmpty())
    r.state = Logic3::Lie;
  else if (!newCurrent->branches[bTrue].IsEmpty()
    && newCurrent->branches[bFalse].IsEmpty())
    r.state = Logic3::Truth;

  return r;
}

void ConditionTree::SwitchBranch ()
{
  CS_ASSERT(currentBranch == bTrue);
  currentBranch = bFalse;
}

void ConditionTree::Ascend (int num)
{
  CS_ASSERT_MSG("Either too many Ascend()s or too few Descend()s", 
    nodeStack.GetSize() > 1);
  while (num-- > 0)
  {
    NodeStackEntry* oldNodes = nodeStack.Pop();
    nodeStackEntryAlloc.Free (oldNodes);
    currentBranch = branchStack.Pop();
    
    if (commitArrays.GetSize() > 0)
    {
      CommitArray ca (commitArrays.Pop ());
      for (size_t n = ca.GetSize(); n-- > 0; )
      {
	Node* node = ca[n].owner;
	node->condition = ca[n].oldCondition;
	node->conditionAffectedSVs = ca[n].oldConditionAffectedSVs;
	node->conditionResults[0] = ca[n].oldConditionResults[0];
	node->conditionResults[1] = ca[n].oldConditionResults[1];
	node->conditionResultsSet = ca[n].oldConditionResultsSet;
      }
      ClearCommitArray (ca);
    }
  }
}

void ConditionTree::Commit ()
{
  for (size_t c = 0; c < commitArrays.GetSize(); c++)
  {
    CommitArray& ca = commitArrays[c];
    for (size_t n = 0; n < ca.GetSize(); n++)
    {
      Node* node = ca[n].owner;
      node->branches[ca[n].branch] = ca[n].newNode;
      node->SetConditionResult (1 << ca[n].branch, node->condition,
	ca[n].branch == bTrue);
      ca[n].newNode->MergeConditionResults (node, ca[n].branch);
    }
  }
  commitArrays.Empty();
}

void ConditionTree::ToResolver (iConditionResolver* resolver, 
                                Node* node, csConditionNode* parent)
{
  if (node->condition == csCondUnknown) return;
  
  csConditionNode* trueNode;
  csConditionNode* falseNode;

  resolver->AddNode (parent, node->condition, trueNode, falseNode,
    node->conditionResults[bTrue], node->conditionResults[bFalse],
    node->conditionResultsSet);
  if (node->branches[bTrue] != 0)
    ToResolver (resolver, node->branches[bTrue], trueNode);
  if (node->branches[bFalse] != 0)
    ToResolver (resolver, node->branches[bFalse], falseNode);
}

void ConditionTree::ToResolver (iConditionResolver* resolver)
{
  if (root->branches[bTrue] != 0)
  {
    ToResolver (resolver, root, 0);
    resolver->FinishAdding();
  }
}

bool ConditionTree::HasContainingCondition (Node* node, 
                                            csConditionID containedCondition,
                                            csConditionID& condition,
                                            int& branch)
{
  if (node->parent == 0) return false;
  condition = node->parent->condition;
  if (evaluator.IsConditionPartOf (containedCondition, condition)
    && (containedCondition != condition))
  {
    branch = (node == node->parent->branches[bTrue]) ? bTrue : bFalse;
    return true;
  }
  return HasContainingCondition (node->parent, containedCondition, condition, 
    branch);
}
  
void ConditionTree::ClearCommitArray (CommitArray& ca)
{
  for (size_t n = 0; n < ca.GetSize(); n++)
  {
    ca[n].newNode->~Node();
    nodeAlloc.Free (ca[n].newNode);
  }
}

void ConditionTree::RecursiveFree (Node* node)
{
  if (node == 0) return;
  RecursiveFree (node->branches[bTrue]);
  RecursiveFree (node->branches[bFalse]);
  node->~Node();
  nodeAlloc.Free (node);
}

void ConditionTree::DumpNode (csString& out, const Node* node, int level)
{
  csString indent;
  for (int l = 0; l < level; l++) indent += " |";
  if (node != 0)
  {
    out += indent;
    node->values.Dump (out);
    out += '\n';
    out += indent;
    out += ' ';
    switch (node->condition)
    {
      case csCondAlwaysTrue:
	out.Append ("condition \"always true\"");
	break;
      case csCondAlwaysFalse:
	out.Append ("condition \"always false\"");
	break;
      default:
        out.AppendFmt ("condition %zu", node->condition);
    }
    out += '\n';
    DumpNode (out, node->branches[bTrue], level+1);
    DumpNode (out, node->branches[bFalse], level+1);
  }
}

void ConditionTree::Dump (csString& out)
{
  DumpNode (out, root, 0);
}

//---------------------------------------------------------------------------

typedef CS::Memory::FixedSizeAllocatorSafe<sizeof (csWrappedDocumentNode::WrappedChild)>
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

#include "csutil/custom_new_enable.h"

//---------------------------------------------------------------------------

CS_LEAKGUARD_IMPLEMENT(csWrappedDocumentNode);

template<typename ConditionEval>
csWrappedDocumentNode::csWrappedDocumentNode (ConditionEval& eval,
                                              csWrappedDocumentNode* parent,
                                              iDocumentNode* wrapped_node,
					      iConditionResolver* res,
					      csWrappedDocumentNodeFactory* shared_fact, 
					      GlobalProcessingState* global_state,
                                              uint parseOpts)
  : scfImplementationType (this), wrappedNode (wrapped_node), parent (parent),
    resolver (res), shared (shared_fact), globalState (global_state)
{
  ProcessWrappedNode (eval, parseOpts);
  globalState = 0;
}
  
csWrappedDocumentNode::csWrappedDocumentNode (csWrappedDocumentNode* parent,
                                              iDocumentNode* wrappedNode,
                                              csWrappedDocumentNodeFactory* shared)
  : scfImplementationType (this), wrappedNode (wrappedNode), parent (parent),
    shared (shared)
{
}
  
csWrappedDocumentNode::csWrappedDocumentNode (csWrappedDocumentNode* parent,
					      iConditionResolver* resolver,
                                              csWrappedDocumentNodeFactory* shared)
  : scfImplementationType (this), parent (parent), resolver (resolver),
    shared (shared)
{
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
  csRef<csWrappedDocumentNode::WrappedChild> child;

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
  newWrapper.child.AttachNew (new WrappedChild);
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
  globalState->condDumper->Dump (newWrapper.child->condition,
    cond, condLen);
}

void csWrappedDocumentNode::CreateElseWrapper (NodeProcessingState* state, 
					       WrapperStackEntry& elseWrapper)
{
  WrapperStackEntry oldCurrentWrapper = state->currentWrapper;
  state->currentWrapper = state->wrapperStack.Pop ();
  elseWrapper = oldCurrentWrapper;
  elseWrapper.child.AttachNew (new WrappedChild);
  elseWrapper.child->condition = oldCurrentWrapper.child->condition;
  elseWrapper.child->SetConditionValue (false);
  
  if (!oldCurrentWrapper.child->childNode.IsValid()
      && (oldCurrentWrapper.child->childrenWrappers.GetSize() == 0))
  {
    state->currentWrapper.child->childrenWrappers.Delete (
      oldCurrentWrapper.child);
  }
}

template<typename ConditionEval>
void csWrappedDocumentNode::ProcessInclude (ConditionEval& eval, 
                                            const TempString<>& filename,
					    NodeProcessingState* state, 
					    iDocumentNode* node,
					    uint parseOpts)
{
  iVFS* vfs = globalState->vfs;
  csRef<iDataBuffer> pathExpanded = vfs->ExpandPath (filename);
  
  csRef<iDocumentNode> includeNode;
  includeNode = globalState->includesCache.Get (pathExpanded->GetData(),
    (iDocumentNode*)0);
  if (!includeNode.IsValid())
  {
    csRef<iFile> include = vfs->Open (filename, VFS_FILE_READ);
    if (!include.IsValid ())
    {
      Report (syntaxErrorSeverity, node,
	"could not open '%s'", filename.GetData ());
    }
    else
    {
      csRef<iDocumentSystem> docsys (
	csQueryRegistry<iDocumentSystem> (shared->objreg));
      if (!docsys.IsValid())
	docsys.AttachNew (new csTinyDocumentSystem ());
  
      csRef<iDocument> includeDoc = docsys->CreateDocument ();
      const char* err = includeDoc->Parse (include, false);
      if (err != 0)
      {
	Report (syntaxErrorSeverity, node,
	  "error parsing '%s': %s", filename.GetData (), err);
	return;
      }
      else
      {
	csRef<iDocumentNode> rootNode = includeDoc->GetRoot ();
        includeNode = rootNode->GetNode ("include");
	if (!includeNode)
	{
	  Report (syntaxErrorSeverity, rootNode,
	    "%s: no <include> node", filename.GetData ());
	  return;
	}
	globalState->includesCache.Put (pathExpanded->GetData(),
          includeNode);
      }
    }
  }
  
  csVfsDirectoryChanger dirChange (vfs);
  dirChange.ChangeTo (filename);

  csRef<iDocumentNodeIterator> it = includeNode->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    ProcessSingleWrappedNode (eval, state, child, parseOpts);
  }
}

template<typename ConditionEval>
bool csWrappedDocumentNode::ProcessTemplate (ConditionEval& eval, 
                                             iDocumentNode* templNode, 
					     NodeProcessingState* state, 
					     uint parseOpts)
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
                shared->DebugProcessing ("Invoking template %s\n", 
                  tokenStr.GetData());
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
          shared->DebugProcessing ("Starting generation\n");
          InvokeTemplate (&generateTempl, params, templatedNodes);
          size_t i;
          for (i = 0; i < templatedNodes.GetSize (); i++)
          {
            ProcessSingleWrappedNode (eval, state, templatedNodes[i], parseOpts);
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
    for (size_t i = 0; i < csMin (params.GetSize (), templ->paramMap.GetSize ()); i++)
    {
      shared->DebugProcessing (" %s -> %s\n", templ->paramMap[i].GetData(), 
        params[i].GetData());
      paramSubst.Put (templ->paramMap[i], params[i]);
    }
    newSubst.AttachNew (new Substitutions (paramSubst));
  }

  for (size_t i = 0; i < templ->nodes.GetSize (); i++)
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
					    const Template::Params& params,
					    uint parseOpts)
{
  shared->DebugProcessing ("Invoking template %s\n", name);
  Template* templNodes = 
    globalState->templates.GetElementPointer (name);

  Template::Nodes nodes;

  if (!InvokeTemplate (templNodes, params, nodes))
    return false;

  size_t i;
  for (i = 0; i < nodes.GetSize (); i++)
  {
    ProcessSingleWrappedNode (eval, state, nodes[i], parseOpts);
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
    for (size_t i = 0; i < paramNames.GetSize (); i++)
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
  ConditionEval& eval, NodeProcessingState* state, iDocumentNode* node,
  uint parseOpts)
{
  CS_ASSERT(globalState);

  if (ProcessTemplate (eval, node, state, parseOpts)) return;
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
      InvokeTemplate (eval, tokenStr, node, state, params, parseOpts);
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
	    if (parseOpts & wdnfpoHandleConditions)
	    {
	      WrapperStackEntry newWrapper;
	      ParseCondition (newWrapper, space + 1, valLen - cmdLen - 1, 
		node);

              csConditionID condition = newWrapper.child->condition;
              // If the parent condition is always false, so is this
              if (currentWrapper.child->IsUnreachable())
              {
                condition = csCondAlwaysFalse;
              }
              Logic3 r = eval.Descend (condition);
              if (r.state == Logic3::Truth)
                newWrapper.child->condition = csCondAlwaysTrue;
              else if (r.state == Logic3::Lie)
                newWrapper.child->condition = csCondAlwaysFalse;
              globalState->ascendStack.Push (1);

	      if (!newWrapper.child->IsUnreachable())
		currentWrapper.child->childrenWrappers.Push (newWrapper.child);
	      wrapperStack.Push (currentWrapper);
	      currentWrapper = newWrapper;
	      handled = true;
	    }
	    break;
	  case csWrappedDocumentNodeFactory::PITOKEN_ENDIF:
	    if (parseOpts & wdnfpoHandleConditions)
	    {
	      bool okay = true;
	      if (space != 0)
	      {
		Report (syntaxErrorSeverity, node,
		  "'endif' has parameters");
		okay = false;
	      }
	      if (okay && (wrapperStack.GetSize () == 0))
	      {
		Report (syntaxErrorSeverity, node,
		  "'endif' without 'if' or 'elsif'");
		okay = false;
	      }
	      if (okay)
              {
                int ascendNum = globalState->ascendStack.Pop ();
                eval.Ascend (ascendNum);
                while (ascendNum-- > 0)
                {
                  WrapperStackEntry lastWrapper = currentWrapper;
		  currentWrapper = wrapperStack.Pop ();
		  if (!lastWrapper.child->childNode.IsValid()
		      && (lastWrapper.child->childrenWrappers.GetSize() == 0))
		  {
		    currentWrapper.child->childrenWrappers.Delete (
		      lastWrapper.child);
		  }
		}
              }
	      handled = true;
	    }
	    break;
	  case csWrappedDocumentNodeFactory::PITOKEN_ELSE:
	    if (parseOpts & wdnfpoHandleConditions)
	    {
	      bool okay = true;
	      if (space != 0)
	      {
		Report (syntaxErrorSeverity, node,
		  "'else' has parameters");
		okay = false;
	      }
	      if (okay && (wrapperStack.GetSize () == 0))
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

		if (!newWrapper.child->IsUnreachable())
		  currentWrapper.child->childrenWrappers.Push (newWrapper.child);
		wrapperStack.Push (currentWrapper);
		currentWrapper = newWrapper;

                eval.SwitchBranch ();
	      }
	      handled = true;
	    }
	    break;
	  case csWrappedDocumentNodeFactory::PITOKEN_ELSIF:
	    if (parseOpts & wdnfpoHandleConditions)
	    {
	      bool okay = true;
	      if (wrapperStack.GetSize () == 0)
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

		if (!elseWrapper.child->IsUnreachable())
		  currentWrapper.child->childrenWrappers.Push (elseWrapper.child);
		wrapperStack.Push (currentWrapper);
		currentWrapper = elseWrapper;

		WrapperStackEntry newWrapper;
		ParseCondition (newWrapper, space + 1, valLen - cmdLen - 1,
		  node);
          
                eval.SwitchBranch ();
                csConditionID condition = newWrapper.child->condition;
                // If the parent condition is always false, so is this
                if (currentWrapper.child->IsUnreachable())
                {
                  condition = csCondAlwaysFalse;
                }
                Logic3 r = eval.Descend (condition);
                if (r.state == Logic3::Truth)
                  newWrapper.child->condition = csCondAlwaysTrue;
                else if (r.state == Logic3::Lie)
                  newWrapper.child->condition = csCondAlwaysFalse;
                globalState->ascendStack[globalState->ascendStack.GetSize()-1]++;

		if (!newWrapper.child->IsUnreachable())
		  elseWrapper.child->childrenWrappers.Push (newWrapper.child);
		wrapperStack.Push (currentWrapper);
		currentWrapper = newWrapper;
	      }
	      handled = true;
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
	    if (parseOpts & wdnfpoExpandTemplates)
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
		ProcessInclude (eval, filename, state, node, parseOpts);
	      }
	      handled = true;
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
	    if (parseOpts & wdnfpoExpandTemplates)
            {
              TempString<> args (valStart + cmdLen, valLen - cmdLen);
              args.LTrim();
              ProcessInstrTemplate (state, node, args, 
                tokenID == csWrappedDocumentNodeFactory::PITOKEN_TEMPLATEWEAK);
	      handled = true;
            }
	    break;
	  case csWrappedDocumentNodeFactory::PITOKEN_ENDTEMPLATE:
            if (shared->plugin->do_verbose)
            {
              Report (CS_REPORTER_SEVERITY_WARNING, node,
                "Deprecated syntax, please use 'Endtemplate'");
	      handled = true;
            }
            // Fall through
	  case csWrappedDocumentNodeFactory::PITOKEN_ENDTEMPLATE_NEW:
	    if (parseOpts & wdnfpoExpandTemplates)
	    {
	      Report (syntaxErrorSeverity, node,
		"'Endtemplate' without 'Template'");
              // ProcessTemplate() would've handled it otherwise
	      handled = true;
	    }
	    break;
          case csWrappedDocumentNodeFactory::PITOKEN_GENERATE:
	    if (parseOpts & wdnfpoExpandTemplates)
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
                    || ((end >= start) && (step <= 0)))
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
                    state->templ = Template();
                  }
                }
              }
	      handled = true;
            }
            break;
	  case csWrappedDocumentNodeFactory::PITOKEN_ENDGENERATE:
	    if (parseOpts & wdnfpoExpandTemplates)
	    {
	      Report (syntaxErrorSeverity, node,
		"'Endgenerate' without 'Generate'");
              // ProcessTemplate() would've handled it otherwise
	    }
	    break;
	  case csWrappedDocumentNodeFactory::PITOKEN_DEFINE:
	    if (parseOpts & wdnfpoExpandTemplates)
            {
              TempString<> args (valStart + cmdLen, valLen - cmdLen);
              args.LTrim();
              ProcessDefine (state, node, args);
	      handled = true;
            }
	    break;
	  case csWrappedDocumentNodeFactory::PITOKEN_UNDEF:
	    if (parseOpts & wdnfpoExpandTemplates)
            {
              TempString<> args (valStart + cmdLen, valLen - cmdLen);
              args.LTrim();
              ProcessUndef (state, node, args);
	      handled = true;
            }
	    break;
	  case csWrappedDocumentNodeFactory::PITOKEN_STATIC_IFDEF:
	  case csWrappedDocumentNodeFactory::PITOKEN_STATIC_IFNDEF:
	    if (parseOpts & wdnfpoExpandTemplates)
            {
              TempString<> args (valStart + cmdLen, valLen - cmdLen);
              args.LTrim();
              ProcessStaticIfDef (state, node, args,
                tokenID == csWrappedDocumentNodeFactory::PITOKEN_STATIC_IFNDEF);
	      handled = true;
            }
            break;
          case csWrappedDocumentNodeFactory::PITOKEN_STATIC_ELSIFDEF:
          case csWrappedDocumentNodeFactory::PITOKEN_STATIC_ELSIFNDEF:
          case csWrappedDocumentNodeFactory::PITOKEN_STATIC_ELSE:
          case csWrappedDocumentNodeFactory::PITOKEN_STATIC_ENDIF:
	    if (parseOpts & wdnfpoExpandTemplates)
            {
	      Report (syntaxErrorSeverity, node,
		"'%s' without 'SIfDef'", shared->pitokens.Request (tokenID));
              // ProcessStaticIf() would've handled it otherwise
	      handled = true;
            }
            break;
	  default:
	    {
	      CS_ASSERT_MSG(
	        "Document has templates but template expansion is disabled",
	        parseOpts & wdnfpoExpandTemplates);
	      Template::Params params;
	      if (space != 0)
	      {
		TempString<> pStr (space + 1, valLen - cmdLen - 1);
		ParseTemplateArguments (pStr, params, false);
	      }
	      if (!InvokeTemplate (eval, tokenStr, node, state, params, parseOpts))
	      {
		Report (syntaxErrorSeverity, node,
		  "Unknown command '%s'", tokenStr.GetData());
	      }
	      handled = true;
	    }
	}
      }
    }
  }
  if (!handled && !currentWrapper.child->IsUnreachable())
  {
    eval.Commit();
    csRef<WrappedChild> newWrapper;
    newWrapper.AttachNew (new WrappedChild);
    if (parseOpts & wdnfpoOnlyOneLevelConditions)
    {
      parseOpts &= ~wdnfpoHandleConditions;
    }
    newWrapper->childNode.AttachNew (new csWrappedDocumentNode (eval, this, 
      node, resolver, shared, globalState, parseOpts));
    currentWrapper.child->childrenWrappers.Push (newWrapper);
  }
}

template<typename ConditionEval>
void csWrappedDocumentNode::ProcessWrappedNode (ConditionEval& eval, 
                                                NodeProcessingState* state, 
						iDocumentNode* wrappedNode,
						uint parseOpts)
{
  if ((wrappedNode->GetType() == CS_NODE_ELEMENT)
    || (wrappedNode->GetType() == CS_NODE_DOCUMENT))
  {
    state->iter = wrappedNode->GetNodes ();
    while (state->iter->HasNext ())
    {
      csRef<iDocumentNode> node = state->iter->Next();
      ProcessSingleWrappedNode (eval, state, node, parseOpts);
    }
    ValidateTemplateEnd (wrappedNode, state);
    ValidateGenerateEnd (wrappedNode, state);
    ValidateStaticIfEnd (wrappedNode, state);
  }
}

template<typename T>
void csWrappedDocumentNode::ProcessWrappedNode (T& eval, uint parseOpts)
{
  NodeProcessingState state;
  state.currentWrapper.child.AttachNew (new WrappedChild);
  wrappedChildren.Push (state.currentWrapper.child);
  ProcessWrappedNode (eval, &state, wrappedNode, parseOpts);
}

void csWrappedDocumentNode::Report (int severity, iDocumentNode* node, 
				    const char* msg, ...)
{
  static const char* messageID = 
    "crystalspace.graphics3d.shadercompiler.xmlshader";

  va_list args;
  va_start (args, msg);

  csRef<iSyntaxService> synsrv =
    csQueryRegistry<iSyntaxService> (shared->objreg);
  if (synsrv.IsValid ())
  {
    csString str;
    str.FormatV (msg, args);
    synsrv->Report (messageID, 
      severity, node, "%s", str.GetData ());
  }
  else
  {
    csReportV (shared->objreg, severity, messageID, msg, args);
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

#include "csutil/custom_new_disable.h"

csRef<iDocumentNodeIterator> csWrappedDocumentNode::GetNodes ()
{
  if (wrappedChildren.GetSize() > 0)
  {
    csWrappedDocumentNodeIterator* iter = 
      new (shared->iterPool) csWrappedDocumentNodeIterator (this, 0);
    return csPtr<iDocumentNodeIterator> (iter);
  }
  else if (wrappedNode.IsValid())
    return wrappedNode->GetNodes();
  else
    return 0;
}

csRef<iDocumentNodeIterator> csWrappedDocumentNode::GetNodes (
  const char* value)
{
  if (wrappedChildren.GetSize() > 0)
  {
    csWrappedDocumentNodeIterator* iter = 
      new (shared->iterPool) csWrappedDocumentNodeIterator (this, value);
    return csPtr<iDocumentNodeIterator> (iter);
  }
  else if (wrappedNode.IsValid())
    return wrappedNode->GetNodes (value);
  else
    return 0;
}

#include "csutil/custom_new_enable.h"

csRef<iDocumentNode> csWrappedDocumentNode::GetNode (const char* value)
{
  if (wrappedChildren.GetSize() > 0)
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
  else if (wrappedNode.IsValid())
    return wrappedNode->GetNode (value);
  else
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

// Magic value to validate it's a wrapped doc and that the format is right
static const uint32 cachedWrappedDocMagic = 0x01214948;

bool csWrappedDocumentNode::StoreToCache (iFile* cacheFile,
  const ConditionsWriter& condWriter)
{
  uint32 magicLE = csLittleEndian::UInt32 (cachedWrappedDocMagic);
  if (cacheFile->Write ((char*)&magicLE, sizeof (magicLE)) != sizeof (magicLE))
    return false;

  ForeignNodeStorage foreignNodes (shared->plugin);
  if (!foreignNodes.StartUse (cacheFile)) return false;
  if (!StoreToCache (cacheFile, foreignNodes, condWriter)) return false;
  if (!foreignNodes.EndUse ()) return false;
  return true;
}

void csWrappedDocumentNode::CollectUsedConditions (ConditionsWriter& condWrite)
{
  CollectUsedConditions (wrappedChildren, condWrite);
}

bool csWrappedDocumentNode::StoreToCache (iFile* cacheFile,
                                          ForeignNodeStorage& foreignNodes,
                                          const ConditionsWriter& condWriter)
{
  int32 wrappedNodeID;
  if (wrappedChildren.GetSize() > 0)
    wrappedNodeID = foreignNodes.StoreNodeShallow (wrappedNode);
  else
    wrappedNodeID = foreignNodes.StoreNodeDeep (wrappedNode);
  int32 wrappedNodeLE = csLittleEndian::Int32 (wrappedNodeID);
  if (cacheFile->Write ((char*)&wrappedNodeLE, sizeof (wrappedNodeLE))
      != sizeof (wrappedNodeLE)) return false;
  
  return StoreWrappedChildren (cacheFile, foreignNodes, wrappedChildren,
    condWriter);
}

bool csWrappedDocumentNode::ReadFromCache (iFile* cacheFile,
  const ConditionsReader& condReader, ConditionDumper& condDump)
{
  uint32 magic;
  if (cacheFile->Read ((char*)&magic, sizeof (magic)) != sizeof (magic))
    return false;
  if (csLittleEndian::UInt32 (magic) != cachedWrappedDocMagic)
    return false;

  ForeignNodeReader foreignNodes (shared->plugin);
  if (!foreignNodes.StartUse (cacheFile)) return false;
  if (!ReadFromCache (cacheFile, foreignNodes, condReader, condDump)) return false;
  if (!foreignNodes.EndUse ()) return false;
  return true;
}

bool csWrappedDocumentNode::ReadFromCache (iFile* cacheFile,
                                           ForeignNodeReader& foreignNodes,
                                           const ConditionsReader& condReader,
					   ConditionDumper& condDump)
{
  int32 wrappedNodeLE;
  if (cacheFile->Read ((char*)&wrappedNodeLE, sizeof (wrappedNodeLE))
      != sizeof (wrappedNodeLE)) return false;
  wrappedNode = foreignNodes.GetNode (csLittleEndian::Int32 (wrappedNodeLE));
  
  return ReadWrappedChildren (cacheFile, foreignNodes, wrappedChildren,
    condReader, condDump);
}

enum
{
  childValue = 0x80000000,
  childIsNull = 0x40000000,
  conditionIsTrue = 0x20000000,

  flagsExtract = 0xe0000000
};

bool csWrappedDocumentNode::StoreWrappedChildren (iFile* file, 
  ForeignNodeStorage& foreignNodes, const csRefArray<WrappedChild>& children,
  const ConditionsWriter& condWriter)
{
  uint32 numChildrenLE = csLittleEndian::UInt32 (children.GetSize());
  if (file->Write ((char*)&numChildrenLE, sizeof (numChildrenLE))
      != sizeof (numChildrenLE)) return false;

  for (size_t i = 0; i < children.GetSize(); i++)
  {
    uint32 flags = 0;
    if (children[i]->GetConditionValue()) flags |= childValue;
    
    csRef<iWrappedDocumentNode> wrapper;
    if (!children[i]->childNode.IsValid())
    {
      flags |= childIsNull;
    }
    else
    {
      wrapper =
        scfQueryInterface<iWrappedDocumentNode> (children[i]->childNode);
      CS_ASSERT(wrapper);
    }
    
    if (children[i]->condition == csCondAlwaysTrue)
      flags |= conditionIsTrue;
    else if (children[i]->condition == csCondAlwaysFalse)
    {
      /* AlwaysFalse and 'true' condition value should've been filtered out
         earlier ... */
      CS_ASSERT (!children[i]->GetConditionValue());
      /* AlwaysFalse and 'false' condition value is equivalent to
         AlwaysTrue and 'true', so convert into that */
      flags |= conditionIsTrue;
      flags &= ~childValue;
    }

    uint32 flagsAndCond = flags;
    if (children[i]->condition != csCondAlwaysTrue)
    {
      flagsAndCond |= condWriter.GetDiskID (children[i]->condition);
    }
    
    uint32 flagsAndCondLE = csLittleEndian::UInt32 (flagsAndCond);
    if (file->Write ((char*)&flagsAndCondLE, sizeof (flagsAndCondLE))
	!= sizeof (flagsAndCondLE)) return false;

    if (wrapper.IsValid())
    {
      csWrappedDocumentNode* child = static_cast<csWrappedDocumentNode*> (
	(iWrappedDocumentNode*)wrapper);
      if (!child->StoreToCache (file, foreignNodes, condWriter))
	return false;
    }
    if (!StoreWrappedChildren (file, foreignNodes, children[i]->childrenWrappers,
        condWriter))
      return false;
  }
  return true;
}
  
void csWrappedDocumentNode::CollectUsedConditions (
  const csRefArray<WrappedChild>& children, ConditionsWriter& condWriter)
{
  for (size_t i = 0; i < children.GetSize(); i++)
  {
    condWriter.GetDiskID (children[i]->condition);
    
    csRef<iWrappedDocumentNode> wrapper;
    wrapper =
      scfQueryInterfaceSafe<iWrappedDocumentNode> (children[i]->childNode);
    
    if (wrapper.IsValid())
    {
      csWrappedDocumentNode* child = static_cast<csWrappedDocumentNode*> (
	(iWrappedDocumentNode*)wrapper);
      child->CollectUsedConditions (condWriter);
    }
    CollectUsedConditions (children[i]->childrenWrappers,
      condWriter);
  }
}

bool csWrappedDocumentNode::ReadWrappedChildren (iFile* file,
  ForeignNodeReader& foreignNodes, csRefArray<WrappedChild>& children,
  const ConditionsReader& condReader, ConditionDumper& condDump)
{
  uint32 numChildrenLE;
  if (file->Read ((char*)&numChildrenLE, sizeof (numChildrenLE))
      != sizeof (numChildrenLE)) return false;
      
  size_t numChildren = csLittleEndian::UInt32 (numChildrenLE);
  children.SetSize (numChildren);
  for (size_t i = 0; i < numChildren; i++)
  {
    uint32 flagsAndCondLE;
    if (file->Read ((char*)&flagsAndCondLE, sizeof (flagsAndCondLE))
	!= sizeof (flagsAndCondLE)) return false;
    uint32 flags = csLittleEndian::UInt32 (flagsAndCondLE) & flagsExtract;
    
    csRef<WrappedChild> child;
    child.AttachNew (new WrappedChild);
    child->SetConditionValue ((flags & childValue) != 0);
    
    if ((flags & conditionIsTrue) != 0)
      child->condition = csCondAlwaysTrue;
    else
    {
      child->condition = condReader.GetConditionID (
        csLittleEndian::UInt32 (flagsAndCondLE) & ~flagsExtract);
      if (condDump.DoesDumping())
      {
        csString condStr (condDump.GetConditionString (child->condition));
        condDump.Dump (child->condition, condStr, condStr.Length());
      }
    }
    if ((flags & childIsNull) == 0)
    {
      csWrappedDocumentNode* childWrapper = new csWrappedDocumentNode (
        this, resolver, shared);
      if (!childWrapper->ReadFromCache (file, foreignNodes, condReader, condDump))
        return false;
      child->childNode.AttachNew (childWrapper);
    }
    if (!ReadWrappedChildren (file, foreignNodes, child->childrenWrappers,
        condReader, condDump))
      return false;
    children.Put (i, child);
  }
  return true;
}

//---------------------------------------------------------------------------

csWrappedDocumentNode::WrapperWalker::WrapperWalker (
  csRefArray<WrappedChild>& wrappedChildren, iConditionResolver* resolver)
{
  SetData (wrappedChildren, resolver);
}
    
csWrappedDocumentNode::WrapperWalker::WrapperWalker ()
{
}

void csWrappedDocumentNode::WrapperWalker::SetData (
  csRefArray<WrappedChild>& wrappedChildren, iConditionResolver* resolver)
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
    if (currentPos->currentIndex < currentPos->currentWrappers->GetSize ())
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
          || (resolver->Evaluate (wrapper.condition) == wrapper.GetConditionValue()))
	{
	  currentPos = &posStack.GetExtend (posStack.GetSize ());
	  currentPos->currentIndex = 0;
	  currentPos->currentWrappers = &wrapper.childrenWrappers;
	}
      }
    }
    else
    {
      posStack.Pop ();
      size_t psl = posStack.GetSize ();
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
  nodeText = CS::StrDup (text);
}

csTextNodeWrapper::~csTextNodeWrapper ()
{
  cs_free (nodeText);
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

#include "csutil/custom_new_disable.h"

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

#include "csutil/custom_new_enable.h"

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
  csXMLShaderCompiler* plugin) : plugin (plugin), objreg (plugin->objectreg)
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

void csWrappedDocumentNodeFactory::DebugProcessing (const char* format, ...)
{
  if (!plugin->debugInstrProcessing) return;

  va_list args;
  va_start (args, format);
  csPrintfV (format, args);
  va_end (args);
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
  void Commit() { condTree.Commit(); }
};

csWrappedDocumentNode* csWrappedDocumentNodeFactory::CreateWrapper (
  iDocumentNode* wrappedNode, iConditionResolver* resolver, 
  csConditionEvaluator& evaluator, 
  const csRefArray<iDocumentNode>& extraNodes, csString* dumpOut,
  uint parseOpts, const MyBitArrayTemp* presetCondResults)
{
  ConditionDumper condDump (dumpOut, &evaluator);

  csWrappedDocumentNode* node;
  {
    EvalCondTree eval (evaluator);
    if (presetCondResults)
      eval.condTree.PresetConditions (*presetCondResults);
    if (parseOpts & wdnfpoHandleConditions)
    {
      for (size_t i = 0; i < extraNodes.GetSize(); i++)
      {
	csRef<csWrappedDocumentNode::GlobalProcessingState> globalState;
	globalState.AttachNew (csWrappedDocumentNode::GlobalProcessingState::Create ());
	globalState->vfs = csQueryRegistry<iVFS> (objreg);
	CS_ASSERT (globalState->vfs);
	globalState->condDumper = &condDump;
	/* "extra nodes" here just contribute to the conditions in the condition
	* tree, so they're parsed, but not retained. */
	delete new csWrappedDocumentNode (eval, 0, extraNodes[i], resolver, 
	  this, globalState, parseOpts);
      }
    }
    csRef<csWrappedDocumentNode::GlobalProcessingState> globalState;
    globalState.AttachNew (csWrappedDocumentNode::GlobalProcessingState::Create ());
    globalState->vfs = csQueryRegistry<iVFS> (objreg);
    CS_ASSERT (globalState->vfs);
    globalState->condDumper = &condDump;
    node = new csWrappedDocumentNode (eval, 0, wrappedNode, resolver, this,
      globalState, parseOpts);
    eval.condTree.ToResolver (resolver);
    if (plugin->doDumpValues && dumpOut)
    {
      *dumpOut << "\n\n";
      eval.condTree.Dump (*dumpOut);
    }
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
  void Commit () {}
};

csWrappedDocumentNode* csWrappedDocumentNodeFactory::CreateWrapperStatic (
  iDocumentNode* wrappedNode, iConditionResolver* resolver, csString* dumpOut,
  uint parseOptions)
{
  ConditionDumper condDump (dumpOut, 0);

  csWrappedDocumentNode* node;
  {
    csRef<csWrappedDocumentNode::GlobalProcessingState> globalState;
    globalState.AttachNew (csWrappedDocumentNode::GlobalProcessingState::Create ());
    globalState->vfs = csQueryRegistry<iVFS> (objreg);
    CS_ASSERT (globalState->vfs);
    globalState->condDumper = &condDump;
    EvalStatic eval (resolver);
    node = new csWrappedDocumentNode (eval, 0, wrappedNode, resolver, this, 
      globalState, parseOptions);
    CS_ASSERT(globalState->GetRefCount() == 1);
  }
  return node;
}

csWrappedDocumentNode* csWrappedDocumentNodeFactory::CreateWrapperFromCache (
  iFile* cacheFile, iConditionResolver* resolver, csConditionEvaluator& evaluator,
  const ConditionsReader& condReader, csString* dumpOut)
{
  ConditionDumper condDump (dumpOut, &evaluator);
  csWrappedDocumentNode* node;
  node = new csWrappedDocumentNode (0, resolver, this);
  if (!node->ReadFromCache (cacheFile, condReader, condDump))
  {
    delete node;
    return 0;
  }
  return node;
}

//---------------------------------------------------------------------------

void ConditionDumper::Dump (size_t id, const char* condStr, size_t condLen)
{
  if (currentOut)
  {
    if ((seenConds.GetSize() > id) && (seenConds[id])) return;
    
    switch (id)
    {
      case csCondAlwaysTrue:
	currentOut->AppendFmt ("condition \"always true\" = '");
	break;
      case csCondAlwaysFalse:
	currentOut->AppendFmt ("condition \"always false\" = '");
	break;
      default:
	{
	  if (seenConds.GetSize() <= id) seenConds.SetSize (id+1);
	  seenConds.SetBit (id);
	  
	  if (currentEval)
	  {
	    const CondOperation& condOp = currentEval->GetCondition (id);
	    if (condOp.left.type == operandOperation)
	    {
	      csString condStr;
	      condStr = currentEval->GetConditionString (condOp.left.operation);
	      Dump (condOp.left.operation, condStr, condStr.Length ());
	    }
	    if (condOp.right.type == operandOperation)
	    {
	      csString condStr;
	      condStr = currentEval->GetConditionString (condOp.right.operation);
	      Dump (condOp.right.operation, condStr, condStr.Length ());
	    }
	  }
	}
        currentOut->AppendFmt ("condition %zu = '", id);
    }
    currentOut->Append (condStr, condLen);
    currentOut->Append ("'\n");
  }
}

}
CS_PLUGIN_NAMESPACE_END(XMLShader)
