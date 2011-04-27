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

#include "csutil/ansicommand.h"
#include "csutil/objreg.h"
#include "csutil/refarr.h"
#include "csutil/scf.h"
#include "csutil/stringquote.h"
#include "csutil/sysfunc.h"
#include "csutil/util.h"
#include "csutil/eventnames.h"
#include "csutil/eventhandlers.h"

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
  for (size_t i = objects.GetSize (); i > 0; i--)
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
  if (objects.GetSize () <= 0) return false;
  return true;
}

const char* csObjectRegistryIterator::GetCurrentTag ()
{
  if (cur_idx >= objects.GetSize ()) return 0;
  return tags[cur_idx];
}

bool csObjectRegistryIterator::HasNext ()
{
  if (cur_idx >= objects.GetSize ()) return false;
  return true;
}

iBase* csObjectRegistryIterator::Next ()
{
  cur_idx++;
  if (cur_idx > objects.GetSize ()) return 0;
  return objects[cur_idx-1];
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
#if defined(CS_DEBUG) || defined (CS_MEMORY_TRACKER)
  if (iSCF::SCF == 0)
  {
    static const char highlight[] = CS_ANSI_FR CS_ANSI_TEXT_BOLD_ON "%s" CS_ANSI_RST " ";
    csFPrintf(stderr, highlight, "Warning - scfInitialize() has been called for you.\n"
      "Your program will crash in optimise mode if you do not do this yourself!\n\n");
    scfInitialize (0); // Make sure we've got an iSCF::SCF
  }
  iSCF::SCF->object_reg = this;
#endif
}

csObjectRegistry::~csObjectRegistry ()
{
  CS_ASSERT (registry.GetSize () == 0);
  CS_ASSERT (tags.GetSize () == 0);
  CS_ASSERT (clearing == false);
}

void csObjectRegistry::Clear ()
{
  CS::Threading::RecursiveMutexScopedLock lock(registryLock);

  clearing = true;
  for (size_t i = registry.GetSize (); i > 0; i--)
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

  CS::Threading::RecursiveMutexScopedLock lock(registryLock);

  CS_ASSERT (registry.GetSize () == tags.GetSize ());
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
  CS::Threading::RecursiveMutexScopedLock lock(registryLock);

  CS_ASSERT (registry.GetSize () == tags.GetSize ());
  if (!clearing && obj != 0)
  {
    for (size_t i = registry.GetSize (); i-- > 0;)
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
  CS::Threading::RecursiveMutexScopedLock lock(registryLock);

  CS_ASSERT (registry.GetSize () == tags.GetSize ());
  for (size_t i = registry.GetSize (); i > 0; i--)
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
  CS::Threading::RecursiveMutexScopedLock lock(registryLock);

  CS_ASSERT (registry.GetSize () == tags.GetSize ());
  for (size_t i = registry.GetSize (); i > 0; i--)
  {
    const char* t = tags[i - 1];
    if (t && !strcmp (tag, t))
    {
      iBase* b = registry[i - 1];
      void* interf = b->QueryInterface (id, version);
      if (!interf)
      {
        csPrintf (CS_ANSI_FY CS_ANSI_TEXT_BOLD_ON 
          "WARNING! Suspicious: object with tag %s does not implement "
          "interface %s!\n" CS_ANSI_RST, CS::Quote::Single (t), 
          CS::Quote::Single (iSCF::SCF->GetInterfaceName(id)));
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

  CS::Threading::RecursiveMutexScopedLock lock(registryLock);

  for (size_t i = registry.GetSize (); i > 0; i--)
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

  CS::Threading::RecursiveMutexScopedLock lock(registryLock);

  for (size_t i = registry.GetSize (); i > 0; i--)
  {
    iBase* b = registry[i - 1];
    const char* t = tags[i - 1];
    iterator->Add (b, t);
  }
  return csPtr<iObjectRegistryIterator> (iterator);
}

