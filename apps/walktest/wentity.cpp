/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein

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
#include "walktest.h"
#include "csgeom/matrix3.h"
#include "iengine/light.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "iengine/portal.h"
#include "wentity.h"

extern WalkTest* Sys;

SCF_IMPLEMENT_IBASE_EXT (csWalkEntity)
  SCF_IMPLEMENTS_INTERFACE (csWalkEntity)
SCF_IMPLEMENT_IBASE_EXT_END

//--------------------------------------------------------------------------

struct AnimPortalCallback : public iPortalCallback
{
  AnimPortalCallback ();
  virtual ~AnimPortalCallback ();
  csAnimatedPortal* animportal;
  SCF_DECLARE_IBASE;
  virtual bool Traverse (iPortal* portal, iBase* context);
};

SCF_IMPLEMENT_IBASE (AnimPortalCallback)
  SCF_IMPLEMENTS_INTERFACE (iPortalCallback)
SCF_IMPLEMENT_IBASE_END

AnimPortalCallback::AnimPortalCallback ()
{
  SCF_CONSTRUCT_IBASE (0);
}

AnimPortalCallback::~AnimPortalCallback ()
{
  SCF_DESTRUCT_IBASE ();
}

bool AnimPortalCallback::Traverse (iPortal*, iBase* )
{
  animportal->visible = true;
  return true;
}

csAnimatedPortal::csAnimatedPortal (iPortal* p,
	int xyz, float max_angle, float speed)
{
  portal = p;
  AnimPortalCallback* cb = new AnimPortalCallback ();
  cb->animportal = this;
  portal->SetPortalCallback (cb);
  cb->DecRef ();
  csAnimatedPortal::xyz = xyz;
  csAnimatedPortal::max_angle = max_angle;
  csAnimatedPortal::speed = speed;
  orig_trans = portal->GetWarp ();
  cur_angle = 0;
  cur_dir = 1;
  visible = false;
}

void csAnimatedPortal::Activate ()
{
  // Push ourselves on to the busy list if we're not already there.
  size_t idx = Sys->busy_entities.Find (this);
  if (idx != csArrayItemNotFound) Sys->busy_entities.DeleteIndex (idx);
  Sys->busy_entities.Push (this);
}

void csAnimatedPortal::NextFrame (float elapsed_time)
{
  if (!visible) return;
  visible = false;

  if (cur_dir == 1)
  {
    cur_angle += elapsed_time/speed;
    if (cur_angle > max_angle) { cur_angle = max_angle; cur_dir = -1; }
  }
  else
  {
    cur_angle -= elapsed_time/speed;
    if (cur_angle < -max_angle) { cur_angle = -max_angle; cur_dir = 1; }
  }

  csReversibleTransform trans = orig_trans;
  switch (xyz)
  {
    case 1:
      trans *= csTransform (csXRotMatrix3 (cur_angle), csVector3 (0));
      break;
    case 2:
      trans *= csTransform (csYRotMatrix3 (cur_angle), csVector3 (0));
      break;
    case 3:
      trans *= csTransform (csZRotMatrix3 (cur_angle), csVector3 (0));
      break;
  }
  portal->SetWarp (trans);
}


//--------------------------------------------------------------------------

