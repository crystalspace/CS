/*
  Copyright (C) 2004 by Frank Richter
	    (C) 2004 by Jorrit Tyberghein

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

#include "csutil/util.h"
#include "csutil/sysfunc.h"
#include "csutil/xmltiny.h"
#include "imap/services.h"
#include "ivaria/reporter.h"
#include "iutil/vfs.h"

#include "docwrap.h"
#include "tokenhelper.h"

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csWrappedDocumentNode)
  SCF_IMPLEMENTS_INTERFACE(iDocumentNode)
SCF_IMPLEMENT_IBASE_END

CS_LEAKGUARD_IMPLEMENT(csWrappedDocumentNode);

csWrappedDocumentNode::csWrappedDocumentNode (csWrappedDocumentNodeFactory* shared,
					      iDocumentNode* wrappedNode,
					      iConditionResolver* resolver)
{
  SCF_CONSTRUCT_IBASE(0);

  csWrappedDocumentNode::objreg = shared->objreg;
  csWrappedDocumentNode::wrappedNode = wrappedNode;
  csWrappedDocumentNode::resolver = resolver;
  CS_ASSERT (resolver);
  csWrappedDocumentNode::shared = shared;

  ProcessWrappedNode ();
}

csWrappedDocumentNode::csWrappedDocumentNode (iDocumentNode* wrappedNode,
					      csWrappedDocumentNode* parent,
					      csWrappedDocumentNodeFactory* shared)
{
  SCF_CONSTRUCT_IBASE(0);

  csWrappedDocumentNode::wrappedNode = wrappedNode;
  csWrappedDocumentNode::parent = parent;
  resolver = parent->resolver;
  objreg = parent->objreg;
  csWrappedDocumentNode::shared = shared;

  ProcessWrappedNode ();
}

csWrappedDocumentNode::~csWrappedDocumentNode ()
{
  SCF_DESTRUCT_IBASE();
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
  csConditionNode* condNodes[2]; // 0 - trueNode, 1 - falseNode
  int currentCondNode;

  WrapperStackEntry ()
  {
    child = 0;
    condNodes[0] = condNodes[1] = 0;
    currentCondNode = 0;
  }
};

struct csWrappedDocumentNode::NodeProcessingState
{
  csArray<WrapperStackEntry> wrapperStack;
  WrapperStackEntry currentWrapper;
};

//#define DUMP_CONDITION_IDS

void csWrappedDocumentNode::ProcessWrappedNode (NodeProcessingState* state, 
						iDocumentNode* wrappedNode)
{
  const int syntaxErrorSeverity = CS_REPORTER_SEVERITY_ERROR;

  if ((wrappedNode->GetType() == CS_NODE_ELEMENT)
    || (wrappedNode->GetType () == CS_NODE_DOCUMENT))
  {
    csArray<WrapperStackEntry>& wrapperStack = state->wrapperStack;
    WrapperStackEntry& currentWrapper = state->currentWrapper;
    
    csRef<iDocumentNodeIterator> iter = wrappedNode->GetNodes ();
    while (iter->HasNext ())
    {
      bool handled = false;
      csRef<iDocumentNode> node = iter->Next();
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

	    if (TokenEquals (valStart, cmdLen, "if"))
	    {
	      WrapperStackEntry newWrapper;
	      newWrapper.child = new WrappedChild;
	      const char* result = resolver->ParseCondition (space + 1,
		valLen - cmdLen - 1, newWrapper.child->condition);
	      if (result)
	      {
		csString condStr;
		condStr.Append (space + 1, valLen - cmdLen);
		Report (syntaxErrorSeverity, node,
		  "Error parsing condition '%s': %s", condStr.GetData(),
		  result);
		newWrapper.child->condition = csCondAlwaysFalse;
	      }
#ifdef DUMP_CONDITION_IDS
	      csPrintf ("condition %s = %lu\n", 
		csString().Append (valStart, valLen).GetDataSafe (),
		newWrapper.child->condition);
#endif
    	  
	      resolver->AddNode (
		currentWrapper.condNodes[currentWrapper.currentCondNode], 
		newWrapper.child->condition, newWrapper.condNodes[0], 
		newWrapper.condNodes[1]);

	      currentWrapper.child->childrenWrappers.Push (newWrapper.child);
	      wrapperStack.Push (currentWrapper);
	      currentWrapper = newWrapper;
	    }
	    else if (TokenEquals (valStart, cmdLen, "endif"))
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
		currentWrapper = wrapperStack.Pop ();
	    }
	    else if (TokenEquals (valStart, cmdLen, "else"))
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
	      if (okay && (currentWrapper.currentCondNode != 0))
	      {
		Report (syntaxErrorSeverity, node,
		  "Double 'else'");
		okay = false;
	      }
	      if (okay)
	      {
		//currentWrapper.currentCondNode = 1;
		WrapperStackEntry oldCurrentWrapper = currentWrapper;
		currentWrapper = wrapperStack.Pop ();
		WrapperStackEntry newWrapper = oldCurrentWrapper;
		newWrapper.child = new WrappedChild;
		newWrapper.currentCondNode = 1;
		newWrapper.child->condition = oldCurrentWrapper.child->condition;
		newWrapper.child->conditionValue = false;

		currentWrapper.child->childrenWrappers.Push (newWrapper.child);
		wrapperStack.Push (currentWrapper);
		currentWrapper = newWrapper;
	      }
	    }
	    else if (TokenEquals (valStart, cmdLen, "elsif"))
	    {
	      bool okay = true;
	      if (wrapperStack.Length() == 0)
	      {
		Report (syntaxErrorSeverity, node,
		  "'elsif' without 'if' or 'elsif'");
		okay = false;
	      }
	      if (okay && (currentWrapper.currentCondNode != 0))
	      {
		Report (syntaxErrorSeverity, node,
		  "Double 'else'");
		okay = false;
	      }
	      if (okay)
	      {
		//currentWrapper.currentCondNode = 1;
		WrapperStackEntry oldCurrentWrapper = currentWrapper;
		currentWrapper = wrapperStack.Pop ();
		WrapperStackEntry elseWrapper = oldCurrentWrapper;
		elseWrapper.child = new WrappedChild;
		elseWrapper.currentCondNode = 1;
		elseWrapper.child->condition = oldCurrentWrapper.child->condition;
		elseWrapper.child->conditionValue = false;

		currentWrapper.child->childrenWrappers.Push (elseWrapper.child);

		WrapperStackEntry newWrapper;
		newWrapper.child = new WrappedChild;
		const char* result = resolver->ParseCondition (space + 1,
		  valLen - cmdLen - 1, newWrapper.child->condition);
		if (result)
		{
		  csString condStr;
		  condStr.Append (space + 1, valLen - cmdLen);
		  Report (syntaxErrorSeverity, node,
		    "Error parsing condition '%s': %s", condStr.GetData(),
		    result);
		}
#ifdef DUMP_CONDITION_IDS
	      csPrintf ("condition %s = %lu\n", 
		csString().Append (valStart, valLen).GetDataSafe (),
		newWrapper.child->condition);
#endif
      	  
		resolver->AddNode (
		  elseWrapper.condNodes[elseWrapper.currentCondNode], 
		  newWrapper.child->condition, newWrapper.condNodes[0], 
		  newWrapper.condNodes[1]);

		elseWrapper.child->childrenWrappers.Push (newWrapper.child);
		wrapperStack.Push (elseWrapper);
		currentWrapper = newWrapper;
	      }
	    }
	    else if (TokenEquals (valStart, cmdLen, "include"))
	    {
	      bool okay = true;
	      csString filename;
	      const char* space = strchr (valStart, ' ');
	      /* The rightmost spaces were skipped and don't interest us
		any more. */
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
		  const char* err = includeDoc->Parse (include, true);
		  if (err != 0)
		  {
		    Report (syntaxErrorSeverity, node,
		      "error parsing '%s': %s", filename.GetData (), err);
		  }
		  else
		  {
		    csRef<iDocumentNode> includeNode = includeDoc->GetRoot ();
		    csRef<iDocumentNodeIterator> it = includeNode->GetNodes ();
		    while (it->HasNext ())
		    {
		      csRef<iDocumentNode> child = it->Next ();
		      ProcessWrappedNode (state, child);
		    }
		  }
		}
	      }
	    }
	    else
	    {
	      csString cmdStr;
	      cmdStr.Append (valStart, cmdLen);
	      Report (syntaxErrorSeverity, node,
		"Unknown command '%s'", cmdStr.GetData());
	    }

	    handled = true;
	  }
	}
      }
      if (!handled)
      {
	WrappedChild* newWrapper = new WrappedChild;
	newWrapper->childNode.AttachNew (new csWrappedDocumentNode (node,
	  this, shared));
	currentWrapper.child->childrenWrappers.Push (newWrapper);
      }
    }
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
  if (contents.GetData () != 0)
    return contents;

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

int csWrappedDocumentNode::GetContentsValueAsInt ()
{
  const char* v = GetContentsValue ();
  if (!v) return 0;
  int val = 0;
  sscanf (v, "%d", &val);
  return val;
}

float csWrappedDocumentNode::GetContentsValueAsFloat ()
{
  const char* v = GetContentsValue ();
  if (!v) return 0;
  float val = 0.0;
  sscanf (v, "%f", &val);
  return val;
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
	if (resolver->Evaluate (wrapper.condition) == wrapper.conditionValue)
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

SCF_IMPLEMENT_IBASE(csEmptyDocumentNodeIterator)
  SCF_IMPLEMENTS_INTERFACE(iDocumentNodeIterator)
SCF_IMPLEMENT_IBASE_END

csEmptyDocumentNodeIterator::csEmptyDocumentNodeIterator ()
{
  SCF_CONSTRUCT_IBASE(0);
}

csEmptyDocumentNodeIterator::~csEmptyDocumentNodeIterator ()
{
  SCF_DESTRUCT_IBASE();
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csEmptyDocumentAttributeIterator)
  SCF_IMPLEMENTS_INTERFACE(iDocumentAttributeIterator)
SCF_IMPLEMENT_IBASE_END

csEmptyDocumentAttributeIterator::csEmptyDocumentAttributeIterator ()
{
  SCF_CONSTRUCT_IBASE(0);
}

csEmptyDocumentAttributeIterator::~csEmptyDocumentAttributeIterator ()
{
  SCF_DESTRUCT_IBASE();
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE_POOLED(csTextNodeWrapper)
  SCF_IMPLEMENTS_INTERFACE(iDocumentNode)
SCF_IMPLEMENT_IBASE_END

csTextNodeWrapper::csTextNodeWrapper (Pool* pool)
{
  SCF_CONSTRUCT_IBASE_POOLED(pool);
}

csTextNodeWrapper::~csTextNodeWrapper ()
{
  delete[] nodeText;
  SCF_DESTRUCT_IBASE();
}

void csTextNodeWrapper::SetData (iDocumentNode* realMe, const char* text)
{
  csTextNodeWrapper::realMe = realMe;
  nodeText = csStrNew (text);
}

//---------------------------------------------------------------------------

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
    csTextNodeWrapper* textNode = parentNode->shared->textNodePool.Alloc ();
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
  iObjectRegistry* objreg)
{
  csWrappedDocumentNodeFactory::objreg = objreg;
}

csWrappedDocumentNode* csWrappedDocumentNodeFactory::CreateWrapper (
  iDocumentNode* wrappedNode, iConditionResolver* resolver)
{
  return new csWrappedDocumentNode (this, wrappedNode, resolver);
}
