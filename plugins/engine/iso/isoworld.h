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

#ifndef __CS_ISOWORLD_H__
#define __CS_ISOWORLD_H__

#include "ivaria/iso.h"

/// node of the gridlist
struct csIsoGridListNode {
  /// grid
  iIsoGrid *grid;
  /// next
  csIsoGridListNode *next;
};

/**
 * isometric world, made up of grids.
*/
class csIsoWorld : public iIsoWorld {
private:
  csIsoGridListNode *gridlist;
public:
  SCF_DECLARE_IBASE;

  ///
  csIsoWorld (iBase *iParent);
  ///
  virtual ~csIsoWorld ();

  //------ iIsoWorld ----------------------------------------------------
  virtual void AddSprite(iIsoSprite *sprite);
  virtual void RemoveSprite(iIsoSprite *sprite);
  virtual void MoveSprite(iIsoSprite *sprite, const csVector3& oldpos,
    const csVector3& newpos);
  virtual iIsoGrid* CreateGrid(int width, int height);
  virtual iIsoGrid* FindGrid(const csVector3& pos);
  virtual void Draw(iIsoRenderView *rview);
};

#endif // __CS_ISOWORLD_H__
