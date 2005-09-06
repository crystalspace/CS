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

#ifndef __CS_IUTIL_DOCUMENT_H__
#define __CS_IUTIL_DOCUMENT_H__

/**\file
 * Document Interface
 */
/**\addtogroup util
 * @{ */
#include "csutil/scf.h"

struct iDocumentNode;
struct iDocumentAttribute;
struct iFile;
struct iDataBuffer;
struct iString;
struct iVFS;

/**
 * Possible node types for iDocumentNode.
 */
enum csDocumentNodeType
{
  /// Document
  CS_NODE_DOCUMENT = 1,
  /// Element
  CS_NODE_ELEMENT,
  /// Comment
  CS_NODE_COMMENT,
  /// Unknown type
  CS_NODE_UNKNOWN,
  /// Text
  CS_NODE_TEXT,
  /// Declaration
  CS_NODE_DECLARATION
};

/** \name Document changeabilty
 * @{
 */
/// The document can not be changed, CreateRoot() is not supported.
#define CS_CHANGEABLE_NEVER		0
/// The document only allows changes with a newly created root.
#define CS_CHANGEABLE_NEWROOT		1
/// The document can be changed.
#define CS_CHANGEABLE_YES		2
/** @} */

//===========================================================================

SCF_VERSION (iDocumentAttributeIterator, 0, 0, 1);

/**
 * An iterator over iDocumentNode attributes.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>iDocumentNode::GetAttributes()
 *   </ul>
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
 * An attribute for an iDocumentNode.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>iDocumentNode::SetAttribute()
 *   <li>iDocumentNode::SetAttributeAsInt()
 *   <li>iDocumentNode::SetAttributeAsFloat()
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>iDocumentNode::GetAttribute()
 *   <li>iDocumentAttributeIterator::Next()
 *   </ul>
 */
struct iDocumentAttribute : public iBase
{
  /// Get name of this attribute.
  virtual const char* GetName () = 0;
  /// Get value of this attribute.
  virtual const char* GetValue () = 0;
  /// Get value of this attribute as integer.
  virtual int GetValueAsInt () = 0;
  /// Get value of this attribute as float.
  virtual float GetValueAsFloat () = 0;
  /// Get value of this attribute as float.
  virtual bool GetValueAsBool () = 0;
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
 * An iterator over iDocumentNode.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>iDocumentNode::GetNodes()
 *   </ul>
 */
struct iDocumentNodeIterator : public virtual iBase
{
  /// Are there more elements?
  virtual bool HasNext () = 0;
  /// Get next element.
  virtual csRef<iDocumentNode> Next () = 0;
};

//===========================================================================

SCF_VERSION (iDocumentNode, 0, 4, 1);

/**
 * Representation of a node in a document.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>iDocument::CreateRoot()
 *   <li>iDocumentNode::CreateNodeBefore()
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>iDocument::GetRoot()
 *   <li>iDocumentNode::GetNode()
 *   <li>iDocumentNode::GetParent()
 *   <li>iDocumentNodeIterator::Next()
 *   </ul>
 */
struct iDocumentNode : public iBase
{
  /**
   * Get the type of this node (one of CS_NODE_...).
   */
  virtual csDocumentNodeType GetType () = 0;

  /**
   * Compare this node with another node. You have to use this
   * function to compare document node objects as equality on the
   * iDocumentNode pointer itself doesn't work in all cases
   * (iDocumentNode is just a wrapper of the real node in some
   * implementations).
   * \remarks Does *not* check equality of contents!
   */
  virtual bool Equals (iDocumentNode* other) = 0;

  /**
   * Get the value of this node.
   * What this is depends on the type of the node:
   * <ul>
   * <li>#CS_NODE_DOCUMENT: filename of the document file
   * <li>#CS_NODE_ELEMENT: name of the element
   * <li>#CS_NODE_COMMENT: comment text
   * <li>#CS_NODE_UNKNOWN: tag contents
   * <li>#CS_NODE_TEXT: text string
   * <li>#CS_NODE_DECLARATION: undefined
   * </ul>
   */
  virtual const char* GetValue () = 0;
  /**
   * Set the value of this node.
   * What this is depends on the type of the node:
   * <ul>
   * <li>#CS_NODE_DOCUMENT: filename of the document file
   * <li>#CS_NODE_ELEMENT: name of the element
   * <li>#CS_NODE_COMMENT: comment text
   * <li>#CS_NODE_UNKNOWN: tag contents
   * <li>#CS_NODE_TEXT: text string
   * <li>#CS_NODE_DECLARATION: undefined
   * </ul>
   */
  virtual void SetValue (const char* value) = 0;
  /// Set value to the string representation of an integer.
  virtual void SetValueAsInt (int value) = 0;
  /// Set value to the string representation of a float.
  virtual void SetValueAsFloat (float value) = 0;

  /// Get the parent.
  virtual csRef<iDocumentNode> GetParent () = 0;

  //---------------------------------------------------------------------
  
  /** 
   * Get an iterator over all children.
   * \remarks Never returns 0.
   */
  virtual csRef<iDocumentNodeIterator> GetNodes () = 0;
  /**
   * Get an iterator over all children of the specified value.
   * \remarks Never returns 0.
   */
  virtual csRef<iDocumentNodeIterator> GetNodes (const char* value) = 0;
  /// Get the first node of the given value.
  virtual csRef<iDocumentNode> GetNode (const char* value) = 0;

  /// Remove a child.
  virtual void RemoveNode (const csRef<iDocumentNode>& child) = 0;
  /// Remove all children returned by iterator
  virtual void RemoveNodes (csRef<iDocumentNodeIterator> children) = 0;
  /// Remove all children.
  virtual void RemoveNodes () = 0;

  /**
   * Create a new node of the given type before the given node.
   * If the given node is 0 then it will be added at the end.
   * Returns the new node or 0 if the given type is not valid
   * (CS_NODE_DOCUMENT is not allowed here for example).
   */
  virtual csRef<iDocumentNode> CreateNodeBefore (csDocumentNodeType type,
  	iDocumentNode* before = 0) = 0;

  /**
   * Get the value of a node.  Scans all child nodes and looks for a node of
   * type 'text', and returns the text of that node. WARNING! Make a copy
   * of this text if you intend to use it later. The document parsing system
   * does not guarantee anything about the lifetime of the returned string.
   */
  virtual const char* GetContentsValue () = 0;
  /**
   * Get the value of a node as an integer.  Scans all child nodes and looks
   * for a node of type 'text', converts the text of that node to a number and
   * returns the represented integer.
   */
  virtual int GetContentsValueAsInt () = 0;
  /**
   * Get the value of a node as float.  Scans all child nodes and looks for a
   * node of type 'text', converts the text of that node to a number and
   * returns the represented floating point value.
   */
  virtual float GetContentsValueAsFloat () = 0;

  //---------------------------------------------------------------------

  /**
   * Get an iterator over all attributes.
   * \remarks Never returns 0.
   */
  virtual csRef<iDocumentAttributeIterator> GetAttributes () = 0;
  /// Get an attribute by name.
  virtual csRef<iDocumentAttribute> GetAttribute (const char* name) = 0;
  /// Get an attribute value by name as a string.
  virtual const char* GetAttributeValue (const char* name) = 0;
  /// Get an attribute value by name as an integer.
  virtual int GetAttributeValueAsInt (const char* name) = 0;
  /// Get an attribute value by name as a floating point value.
  virtual float GetAttributeValueAsFloat (const char* name) = 0;
  /**
   * Get an attribute value by name as a bool.  "yes", "true", and "1" all
   * are returned as true.
   */
  virtual bool GetAttributeValueAsBool (const char* name,
  	bool defaultvalue=false) = 0;

  /// Remove an attribute.
  virtual void RemoveAttribute (const csRef<iDocumentAttribute>& attr) = 0;
  /// Remove all attributes.
  virtual void RemoveAttributes () = 0;

  /// Change or add an attribute.
  virtual void SetAttribute (const char* name, const char* value) = 0;
  /// Change or add an attribute to a string representation of an integer.
  virtual void SetAttributeAsInt (const char* name, int value) = 0;
  /// Change or add an attribute to a string representation of a float.
  virtual void SetAttributeAsFloat (const char* name, float value) = 0;
};

//===========================================================================

SCF_VERSION (iDocument, 0, 2, 0);

/**
 * Representation of a document containing a hierarchical structure of nodes.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>iDocumentSystem::CreateDocument()
 *   </ul>
 */
struct iDocument : public iBase
{
  /// Clear the document.
  virtual void Clear () = 0;

  /// Create a root node. This will clear the previous root node if any.
  virtual csRef<iDocumentNode> CreateRoot () = 0;

  /**
   * Get the current root node.
   * \remarks Never returns 0.
   */
  virtual csRef<iDocumentNode> GetRoot () = 0;

  /**
   * Parse document file from an iFile.
   * \param file The file to parse. It should contain data in whatever format
   *   is understood by the iDocument implementation receiving this invocation.
   * \param collapse Some document format implementations may have the option
   *   of condensing extraneous whitespace in returned #CS_NODE_TEXT nodes.
   *   Set this to true to enable this option.  By default, whitepsace is
   *   preserved.
   * \return 0 if all is okay; otherwise an error message.
   * \remarks This will clear the previous root node if any.
   */
  virtual const char* Parse (iFile* file, bool collapse = false) = 0;

  /**
   * Parse document file from an iDataBuffer.
   * \param buf The buffer to parse. It should contain data in whatever format
   *   is understood by the iDocument implementation receiving this invocation.
   * \param collapse Some document format implementations may have the option
   *   of condensing extraneous whitespace in returned #CS_NODE_TEXT nodes.
   *   Set this to true to enable this option.  By default, whitepsace is
   *   preserved.
   * \return 0 if all is okay; otherwise an error message.
   * \remarks This will clear the previous root node if any.
   */
  virtual const char* Parse (iDataBuffer* buf, bool collapse = false) = 0;

  /**
   * Parse document file from an iString.
   * \param str The string to parse. It should contain data in whatever format
   *   is understood by the iDocument implementation receiving this invocation.
   * \param collapse Some document format implementations may have the option
   *   of condensing extraneous whitespace in returned #CS_NODE_TEXT nodes.
   *   Set this to true to enable this option.  By default, whitepsace is
   *   preserved.
   * \return 0 if all is okay; otherwise an error message.
   * \remarks This will clear the previous root node if any.
   */
  virtual const char* Parse (iString* str, bool collapse = false) = 0;

  /**
   * Parse document file from a null-terminated C-string.
   * \param buf The buffer to parse. It should contain data in whatever format
   *   is understood by the iDocument implementation receiving this invocation.
   * \param collapse Some document format implementations may have the option
   *   of condensing extraneous whitespace in returned #CS_NODE_TEXT nodes.
   *   Set this to true to enable this option.  By default, whitepsace is
   *   preserved.
   * \return 0 if all is okay; otherwise an error message.
   * \remarks This will clear the previous root node if any.
   */
  virtual const char* Parse (const char* buf, bool collapse = false) = 0;

  /**
   * Write out document file to an iFile.
   * This will return 0 if all is ok. Otherwise it will return an
   * error string.
   */
  virtual const char* Write (iFile* file) = 0;

  /**
   * Write out document file to an iString.
   * This will return 0 if all is ok. Otherwise it will return an
   * error string.
   */
  virtual const char* Write (iString* str) = 0;

  /**
   * Write out document file to a VFS file.
   * This will return 0 if all is ok. Otherwise it will return an
   * error string.
   */
  virtual const char* Write (iVFS* vfs, const char* filename) = 0;
  
  /**
   * Returns how far this document can be changed.
   * \sa
   * 	#CS_CHANGEABLE_NEVER
   * 	#CS_CHANGEABLE_NEWROOT
   * 	#CS_CHANGEABLE_YES
   */
  virtual int Changeable () = 0;
};

//===========================================================================

SCF_VERSION (iDocumentSystem, 0, 0, 1);

/**
 * An iDocument factory.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>XmlRead Document System plugin (crystalspace.documentsystem.xmlread)
 *   <li>XmlTiny Document System plugin (crystalspace.documentsystem.tinyxml)
 *   <li>Binary Document System plugin (crystalspace.documentsystem.binary)
 *   <li>Document System Multiplexer plugin
 *       (crystalspace.documentsystem.multiplexer)
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>CS_QUERY_REGISTRY()
 *   </ul>
 */
struct iDocumentSystem : public virtual iBase
{
  /// Create a new empty document.
  virtual csRef<iDocument> CreateDocument () = 0;
};

/** @} */

#endif // __CS_IUTIL_DOCUMENT_H__
