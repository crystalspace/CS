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

/**
 * \file
 */

#ifndef __CS_OBJITER_H__
#define __CS_OBJITER_H__

#include "csextern.h"
#include "iutil/object.h"

/**
 * Defines a typed objectiterator class, descending from 
 * csTypedObjectIterator. This macro assumes that the requested interface 
 * is already declared as a fast interface.
 */
#define CS_DECLARE_OBJECT_ITERATOR(NAME,INTERFACE)			\
  class NAME : public csTypedObjectIterator				\
  {									\
  protected:								\
    virtual void GetRequestedInterface (scfInterfaceID &id,		\
      int &ver) const							\
    { id = scfInterface<INTERFACE>::GetID();				\
      ver = scfInterface<INTERFACE>::GetVersion(); }			\
  public:								\
    inline NAME (iObject *Parent) : csTypedObjectIterator (Parent)	\
      { }								\
    inline INTERFACE *Next ()						\
      { return (INTERFACE*)(iBase*)csTypedObjectIterator::Next (); }	\
  };

/**
 * Helper class for #CS_DECLARE_OBJECT_ITERATOR macro.
 */
class CS_CSUTIL_EXPORT csTypedObjectIterator
{
protected:
  csRef<iObjectIterator> iter;
  csRef<iBase> CurrentTypedObject;
  bool fetched;

  void FetchObject ();
  iBase* GetCurrentObject();
  virtual void GetRequestedInterface (scfInterfaceID &id, int &ver) const = 0;

public:
  /// constructor
  csTypedObjectIterator (iObject *Parent);
  /// destructor
  virtual ~csTypedObjectIterator ();

  /// Move forward
  iBase* Next()
  {
    iBase* cur = GetCurrentObject();
    FetchObject ();
    return cur;
  }
  /// Reset the iterator to the beginning
  void Reset () { iter->Reset (); fetched = false; }
  /// Get the parent object
  iObject *GetParentObj() const { return iter->GetParentObj (); }
  /// Check if we have any children of requested type
  bool HasNext () const
  { return const_cast<csTypedObjectIterator*>(this)->GetCurrentObject() != 0; }
  /// Find the object with the given name
  iBase* FindName (const char* name);
};

#endif // __CS_OBJITER_H__
