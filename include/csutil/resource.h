/*
  Copyright (C) 2011 by Michael Gist

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
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

#ifndef __CS_CSUTIL_RESOURCE_H__
#define __CS_CSUTIL_RESOURCE_H__

#include "imap/resource.h"

#include "csutil/threading/future.h"
#include "csutil/scf_implementation.h"

namespace CS
{
  namespace Resource
  {
    template<size_t N, size_t I = 0>
    struct HashIDCalc
    {
      CS_FORCEINLINE_TEMPLATEMETHOD
      static TypeID hash (const char (&s)[N])
      {
        return (HashIDCalc<N, I+1>::hash (s) ^ s[I]) * 16777619u;
      }
    };

    template<size_t N>
    struct HashIDCalc<N, N>
    {
      CS_FORCEINLINE_TEMPLATEMETHOD
      static TypeID hash (const char (&s)[N])
      {
        return 2166136261u;
      }
    };

    template<size_t N>
    CS_FORCEINLINE_TEMPLATEMETHOD
    TypeID HashID (const char (&s)[N])
    {
      return HashIDCalc<N>::hash (s);
    }

    /**
     * Class which can be inherited by resources with
     * no resource dependencies.
     */
    class NoDepResource : public virtual iResource
    {
    public:
      virtual const csArray<ResourceReference>& GetDependencies () const
      {
        return emptyDeps;
      }

      virtual void SetProperty (csStringID propery, iResource* resource) {};

    private:
      csArray<ResourceReference> emptyDeps;
    };
    
    /**
     * Default implementation of the iLoading interface, for use with ResourceManagers.
     */
    class LoadingResource : public scfImplementation1<LoadingResource, iLoadingResource>
    {
    public:
      LoadingResource() : scfImplementationType(this)
      {
      }
      
      LoadingResource(const CS::Threading::Future<csRef<iResource> >& ref) : scfImplementationType(this)
      {
        p = ref;
      }
      
      csRef<iResource> operator->()
      {
          return p.Get();
      }
      
      virtual csRef<iResource> Get()
      {
          p.Wait();
          return p.Get();
      }
      
      virtual const char* GetName()
      {
        return 0;
      }
      
      virtual bool Ready() const 
      {
        return p.Ready();
      }
      
      
      virtual void AddListener(iResourceListener* listener)
      {
        if (Ready())
          listener->OnLoaded(this);
        else
          listeners.Push(listener);
      }
      
      virtual void RemoveListener(iResourceListener* listener) 
      {
        listeners.Delete(listener);
      }
      
    private:
      CS::Threading::Future<csRef<iResource> > p;
      
      csRefArray<iResourceListener> listeners;
      virtual void TriggerCallback() 
      {
        for (size_t i = 0; i < listeners.GetSize(); i++)
          listeners.Get(i)->OnLoaded(this);
        listeners.DeleteAll();
      }
    };
  }
}

#endif // __CS_CSUTIL_RESOURCE_H__
