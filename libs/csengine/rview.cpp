/*
    Copyright (C) 2000 by Andrew Zabolotny

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
#include "csengine/rview.h"
#include "igraph3d.h"

csFrustumView::csFrustumView () : light_frustum (NULL), callback (NULL),
  callback_data (NULL)
{
  memset (this, 0, sizeof (csFrustumView));
}

csFrustumView::csFrustumView (const csFrustumView &iCopy)
{
  // hehe. kind of trick.
  memcpy (this, &iCopy, sizeof (csFrustumView));
  // Leave cleanup actions alone to original copy
  cleanup = NULL;
}

csFrustumView::~csFrustumView ()
{
  while (cleanup)
  {
    csFrustrumViewCleanup *next = cleanup->next;
    cleanup->action (this, cleanup);
    cleanup = next;
  }
  delete light_frustum;
}

bool csFrustumView::DeregisterCleanup (csFrustrumViewCleanup *action)
{
  csFrustrumViewCleanup **pcur = &cleanup;
  csFrustrumViewCleanup *cur = cleanup;
  while (cur)
  {
    if (cur == action)
    {
      *pcur = cur->next;
      return true;
    }
    pcur = &cur->next;
    cur = cur->next;
  }
  return false;
}

//---------------------------------------------------------------------------

void csRenderView::ComputeAngle ()
{
  float rview_fov = (float)GetFOV ()/2.;
  float disp_width = (float)g3d->GetWidth ()/2.;
  float disp_radius = sqrt (rview_fov*rview_fov + disp_width*disp_width);
  fov_angle = 2. * acos (disp_width / disp_radius) * (360./(2.*M_PI));
}

void csRenderView::SetFOV (int a)
{
  csCamera::SetFOV (a);
  ComputeAngle ();
}

csRenderView::csRenderView () :
    csCamera (), world (NULL), view (NULL), g3d (NULL), g2d (NULL),
    portal_polygon (NULL), do_clip_plane (false), do_clip_frustum (false),
    callback (NULL), callback_data (NULL), fog_info (NULL),
    added_fog_info (false)
{
  ComputeAngle ();
}

csRenderView::csRenderView (const csCamera& c) :
    csCamera (c), world (NULL), view (NULL), g3d (NULL), g2d (NULL),
    portal_polygon (NULL), do_clip_plane (false), do_clip_frustum (false),
    callback (NULL), callback_data (NULL), fog_info (NULL),
    added_fog_info (false)
{
  ComputeAngle ();
}

csRenderView::csRenderView (const csCamera& c, csClipper* v, iGraphics3D* ig3d,
    iGraphics2D* ig2d) :
    csCamera (c), world (NULL), view (v), g3d (ig3d), g2d (ig2d),
    portal_polygon (NULL), do_clip_plane (false), do_clip_frustum (false),
    callback (NULL), callback_data (NULL), fog_info (NULL),
    added_fog_info (false)
{
  ComputeAngle ();
}

