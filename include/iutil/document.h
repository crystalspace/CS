/*
    Crystal Space Document Interface
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

#ifndef __IUTIL_DOCUMENT_H__
#define __IUTIL_DOCUMENT_H__

#include "csutil/scf.h"
#include "csutil/ref.h"

struct iDocumentNode;
struct iDocumentAttribute;
struct iFile;
struct iDataBuffer;
struct iString;
struct iVFS;

/**
 * Possible node types for documents.
 */
enum csDocumentNodeType
{
  CS_NODE_DOCUMENT = 1,
  CS_NODE_ELEMENT,
  CS_NODE_COMMENT,
  CS_NODE_UNKNOWN,
  CS_NODE_TEXT,
  CS_NODE_DECLARATION
};

//===========================================================================

SCF_VERSION (iDocumentAttributeIterator, 0, 0, 1);

/**
 * An iterator over attributes.
 */
struct iDocumentAttributeIterator : public iBase
{
  /// Are there more elements?
  virtual bool HasNext () = 0;
  /// Get next element.
  virtual csRef<iDocumentAttribute> Next () = 0;
};

//===========================================================================

SCF_VERSION (iDocumentAttribute, 0, 0, 1);

/**
 * This represents an attribute for a document node.
 */
struct iDocumentAttribute : public iBase
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

SCF_VERSION (iDocumentNodeIterator, 0, 0, 1);

/**
 * An iterator over nodes.
 */
struct iDocumentNodeIterator : public iBase
{
  /// Are there more elements?
  virtual bool HasNext () = 0;
  /// Get next element.
  virtual csRef<iDocumentNode> Next () = 0;
};

//===========================================================================

SCF_VERSION (iDocumentNode, 0, 3, 0);

/**
 * This represents a node in a document.
 */
struct iDocumentNode : public iBase
{
  /**
   * Get the type of this node (one of CS_NODE_...).
   */
  virtual csDocumentNodeType GetType () = 0;

  /**
   * Get the value of this node.
   * What this is depends on the type of the node:
   * <ul>
   * <li>CS_NODE_DOCUMENT: filename of the xml file
   * <li>CS_NODE_ELEMENT: name of the element
   * <li>CS_NODE_COMMENT: comment text
   * <li>CS_NODE_UNKNOWN: tag contents
   * <li>CS_NODE_TEXT: text string
   * <li>CS_NODE_DECLARATION: undefined
   * </ul>
   */
  virtual const char* GetValue () = 0;
  /**
   * Set the value of this node.
   * What this is depends on the type of the node:
   * <ul>
   * <li>CS_NODE_DOCUMENT: filename of the xml file
   * <li>CS_NODE_ELEMENT: name of the element
   * <li>CS_NODE_COMMENT: comment text
   * <li>CS_NODE_UNKNOWN: tag contents
   * <li>CS_NODE_TEXT: text string
   * <li>CS_NODE_DECLARATION: undefined
   * </ul>
   */
  virtual void SetValue (const char* value) = 0;
  /// Get the parent.
  virtual csRef<iDocumentNode> GetParent () = 0;

  //---------------------------------------------------------------------
  
  /// Get an iterator over all children.
  virtual csRef<iDocumentNodeIterator> GetNodes () = 0;
  /// Get an iterator over all children of the specified type.
  virtual csRef<iDocumentNodeIterator> GetNodes (const char* type) = 0;
  /// Get the first node of the given type.
  virtual csRef<iDocumentNode> GetNode (const char* type) = 0;

  /// Remove a child.
  virtual void RemoveNode (const csRef<iDocumentNode>& child) = 0;
  /// Remove all children.
  virtual void RemoveNodes () = 0;

  /**
   * Create a new node of the given type before the given node.
   * If the given node is NULL then it will be added at the end.
   * Returns the new node or NULL if the given type is not valid
   * (CS_NODE_DOCUMENT is not allowed here for example).
   */
  virtual csRef<iDocumentNode> CreateNodeBefore (csDocumentNodeType type,
  	iDocumentNode* before) = 0;

  /**
   * Get the value of a node. What this does is scan all child nodes
   * and look for a node of type 'text'. It will return the text of that
   * node.
   */
  virtual const char* GetContentsValue () = 0;
  /**
   * Get the value of a node as int. What this does is scan all child nodes
   * and look for a node of type 'text'. It will return the text of that
   * node.
   */
  virtual int GetContentsValueAsInt () = 0;
  /**
   * Get the value of a node as float. What this does is scan all child nodes
   * and look for a node of type 'text'. It will return the text of that
   * node.
   */
  virtual float GetContentsValueAsFloat () = 0;

  //---------------------------------------------------------------------

  /// Get an iterator over all attributes.
  virtual csRef<iDocumentAttributeIterator> GetAttributes () = 0;
  /// Get an attribute by name.
  virtual csRef<iDocumentAttribute> GetAttribute (const char* name) = 0;
  /// Get an attribute value by name.
  virtual const char* GetAttributeValue (const char* name) = 0;
  /// Get an attribute value by name.
  virtual int GetAttributeValueAsInt (const char* name) = 0;
  /// Get an attribute value by name.
  virtual float GetAttributeValueAsFloat (const char* name) = 0;

  /// Remove an attribute.
  virtual void RemoveAttribute (const csRef<iDocumentAttribute>& attr) = 0;
  /// Remove all attributes.
  virtual void RemoveAttributes () = 0;

  /// Change or add an attribute.
  virtual void SetAttribute (const char* name, const char* value) = 0;
  /// Change or add an attribute as int.
  virtual void SetAttributeAsInt (const char* name, int value) = 0;
  /// Change or add an attribute as float.
  virtual void SetAttributeAsFloat (const char* name, float value) = 0;
};

//===========================================================================

SCF_VERSION (iDocument, 0, 1, 0);

/**
 * This represents a document.
 */
struct iDocument : public iBase
{
  /// Clear the document fully.
  virtual void Clear () = 0;

  /// Create a root node. This will clear the previous root node if any.
  virtual csRef<iDocumentNode> CreateRoot () = 0;

  /// Get the current root node.
  virtual csRef<iDocumentNode> GetRoot () = 0;

  /**
   * Parse document file from an iFile.
   * This will clear the previous root node if any.
   * Returns NULL if all is ok. Otherwise it will return an error
   * string.
   */
  virtual const char* Parse (iFile* file) = 0;

  /**
   * Parse document file from an iDataBuffer.
   * This will clear the previous root node if any.
   * Returns NULL if all is ok. Otherwise it will return an error
   * string.
   */
  virtual const char* Parse (iDataBuffer* buf) = 0;

  /**
   * Parse document file from an iString.
   * This will clear the previous root node if any.
   * Returns NULL if all is ok. Otherwise it will return an error
   * string.
   */
  virtual const char* Parse (iString* str) = 0;

  /**
   * Parse document file from a char array.
   * This will clear the previous root node if any.
   * Returns NULL if all is ok. Otherwise it will return an error
   * string.
   */
  virtual const char* Parse (const char* buf) = 0;

  /**
   * Write out document file to an iFile.
   * This will return NULL if all is ok. Otherwise it will return an
   * error string.
   */
  virtual const char* Write (iFile* file) = 0;

  /**
   * Write out document file to an iString.
   * This will return NULL if all is ok. Otherwise it will return an
   * error string.
   */
  virtual const char* Write (iString* str) = 0;

  /**
   * Write out document file to a VFS file.
   * This will return NULL if all is ok. Otherwise it will return an
   * error string.
   */
  virtual const char* Write (iVFS* vfs, const char* filename) = 0;
};

//===========================================================================

SCF_VERSION (iDocumentSystem, 0, 0, 1);

/**
 * The document system.
 */
struct iDocumentSystem : public iBase
{
  /// Create a new empty document.
  virtual csRef<iDocument> CreateDocument () = 0;
};


#endif // __IUTIL_DOCUMENT_H__

