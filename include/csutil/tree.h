/*
    Copyright (C) 2000 by Norman Kraemer

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

#ifndef __CS_CSTREENODE_H__
#define __CS_CSTREENODE_H__

/**\file
 * Generic tree class
 */

#include "csextern.h"
#include "array.h"

/**
 * A generic tree class.
 */
class csTreeNode
{
public:
  /// Returns true if this node has no children.
  bool IsLeaf ()
  { return children.Length () == 0; }

  /// Remove a child node.
  void RemoveChild (csTreeNode *child)
  {
    size_t idx = children.Find (child);
    if (idx != csArrayItemNotFound) children.DeleteIndex (idx);
  }

  /// Add a child node.
  void AddChild (csTreeNode *child)
  { children.Push (child); child->parent = this; }

  /// Create node, optionally as a child of <code>theParent</code>.
  csTreeNode (csTreeNode *theParent=0)
  { parent=theParent; if (parent) parent->children.Push (this); }

  virtual ~csTreeNode ()
  {
    size_t i;
    for(i=children.Length (); i>0; i--)
      delete children.Get (i - 1);
    if (parent)
      parent->RemoveChild (this);
  }

  /**
   * Execute a function on this node and its children. Do this in
   * "DepthSearchFirst" order, that is check a childs children
   * before testing the next direct child.
   * Returns the last node where TreeFunc resulted in TRUE.
   * If stopOnSuccess is true, then execution is stoped after first
   * successful execution of TreeFunc.
   * SelBranch lets you decide which children to select for further
   * investigation. 0 means all children.
   */
  csTreeNode *DSF (bool (*TreeFunc)(csTreeNode *node, void* param,
  					bool stopOnSuccess),
		   bool (*SelBranch)(csTreeNode *node), void* param,
		   			bool stopOnSuccess)
  {
    csTreeNode *foundNode = 0;
    size_t i=0;
    bool dive;
    if (TreeFunc (this, param, stopOnSuccess))
      foundNode = this;
    while (i<children.Length () && !(foundNode && stopOnSuccess))
    {
      dive = (SelBranch == 0) || SelBranch (children[i]);
      if (dive)
        foundNode = (children[i])->DSF (TreeFunc, SelBranch,
		param, stopOnSuccess);
      i++;
    }
    return foundNode;
  }

  /**
   * Execute a function on this node and its children. Do this in
   * "BreadthSearchFirst" order, that is check first all
   * direct children before diving into subchildren.
   * Returns the last node where TreeFunc resulted in TRUE.
   * If stopOnSuccess is true, then execution is stoped after first
   * successful execution of TreeFunc.
   * SelBranch lets you decide which children to select for further
   * investugation. 0 means all children.
   */
  csTreeNode *BSF (bool (*TreeFunc)(csTreeNode *node, void* param,
  				bool stopOnSuccess),
		   bool (*SelBranch)(csTreeNode *node), void* param,
		   		bool stopOnSuccess)
  {
    csTreeNode *node, *foundNode = 0;
    csArray<csTreeNode*> fifo;

    fifo.Push (this);
    while (fifo.Length () > 0 && !(foundNode && stopOnSuccess))
    {
      node = fifo[0]; fifo.DeleteIndex (0);
      if (TreeFunc (node, param, stopOnSuccess))
        foundNode = node;
      if (!node->IsLeaf () && (SelBranch==0 || SelBranch (node)))
      {
	size_t i;
        for (i=0; i < node->children.Length (); i++ )
          fifo.Push (node->children[i]);
      }
    }
    fifo.DeleteAll ();
    return foundNode;
  }

 public:
  csTreeNode *parent; // parent node or 0 if toplevel
  csArray<csTreeNode*> children; // node children
};

#endif // __CS_CSTREENODE_H__
