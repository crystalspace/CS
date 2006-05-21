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

#ifndef __CS_AWS_LAYOT_H__
#define __CS_AWS_LAYOT_H__

#include "iaws/aws.h"
#include "csutil/scanstr.h"
#include "csgeom/csrect.h"

/// Base class for layouts.
class awsLayoutManager : public iAwsLayoutManager
{
protected:
  iAwsComponent *owner;
  iAwsPrefManager* pm;
public:
  awsLayoutManager (
    iAwsComponent *_owner,
    iAwsComponentNode*,
    iAwsPrefManager* _pm)
    : owner (_owner), pm (_pm)
  { 
    SCF_CONSTRUCT_IBASE (0);
  }

  virtual ~awsLayoutManager ()
  {
    SCF_DESTRUCT_IBASE ();
  }

  SCF_DECLARE_IBASE;

  /**
   * Sets the owner. Normally the owner should never change, but in some
   * rare cases (like in the Window class) the owner is set improperly by
   * the setup code and must be fixed by the embedder. This should ALWAYS
   * be used by widgets which embed the component and use delegate wrappers
   * (i.e. awsecomponent).
   */
  virtual void SetOwner (iAwsComponent *_owner) { owner = _owner; }

  /**
   * Adds a component to the layout, returning it's actual rect. 
   */
  virtual csRect AddComponent (
    iAwsComponent *cmp,
    iAwsComponentNode* settings) = 0;

  /// Removes a component from the layout.
  virtual void RemoveComponent (iAwsComponent* ) { }

  /// Lays out components properly.
  virtual void LayoutComponents () = 0;
};

#endif // __CS_AWS_LAYOT_H__
