/*
    Copyright (C) 2000 by Norman Krämer

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

#include "csutil/csvector.h"

/**
 * A generic tree class.
 */
class csTreeNode
{
 public:

  bool IsLeaf ()
  { return children.Length () == 0; }

  void RemoveChild (csTreeNode *child)
  { int idx = children.Find (child); if (idx != -1) children.Delete (idx); }

  void AddChild (csTreeNode *child)
  { children.Push (child); child->parent = this; }

  csTreeNode (csTreeNode *theParent=NULL)
  { parent=theParent; if (parent) parent->children.Push (this); }

  virtual ~csTreeNode ()
  {
	int i;
    for(i=children.Length ()-1; i>=0; i--)
      delete (csTreeNode*)children.Get (i);
    if (parent)
      parent->RemoveChild (this);
  }

  /**
   * Execute a function on this node and its children. Do this in "DepthSearchFirst" order, that is check a childs children
   * before testing the next direct child.
   * Returns the last node where TreeFunc resulted in TRUE. If stopOnSuccess is true, then execution is stoped after first
   * successful execution of TreeFunc.
   * SelBranch lets you decide which children to select for further investugation. NULL means all children.
   */
  csTreeNode *DSF (bool (*TreeFunc)(csTreeNode *node, csSome param, bool stopOnSuccess),
		   bool (*SelBranch)(csTreeNode *node), csSome param, bool stopOnSuccess)
  {
    csTreeNode *foundNode = NULL;
    int i=0;
    bool dive;
    if (TreeFunc (this, param, stopOnSuccess))
      foundNode = this;
    while (i<children.Length () && !(foundNode && stopOnSuccess))
    {
      dive = (SelBranch == NULL) || SelBranch ((csTreeNode*)children.Get (i));
      if (dive)
        foundNode = ((csTreeNode*)children.Get (i))->DSF (TreeFunc, SelBranch,
	                                                  param, stopOnSuccess);
	i++;
    }
    return foundNode;
  }

  /**
   * Execute a function on this node and its children. Do this in "BreadthSearchFirst" order, that is check first all
   * direct children before diving into subchildren.
   * Returns the last node where TreeFunc resulted in TRUE. If stopOnSuccess is true, then execution is stoped after first
   * successful execution of TreeFunc.
   * SelBranch lets you decide which children to select for further investugation. NULL means all children.
   */
  csTreeNode *BSF (bool (*TreeFunc)(csTreeNode *node, csSome param, bool stopOnSuccess),
		   bool (*SelBranch)(csTreeNode *node), csSome param, bool stopOnSuccess)
  {
    csTreeNode *node, *foundNode = NULL;
    csVector fifo;

    fifo.Push (this);
    while (fifo.Length () > 0 && !(foundNode && stopOnSuccess))
    {
      node = (csTreeNode*)fifo.Get (0); fifo.Delete (0);
      if (TreeFunc (node, param, stopOnSuccess))
        foundNode = node;
      if (!node->IsLeaf () && (SelBranch==NULL || SelBranch (node)))
	  {
		int i;
        for (i=0; i < node->children.Length (); i++ )
          fifo.Push (node->children.Get (i));
	  }
    }
    fifo.DeleteAll ();
    return foundNode;
  }

 public:
  csTreeNode *parent; // parent node or NULL if toplevel
  csVector children; // node children
};

#endif // __CS_CSTREENODE_H__
