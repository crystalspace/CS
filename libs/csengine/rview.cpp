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
#include "csgeom/polyclip.h"
#include "igraph3d.h"
#include "iclip2.h"
#include "icamera.h"

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

IMPLEMENT_IBASE (csRenderView)
  IMPLEMENTS_EMBEDDED_INTERFACE (iRenderView)
IMPLEMENT_IBASE_END

IMPLEMENT_EMBEDDED_IBASE (csRenderView::RenderView)
  IMPLEMENTS_INTERFACE (iRenderView)
IMPLEMENT_EMBEDDED_IBASE_END

csRenderView::csRenderView () :
    csCamera (), engine (NULL), view (NULL), g3d (NULL), g2d (NULL),
    portal_polygon (NULL), previous_sector (NULL), this_sector (NULL),
    do_clip_plane (false), do_clip_frustum (false),
    callback (NULL), callback_data (NULL), fog_info (NULL),
    added_fog_info (false)
{
  CONSTRUCT_IBASE (NULL);
  CONSTRUCT_EMBEDDED_IBASE (scfiRenderView);
}

csRenderView::csRenderView (const csCamera& c) :
    csCamera (c), engine (NULL), view (NULL), g3d (NULL), g2d (NULL),
    portal_polygon (NULL), previous_sector (NULL), this_sector (NULL),
    do_clip_plane (false), do_clip_frustum (false),
    callback (NULL), callback_data (NULL), fog_info (NULL),
    added_fog_info (false)
{
  CONSTRUCT_IBASE (NULL);
  CONSTRUCT_EMBEDDED_IBASE (scfiRenderView);
}

csRenderView::csRenderView (const csCamera& c, csClipper* v, iGraphics3D* ig3d,
    iGraphics2D* ig2d) :
    csCamera (c), engine (NULL), view (v), g3d (ig3d), g2d (ig2d),
    portal_polygon (NULL), previous_sector (NULL), this_sector (NULL),
    do_clip_plane (false), do_clip_frustum (false),
    callback (NULL), callback_data (NULL), fog_info (NULL),
    added_fog_info (false)
{
  CONSTRUCT_IBASE (NULL);
  CONSTRUCT_EMBEDDED_IBASE (scfiRenderView);
}

iClipper2D* csRenderView::RenderView::GetClipper ()
{
  return QUERY_INTERFACE (scfParent->view, iClipper2D);
}

iCamera* csRenderView::RenderView::GetCamera ()
{
  return QUERY_INTERFACE (scfParent, iCamera);
}

