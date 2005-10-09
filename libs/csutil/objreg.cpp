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

#include "cssysdef.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "csutil/ansicolor.h"
#include "csutil/objreg.h"
#include "csutil/refarr.h"
#include "csutil/scf.h"
#include "csutil/scopedmutexlock.h"
#include "csutil/sysfunc.h"
#include "csutil/util.h"

class csObjectRegistryIterator : 
  public scfImplementation1<csObjectRegistryIterator, iObjectRegistryIterator>
{
private:
  csRefArray<iBase> objects;
  csStringArray tags;
  size_t cur_idx;

public:
  csObjectRegistryIterator ();
  virtual ~csObjectRegistryIterator ();

  void Add (iBase* obj, char const* tag);

  virtual bool Reset ();
  virtual const char* GetCurrentTag ();
  virtual bool HasNext ();
  virtual iBase* Next ();
};


csObjectRegistryIterator::csObjectRegistryIterator ()
  : scfImplementationType (this), cur_idx (0)
{
}

csObjectRegistryIterator::~csObjectRegistryIterator ()
{
  for (size_t i = objects.Length(); i > 0; i--)
  {
    // Take special care to ensure that this object is no longer on the list
    // before calling DecRef(), since we don't want some other object asking
    // for it during its own destruction.
    objects.DeleteIndex (i - 1);
    tags.DeleteIndex (i - 1);
  }
}

bool csObjectRegistryIterator::Reset ()
{
  cur_idx = 0;
  if (objects.Length () <= 0) return false;
  return true;
}

const char* csObjectRegistryIterator::GetCurrentTag ()
{
  if (cur_idx >= objects.Length ()) return 0;
  return tags[cur_idx];
}

bool csObjectRegistryIterator::HasNext ()
{
  if (cur_idx >= objects.Length ()) return false;
  return true;
}

iBase* csObjectRegistryIterator::Next ()
{
  cur_idx++;
  if (cur_idx >= objects.Length ()) return 0;
  return objects[cur_idx];
}

void csObjectRegistryIterator::Add (iBase* obj, char const* tag)
{
  objects.Push (obj);
  tags.Push (tag);
}

//-------------------------------------------------------------------------

csObjectRegistry::csObjectRegistry () 
  : scfImplementationType (this), clearing (false)
{
  // We need a recursive mutex.
  mutex = csMutex::Create (true);
#if defined(CS_DEBUG) || defined (CS_MEMORY_TRACKER)
  if (iSCF::SCF == 0)
    scfInitialize (0); // Make sure we've got an iSCF::SCF
  iSCF::SCF->object_reg = this;
#endif
}

csObjectRegistry::~csObjectRegistry ()
{
  CS_ASSERT (registry.Length () == 0);
  CS_ASSERT (tags.Length () == 0);
  CS_ASSERT (clearing == false);
}

void csObjectRegistry::Clear ()
{
  csScopedMutexLock lock (mutex);

  clearing = true;
  size_t i;
  for (i = registry.Length(); i > 0; i--)
  {
    // Take special care to ensure that this object is no longer on the list
    // before calling DecRef(), since we don't want some other object asking
    // for it during its own destruction.
    iBase* b = registry[i - 1];
    registry.DeleteIndex (i - 1); // Remove from list before DecRef().
    tags.DeleteIndex (i - 1);
    b->DecRef ();
  }
  clearing = false;
}

bool csObjectRegistry::Register (iBase* obj, char const* tag)
{
  if (obj == 0)
    return false;

  csScopedMutexLock lock (mutex);

  CS_ASSERT (registry.Length () == tags.Length ());
  if (!clearing)
  {
    // Don't allow adding an object with an already existing tag.
    if (tag)
    {
      iBase* o = Get (tag);
      if (o)
      {
        // DecRef() o because Get() already increffed it.
        o->DecRef ();
        return false;
      }
    }

    obj->IncRef ();
    registry.Push (obj);
    tags.Push (tag);
    return true;
  }
  return false;
}

void csObjectRegistry::Unregister (iBase* obj, char const* tag)
{
  csScopedMutexLock lock (mutex);

  CS_ASSERT (registry.Length () == tags.Length ());
  if (!clearing && obj != 0)
  {
    size_t i;
    for (i = registry.Length(); i-- > 0;)
    {
      iBase* b = registry[i];
      if (b == obj)
      {
        const char* t = tags[i];
        if ((t == 0 && tag == 0) || (t != 0 && tag != 0 && !strcmp (tag, t)))
        {
	  registry.DeleteIndex (i);
	  tags.DeleteIndex (i);
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
  csScopedMutexLock lock (mutex);

  CS_ASSERT (registry.Length () == tags.Length ());
  size_t i;
  for (i = registry.Length(); i > 0; i--)
  {
    const char* t = tags[i - 1];
    if (t && !strcmp (tag, t))
    {
      iBase* b = registry[i - 1];
      b->IncRef ();
      return b;
    }
  }
  return 0;
}

iBase* csObjectRegistry::Get (char const* tag, scfInterfaceID id, int version)
{
  csScopedMutexLock lock (mutex);

  CS_ASSERT (registry.Length () == tags.Length ());
  size_t i;
  for (i = registry.Length(); i > 0; i--)
  {
    const char* t = tags[i - 1];
    if (t && !strcmp (tag, t))
    {
      iBase* b = registry[i - 1];
      void* interf = b->QueryInterface (id, version);
      if (!interf)
      {
        csPrintf (CS_ANSI_FY CS_ANSI_FI "WARNING! Suspicious: object with "
	  "tag '%s' does not implement interface '%s'!\n" CS_ANSI_RST, t, 
	  iSCF::SCF->GetInterfaceName(id));
	fflush (stdout);
	return 0;
      }
      return b; //do not return interf.  
    }
  }
  return 0;
}

csPtr<iObjectRegistryIterator> csObjectRegistry::Get (
	scfInterfaceID id, int version)
{
  csObjectRegistryIterator* iterator = new csObjectRegistryIterator ();
  size_t i;
  csScopedMutexLock lock (mutex);
  for (i = registry.Length(); i > 0; i--)
  {
    iBase* b = registry[i - 1];
    void* interf = b->QueryInterface (id, version);
    if (interf)
    {
      const char* t = tags[i - 1];
      iterator->Add (b, t);
      b->DecRef ();
    }
  }
  return csPtr<iObjectRegistryIterator> (iterator);
}

csPtr<iObjectRegistryIterator> csObjectRegistry::Get ()
{
  csObjectRegistryIterator* iterator = new csObjectRegistryIterator ();
  size_t i;
  csScopedMutexLock lock (mutex);
  for (i = registry.Length(); i > 0; i--)
  {
    iBase* b = registry[i - 1];
    const char* t = tags[i - 1];
    iterator->Add (b, t);
  }
  return csPtr<iObjectRegistryIterator> (iterator);
}

