/*
    Copyright (C) 2008 by Jorrit Tyberghein

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

class WalkTest;
struct iCamera;

#ifndef __WALKTEST_SPLITVIEW_H__
#define __WALKTEST_SPLITVIEW_H__

#include "ivaria/view.h"

/**
 * Class to help with splitting views.
 */
class WalkTestViews
{
private:
  WalkTest* walktest;

  /// Value to indicate split state
  /// -1 = not split, other value is index of current view
  int split;
  csRef<iView> views[2];

  /// The view on the world.
  iView* view;

public:
  WalkTestViews (WalkTest* walktest);

  void Draw ();
  bool SetupViewStart ();

  bool SplitView ();
  bool UnsplitView ();
  bool ToggleView ();

  iView* GetView () { return view; }
  iCamera* GetCamera () { return view->GetCamera (); }
};


#endif // __WALKTEST_SPLITVIEW_H__

