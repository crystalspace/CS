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
struct iFile;
struct iDataBuffer;
struct iString;

//===========================================================================

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

//===========================================================================

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

//===========================================================================

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

//===========================================================================

SCF_VERSION (iXmlNode, 0, 0, 1);

/**
 * This represents a node in XML.
 */
struct iXmlNode : public iBase
{
  /// Get the type of this node.
  virtual const char* GetType () = 0;
  /// Set the type of this node.
  virtual void SetType (const char* type) = 0;
  /// Get number of children.
  virtual int GetChildCount () const = 0;

  //---------------------------------------------------------------------
  
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

  /**
   * Create a new node of the given type at the given index.
   * The node at that index (and all following nodes) will be shifted
   * up. If index is -1 then the node will be created last.
   */
  virtual csRef<iXmlNode> CreateNode (const char* type, int index = -1) = 0;
  /**
   * Move a node (which should be a child of this node) to the given index.
   * All nodes at that index (and all following nodes) will be shifted up.
   * If index is -1 then the node will be moved to the last position.
   */
  virtual void MoveNode (const csRef<iXmlNode>& node, int index) = 0;

  //---------------------------------------------------------------------

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

  /// Remove an attribute by index.
  virtual void RemoveAttribute (int idx) = 0;
  /// Remove all attributes with that name.
  virtual void RemoveAttribute (const char* name) = 0;
  /// Remove an attribute.
  virtual void RemoveAttribute (const csRef<iXmlAttribute>& attr) = 0;
  /// Remove all attributes.
  virtual void RemoveAttributes () = 0;

  /// Change or add an attribute.
  virtual void SetAttribute (const char* name, const char* value) = 0;
  /**
   * Create a new attribute at the given index.
   * The attribute at that index (and all following attributes) will be shifted
   * up. If index is -1 then the attribute will be created last.
   */
  virtual csRef<iXmlAttribute> CreateAttribute (int index = -1) = 0;
  /**
   * Move an attribute (which should be a child of this node) to the given
   * index. The attribute at that index (and all following attributes) will
   * be shifted up. If index is -1 then the attribute will be moved to
   * the last position.
   */
  virtual void MoveAttribute (const csRef<iXmlAttribute>& attr, int index) = 0;
};

//===========================================================================

SCF_VERSION (iXmlDocument, 0, 0, 1);

/**
 * This represents a document in XML.
 */
struct iXmlDocument : public iBase
{
  /// Clear the document fully.
  virtual void Clear () = 0;

  /// Create a root node. This will clear the previous root node if any.
  virtual csRef<iXmlNode> CreateRoot () = 0;

  /// Get the current root node.
  virtual csRef<iXmlNode> GetRoot () = 0;

  /**
   * Parse XML file from an iFile.
   * This will clear the previous root node if any.
   * Returns NULL if all is ok. Otherwise it will return an error
   * string.
   */
  virtual const char* ParseXML (iFile* file) = 0;

  /**
   * Parse XML file from an iDataBuffer.
   * This will clear the previous root node if any.
   * Returns NULL if all is ok. Otherwise it will return an error
   * string.
   */
  virtual const char* ParseXML (iDataBuffer* buf) = 0;

  /**
   * Parse XML file from an iString.
   * This will clear the previous root node if any.
   * Returns NULL if all is ok. Otherwise it will return an error
   * string.
   */
  virtual const char* ParseXML (iString* str) = 0;

  /**
   * Parse XML file from a char array.
   * This will clear the previous root node if any.
   * Returns NULL if all is ok. Otherwise it will return an error
   * string.
   */
  virtual const char* ParseXML (const char* buf) = 0;

  /**
   * Write out XML file to an iFile.
   * This will return NULL if all is ok. Otherwise it will return an
   * error string.
   */
  virtual const char* WriteXML (iFile* file) = 0;

  /**
   * Write out XML file to an iString.
   * This will return NULL if all is ok. Otherwise it will return an
   * error string.
   */
  virtual const char* WriteXML (iString& str) = 0;
};

//===========================================================================

SCF_VERSION (iXmlSystem, 0, 0, 1);

/**
 * The XML plugin.
 */
struct iXmlSystem : public iBase
{
  /// Create a new empty document.
  virtual csRef<iXmlDocument> CreateDocument () = 0;
};


#endif // __IUTIL_XML_H__

