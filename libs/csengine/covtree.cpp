/*
    Copyright (C) 1999 by Jorrit Tyberghein
  
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

#include "sysdef.h"
#include "csgeom/math2d.h"
#include "csengine/covtreep.h"

template <class Child>
bool csCovTreeNode<Child>::TestPolygon (csVector2* poly, int num_poly) const
{
  (void)poly; (void)num_poly;
  // @@@ Dummy functionality to test the recursion with templates.
  return children[0].TestPolygon (poly, num_poly);
}

template <class Child>
bool csCovTreeNode<Child>::InsertPolygon (csVector2* poly, int num_poly)
{
  (void)poly; (void)num_poly;
  // @@@ Dummy functionality to test the recursion with templates.
  return children[0].InsertPolygon (poly, num_poly);
}

bool csCovTreeNode0::TestPolygon (csVector2* poly, int num_poly) const
{
  (void)poly; (void)num_poly;
  return false;
}

bool csCovTreeNode0::InsertPolygon (csVector2* poly, int num_poly)
{
  (void)poly; (void)num_poly;
  return false;
}

