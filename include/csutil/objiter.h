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

/**\file
 * Typed iObject iterator.
 */

#ifndef __CS_OBJITER_H__
#define __CS_OBJITER_H__

#include "csextern.h"
#include "csutil/ref.h"
#include "iutil/object.h"

/**
 * Typed object iterator class.
 */
template<typename T> 
class csTypedObjectIterator
{
protected:
  csRef<iObjectIterator> iter;
  csRef<T> CurrentTypedObject;

  void FetchObject()
  {
    CurrentTypedObject.Invalidate();
    while (iter->HasNext())
    {
      CurrentTypedObject = scfQueryInterface<T> (iter->Next());
      if (CurrentTypedObject.IsValid())
	return;
    }
  }

public:
  /// Constructor.
  csTypedObjectIterator(iObject* parent)
  {
    iter = parent->GetIterator();
    FetchObject();
  }

  /// Destructor.
  ~csTypedObjectIterator() {}

  /// Move forward
  T* Next()
  {
    iBase* cur = CurrentTypedObject;
    FetchObject();
    return (T*)cur;
  }

  /// Reset the iterator to the beginning.
  void Reset() { iter->Reset(); FetchObject(); }

  /// Get the parent object.
  iObject* GetParentObj() const { return iter->GetParentObj(); }

  /// Check if we have any children of requested type.
  bool HasNext() const { return CurrentTypedObject.IsValid(); }

  /// Find the object with the given name.
  T* FindName (const char* name)
  {
    iObject* obj = iter->FindName(name);
    if (obj != 0)
      CurrentTypedObject = scfQueryInterface<T> (obj);
    else
      CurrentTypedObject.Invalidate();
    return (T*)CurrentTypedObject;
  }
};

#endif // __CS_OBJITER_H__
