/*
    Crystal Space XML Interface
    Copyright (C) 2002 by Jorrit Tyberghein

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

#ifndef __IUTIL_XML_H__
#define __IUTIL_XML_H__

#include "csutil/scf.h"
#include "csutil/ref.h"

struct iXmlNode;
struct iXmlAttribute;

SCF_VERSION (iXmlAttributeIterator, 0, 0, 1);

/**
 * An iterator over attributes.
 */
struct iXmlAttributeIterator : public iBase
{
  /// Are there more elements?
  virtual bool HasNext () = 0;
  /// Get next element.
  virtual csRef<iXmlAttribute> Next () = 0;
};

SCF_VERSION (iXmlAttribute, 0, 0, 1);

/**
 * This represents an attribute in XML.
 */
struct iXmlAttribute : public iBase
{
  /// Get name of this attribute.
  virtual const char* GetName () const = 0;
  /// Get value of this attribute.
  virtual const char* GetValue () const = 0;
  /// Set name of this attribute.
  virtual void SetName (const char* name) = 0;
  /// Set value of this attribute.
  virtual void SetValue (const char* value) = 0;
};

SCF_VERSION (iXmlNodeIterator, 0, 0, 1);

/**
 * An iterator over nodes.
 */
struct iXmlNodeIterator : public iBase
{
  /// Are there more elements?
  virtual bool HasNext () = 0;
  /// Get next element.
  virtual csRef<iXmlNode> Next () = 0;
};

SCF_VERSION (iXmlNode, 0, 0, 1);

/**
 * This represents a node in XML.
 */
struct iXmlNode : public iBase
{
  /// Get number of children.
  virtual int GetChildCount () const = 0;

  /// Get child with index.
  virtual csRef<iXmlNode> GetChild (int idx) = 0;
  /// Get child by type.
  virtual csRef<iXmlNode> GetChild (const char* type) = 0;
  /// Find the index for some child.
  virtual int GetChildIndex (const csRef<iXmlNode>& child) = 0;
  /// Get an iterator over all children.
  virtual csRef<iXmlNodeIterator> GetChildren () = 0;
  /// Get an iterator over all children of the specified type.
  virtual csRef<iXmlNodeIterator> GetChildren (const char* type) = 0;

  /// Remove a child by index.
  virtual void RemoveChild (int idx) = 0;
  /// Remove a child by type.
  virtual void RemoveChild (const char* type) = 0;
  /// Remove a child.
  virtual void RemoveChild (const csRef<iXmlNode>& child) = 0;
  /// Remove all children.
  virtual void RemoveChildren () = 0;

  /// Get number of attributes.
  virtual int GetAttributeCount () const = 0;

  /// Get attribute with index.
  virtual csRef<iXmlAttribute> GetAttribute (int idx) = 0;
  /// Get attribute by name.
  virtual csRef<iXmlAttribute> GetAttribute (const char* name) = 0;
  /// Find the index for some attribute.
  virtual int GetAttributeIndex (const csRef<iXmlAttribute>& attr) = 0;
  /// Get an iterator over all attributes.
  virtual csRef<iXmlAttributeIterator> GetAttributes () = 0;
  /// Get an iterator over all attributes of the specified name.
  virtual csRef<iXmlAttributeIterator> GetAttributes (const char* name) = 0;

  /// Change or add an attribute.
  virtual void SetAttribute (const char* name, const char* value) = 0;

  /// Remove an attribute by index.
  virtual void RemoveAttribute (int idx) = 0;
  /// Remove all attributes with that name.
  virtual void RemoveAttribute (const char* name) = 0;
  /// Remove an attribute.
  virtual void RemoveAttribute (const csRef<iXmlAttribute>& attr) = 0;
  /// Remove all attributes.
  virtual void RemoveAttributes () = 0;
};

#endif // __IUTIL_XML_H__

