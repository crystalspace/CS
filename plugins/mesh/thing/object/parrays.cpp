/*
    Dynamic arrays of engine objects
    Copyright (C) 1999 by Andrew Zabolotny

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

//------------------------------------------------------+ csPolygonArray +----//
csPolygonArray::~csPolygonArray ()
{
  DeleteAll ();
}

bool csPolygonArray::FreeItem (void* Item)
{
  thing_type->blk_polygon3d.Free ((csPolygon3D*)Item);
  //delete (csPolygon3D *)Item;
  return true;
}

int csPolygonArray::CompareKey (void* Item, const void* Key, int Mode) const
{
  (void)Mode;

  const char *name = ((csPolygon3D *) Item)->GetStaticData ()->GetName ();
  return name ? strcmp (name, (char *)Key) : -1;
}

csPolygon3D *csPolygonArray::Get (int iIndex) const
{
  return (csPolygon3D *)csVector::Get (iIndex);
}

//-----------------------------------------------+ csPolygonStaticArray +----//
csPolygonStaticArray::~csPolygonStaticArray ()
{
  DeleteAll ();
}

bool csPolygonStaticArray::FreeItem (void* Item)
{
  thing_type->blk_polygon3dstatic.Free ((csPolygon3DStatic*)Item);
  //delete (csPolygon3DStatic *)Item;
  return true;
}

int csPolygonStaticArray::CompareKey (void* Item, const void* Key,
		int Mode) const
{
  (void)Mode;

  const char *name = ((csPolygon3DStatic *) Item)->GetName ();
  return name ? strcmp (name, (char *)Key) : -1;
}

csPolygon3DStatic *csPolygonStaticArray::Get (int iIndex) const
{
  return (csPolygon3DStatic *)csVector::Get (iIndex);
}

