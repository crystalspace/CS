/*
    Dynamic arrays of engine objects
    Copyright (C) 1999 by Andrew Zabolotny
    Copyright (C) 2003 by Jorrit Tyberghein

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
#include "parrays.h"
#include "polygon.h"
#include "thing.h"

//-----------------------------------------------+ csPolygonStaticArray +----//
csPolygonStaticArray::~csPolygonStaticArray ()
{
  FreeAll ();
}

void csPolygonStaticArray::FreeAll ()
{
  size_t i;
  for (i = 0 ; i < Length () ; i++)
  {
    FreeItem (Get (i));
  }
  DeleteAll ();
}

void csPolygonStaticArray::FreeItem (csPolygon3DStatic* Item)
{
  thing_type->blk_polygon3dstatic.Free (Item);
}

int csPolygonStaticArray::CompareKey (csPolygon3DStatic* const& Item,
				      char const* const& Key)
{
  const char *name = Item->GetName ();
  return name ? strcmp (name, Key) : -1;
}

