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
  /// Get value of this attribute as int.
  virtual int GetValueAsInt () const = 0;
  /// Get value of this attribute as float.
  virtual float GetValueAsFloat () const = 0;
  /// Set name of this attribute.
  virtual void SetName (const char* name) = 0;
  /// Set value of this attribute.
  virtual void SetValue (const char* value) = 0;
  /// Set int value of this attribute.
  virtual void SetValueAsInt (int v) = 0;
  /// Set float value of this attribute.
  virtual void SetValueAsFloat (float f) = 0;
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
  /// Get the parent.
  virtual csRef<iXmlNode> GetParent () = 0;

  //---------------------------------------------------------------------
  
  /// Get an iterator over all children.
  virtual csRef<iXmlNodeIterator> GetNodes () = 0;
  /// Get an iterator over all children of the specified type.
  virtual csRef<iXmlNodeIterator> GetNodes (const char* type) = 0;
  /// Get the first node of the given type.
  virtual csRef<iXmlNode> GetNode (const char* type) = 0;

  /// Remove a child.
  virtual void RemoveNode (const csRef<iXmlNode>& child) = 0;
  /// Remove all children.
  virtual void RemoveNodes () = 0;

  /// Create a new node of the given type at the end.
  virtual csRef<iXmlNode> CreateNode (const char* type) = 0;
  /// Create a new node of the given type before the specified node.
  virtual csRef<iXmlNode> CreateNodeBefore (const char* type,
  	const csRef<iXmlNode>& node) = 0;
  /// Create a new node of the given type after the specified node.
  virtual csRef<iXmlNode> CreateNodeAfter (const char* type,
  	const csRef<iXmlNode>& node) = 0;
  /**
   * Move a node (which should be a child of this node) before the given
   * node.
   */
  virtual void MoveNodeBefore (const csRef<iXmlNode>& node,
  	const csRef<iXmlNode>& before) = 0;
  /**
   * Move a node (which should be a child of this node) after the given
   * node.
   */
  virtual void MoveNodeAfter (const csRef<iXmlNode>& node,
  	const csRef<iXmlNode>& after) = 0;

  //---------------------------------------------------------------------

  /// Get an iterator over all attributes.
  virtual csRef<iXmlAttributeIterator> GetAttributes () = 0;
  /// Get an attribute by name.
  virtual csRef<iXmlAttribute> GetAttribute (const char* name) = 0;
  /// Get an attribute value by name.
  virtual const char* GetAttributeValue (const char* name) = 0;
  /// Get an attribute value by name.
  virtual int GetAttributeValueAsInt (const char* name) = 0;
  /// Get an attribute value by name.
  virtual float GetAttributeValueAsFloat (const char* name) = 0;

  /// Remove an attribute.
  virtual void RemoveAttribute (const csRef<iXmlAttribute>& attr) = 0;
  /// Remove all attributes.
  virtual void RemoveAttributes () = 0;

  /// Change or add an attribute.
  virtual void SetAttribute (const char* name, const char* value) = 0;
  /**
   * Create a new attribute at the end.
   */
  virtual csRef<iXmlAttribute> CreateAttribute () = 0;
  /**
   * Create a new attribute before the given attribute.
   */
  virtual csRef<iXmlAttribute> CreateAttributeBefore (
  	const csRef<iXmlAttribute>& attr) = 0;
  /**
   * Create a new attribute after the given attribute.
   */
  virtual csRef<iXmlAttribute> CreateAttributeAfter (
  	const csRef<iXmlAttribute>& attr) = 0;
  /**
   * Move an attribute (which should be a child of this node) before the
   * given attribute.
   */
  virtual void MoveAttributeBefore (const csRef<iXmlAttribute>& attr,
  	const csRef<iXmlAttribute>& before) = 0;
  /**
   * Move an attribute (which should be a child of this node) after the
   * given attribute.
   */
  virtual void MoveAttributeAfter (const csRef<iXmlAttribute>& attr,
  	const csRef<iXmlAttribute>& after) = 0;
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

