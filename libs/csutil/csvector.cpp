/*
  Crystal Space Windowing System: vector class
  Copyright (C) 1998 by Jorrit Tyberghein
  Written by Andrew Zabolotny <bit@eltech.ru>

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

#include <stdlib.h>
#include <string.h>
#include "sysdef.h"
#include "csutil/csvector.h"

csVector::csVector (int ilimit, int ithreshold)
{
  root = (csSome *)malloc ((limit = ilimit) * sizeof (csSome));
  count = 0; threshold = ithreshold;
}

csVector::~csVector ()
{
//not much sense to call DeleteAll () since even for inherited classes
//anyway will be called csVector::FreeItem which is empty.
//DeleteAll ();
  if (root) free (root);
}

void csVector::DeleteAll ()
{
  int idx = count - 1;
  while (idx >= 0)
    if (FreeItem (root [idx]))
      idx--;
    else
      break;
  SetLength (idx + 1);
  while (idx >= 0)
    Delete (idx--);
}

void csVector::SetLength (int n)
{
  count = n;
  if ((n > limit) || ((limit > threshold) && (n < limit - threshold)))
  {
    n = ((n + threshold - 1) / threshold) * threshold;
    if (n > 0)
      root = (csSome *)realloc (root, n * sizeof (csSome));
    limit = n;
  }
}

bool csVector::FreeItem (csSome Item)
{
  (void)Item;
  return true;
}

bool csVector::Delete (int n)
{
  if (n < count)
  {
    if (!FreeItem (root [n]))
      return false;
    int ncount = count - 1;
    memmove (&root [n], &root [n + 1], (ncount - n) * sizeof (csSome));
    SetLength (ncount);
    return true;
  }
  else
    return false;
}

bool csVector::Insert (int n, csSome Item)
{
  if (n <= count)
  {
    SetLength (count + 1);
    memmove (&root [n + 1], &root [n], (count - n) * sizeof (csSome));
    root [n] = Item;
    return true;
  }
  else
   return false;
}

int csVector::Find (csSome which) const
{
  for (int i = 0; i < Length (); i++)
    if (root [i] == which)
      return i;
  return -1;
}

int csVector::FindKey (csConstSome Key) const
{
  for (int i = 0; i < Length (); i++)
    if (Equal (root [i], Key))
      return i;
  return -1;
}

bool csVector::Equal (csSome Item, csConstSome Key) const
{
  return (Item == Key);
}
