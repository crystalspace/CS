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

#ifndef __CS_ISOCELL_H__
#define __CS_ISOCELL_H__

#include "csutil/scf.h"
#include "ivaria/iso.h"

/// structure for use in cell tree
struct csIsoCellNode {
  /// left & right subtrees
  csIsoCellNode *left, *right;
  /// value in this node
  iIsoSprite *drawpart;
  csIsoCellNode (){left = right = 0; drawpart = 0;}
  ~csIsoCellNode ();
};

/**
 * iso cell, a 1.0x1.0 space in the world.
 * Internally sorted on y.
 * same y is put in the right branch - so that the same y - but added
 * later, is put in the 'larger' branch - and drawn later.
 * When a drawing part changes position, it must be removed *before*
 * changin the position, and added again after changing to the new position.
*/
class csIsoCell : public iIsoCell {
private:
  /// The tree
  csIsoCellNode *root;

  /// Traverse in post-order & call the routine (sprite, data).
  void TraversePost(csIsoCellNode *tree, void (*func)(csIsoCellNode *, void *),
    void *data);
  /// Traverse in-order & call the routine (sprite, data).
  void TraverseInOrder(csIsoCellNode *tree,
    void (*func)(csIsoCellNode *, void *), void *data);

public:
  SCF_DECLARE_IBASE;

  ///
  csIsoCell (iBase *iParent);
  ///
  virtual ~csIsoCell ();

  //----- iIsoCell -----------------------------------------------
  virtual void AddSprite(iIsoSprite *sprite, const csVector3& pos);
  virtual void RemoveSprite(iIsoSprite *sprite, const csVector3& pos);
  virtual void Draw(iIsoRenderView *rview);
  virtual void Traverse(iIsoCellTraverseCallback* func);


};

#endif // __CS_ISOCELL_H__
