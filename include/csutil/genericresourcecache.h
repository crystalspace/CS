/*
    Copyright (C) 2007-2008 by Frank Richter

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

#ifndef __CS_CSUTIL_GENERICRESOURCECACHE_H__
#define __CS_CSUTIL_GENERICRESOURCECACHE_H__

#include "csutil/blockallocator.h"
#include "csutil/comparator.h"
#include "csutil/list.h"
#include "csutil/redblacktree.h"

namespace CS
{
  namespace Utility
  {
    /**
     * Mixins intended for use with GenericResourceCache<>.
     */
    namespace ResourceCache
    {
      /// Resource sorting that does not sort
      class SortingNone
      {
      public:
	struct KeyType {};
	  
	template<typename T1, typename T2>
	static bool IsLargerEqual (const T1&, const T2&)
	{
	  return 0;
	}
	template<typename T1, typename T2>
	static bool IsEqual (const T1&, const T2&)
	{
	  return 0;
	}
      };
      
      /**
       * Reuse condition: a resource is reused after a certain time has passed
       */
      template<typename TimeType = uint>
      class ReuseConditionAfterTime
      {
      public:
	struct AddParameter
	{
          /// Lifetime of a resource
	  TimeType lifeTime;
	  
	  AddParameter (TimeType lifeTime = 0) : lifeTime (lifeTime) {}
	};
	struct StoredAuxiliaryInfo
	{
          /// Time after which the resource can be reused
	  TimeType timeToDie;
	  TimeType lifeTime;
	  
	  template<typename ResourceCacheType>
	  StoredAuxiliaryInfo (const ResourceCacheType& cache, 
	    const AddParameter& param) : 
	    timeToDie (0),
	    lifeTime (param.lifeTime)
	  {}
	};
	
	/// Called when the resource turns "active"
	template<typename ResourceCacheType>
	void MarkActive (const ResourceCacheType& cache,
	  StoredAuxiliaryInfo& elementInfo)
	{
	  elementInfo.timeToDie = cache.GetCurrentTime() + elementInfo.lifeTime;
	}

	template<typename ResourceCacheType>
	bool IsReusable (const ResourceCacheType& cache,
	  const StoredAuxiliaryInfo& elementInfo,
	  const typename ResourceCacheType::CachedType& data)
	{
	  return cache.GetCurrentTime() > elementInfo.timeToDie;
	}
      };

      /// Reuse condition: a resource is reused after being flagged as such
      class ReuseConditionFlagged
      {
      public:
	struct AddParameter
	{
	  AddParameter () {}
	};
	struct StoredAuxiliaryInfo
	{
          /// Whether a resource can be reused.
	  bool reusable;
	  
	  template<typename ResourceCacheType>
	  StoredAuxiliaryInfo (const ResourceCacheType& cache, 
	    const AddParameter& param) : reusable (false)
	  {}
	};
	
	
	/// Called when the resource turns "active"
	template<typename ResourceCacheType>
	void MarkActive (const ResourceCacheType& cache,
	  StoredAuxiliaryInfo& elementInfo)
	{
	  // Reset flag
	  elementInfo.reusable = false;
	}

	template<typename ResourceCacheType>
	bool IsReusable (const ResourceCacheType& cache,
	  const StoredAuxiliaryInfo& elementInfo,
	  const typename ResourceCacheType::CachedType& data)
	{
	  return elementInfo.reusable;
	}
      };
      
      /**
       * Reuse condition: a resource is reused if only one reference is held 
       * to it. (The resource type must be a csRef<>.)
       */
      class ReuseIfOnlyOneRef
      {
      public:
	struct AddParameter
	{
	  AddParameter () {}
	};
	struct StoredAuxiliaryInfo
	{
	  template<typename ResourceCacheType>
	  StoredAuxiliaryInfo (const ResourceCacheType& cache, 
	    const AddParameter& param) {}
	};
	
	template<typename ResourceCacheType>
	void MarkActive (const ResourceCacheType& cache,
	    StoredAuxiliaryInfo& elementInfo)
	{ }
  
	template<typename ResourceCacheType>
	bool IsReusable (const ResourceCacheType& cache,
	  const StoredAuxiliaryInfo& elementInfo,
	  const typename ResourceCacheType::CachedType& data)
	{
	  return data->GetRefCount() == 1;
	}
      };
      
      /**
       * Reuse condition: allow immediate reuse.
       */
      class ReuseAlways
      {
      public:
	struct AddParameter
	{
	  AddParameter () {}
	};
	struct StoredAuxiliaryInfo
	{
	  template<typename ResourceCacheType>
	  StoredAuxiliaryInfo (const ResourceCacheType& cache, 
	    const AddParameter& param) {}
	};
	
	template<typename ResourceCacheType>
	void MarkActive (const ResourceCacheType& cache,
	    StoredAuxiliaryInfo& elementInfo)
	{ }
  
	template<typename ResourceCacheType>
	bool IsReusable (const ResourceCacheType& cache,
	  const StoredAuxiliaryInfo& elementInfo,
	  const typename ResourceCacheType::CachedType& data)
	{
	  return true;
	}
      };
      
      /**
       * Purge condition: a resource is purged after a certain time has passed
       */
      template<typename TimeType = uint>
      class PurgeConditionAfterTime
      {
	/**
	  * Time interval since last use after which a resource is considered
	  * "aged" and may be purged
	  */
	TimeType purgeAge;
      public:
	struct AddParameter
	{
	  AddParameter () {}
	};
	struct StoredAuxiliaryInfo
	{
	  TimeType lastTimeUsed;
	  
	  template<typename ResourceCacheType>
	  StoredAuxiliaryInfo (const ResourceCacheType& cache, 
	    const AddParameter& param) : lastTimeUsed(0) {}
	};
	
	PurgeConditionAfterTime (TimeType purgeAge = 60)
	  : purgeAge (purgeAge) {}
	
	/// Called when the resource turns "active"
	template<typename ResourceCacheType>
	void MarkActive (const ResourceCacheType& cache,
	  StoredAuxiliaryInfo& elementInfo)
	{
	  elementInfo.lastTimeUsed = cache.GetCurrentTime();
	}

	template<typename ResourceCacheType>
	bool IsPurgeable (const ResourceCacheType& cache,
	  const StoredAuxiliaryInfo& elementInfo,
	  const typename ResourceCacheType::CachedType& data)
	{
	  return (cache.GetCurrentTime() > 
	    elementInfo.lastTimeUsed + purgeAge);
	}
      };

      /**
       * Purge condition: a resource is purged if only one reference is held 
       * to it. (The resource type must be a csRef<>.)
       */
      class PurgeIfOnlyOneRef
      {
      public:
	struct AddParameter
	{
	  AddParameter () {}
	};
	struct StoredAuxiliaryInfo
	{
	  template<typename ResourceCacheType>
	  StoredAuxiliaryInfo (const ResourceCacheType& cache, 
	    const AddParameter& param) {}
	};
	
	template<typename ResourceCacheType>
	void MarkActive (const ResourceCacheType& cache,
	    StoredAuxiliaryInfo& elementInfo)
	{ }
  
	template<typename ResourceCacheType>
	bool IsPurgeable (const ResourceCacheType& cache,
	  StoredAuxiliaryInfo& elementInfo,
	  const typename ResourceCacheType::CachedType& data)
	{
	  return data->GetRefCount() == 1;
	}
      };
      
    } // namespace ResourceCache
    
    /**
     * Generic cache for resources.
     * It can cache frequently needed, reuseable data, can fetch cached 
     * resources based on a given sorting (not just exact matches, also 
     * resources that are bigger than some given key) and free resources that
     * were cached but not used for some time.
     */
    template<typename T,
      typename _TimeType = uint, 
      typename _ResourceSorting = ResourceCache::SortingNone,
      typename _ReuseCondition = ResourceCache::ReuseConditionAfterTime<_TimeType>,
      typename _PurgeCondition = ResourceCache::PurgeConditionAfterTime<_TimeType> >
    class GenericResourceCache
    {
    public:
      typedef T CachedType;
      typedef _TimeType TimeType;
      typedef _ResourceSorting ResourceSorting;
      typedef _ReuseCondition ReuseCondition;
      typedef _PurgeCondition PurgeCondition;
      
    protected:
      typedef typename ResourceSorting::KeyType ResourceSortingKeyType;
      typedef typename ReuseCondition::AddParameter ReuseConditionAddParameter;
      typedef typename ReuseCondition::StoredAuxiliaryInfo ReuseConditionAuxiliary;
      typedef typename PurgeCondition::AddParameter PurgeConditionAddParameter;
      typedef typename PurgeCondition::StoredAuxiliaryInfo PurgeConditionAuxiliary;

      template<typename Super>
      struct DataStorage : public CS::Memory::CustomAllocatedDerived<Super>
      {
	T data;
	
	template<typename A>
	DataStorage (const T& data, const A& a) : 
          CS::Memory::CustomAllocatedDerived<Super> (a), data (data) {}
      };
      struct Element : public DataStorage<ReuseConditionAuxiliary>,
                       public PurgeConditionAuxiliary
      {
	
	
	Element (const T& data,
	  const ReuseConditionAuxiliary& reuseAux,
	  const PurgeConditionAuxiliary& purgeAux)
	  : DataStorage<ReuseConditionAuxiliary> (data, reuseAux),
	    PurgeConditionAuxiliary (purgeAux)
	{
	}
	
	ReuseConditionAuxiliary& GetReuseAuxiliary()
	{ return *(static_cast<ReuseConditionAuxiliary*> (this)); }
	const ReuseConditionAuxiliary& GetReuseAuxiliary() const
	{ return *(static_cast<const ReuseConditionAuxiliary*> (this)); }
        
	PurgeConditionAuxiliary& GetPurgeAuxiliary()
	{ return *(static_cast<PurgeConditionAuxiliary*> (this)); }
	const PurgeConditionAuxiliary& GetPurgeAuxiliary() const
	{ return *(static_cast<const PurgeConditionAuxiliary*> (this)); }
      };
      
      struct ElementWrapper
      {
	Element* ptr;
	
	ElementWrapper () {}
	ElementWrapper (Element* ptr) : ptr (ptr) {}
	ElementWrapper (const ElementWrapper& other) : ptr (other.ptr) { }
	
	Element* operator->() { return ptr; }
	
	// Comparisons among ElementWrappers
	bool operator== (const ElementWrapper& other) const
	{
	  return ResourceSorting::IsEqual (ptr->data, other.ptr->data);
	}
	bool operator<= (const ElementWrapper& other) const
	{
	  return ResourceSorting::IsLargerEqual (ptr->data, other.ptr->data);
	}
	
	// Comparisons against ResourceSortingKeyType
	bool operator== (const ResourceSortingKeyType& other) const
	{
	  return ResourceSorting::IsEqual (ptr->data, other);
	}
	bool operator<= (const ResourceSortingKeyType& other) const
	{
	  return ResourceSorting::IsLargerEqual (ptr->data, other);
	}
	friend bool operator<= (const ResourceSortingKeyType& key, 
			        const ElementWrapper& el)
	{
	  return ResourceSorting::IsLargerEqual (key, el.ptr->data);
	}
	
      };
      
      csBlockAllocator<Element> elementAlloc;
      typedef csRedBlackTree<ElementWrapper,
	  CS::Container::DefaultRedBlackTreeAllocator<ElementWrapper>,
	  CS::Container::RedBlackTreeOrderingPartial> AvailableResourcesTree;
      // Tree of available resources
      struct AvailableResourcesWrapper : public ReuseCondition
      {
      private:
	csBlockAllocator<Element>& elementAlloc;
	struct RBTraverser
	{
	  csBlockAllocator<Element>& elementAlloc;
	  
	  RBTraverser (csBlockAllocator<Element>& elementAlloc) :
	    elementAlloc (elementAlloc) {}
	  
	  void operator() (ElementWrapper& el)
	  {
	    elementAlloc.Free (el.ptr);
	  }
	};
      public:
	AvailableResourcesTree v;
        
        AvailableResourcesWrapper (const ReuseCondition& other,
          csBlockAllocator<Element>& elementAlloc)
          : ReuseCondition (other), elementAlloc (elementAlloc) { }
	
	void Destroy()
	{
	  RBTraverser trav (elementAlloc);
	  v.TraverseInOrder (trav);
	  v.DeleteAll();
	}
      } availableResources;
      // List of active resources
      struct ActiveResourcesWrapper : public PurgeCondition
      {
      protected:
	csBlockAllocator<Element>& elementAlloc;
      public:
	csList<ElementWrapper> v;
        
        ActiveResourcesWrapper (const PurgeCondition& other,
          csBlockAllocator<Element>& elementAlloc)
	  : PurgeCondition (other), elementAlloc (elementAlloc) { }
	
	void Destroy()
	{
	  typename csList<ElementWrapper>::Iterator listIt (v);
	  while (listIt.HasNext())
	  {
	    Element* el = listIt.Next().ptr;
	    elementAlloc.Free (el);
	  }
	  v.DeleteAll();
	}
      } activeResources;
      
      TimeType currentTime;
      /**
       * The last time the momentarily available resources were 
       * scanned for "aged" (long time unused) resources that could be freed */
      TimeType lastPurgeAged;
      /// Whether a "clear" is pending.
      bool clearReq;
      
      ReuseCondition& GetReuseCondition()
      { return *(static_cast<ReuseCondition*> (&availableResources)); }
      PurgeCondition& GetPurgeCondition()
      { return *(static_cast<PurgeCondition*> (&activeResources)); }
      
      class SearchDataTraverser
      {
	T* entry;
	Element*& ret;
	
      public:
	SearchDataTraverser (T* entry, Element*& ret) 
	  : entry (entry), ret (ret) {}
	
        bool operator() (ElementWrapper& el)
	{
	  if (&(el.ptr->data) == entry)
	  {
	    ret = el.ptr;
	    return false;
	  }
	  return true;
	}
      };
      
      Element* ElementFromData (T* entry)
      {
	// Given some cached data, obtain the Element from it.
	
	/* @@@ Right now the only way is to search both the active and
	 * available resource lists. */
	
	// Search active resource list.
	typename csList<ElementWrapper>::Iterator listIt (activeResources.v);
	while (listIt.HasNext())
	{
	  Element* el = listIt.Next().ptr;
	  if (&(el->data) == entry) return el;
	}
	
	// Search available resource tree.
	Element* treeSearchRet = 0;
	SearchDataTraverser sdt (entry, treeSearchRet);
	availableResources.v.TraverseInOrder (sdt);
	if (treeSearchRet != 0) return treeSearchRet;
	
	CS_ASSERT_MSG("Element is not from this resource cache", false);
	return 0;
      }
    public:
      /// Interval for the aged resource scan
      TimeType agedPurgeInterval;
    
      GenericResourceCache (const ReuseCondition& reuse = ReuseCondition(),
        const PurgeCondition& purge = PurgeCondition()) : 
	availableResources (reuse, elementAlloc),
        activeResources (purge, elementAlloc), 
        currentTime (0), lastPurgeAged (0),
	clearReq (false), agedPurgeInterval (60)
      {
      }
      
      ~GenericResourceCache()
      {
        availableResources.Destroy ();
      }

      /**
       * Clear all cached resources. \a instaClear immediately clears all
       * resources when set, if not, clearing is delayed until the next
       * AdvanceTime() call (useful when resources may still be in use).
       */
      void Clear (bool instaClear = false)
      {
	if (instaClear)
	{
	  availableResources.Destroy ();
	  activeResources.Destroy ();
	  clearReq = false;
	}
	else
	  // Don't clear just yet, rather, clear when we advance the next time
	  clearReq = true;
      }
      
#ifdef CS_DEBUG
      class VerifyTraverser
      {
	Element* el;
      public:
	VerifyTraverser (Element* el) : el (el) {}
	
        bool operator() (ElementWrapper& el)
	{
	  CS_ASSERT(el.ptr != this->el);
	  return true;
	}
      };
#endif

      void VerifyElementNotInTree (Element* el)
      {
        (void)el;
#ifdef CS_DEBUG
	VerifyTraverser verify (el);
	availableResources.v.TraverseInOrder (verify);
#endif
      }
      
      /**
       * Advance the time kept by the cache. Determines what resources
       * can be reused or even freed.
       */
      void AdvanceTime (TimeType time)
      {
	if (clearReq)
	{
	  Clear (true);
	  clearReq = false;
	}
	
	currentTime = time;
	
	if (time >= lastPurgeAged + agedPurgeInterval)
	{
	  typename AvailableResourcesTree::Iterator treeIt (
	    availableResources.v.GetIterator ());
	  while (treeIt.HasNext())
	  {
	    Element* el = treeIt.PeekNext ().ptr;
	    if (GetPurgeCondition().IsPurgeable (*this, 
		el->GetPurgeAuxiliary(), el->data))
	    {
	      availableResources.v.Delete (treeIt);
	      VerifyElementNotInTree (el);
	      elementAlloc.Free (el);
	    }
	    else
	    {
	      treeIt.Next ();
	    }
	  }
	  
	  lastPurgeAged = time;
	}
	
	typename csList<ElementWrapper>::Iterator listIt (activeResources.v);
	while (listIt.HasNext())
	{
	  ElementWrapper& el = listIt.Next();
	  
	  if (GetReuseCondition().IsReusable (*this,
	      el->GetReuseAuxiliary(), el->data))
	  {
	    VerifyElementNotInTree (el.ptr);
	    availableResources.v.Insert (el);
	    activeResources.v.Delete (listIt);
	  }
	}
      }
      TimeType GetCurrentTime() const { return currentTime; }

      /// Query a resource. Returns 0 if none is available.
      T* Query (const ResourceSortingKeyType& key = 
	ResourceSortingKeyType(), bool exact = false)
      {
	const AvailableResourcesTree& constTree = availableResources.v;
	ElementWrapper const* el;
	if (exact)
	  el = constTree.Find (key);
	else
	  el = constTree.FindGreatestSmallerEqual (key);
	if (el != 0)
	{
	  ElementWrapper myElement = *el;
	  availableResources.v.DeleteExact (el);
	  VerifyElementNotInTree (myElement.ptr);
	  activeResources.v.PushFront (myElement);
	  GetPurgeCondition().MarkActive (*this, myElement->GetPurgeAuxiliary());
	  GetReuseCondition().MarkActive (*this, myElement->GetReuseAuxiliary());
	  return &(myElement->data);
	}
	return 0;
      }
      /**
       * Add a resource as currently active. (But will be reused once 
       * possible.)
       */
      T* AddActive (const T& value,
        const ReuseConditionAddParameter& reuseParam = ReuseConditionAddParameter (),
        const PurgeConditionAddParameter& purgeParam = PurgeConditionAddParameter ())
      {
	ReuseConditionAuxiliary reuseAux (*this, reuseParam);
	PurgeConditionAuxiliary purgeAux (*this, purgeParam);
	Element* el = elementAlloc.Alloc (value, reuseAux, purgeAux);
        GetPurgeCondition().MarkActive (*this, el->GetPurgeAuxiliary());
        GetReuseCondition().MarkActive (*this, el->GetReuseAuxiliary());
	activeResources.v.PushFront (el);
        return &(el->data);
      }

      /**
       * Change the last used time of a resource to the current time. Can be
       * used to prevent resources which are tracked as "available" from
       * being purged.
       * \warning Never, ever, ever pass a resource which did *not* came from
       *  the instance of the cache you call this method on. Not heeding this
       *  warning will result in memory corruption.
       */
      void NudgeLastUsedTime (T* data)
      {
	Element* el = static_cast<Element*> (
	  DataStorage<typename ReuseCondition::StoredAuxiliaryInfo>::CastFromData (data));
	el->lastTimeUsed = currentTime;
      }


      /**
       * Manually mark a resource that is currently "active" as "available".
       * \warning Never, ever, ever pass a resource which did *not* came from
       *  the instance of the cache you call this method on. Not heeding this
       *  warning will result in memory corruption.
       */
      void SetAvailable (T* data)
      {
	Element* el = ElementFromData (data);
	  
	VerifyElementNotInTree (el);
	availableResources.v.Insert (el);
	activeResources.v.Delete (el);
      }
      
      /**
       * Free up a resource which is currently "active".
       */
      void RemoveActive (T* data)
      {
	Element* el = ElementFromData (data);
	  
        activeResources.v.Delete (el);
        elementAlloc.Free (el);
      }
      
      /**
       * Request the auxiliary information data of the reuse condition
       * mixin for a cache entry.
       */
      typename ReuseCondition::StoredAuxiliaryInfo* GetReuseAuxiliary (
        T* entry)
      {
        return &(ElementFromData (entry)->GetReuseAuxiliary());
      }
      
      /**
       * Request the auxiliary information data of the purge condition
       * mixin for a cache entry.
       */
      typename PurgeCondition::StoredAuxiliaryInfo* GetPurgeAuxiliary (
        T* entry)
      {
        return &(ElementFromData (entry)->GetPurgeAuxiliary());
      }
    };
    
  } // namespace Utility
} // namespace CS

#endif // __CS_CSUTIL_GENERICRESOURCECACHE_H__
