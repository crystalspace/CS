/*
    Copyright (C) 2002 by Jorrit Tyberghein

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

#ifndef __CS_DYNHISTMGR_H__
#define __CS_DYNHISTMGR_H__

#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/dbghelp.h"
#include "csutil/csvector.h"
#include "csutil/scf.h"
#include "csgeom/plane3.h"
#include "iengine/viscull.h"
#include "dmodel.h"

enum csVisReason
{
  INVISIBLE_PARENT = 0,	// Invisible because some parent node is invisible.
  INVISIBLE_FRUSTUM,	// Invisible because object outside frustum.
  INVISIBLE_TESTRECT,	// Invisible because covbuf->TestRectangle() failed.
  VISIBLE,		// Just visible.
  VISIBLE_INSIDE,	// Visible because camera is inside bbox.
  VISIBLE_HISTORY,	// Visible because it was visible last frame.
  LAST_REASON
};

/**
 * This class holds all historical data relevant to visibility
 * culling from previous frame. It is used both for regular objects
 * as for kdtree nodes.
 */
class csVisibilityObjectHistory : public iBase
{
public:
  csVisReason reason;	// Reason object is visible/invisible.

  // If the folloging counter > 0 then the object will be assumed visible
  // automatically. The counter will be decremented then.
  int vis_cnt;

  SCF_DECLARE_IBASE;

  csVisibilityObjectHistory ()
  {
    SCF_CONSTRUCT_IBASE (NULL);
    vis_cnt = 0;
  }
};

#endif // __CS_DYNHISTMGR_H__

