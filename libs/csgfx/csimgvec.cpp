/*
    Copyright (C) 2003 by Philipp Aumayr

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
#include "csgfx/csimgvec.h"

SCF_IMPLEMENT_IBASE(csImageVector);
  SCF_IMPLEMENTS_INTERFACE (iImageVector);
SCF_IMPLEMENT_IBASE_END

CS_LEAKGUARD_IMPLEMENT (csImageVector);

csImageVector::csImageVector()
{
  SCF_CONSTRUCT_IBASE(0);
}

csImageVector::~csImageVector()
{
  // image references are 0'd in ~csRefArray
  SCF_DESTRUCT_IBASE ();
}

void csImageVector::AddImage(iImage* img)
{
  image.Push (img);
}

void csImageVector::InsertImage(iImage* img, size_t index)
{
  image.Insert (index, img);
}

void csImageVector::operator += (iImage* img)
{
  image.Push (img);
}

csRef<iImage> csImageVector::GetImage(size_t index)
{
  csRef<iImage> r = image[index];
  return r;
}

void csImageVector::SetImage (size_t index, iImage* img)
{
  image.Put (index, img);
}

size_t csImageVector::Length()
{
  return image.Length();
}

void csImageVector::DeleteIndex(size_t index)
{
  image.DeleteIndex (index);
}

void csImageVector::RemoveAll()
{
  image.DeleteAll();
}
