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
    CleanupAction *next = cleanup->next;
    cleanup->action (this, cleanup);
    cleanup = next;
  }
  delete light_frustum;
}

bool csFrustumView::DeregisterCleanup (CleanupAction *action)
{
  CleanupAction **pcur = &cleanup;
  CleanupAction *cur = cleanup;
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
