/*
    Copyright (C) Aleksandras Gluchovas
    CS port by Norman Krämer <norman@users.sourceforge.net>

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

#ifndef __ABSOLUTELAYOUT_G__
#define __ABSOLUTELAYOUT_G__

#include "cslayout.h"

/**
 * This is just for completness. Set absolute positions just like you
 * normally do with SetPos, SetSize and SetRect.
 */
class csAbsoluteLayout : public csLayout
{
 public:

  csAbsoluteLayout(csComponent *pParent);

  virtual void SuggestSize (int &sugw, int& sugh);
  virtual void LayoutContainer ();
};

#endif // __ABSOLUTELAYOUT_G__
