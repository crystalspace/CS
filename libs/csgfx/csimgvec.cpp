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
SCF_IMPLEMENT_IBASE_END;

csImageVector::csImageVector()
{
  SCF_CONSTRUCT_IBASE(NULL);
}

csImageVector::~csImageVector()
{
  // image references are NULL'd in ~csRefArray
}

void csImageVector::AddImage(csRef<iImage> img, int index)
{
  image[index] = img;
}

void csImageVector::operator += (csRef<iImage> img)
{
  image[image.Length()] = img;
}

csRef<iImage> &csImageVector::GetImage(int index)
{
  return image[index];
}

csRef<iImage> &csImageVector::operator[](int index)
{
  return image[index];
}

int csImageVector::Length()
{
  return image.Length();
}

void csImageVector::RemoveAll()
{
  image.DeleteAll();
}
