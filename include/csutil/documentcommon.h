/*
    Copyright (C) 2005 by Jorrit Tyberghein
	      (C) 2005 by Frank Richter

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**\file
 * Partial iDocumentNode implementation with commonly duplicated logic.
 */
 
#ifndef __CS_CSUTIL_DOCUMENTCOMMON_H__
#define __CS_CSUTIL_DOCUMENTCOMMON_H__

#include "csextern.h"
#include "csutil/leakguard.h"
#include "csutil/scf_implementation.h"
#include "iutil/document.h"

/**
 * Document node iterator does not return any nodes.
 */
class CS_CRYSTALSPACE_EXPORT csEmptyDocumentNodeIterator : 
  public scfImplementation1<csEmptyDocumentNodeIterator, iDocumentNodeIterator>
{
public:
  CS_LEAKGUARD_DECLARE(csEmptyDocumentNodeIterator);

  csEmptyDocumentNodeIterator ();
  virtual ~csEmptyDocumentNodeIterator ();

  virtual bool HasNext () { return false; }
  virtual csRef<iDocumentNode> Next () { return 0; }
};

/**
 * Document attribute iterator does not return any attributes.
 */
class CS_CRYSTALSPACE_EXPORT csEmptyDocumentAttributeIterator : 
  public scfImplementation1<csEmptyDocumentAttributeIterator, iDocumentAttributeIterator>
{
public:
  CS_LEAKGUARD_DECLARE(csEmptyDocumentAttributeIterator);

  csEmptyDocumentAttributeIterator ();
  virtual ~csEmptyDocumentAttributeIterator ();

  virtual bool HasNext () { return false; }
  virtual csRef<iDocumentAttribute> Next () { return 0; }
};

/**
 * Partial iDocumentNode implementation with commonly duplicated logic.
 */
class CS_CRYSTALSPACE_EXPORT csDocumentNodeCommon : 
  public scfImplementation1<csDocumentNodeCommon, iDocumentNode>
{
public:
  csDocumentNodeCommon () : scfImplementationType (this) {}
  //@{
  /// Converts 'value' to a string and calls SetValue() with it.
  virtual void SetValueAsInt (int value);
  virtual void SetValueAsFloat (float value);
  //@}

  /// Return empty iterator.
  virtual csRef<iDocumentNodeIterator> GetNodes ()
  { return csPtr<iDocumentNodeIterator> (new csEmptyDocumentNodeIterator); }
  /**
   * Return iterator wrapping around the one returned by GetNodes() and
   * filtering for nodes with a value of \a value.
   */
  virtual csRef<iDocumentNodeIterator> GetNodes (const char* value);
  /// Dummy implementation
  virtual csRef<iDocumentNode> GetNode (const char* /*value*/)
  { return 0; }

  //@{
  /**
   * Returns value of first child text node. Converts from a string if 
   * necessary.
   */
  virtual const char* GetContentsValue ();
  virtual int GetContentsValueAsInt ();
  virtual float GetContentsValueAsFloat ();
  //@}

  /// Return empty iterator.
  virtual csRef<iDocumentAttributeIterator> GetAttributes ()
  { 
    return csPtr<iDocumentAttributeIterator> 
      (new csEmptyDocumentAttributeIterator); 
  }

  /// Dummy implementation.
  csRef<iDocumentAttribute> GetAttribute (const char*) { return 0; }
  //@{
  /// Returns value of an attribute.
  virtual const char* GetAttributeValue (const char* name);
  virtual int GetAttributeValueAsInt (const char* name);
  virtual float GetAttributeValueAsFloat (const char* name);
  virtual bool GetAttributeValueAsBool (const char* name, 
    bool defaultvalue = false);
  //@}

  //@{
  /// Converts 'value' to a string and calls SetAttribute() with it.
  virtual void SetAttributeAsInt (const char* name, int value);
  virtual void SetAttributeAsFloat (const char* name, float value);
  //@}
};

/**
 * Partial iDocumentAttribute implementation with commonly duplicated logic.
 */
class CS_CRYSTALSPACE_EXPORT csDocumentAttributeCommon : 
  public scfImplementation1<csDocumentAttributeCommon, iDocumentAttribute>
{
public:
  csDocumentAttributeCommon () : scfImplementationType (this)
  {}
  //@{
  /**
   * Returns value. Uses GetValue() and converts the string to the return type.
   */
  virtual int GetValueAsInt ();
  virtual float GetValueAsFloat ();
  virtual bool GetValueAsBool ();
  //@}

  //@{
  /// Converts 'value' to a string and calls SetValue() with it.
  virtual void SetValueAsInt (int v);
  virtual void SetValueAsFloat (float f);
  //@}
};

/**
 * Partial iDocumentNode implementation with commonly duplicated logic and
 * dummy implementations for manipulation methods.
 */
class CS_CRYSTALSPACE_EXPORT csDocumentNodeReadOnly : 
  public scfImplementationExt0<csDocumentNodeReadOnly, csDocumentNodeCommon>
{
public:
  csDocumentNodeReadOnly () 
    : scfImplementationType (this)
  {}
  //@{
  /// Dummy implementation for nodes that cannot be changed
  virtual void SetValue (const char* /*value*/) {}
  virtual void SetValueAsInt (int /*value*/) {}
  virtual void SetValueAsFloat (float /*value*/) {}

  virtual void RemoveNode (const csRef<iDocumentNode>& /*child*/) {}
  virtual void RemoveNodes (csRef<iDocumentNodeIterator> /*children*/) {}
  virtual void RemoveNodes () {}

  virtual csRef<iDocumentNode> CreateNodeBefore (csDocumentNodeType /*type*/,
  	iDocumentNode* /*before*/)
  { return 0; }

  virtual void RemoveAttribute (const csRef<iDocumentAttribute>& /*attr*/) {}
  virtual void RemoveAttributes () {}

  virtual void SetAttribute (const char* /*name*/, const char* /*value*/) {}
  virtual void SetAttributeAsInt (const char* /*name*/, int /*value*/) {}
  virtual void SetAttributeAsFloat (const char* /*name*/, float /*value*/) {}
  //@}
};

#endif // __CS_CSUTIL_DOCUMENTCOMMON_H__
