/*
    Copyright (C) 2001 by Martin Geisse <mgeisse@gmx.net>

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

#ifndef __OBJITER_H__
#define __OBJITER_H__

#include "iutil/object.h"

/**
 * Define a typed objectiterator class. This macro assumes that the
 * requested interface is already declared as a fast interface.
 */
#define CS_DECLARE_OBJECT_ITERATOR(NAME,INTERFACE)			\
  class NAME : public csTypedObjectIterator				\
  {									\
  protected:								\
    virtual void GetRequestedInterface (scfInterfaceID &id, int &ver) const	\
    { id = scfGetID_##INTERFACE (); ver = VERSION_##INTERFACE; }	\
  public:								\
    inline NAME (iObject *Parent) : csTypedObjectIterator (Parent)	\
      { FetchObject (); }						\
    inline INTERFACE *Get ()						\
      { return (INTERFACE*)CurrentTypedObject; }			\
  };

class csTypedObjectIterator
{
protected:
  iObjectIterator *iter;
  iBase *CurrentTypedObject;

  void FetchObject ();
  virtual void GetRequestedInterface (scfInterfaceID &id, int &ver) const = 0;

public:
  // constructor
  csTypedObjectIterator (iObject *Parent);
  // destructor
  virtual ~csTypedObjectIterator ();

  // Move forward
  inline bool Next();
  // Reset the iterator to the beginning
  inline void Reset();
  // Get the object we are pointing at
  inline iObject *GetObject () const;
  // Get the parent object
  inline iObject *GetParentObj() const;
  // Check if we have any children of requested type
  inline bool IsFinished () const;
  // Find the object with the given name
  inline bool FindName (const char* name);
};

bool csTypedObjectIterator::Next()
  { bool r = iter->Next (); FetchObject (); return r; }
void csTypedObjectIterator::Reset()
  { iter->Reset (); FetchObject (); }
iObject *csTypedObjectIterator::GetObject () const
  { return iter->GetObject (); }
iObject *csTypedObjectIterator::GetParentObj() const
  { return iter->GetParentObj (); }
bool csTypedObjectIterator::IsFinished () const
  { return iter->IsFinished (); }
bool csTypedObjectIterator::FindName (const char* name)
  { bool r = iter->FindName (name); FetchObject (); return r; }

#endif // __OBJITER_H__
