/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "cssysdef.h"
#include "csutil/util.h"
#include "csutil/objreg.h"

class csObjectRegistryIterator : public iObjectRegistryIterator
{
private:
  csVector objects;
  csVector tags;
  int cur_idx;

public:
  csObjectRegistryIterator ();
  virtual ~csObjectRegistryIterator ();

  void Add (iBase* obj, char const* tag);

  SCF_DECLARE_IBASE;
  virtual bool Restart ();
  virtual iBase* GetCurrent ();
  virtual const char* GetCurrentTag ();
  virtual bool Next ();
};

SCF_IMPLEMENT_IBASE (csObjectRegistryIterator)
  SCF_IMPLEMENTS_INTERFACE (iObjectRegistryIterator)
SCF_IMPLEMENT_IBASE_END

csObjectRegistryIterator::csObjectRegistryIterator ()
{
  SCF_CONSTRUCT_IBASE (NULL);
  cur_idx = 0;
}

csObjectRegistryIterator::~csObjectRegistryIterator ()
{
  int i;
  for (i = objects.Length() - 1; i >= 0; i--)
  {
    // Take special care to ensure that this object is no longer on the list
    // before calling DecRef(), since we don't want some other object asking
    // for it during its own destruction.
    iBase* b = (iBase*)objects[i];
    char* t = (char*)tags[i];
    objects.Delete (i); // Remove from list before DecRef().
    tags.Delete (i);
    b->DecRef ();
    delete[] t;
  }
}

bool csObjectRegistryIterator::Restart ()
{
  cur_idx = 0;
  if (objects.Length () <= 0) return false;
  return true;
}

iBase* csObjectRegistryIterator::GetCurrent ()
{
  if (cur_idx >= objects.Length ()) return NULL;
  return (iBase*)objects[cur_idx];
}

const char* csObjectRegistryIterator::GetCurrentTag ()
{
  if (cur_idx >= objects.Length ()) return NULL;
  return (const char*)tags[cur_idx];
}

bool csObjectRegistryIterator::Next ()
{
  if (cur_idx >= objects.Length ()-1) return false;
  cur_idx++;
  return true;
}

void csObjectRegistryIterator::Add (iBase* obj, char const* tag)
{
  obj->IncRef ();
  objects.Push (obj);
  tags.Push (tag ? csStrNew(tag) : 0);
}

//-------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csObjectRegistry)
  SCF_IMPLEMENTS_INTERFACE (iObjectRegistry)
SCF_IMPLEMENT_IBASE_END

csObjectRegistry::csObjectRegistry () : clearing (false)
{
  SCF_CONSTRUCT_IBASE (NULL);
}

csObjectRegistry::~csObjectRegistry ()
{
  CS_ASSERT (registry.Length () == 0);
  CS_ASSERT (tags.Length () == 0);
  CS_ASSERT (clearing == false);
}

void csObjectRegistry::Clear ()
{
  clearing = true;
  int i;
  for (i = registry.Length() - 1; i >= 0; i--)
  {
    // Take special care to ensure that this object is no longer on the list
    // before calling DecRef(), since we don't want some other object asking
    // for it during its own destruction.
    iBase* b = (iBase*)registry[i];
    char* t = (char*)tags[i];
    registry.Delete (i); // Remove from list before DecRef().
    tags.Delete (i);
    b->DecRef ();
    delete[] t;
  }
  clearing = false;
}

bool csObjectRegistry::Register (iBase* obj, char const* tag)
{
  CS_ASSERT (registry.Length () == tags.Length ());
  if (!clearing)
  {
    // Don't allow adding an object with an already existing tag.
    if (tag && Get (tag)) return false;

    obj->IncRef ();
    registry.Push (obj);
    tags.Push (tag ? csStrNew (tag) : 0);
    return true;
  }
  return false;
}

void csObjectRegistry::Unregister (iBase* obj, char const* tag)
{
  CS_ASSERT (registry.Length () == tags.Length ());
  if (!clearing)
  {
    int i;
    for (i = registry.Length() - 1; i >= 0; i--)
    {
      iBase* b = (iBase*)registry[i];
      if (b == obj)
      {
        char* t = (char*)tags[i];
        if ((t == 0 && tag == 0) || (t != 0 && tag != 0 && !strcmp (tag, t)))
        {
          delete[] t;
	  registry.Delete (i);
	  tags.Delete (i);
          b->DecRef ();
	  if (tag != 0) // For a tagged object, we're done.
	    break;
        }
      }
    }
  }
}

iBase* csObjectRegistry::Get (char const* tag)
{
  CS_ASSERT (registry.Length () == tags.Length ());
  int i;
  for (i = registry.Length() - 1; i >= 0; i--)
  {
    char* t = (char*)tags[i];
    if (t && !strcmp (tag, t))
    {
      iBase* b = (iBase*)registry[i];
      b->IncRef ();
      return b;
    }
  }
  return NULL;
}

iBase* csObjectRegistry::Get (char const* tag, scfInterfaceID id, int version)
{
  CS_ASSERT (registry.Length () == tags.Length ());
  int i;
  for (i = registry.Length() - 1; i >= 0; i--)
  {
    char* t = (char*)tags[i];
    if (t && !strcmp (tag, t))
    {
      iBase* b = (iBase*)registry[i];
      iBase* interf = (iBase*)(b->QueryInterface (id, version));
      if (!interf)
      {
        printf ("WARNING! Suspicious: object with tag '%s' doesn't implement interface '%s'!\n", t, t);
	fflush (stdout);
	return NULL;
      }
      return interf;
    }
  }
  return NULL;
}

iObjectRegistryIterator* csObjectRegistry::Get (scfInterfaceID id, int version)
{
  csObjectRegistryIterator* iterator = new csObjectRegistryIterator ();
  int i;
  for (i = registry.Length() - 1; i >= 0; i--)
  {
    iBase* b = (iBase*)registry[i];
    iBase* interf = (iBase*)(b->QueryInterface (id, version));
    if (interf)
    {
      char* t = (char*)tags[i];
      iterator->Add (interf, t);
      interf->DecRef ();
    }
  }
  return iterator;
}

iObjectRegistryIterator* csObjectRegistry::Get ()
{
  csObjectRegistryIterator* iterator = new csObjectRegistryIterator ();
  int i;
  for (i = registry.Length() - 1; i >= 0; i--)
  {
    iBase* b = (iBase*)registry[i];
    char* t = (char*)tags[i];
    iterator->Add (b, t);
  }
  return iterator;
}

