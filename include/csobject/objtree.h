/*
    Copyright (C) 1998 by Jorrit Tyberghein
    csObject library (C) 1999 by Ivan Avramovic <ivan@avramovic.com>
  
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

#ifndef __OBJTREE_H_
#define __OBJTREE_H_

#include "csobject/fakertti.h"

class csObject;

/**
 * A tree of csObjects, used internally by the csObject class.
 */
class csObjTree
{
protected:
  ///
  csObjTree*   *subnodes;
  ///
  int           subnode_cnt;
  ///
  csObject*    *children;
  ///
  int           child_cnt;
  ///
  csIdType      nodetype;
    

  /// If this is a redundant node in the tree, remove it.
  void CollapseTree();

public:
  
  /**
   * Create a new object tree.
   * If csobj is non-NULL, the object tree is started with the single object
   * 'csobj' in it.
   */
  csObjTree(csObject* csobj = NULL);

  ///
  virtual ~csObjTree();

  /**
   * Returns a csObjTree which points to the same data as this tree.
   * The copy of the csObjTree functions the same as the original, except
   * that the csObject data within is not freed when the csObjTree is 
   * destroyed.  
   * If the original tree is destroyed before the copy, then the data in the
   * copy is no longer valid.
   */
  csObjTree* GetCopy();

  /**
   * Returns the tree node representing the given object type.
   * This function will return the highest-level node in the tree
   * which contains only objects which are of the polymorphic type
   * (or children of the polymorphic type) given by objtype.  Returns
   * NULL if no such node exists.
   */
  csObjTree* GetNode(const csIdType& objtype);
    
  ///
  int GetNumObjects() const { return child_cnt; }

  ///
  csObject* GetObj(int i) { return children[i]; }

  ///
  int GetNumSubNodes() const { return subnode_cnt; }
  
  ///
  csObjTree* GetSubNode(int i) { return subnodes[i]; }

  /// Returns 'true' if this tree contains no csObjects.
  bool IsEmpty();

  /// Add the csObject to the tree.
  virtual void ObjAdd(csObject* csobj);

  /// Release the the csObject from the tree.
  virtual void ObjRelease(csObject* csobj);

  /// Returns the polymorphic type of this node
  const csIdType& GetType() const { return nodetype; }
};

#endif /* __OBJTREE_H_ */
