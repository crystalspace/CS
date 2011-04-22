/*
    Copyright (C) 2007 by Seth Yastrov

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

#ifndef __OBJECTLIST_H__
#define __OBJECTLIST_H__

#include "ieditor/objectlist.h"

#include <csutil/refarr.h>
#include <csutil/hash.h>

template<>
class csHashComputer<iBase*> : public csHashComputerIntegral<iBase*> {};

namespace CS {
namespace EditorApp {

struct iEditorObjectIterator;
struct iObjectListListener;
struct iEditorObject;

class ObjectList : public scfImplementation1<ObjectList,iObjectList>
{
protected:
  typedef csHash<csRef<iEditorObject>, iBase*> HashType;
  
  HashType objects;
  csRefArray<iObjectListListener> listeners;
  
public:
  ObjectList ();
  virtual ~ObjectList ();
  
  virtual void Add (iEditorObject* obj);

  virtual void Remove (iEditorObject* obj);
  
  virtual void Clear ();

  virtual csPtr<iEditorObjectIterator> GetIterator ();

  virtual void AddListener (iObjectListListener* listener);

  virtual void RemoveListener (iObjectListListener* listener);

  virtual iEditorObject* FindObject (iBase* obj);

  class ObjectListIterator : public scfImplementation1<ObjectListIterator,iEditorObjectIterator>
  {
  public:
    virtual bool HasNext () const
    { return it.HasNext (); }

    virtual iEditorObject* Next ()
    { return it.Next (); }

    virtual void Reset ()
    { it.Reset (); }

  protected:
    ObjectListIterator(HashType& hash)
    : scfImplementationType (this), it (hash.GetIterator ())
    { ; }

  private:
    HashType::GlobalIterator it;

    friend class ObjectList;
  };
};

} // namespace EditorApp
} // namespace CS

#endif
