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

#ifndef __TREEITR_H_
#define __TREEITR_H_

#include "csobject/objtree.h"

///
class csObjIterator
{
protected:
  ///
  class csObjItrNode
  {
  public:
    ///
    csObjItrNode* next;
    ///
    csObjTree* treenode;
    ///
    int child_ind, subnode_ind;
    ///
    csObjItrNode(csObjTree* node);
    ///
    ~csObjItrNode();
  };
  ///
  csObjItrNode* top; 

  /// Seeks the first node in the tree that contains at least one csObject.
  void NodeSeek();

public:

  ///
  csObjIterator(csObjTree* node);
  ///
  csObjIterator(const csObjIterator& iter);

  ///
  ~csObjIterator();
  ///
  bool IsNull() const;
  ///
  csObject& operator* ();
  ///
  csObjIterator& operator++ ();
};

#endif /* __TREEITR_H_ */
