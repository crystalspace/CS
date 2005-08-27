/*
  Copyright (C) 2005 by Frank Richter
	    (C) 2005 by Jorrit Tyberghein

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

#ifndef __CS_DOCWRAP_REPLACER_H__
#define __CS_DOCWRAP_REPLACER_H__

#include "csutil/csstring.h"
#include "csutil/documentcommon.h"
#include "csutil/leakguard.h"
#include "csutil/parray.h"
#include "csutil/pooledscfclass.h"
#include "csutil/ref.h"
#include "csutil/refarr.h"
#include "csutil/scf.h"
#include "csutil/strhash.h"
#include "csutil/weakref.h"

#include "iutil/document.h"
#include "iutil/objreg.h"

class csReplacerDocumentNodeFactory;

typedef csHash<csString, csString> Substitutions;

/**
 * Wrapper around a document node, supporting conditionals.
 */
class csReplacerDocumentNode : public csDocumentNodeReadOnly
{
  friend class csReplacerDocumentNodeIterator;
  friend class csReplacerDocumentAttributeIterator;
  friend class csReplacerDocumentAttribute;

  csRef<iDocumentNode> wrappedNode;
  csWeakRef<csReplacerDocumentNode> parent;
  csString value;
  csReplacerDocumentNodeFactory* shared;

  const Substitutions* subst;
public:
  CS_LEAKGUARD_DECLARE(csReplacerDocumentNode);
  SCF_DECLARE_IBASE_POOLED(csReplacerDocumentNode);

  csReplacerDocumentNode (Pool* pool);
  virtual ~csReplacerDocumentNode ();
  void Set (iDocumentNode* wrappedNode, csReplacerDocumentNode* parent, 
    csReplacerDocumentNodeFactory* shared, const Substitutions& subst);

  virtual csDocumentNodeType GetType ()
  { return wrappedNode->GetType(); }
  virtual bool Equals (iDocumentNode* other);
  virtual const char* GetValue ()
  { return value; }

  virtual csRef<iDocumentNode> GetParent ()
  { return parent ? ((iDocumentNode*)parent) : wrappedNode->GetParent(); }
  virtual csRef<iDocumentNodeIterator> GetNodes ();
  virtual csRef<iDocumentNode> GetNode (const char* value);

  virtual csRef<iDocumentAttributeIterator> GetAttributes ();
  virtual csRef<iDocumentAttribute> GetAttribute (const char* name);
};

class csReplacerDocumentNodeIterator : public iDocumentNodeIterator
{
  csReplacerDocumentNode* node;
  csRef<iDocumentNodeIterator> wrappedIter;
public:
  CS_LEAKGUARD_DECLARE(csReplacerDocumentNodeIterator);
  SCF_DECLARE_IBASE_POOLED(csReplacerDocumentNodeIterator);

  csReplacerDocumentNodeIterator (Pool* pool);
  virtual ~csReplacerDocumentNodeIterator ();

  void SetData (csReplacerDocumentNode* node);

  virtual bool HasNext ();
  virtual csRef<iDocumentNode> Next ();
};

class csReplacerDocumentAttributeIterator : public iDocumentAttributeIterator
{
  csReplacerDocumentNode* node;
  csRef<iDocumentAttributeIterator> wrappedIter;
public:
  CS_LEAKGUARD_DECLARE(csReplacerDocumentAttributeIterator );
  SCF_DECLARE_IBASE_POOLED(csReplacerDocumentAttributeIterator );

  csReplacerDocumentAttributeIterator (Pool* pool);
  virtual ~csReplacerDocumentAttributeIterator ();

  void SetData (csReplacerDocumentNode* node);

  virtual bool HasNext ();
  virtual csRef<iDocumentAttribute> Next ();
};

class csReplacerDocumentAttribute : public csDocumentAttributeCommon
{
  csString name;
  csString val;
public:
  CS_LEAKGUARD_DECLARE(csReplacerDocumentAttribute);
  SCF_DECLARE_IBASE_POOLED(csReplacerDocumentAttribute);

  csReplacerDocumentAttribute (Pool* pool);
  virtual ~csReplacerDocumentAttribute ();

  void Set (csReplacerDocumentNode* node, iDocumentAttribute* wrappedAttr);

  virtual const char* GetName () { return name; }
  virtual const char* GetValue () { return val; }

  virtual void SetName (const char* name) {}
  virtual void SetValue (const char* value) {}
};

class csReplacerDocumentNodeFactory
{
  friend class csReplacerDocumentNode;
  friend class csReplacerDocumentNodeIterator;
  friend class csReplacerDocumentAttributeIterator;
  friend class csReplacerDocumentAttribute;

  csReplacerDocumentNode::Pool nodePool;
  csReplacerDocumentAttribute::Pool attrPool;
  csReplacerDocumentNodeIterator::Pool iterPool;
  csReplacerDocumentAttributeIterator::Pool attrIterPool;

  void Substitute (const char* in, csString& out, 
    const Substitutions& subst);
public:
  csReplacerDocumentNodeFactory ();

  csRef<iDocumentNode> CreateWrapper (iDocumentNode* wrappedNode, 
    csReplacerDocumentNode* parent, const Substitutions& subst);
};

#endif // __CS_DOCWRAP_REPLACER_H__
