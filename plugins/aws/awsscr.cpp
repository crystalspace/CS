/*
    Copyright (C) 2001 by Christopher Nelson

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
#include "iaws/aws.h"
#include "awsscr.h"
#include "awsfparm.h"

awsActionDispatcher::awsActionDispatcher(iAws* a) : strset(a->GetStringTable())
{
}

void awsActionDispatcher::Register (const char *name,
  void (Action) (void *owner, iAwsParmList* parmlist))
{
  awsActionMap *map = new awsActionMap ();

  map->name = strset->Request(name);
  map->Action = Action;

  actions.Push (map);
}

void awsActionDispatcher::Execute (
  const char *action,
  void *owner,
  iAwsParmList* parmlist)
{
  unsigned long name = strset->Request(action);

  size_t i;
  for (i = 0; i < actions.Length (); ++i)
  {
    awsActionMap *map = actions[i];

    if (name == map->name) map->Action (owner, parmlist);
  }
}
