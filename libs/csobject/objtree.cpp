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

#include "sysdef.h"
#include "csobject/objtree.h"
#include "csobject/csobj.h"

csObjTree::csObjTree (csObject* csobj) : 
  subnodes(NULL), subnode_cnt(0), children(NULL), child_cnt(0), nodetype()
{
  if (csobj)
  {
    CHK (children = new csObject * [1]);
    nodetype = csobj->GetType();
    child_cnt++;
    children [0] = csobj;
  }
}

csObjTree* csObjTree::GetNode(const csIdType& objtype)
{
  if (objtype <= nodetype) return this;
  if (subnode_cnt == 0) return NULL;
  if ( !(objtype >= nodetype) ) return NULL;
    
/* The following version is slightly more efficient, but requires a more
   exposed version of csIdType. */
//    if (objtype.Length() <= nodetype.Length())
//      return (*objtype == nodetype[objtype.Length()]) ? this : NULL;
//
//    if (subnode_cnt == 0) return NULL;
//    if (objtype[nodetype.Length()] != *nodetype) return NULL;

  csObjTree* node = NULL;
  for (int i=0;  i<subnode_cnt;  i++)
    if ((node = subnodes[i]->GetNode(objtype)) != NULL) break;
  return node; 
} 

bool csObjTree::IsEmpty()
{
  if (child_cnt > 0) return false;
  for (int i=0;  i<subnode_cnt;  i++)
    if (! subnodes[i]->IsEmpty()) return false;
  return true;
}

csObjTree::~csObjTree()
{
  if (children)
  {
    for (int i = 0;  i < child_cnt;  i++)
      CHKB (delete children [i]);
    CHK (delete [] children);
  }
  if (subnodes) 
  {
    for (int i = 0;  i < subnode_cnt; i++)
      if (subnodes [i]) CHKB (delete subnodes [i]);
    CHK (delete [] subnodes);
  }
}

void csObjTree::ObjAdd(csObject* csobj)
{
  if (!csobj) return;
  if (IsEmpty())
  {
    nodetype = csobj->GetType();
    CHK(children = new csObject*[1]);
    if (children) { child_cnt++;  children[0] = csobj; }
    return;
  }
  if (csobj->GetType() == nodetype)
  {
    CHK(csObject* *newlist = new csObject*[child_cnt+1]);
    if (newlist)
    {
      for (int i=0;  i<child_cnt;  i++) newlist[i] = children[i];
      newlist[child_cnt] = csobj;
      child_cnt++;
      if (children) CHKB(delete[] children);
      children = newlist;
    }
    return;
  }
  if (csobj->GetType() > nodetype)
  {
    int i;
    for (i = 0;  i<subnode_cnt;  i++)
      if (csobj->GetType() >= subnodes[i]->GetType())
      {
        subnodes[i]->ObjAdd(csobj); 
        return;
      }
    for (i = 0;  i<subnode_cnt;  i++)
      if (! (csIdFunc::GetCommon(csobj->GetType(), subnodes[i]->GetType()) ==
             nodetype) )
      {
        subnodes[i]->ObjAdd(csobj);
        return;
      }

    CHK(csObjTree* *newsubnodes = new csObjTree*[subnode_cnt+1]);
    if (newsubnodes)
    {
      for (i=0;  i<subnode_cnt;  i++) newsubnodes[i] = subnodes[i];
      CHK(newsubnodes[subnode_cnt] = new csObjTree(csobj));
      subnode_cnt++;
      if (subnodes) CHKB(delete[] subnodes);
      subnodes = newsubnodes;
    } 
    return;
  }
  
  /// copy the current tree into a new object
  CHK(csObjTree* newnode = new csObjTree(*this)); 
  CHK(subnodes = new csObjTree*[2]);
  child_cnt = 0;  children = NULL;  subnode_cnt = 2;
  subnodes[0] = newnode;
  CHK(subnodes[1] = new csObjTree(csobj));
  nodetype = csIdFunc::GetCommon(csobj->GetType(), nodetype);
}

void csObjTree::ObjRelease(csObject* csobj)
{
  if (!csobj) return;

  /// First, check if the csObject is in the current node.

  if (csobj->GetType() == nodetype) 
  {
    for (int i=0;  i<child_cnt;  i++)
      if (children[i] == csobj)
      {
        if (--child_cnt) children[i] = children[child_cnt];
        else
        {
          CHK(delete[] children); 
          children = NULL;
          CollapseTree();
        }
        break; 
      }
    return;
  }

  /// The object is not in the current node, so check the subnodes.

  for (int i=0;  i<subnode_cnt;  i++)
    if (csobj->GetType() >= subnodes[i]->GetType())
    {
      subnodes[i]->ObjRelease(csobj);
      if (subnodes[i]->IsEmpty())
      {
        CHK(delete subnodes[i]);
        if (--subnode_cnt) 
        {
          subnodes[i] = subnodes[subnode_cnt];
          CollapseTree();
        }
        else 
        {
          CHKB(delete[] subnodes);
          subnodes = NULL;
        }
      } 
      break;
    }
}

void csObjTree::CollapseTree()
{
  if (child_cnt > 0 || subnode_cnt != 1) return;
  if (children) CHKB(delete[] children);
  csObjTree* *tempnodes = subnodes;
 
  // copy initialize
  *this = *(tempnodes[0]);

  tempnodes[0]->child_cnt = tempnodes[0]->subnode_cnt = 0;
  tempnodes[0]->children = NULL;
  tempnodes[0]->subnodes = NULL;

  CHK(delete tempnodes[0]);
  CHK(delete[] tempnodes);
}

class csObjTreeCopy : public csObjTree
{
protected:
  csObjTree* orig_tree;
public:
  csObjTreeCopy(csObjTree& objtree) 
  : csObjTree(objtree), orig_tree(&objtree) {}

  ~csObjTreeCopy()
  { 
    children = NULL;
    subnodes = NULL;
  }

  void Update()
  { *this = *orig_tree; }

  virtual void ObjAdd(csObject* csobj)
  { orig_tree->ObjAdd(csobj);  Update(); } 
  virtual void ObjRelease(csObject* csobj)
  { orig_tree->ObjRelease(csobj);  Update(); }
};

csObjTree* csObjTree::GetCopy()
{
  CHK(csObjTree* otree = new csObjTreeCopy(*this));
  return otree;
}
