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

///
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
  
  ///
  csObjTree(csObject* csobj = NULL);

  ///
  ~csObjTree();

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
  csObject* GetObject(int i) { return children[i]; }

  ///
  int GetNumSubNodes() const { return subnode_cnt; }
  
  ///
  csObjTree* GetSubNode(int i) { return subnodes[i]; }

  /// Returns 'true' if this tree contains no csObjects.
  bool IsEmpty();

  /// Add the csObject to the tree.
  void ObjAdd(csObject* csobj);

  /// Release the the csObject from the tree.
  void ObjRelease(csObject* csobj);

  /// Returns the polymorphic type of this node
  const csIdType& GetType() const { return nodetype; }
};

#endif /* __OBJTREE_H_ */
