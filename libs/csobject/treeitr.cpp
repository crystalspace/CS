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

#include "csobject/treeitr.h"

csObjIterator::csObjItrNode::csObjItrNode(csObjTree* node) :
  next(NULL), treenode(node), child_ind(0), subnode_ind(0) {}

csObjIterator::csObjItrNode::~csObjItrNode()
{  if (next) CHKB(delete next); }

csObjIterator::csObjIterator(csObjTree* node)
{
  if (node) CHKB(top = new csObjItrNode(node))
  else top = NULL;
  NodeSeek();
}

csObjIterator::csObjIterator(const csObjIterator& iter)
{
  csObjItrNode* node = iter.top;
  csObjItrNode** tptr = &top;
  while (node)
  {
    CHK(*tptr = new csObjItrNode(*node));
    node = node->next;
    tptr = &((*tptr)->next);
  }
  *tptr = NULL;
}

csObjIterator::~csObjIterator()
{ if (top) CHKB(delete top); }

bool csObjIterator::IsNull() const
{
  return (top==NULL);
}

csObject& csObjIterator::operator* ()
{
  /* Yes, this does cause an error if 'top' is NULL.  As it should.
     Compare to dereferencing a NULL pointer. */
  return *(top->treenode->GetObject(top->child_ind));
}

csObjIterator& csObjIterator::operator++ () 
{
  if (!top) return *this; 
  if (top->child_ind < top->treenode->GetNumObjects()) top->child_ind++; 
  NodeSeek();
  return *this;
}

void csObjIterator::NodeSeek()
{
  while (top && top->child_ind >= top->treenode->GetNumObjects())
  {
    if (top->subnode_ind < top->treenode->GetNumSubNodes())
    {
      csObjTree* tmptree = top->treenode->GetSubNode(top->subnode_ind);
      CHK(csObjItrNode* tmpnode = new csObjItrNode(tmptree));
      tmpnode->next = top;  top = tmpnode;
    }
    else
    {
      csObjItrNode* tmpnode = top;
      top = top->next; 
      tmpnode->next = NULL;
      CHK(delete tmpnode);

      if (top) top->subnode_ind++;
    }
  }
}

