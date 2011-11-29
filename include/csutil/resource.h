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

#include "csutil/hash.h"
#include "csutil/threading/future.h"
#include "csutil/scf_implementation.h"

namespace CS
{
  namespace Resource
  {
    namespace Impl
    {
      // Implements the DJB hash algorithm.
      template<size_t N, size_t I = 1>
      struct TypeIDCalc
      {
        CS_FORCEINLINE_TEMPLATEMETHOD
          static TypeID hash (const char (&s)[N])
        {
          TypeID typeID = TypeIDCalc<N, I+1>::hash (s);
          return ((typeID << 5) + typeID) + s[N-I];
        }
      };

      template<size_t N>
      struct TypeIDCalc<N, N>
      {
        CS_FORCEINLINE_TEMPLATEMETHOD
          static TypeID hash (const char (&s)[N])
        {
          return s[0];
        }
      };
    }

    template<size_t N>
    CS_FORCEINLINE_TEMPLATEMETHOD
    TypeID GetTypeID (const char (&s)[N])
    {
      return Impl::TypeIDCalc<N>::hash (s);
    }

    CS_FORCEINLINE
    TypeID GetTypeID (const char* s)
    {
      return csHashCompute (s);
    }

    /**
     * Helper class which can be inherited by resources with
     * no resource dependencies or properties.
     */
    class NoDepResource : public virtual iResource
    {
    public:
      virtual bool DependenciesSatisfied () const
      {
        return true;
      }

      virtual const csArray<ResourceReference>& GetDependencies () const
      {
        return emptyDeps;
      }

      virtual void SetProperty (csStringID propery, iResource* resource) {};

    private:
      csArray<ResourceReference> emptyDeps;
    };

    /**
     * Helper class which can be inherited by resources with 
     * resource dependencies.
     */
    class DepResource : public virtual iResource
    {
    public:
      virtual bool DependenciesSatisfied () const
      {
        csArray<bool> deps = satisfiedDeps.GetAll ();
        for (size_t i = 0; i < deps.GetSize (); ++i)
        {
          if (!deps[i]) return false;
        }

        return true;
      }

      virtual const csArray<ResourceReference>& GetDependencies () const
      {
        return deps;
      }

    protected:
      void AddDependency (csString id, TypeID typeID,
        csStringID property, iDocumentNode* node = 0)
      {
        ResourceReference rref;
        rref.id = id;
        rref.node = node;
        rref.property = property;
        rref.typeID = typeID;
        deps.Push (rref);

        satisfiedDeps.Put (property, false);
      }

      void DependencySatisfied (csStringID property)
      {
        bool fallback;
        bool& satisfied = satisfiedDeps.Get (property, fallback);
        satisfied = true;
      }

    private:
      csArray<ResourceReference> deps;
      csHash<bool, csStringID> satisfiedDeps;
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
      
      virtual bool Ready() 
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
