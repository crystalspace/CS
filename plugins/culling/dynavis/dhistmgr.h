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
#include "csutil/scf.h"
#include "csgeom/plane3.h"
#include "iengine/viscull.h"
#include "dmodel.h"

// @@@ Hack(s) to avoid problems with static linking
#ifdef DYNAVIS_DEBUG
#define csVisibilityObjectHistory	csVisibilityObjectHistory_DEBUG
#endif

enum csVisReason
{
  INVISIBLE_PARENT = 0,	// Invisible because some parent node is invisible.
  INVISIBLE_FRUSTUM,	// Invisible because object outside frustum.
  INVISIBLE_TESTRECT,	// Invisible because covbuf->TestRectangle() failed.
  VISIBLE,		// Just visible.
  VISIBLE_INSIDE,	// Visible because camera is inside bbox.
  VISIBLE_HISTORY,	// Visible because it was visible last frame.
  VISIBLE_VPT,		// Visible because of VPT (visible point tracking).
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

  // If the following value is greater then the global history_frame_cnt
  // then the object will be assumed visible automatically.
  uint32 vis_cnt;

  // If the following value is greater then the global history_frame_cnt
  // then we will not do write queue tests because they are not useful
  // most likely.
  uint32 no_writequeue_vis_cnt;

  // If the following value is greater then the global history_frame_cnt
  // then we will not use this object as an occluder.
  uint32 no_occluder_vis_cnt;

  // When this object was last made visible.
  uint32 history_frame_cnt;

  // Here we remember the frustum mask which we will restore in case
  // of history culling for a node.
  uint32 history_frustum_mask;

  // 3D point to use for VPT testing (object space).
  bool has_vpt_point;
  csVector3 vpt_point;

  SCF_DECLARE_IBASE;

  csVisibilityObjectHistory ()
  {
    SCF_CONSTRUCT_IBASE (0);
    vis_cnt = 0;
    no_writequeue_vis_cnt = 0;
    history_frame_cnt = 0;
    no_occluder_vis_cnt = 0;
    reason = LAST_REASON;
    has_vpt_point = false;
  }

  virtual ~csVisibilityObjectHistory()
  {
    SCF_DESTRUCT_IBASE();
  }
};

#endif // __CS_DYNHISTMGR_H__

