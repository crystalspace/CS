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

#ifndef __CS_AWS_SCR_H__
#define __CS_AWS_SCR_H__

#include "csutil/array.h"
#include "iutil/strset.h"
struct iAws;

struct awsActionMap
{
  /// The name of the action reduced to a long.
  unsigned long name;

  /// The action to execute.
  void (*Action) (void *owner, iAwsParmList* parmlist);
};

class awsActionDispatcher
{
private:
  /// List of actions to execute.
  csArray<awsActionMap*> actions;
  /// Shared string table.
  csRef<iStringSet> strset;
public:
  /// Constructor.
  awsActionDispatcher(iAws*);
  /// Register an action.
  void Register (
    const char *name,
    void (Action) (void *owner,
    iAwsParmList* parmlist));

  /// Execute the corresponding action.
  void Execute (const char *action, void *owner, iAwsParmList* parmlist);
};

#endif // __CS_AWS_SCR_H__
