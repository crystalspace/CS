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
#include "isoworld.h"
#include "isogrid.h"

SCF_IMPLEMENT_IBASE (csIsoWorld)
  SCF_IMPLEMENTS_INTERFACE (iIsoWorld)
SCF_IMPLEMENT_IBASE_END

csIsoWorld::csIsoWorld (iBase *iParent)
{
  SCF_CONSTRUCT_IBASE (iParent);
  gridlist = 0;
}

csIsoWorld::~csIsoWorld ()
{
  csIsoGridListNode *p = gridlist, *np;
  while(p)
  {
    np = p->next;
    p->grid->DecRef();
    delete p;
    p = np;
  }
  SCF_DESTRUCT_IBASE();
}

void csIsoWorld::AddSprite(iIsoSprite *sprite)
{
  iIsoGrid *grid = FindGrid(sprite->GetPosition());
  if(!grid)
  {
#ifdef CS_DEBUG
    printf("World: no grid to add sprite to.\n");
#endif
    return;
  }
  grid->AddSprite(sprite);
  sprite->SetGrid(grid);
}

void csIsoWorld::RemoveSprite(iIsoSprite *sprite)
{
  iIsoGrid *grid = FindGrid(sprite->GetPosition());
  if(!grid) return; // nothing to do.
  grid->RemoveSprite(sprite);
}

void csIsoWorld::MoveSprite(iIsoSprite *sprite, const csVector3& oldpos,
  const csVector3& newpos)
{
  iIsoGrid *grid = FindGrid(oldpos);
  if(!grid) return; // nothing to do.
  grid->MoveSprite(sprite, oldpos, newpos);
}

iIsoGrid* csIsoWorld::CreateGrid(int width, int height)
{
  iIsoGrid *grid = new csIsoGrid(0, this, width, height);
  csIsoGridListNode *node = new csIsoGridListNode;
  node->next = gridlist;
  node->grid = grid;
  gridlist = node;
  return grid;
}

iIsoGrid* csIsoWorld::FindGrid(const csVector3& pos)
{
  csIsoGridListNode *p = gridlist;
  while(p)
  {
    if(p->grid->Contains(pos))
      return p->grid;
    p = p->next;
  }
  return 0;
}

void csIsoWorld::Draw(iIsoRenderView *rview)
{
  csIsoGridListNode *p = gridlist;
  while(p)
  {
    p->grid->Draw(rview);
    p = p->next;
  }
}

