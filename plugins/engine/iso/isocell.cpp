/*
    Copyright (C) 2001 by W.C.A. Wijngaards

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

#include "cssysdef.h"
#include "isocell.h"
#include "ivideo/graph3d.h"

SCF_IMPLEMENT_IBASE (csIsoCell)
  SCF_IMPLEMENTS_INTERFACE (iIsoCell)
SCF_IMPLEMENT_IBASE_END

csIsoCellNode::~csIsoCellNode ()
{
  if (drawpart) drawpart->DecRef ();
}

csIsoCell::csIsoCell (iBase *iParent)
{
  SCF_CONSTRUCT_IBASE (iParent);
  root = 0;
}

/// helpder deleting function.
static void deletenode(csIsoCellNode* node, void * /*data*/)
{
  delete node;
}

csIsoCell::~csIsoCell ()
{
  /// traverse in post order and delete all nodes
  if(root)
    TraversePost(root, deletenode, (void*)0);
  SCF_DESTRUCT_IBASE();
}

void csIsoCell::AddSprite(iIsoSprite *sprite, const csVector3& pos)
{
  /// same in right subtree....
  csIsoCellNode *p = root, *parent = 0;
  float inserty = pos.y;
  /// find a spot
  while(p)
  {
    parent = p;
    if(inserty < p->drawpart->GetPosition().y)
      p = p->left;
    else p = p->right;
  }
  /// create new node;
  csIsoCellNode *newnode = new csIsoCellNode;
  newnode->drawpart = sprite;
  newnode->left = 0;
  newnode->right = 0;
  sprite->IncRef ();

  if(!parent)
  {
    /// inserted becomes the new root
    root = newnode;
  }
  else
  {
    /// got parent
    if(inserty < parent->drawpart->GetPosition().y)
      parent->left = newnode;
    else parent->right = newnode;
  }
}


void csIsoCell::RemoveSprite(iIsoSprite *sprite, const csVector3& pos)
{
  csIsoCellNode *p = root, *parent = 0;
  float removey = pos.y;
  /// first find the node
  while(p && (p->drawpart != sprite))
  {
    parent = p;
    if(removey < p->drawpart->GetPosition().y)
      p = p->left;
    else p = p->right;
  }
  if(p==0) return; /// not found, nothing to do.
  if(p->left==0)
  {
    if(!parent) root = p->right;
    else if(removey < parent->drawpart->GetPosition().y)
      parent->left = p->right;
    else parent->right = p->right;
  }
  else if(p->right == 0)
  {
    if(!parent) root = p->left;
    else if(removey < parent->drawpart->GetPosition().y)
      parent->left = p->left;
    else parent->right = p->left;
  }
  else
  {
    /// p has both left and right subtrees
    /// since possibly some drawpart with the same y can be in
    /// the right subtree, a member from that tree must replace this one.
    /// the leftmost member of the right subtree is thus used.
    csIsoCellNode *leftmost = p->right, *leftmostparent = 0;
    while(leftmost && leftmost->left)
    {
      leftmostparent = leftmost;
      leftmost = leftmost->left;
    }
    // found leftmost of right subtree, swap it in.
    p->drawpart = leftmost->drawpart;
    if(!leftmostparent) p->right = leftmost->right;
    else leftmostparent->left = leftmost->right;
    delete leftmost;
  }
  sprite->DecRef ();
}


/// helper render func
static void renderfunc(csIsoCellNode *node, void *data)
{
  iIsoRenderView* rview = (iIsoRenderView*)data;
  if( (rview->GetRenderPass()==CSISO_RENDERPASS_MAIN)
    && ((node->drawpart->GetMixMode()&CS_FX_MASK_MIXMODE) == CS_FX_COPY))
    node->drawpart->Draw((iIsoRenderView*)data);
  else if( (rview->GetRenderPass()==CSISO_RENDERPASS_FG)
    && ((node->drawpart->GetMixMode()&CS_FX_MASK_MIXMODE) != CS_FX_COPY))
    node->drawpart->Draw((iIsoRenderView*)data);
}

void csIsoCell::Draw(iIsoRenderView *rview)
{
  //printf("IsoCell::Draw()\n");
  if(root) TraverseInOrder(root, renderfunc, (void*)rview);
}

void csIsoCell::TraversePost(csIsoCellNode *tree,
  void (*func)(csIsoCellNode *, void *), void *data)
{
  if(tree->left) TraversePost(tree->left, func, data);
  if(tree->right) TraversePost(tree->right, func, data);
  func(tree, data);
}

void csIsoCell::TraverseInOrder(csIsoCellNode *tree,
  void (*func)(csIsoCellNode *, void *), void *data)
{
  if(tree->left) TraverseInOrder(tree->left, func, data);
  func(tree, data);
  if(tree->right) TraverseInOrder(tree->right, func, data);
}


static void callfunc(csIsoCellNode *node, void *data)
{
  iIsoCellTraverseCallback* func = (iIsoCellTraverseCallback*)data;
  func->Traverse (node->drawpart);
}
void csIsoCell::Traverse(iIsoCellTraverseCallback* func)
{
  if(root)
    TraverseInOrder(root, callfunc, (void*)func);
}

