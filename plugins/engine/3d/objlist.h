/*
    Copyright (C) 2007 by Frank Richter

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

#ifndef __CS_CSENGINE_OBJLIST_H__
#define __CS_CSENGINE_OBJLIST_H__

CS_PLUGIN_NAMESPACE_BEGIN(Engine)
{
  template<typename T, typename IF>
  class ObjectList : 
    public scfImplementation1<ObjectList<T, IF>, IF>
  {
  protected:
    csRefArrayObject<T> list;
  public:
    ObjectList () : scfImplementationType (this) {}
    virtual ~ObjectList () {}
  
    inline size_t GetSize() const { return list.GetSize(); }
    inline T* operator[] (size_t n) const { return list[n]; }
    inline void DeleteAll() { list.DeleteAll(); }
    inline size_t Push (T* s) { return list.Push (s); }
    inline bool Delete (T* s) { return list.Delete (s); }
    inline bool DeleteIndex (size_t n) { return list.DeleteIndex (n); }
    inline void Empty(){ list.Empty(); }
  
    virtual int GetCount () const { return (int)GetSize (); }
    virtual T* Get (int n) const { return (*this)[n]; }
    virtual int Add (T* obj) { return (int)this->Push (obj); }
    virtual bool Remove (T* obj) { return this->Delete (obj); }
    virtual bool Remove (int n) { return this->DeleteIndex (n); }
    virtual void RemoveAll () { this->DeleteAll (); }
    virtual int Find (T* obj) const { return (int)list.Find (obj); }
    virtual T* FindByName (const char *Name) const
    { return list.FindByName (Name); }
  };
  
  
}
CS_PLUGIN_NAMESPACE_END(Engine)

#endif // __CS_CSENGINE_OBJLIST_H__
