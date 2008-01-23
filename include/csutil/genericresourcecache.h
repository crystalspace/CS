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

#ifndef __CS_CSUTIL_GENERICRESOURCECACHE_H__
#define __CS_CSUTIL_GENERICRESOURCECACHE_H__

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
	  
	  template<typename ResourceCacheType>
	  StoredAuxiliaryInfo (const ResourceCacheType& cache, 
	    const AddParameter& param) : 
	    timeToDie (cache.GetCurrentTime() + param.lifeTime)
	  {}

	  template<typename ResourceCacheType>
	  bool IsReusable (const ResourceCacheType& cache)
	  {
	    return cache.GetCurrentTime() > timeToDie;
	  }
	};
	
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

	  template<typename ResourceCacheType>
	  bool IsReusable (const ResourceCacheType& cache)
	  {
	    return reusable;
	  }
	};
	
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
      typename ResourceSorting = ResourceCache::SortingNone,
      typename ReuseCondition = ResourceCache::ReuseConditionAfterTime<_TimeType> >
    class GenericResourceCache
    {
    public:
      typedef _TimeType TimeType;
    protected:
      template<typename Super>
      struct DataStorage : public CS::Memory::CustomAllocatedDerived<Super>
      {
	T data;
	
	template<typename A>
	DataStorage (const T& data, const A& a) : 
          CS::Memory::CustomAllocatedDerived<Super> (a), data (data) {}
      };
      enum NodeColor { Black = 0, Red = 1 };
      struct Element : 
	public DataStorage<typename ReuseCondition::StoredAuxiliaryInfo>
      {
	union
	{
	  Element* treeLeft;
	  Element* listPrev;
	};
	union
	{
	  Element* treeRight;
	  Element* listNext;
	};
	Element* treeParent;
	TimeType lastTimeUsed;
	
	Element (const T& data,
	  const typename ReuseCondition::StoredAuxiliaryInfo& reuseAux) : 
	  DataStorage<typename ReuseCondition::StoredAuxiliaryInfo> (data, reuseAux)
	{
	  treeLeft = 0;
	  treeRight = 0;
	  treeParent = 0;
	}
	
        inline Element* GetParent() const
        { return (Element*)((uintptr_t)treeParent & (uintptr_t)~1); }
        void SetParent(Element* p)
        { treeParent = (Element*)(((uintptr_t)p & (uintptr_t)~1) | (uint)GetColor()); }

	NodeColor GetColor() const
	{ // Expression split over two statements to pacify some broken gcc's which
	  // barf saying "can't convert Node* to NodeColor".
	  uintptr_t const v = ((uintptr_t)treeParent & 1); 
	  return (NodeColor)v;
	}
	void SetColor (NodeColor color)
	{ 
	  treeParent = (Element*)(((uintptr_t)treeParent & (uintptr_t)~1) 
	    | (uint)color); 
	}
      };
      
      /// Tree of 'Element's.
      /* Basically lifted csRedBlackTree. Duplicated b/c we actually have a
         tree/list hybrid. */
      struct ElementTree
      {
      private:
	void RecursiveFree (Element* el)
	{
	  if (el == 0) return;
	  RecursiveFree (el->treeLeft);
	  RecursiveFree (el->treeRight);
	  delete el;
	}
	/// Locate the place where a new node needs to be inserted.
	void RecursiveInsert (Element* parent, Element*& node, Element* el)
	{
	  if (node == 0)
	  {
	    node = el;
            node->SetParent (parent);
	    node->SetColor (Red);
	  }
	  else
	  {
	    bool gte = ResourceSorting::IsLargerEqual (el->data, node->data);
	    if (gte)
	      RecursiveInsert (node, node->treeRight, el);
	    else
	      RecursiveInsert (node, node->treeLeft, el);
	  }
	}
	/// Left-rotate subtree around 'pivot'.
	void RotateLeft (Element* pivot)
	{
	  Element* pivotReplace = pivot->treeRight;
	  pivot->treeRight = pivotReplace->treeLeft;
	  if (pivotReplace->treeLeft != 0) 
            pivotReplace->treeLeft->SetParent (pivot);
          Element* pivotParent = pivot->GetParent();
          pivotReplace->SetParent (pivotParent);
	  if (pivotParent == 0)
	    root = pivotReplace;
	  else
	  {
	    if (pivot == pivotParent->treeLeft)
	      pivotParent->treeLeft = pivotReplace;
	    else
	      pivotParent->treeRight = pivotReplace;
	  }
	  pivotReplace->treeLeft = pivot;
          pivot->SetParent (pivotReplace);
	}
	/// Right-rotate subtree around 'pivot'.
	void RotateRight (Element* pivot)
	{
	  Element* pivotReplace = pivot->treeLeft;
	  pivot->treeLeft = pivotReplace->treeRight;
	  if (pivotReplace->treeRight != 0)
            pivotReplace->treeRight->SetParent (pivot);
          Element* pivotParent = pivot->GetParent ();
          pivotReplace->SetParent (pivotParent);
	  if (pivotParent == 0)
	    root = pivotReplace;
	  else
	  {
	    if (pivot == pivotParent->treeLeft)
	      pivotParent->treeLeft = pivotReplace;
	    else
	      pivotParent->treeRight = pivotReplace;
	  }
	  pivotReplace->treeRight = pivot;
          pivot->SetParent (pivotReplace);
	}
	/// Check whether a node is black. Note that 0 nodes are by definition black.
	bool IsBlack (Element* node) const
	{ return (node == 0) || (node->GetColor() == Black); }
	/// Check whether a node is red.
	bool IsRed (Element* node) const
	{ return (node != 0) && (node->GetColor() == Red); }
	/// Fix up the RB tree after an insert.
	void InsertFixup (Element* node)
	{
          Element* nilParent = 0;
	  Element* p;
          while (((p = node->GetParent ()) != 0) && IsRed (p))
	  {
	    Element* pp = p->GetParent ();
	    //if (pp == 0) break;
	    if (p == pp->treeLeft)
	    {
	      Element* y = pp->treeRight;
	      if (IsRed (y))
	      {
		// Uncle of 'node' is red
		p->SetColor (Black);
		y->SetColor (Black);
		pp->SetColor (Red);
		node = pp;
	      }
	      else 
	      {
		if (node == p->treeRight)
		{
		  // Uncle of 'node' is black, node is right child
		  node = p;
		  RotateLeft (node);
                  p = node->GetParent ();
		}
		// Uncle of 'node' is black, node is left child
		p->SetColor (Black);
		pp->SetColor (Red);
		RotateRight (pp);
	      }
	    }
	    else
	    {
	      Element* y = pp->treeLeft;
	      if (IsRed (y))
	      {
		// Uncle of 'node' is red
		p->SetColor (Black);
		y->SetColor (Black);
		pp->SetColor (Red);
		node = pp;
	      }
	      else 
	      {
		if (node == p->treeLeft)
		{
		  // Uncle of 'node' is black, node is left child
		  node = p;
		  RotateRight (node);
                  p = node->GetParent ();
		}
		// Uncle of 'node' is black, node is right child
		p->SetColor (Black);
		pp->SetColor (Red);
		RotateLeft (pp);
	      }
	    }
	  }
	  root->SetColor (Black);
	}
	Element* RecursiveFind (Element* node, 
          const typename ResourceSorting::KeyType& key) const
	{
	  if (node == 0) return 0;
	  if (ResourceSorting::IsEqual (node->data, key))
	    return node;
	  else if (ResourceSorting::IsLargerEqual (node->data, key))
	    return RecursiveFindSmallestGreaterEqual (node->treeRight, key);
	  else 
	    return RecursiveFindSmallestGreaterEqual (node->treeLeft, key);
	}
	Element* RecursiveFindSmallestGreaterEqual (Element* node, 
          const typename ResourceSorting::KeyType& key) const
	{
	  if (node == 0) return 0;
	  if (ResourceSorting::IsLargerEqual (node->data, key))
	  {
	    if (node->treeRight != 0)
	      return RecursiveFindSmallestGreaterEqual (node->treeRight, key);
	    else 
	      return node;
	  }
	  else 
	    return RecursiveFindSmallestGreaterEqual (node->treeLeft, key);
	}
	/// Return smallest node with a key greater than 'node's.
	Element* Successor (Element* el) const
	{
	  Element* succ;
	  if (el->treeRight != 0)
	  {
	    succ = el->treeRight;
	    while (succ->treeLeft != 0) succ = succ->treeLeft;
	    return succ;
	  }
	  Element* y = el->GetParent();
          while ((y != 0) && (el == y->treeRight))
	  {
	    el = y;
	    y = y->GetParent ();
	  }
	  return y;
	}
	/// Fix up the RB tree after a deletion.
	void DeleteFixup (Element* node, Element* nilParent)
	{
          while ((node != root) && IsBlack (node))
	  {
            Element* p = node ? node->GetParent() : nilParent;
	    if (node == p->treeLeft)
	    {
	      Element* w = p->treeRight;
	      if (IsRed (w))
	      {
		w->SetColor (Black);
		p->SetColor (Red);
		RotateLeft (p);
		w = p->treeRight;
	      }
	      if (IsBlack (w->treeLeft) && IsBlack (w->treeRight))
	      {
		w->SetColor (Red);
		node = p;
	      }
	      else
	      {
		if (IsBlack (w->treeRight))
		{
		  w->treeLeft->SetColor (Red);
		  w->SetColor (Red);
		  RotateRight (w);
		  w = p->treeRight;
		}
		w->SetColor (p->GetColor ());
		p->SetColor (Black);
		w->treeRight->SetColor (Black);
		RotateLeft (p);
		node = root;
	      }
	    }
	    else
	    {
	      Element* w = p->treeLeft;
	      if (IsRed (w))
	      {
		w->SetColor (Black);
		p->SetColor (Red);
		RotateRight (p);
		w = p->treeLeft;
	      }
	      if (IsBlack (w->treeLeft) && IsBlack (w->treeRight))
	      {
		w->SetColor (Red);
		node = p;
	      }
	      else
	      {
		if (IsBlack (w->treeLeft))
		{
		  w->treeRight->SetColor (Red);
		  w->SetColor (Red);
		  RotateLeft (w);
		  w = p->treeLeft;
		}
		w->SetColor (p->GetColor ());
		p->SetColor (Black);
		w->treeLeft->SetColor (Black);
		RotateRight (p);
		node = root;
	      }
	    }
	  }
	  if (node != 0) node->SetColor (Black);
	}
      public:
	Element* root;
      
        ElementTree() : root (0) {}
	~ElementTree() { Destroy(); }
	
	void Destroy ()
	{
	  RecursiveFree (root);
	}
	
	void Insert (Element* el)
	{
	  el->treeLeft = 0;
	  el->treeRight = 0;
	  RecursiveInsert (0, root, el);
	  InsertFixup (el);
	}
	Element* Find (const typename ResourceSorting::KeyType& key)
	{
	  return RecursiveFind (root, key);
	}
	Element* FindSmallestGreaterEqual (const typename ResourceSorting::KeyType& key)
	{
	  return RecursiveFindSmallestGreaterEqual (root, key);
	}
        void SwapElements (Element* a, Element* b)
        {
          /*
            Swap places of two nodes in the tree.
            Note that when this method is called, nothing can be assumed about
            the sanity of the nodes - e.g. they don't have to be a child of the
            node pointed to by treeParent. In fact, the nodes can be quite insane
            and e.g. have the same nodes as a child. Thus a lot of checks before
            anything is changed to avoid corruption that insanity can cause.
           */
          if (a->GetParent() == 0)
            root = b;
          else
          {
            if (a == a->GetParent()->treeLeft)
              a->GetParent()->treeLeft = b;
            if (a == a->GetParent()->treeRight)
              a->GetParent()->treeRight = b;
          }
          if (b->GetParent() == 0)
            root = a;
          else
          {
            if (b == b->GetParent()->treeLeft)
              b->GetParent()->treeLeft = a;
            if (b == b->GetParent()->treeRight)
              b->GetParent()->treeRight = a;
          }
          {
            Element* p = a->GetParent ();
            a->SetParent (b->GetParent ());
            b->SetParent (p);
          }
          CS::Swap (a->treeLeft, b->treeLeft);
          if (a->treeLeft == b->treeLeft)
          {
            if (a->treeLeft != 0)
            {
              if (a->treeLeft->GetParent() == a)
                a->treeLeft->SetParent (b);
              else if (a->treeLeft->GetParent() == b)
                a->treeLeft->SetParent (a);
            }
          }
          else
          {
            if (a->treeLeft != 0) a->treeLeft->SetParent (a);
            if (b->treeLeft != 0) b->treeLeft->SetParent (b);
          }
          CS::Swap (a->treeRight, b->treeRight);
          if (a->treeRight == b->treeRight)
          {
            if (a->treeRight != 0)
            {
              if (a->treeRight->GetParent() == a)
                a->treeRight->SetParent (b);
              else if (a->treeRight->GetParent() == b)
                a->treeRight->SetParent (a);
            }
          }
          else
          {
            if (a->treeRight != 0) a->treeRight->SetParent (a);
            if (b->treeRight != 0) b->treeRight->SetParent (b);
          }

          {
            NodeColor c = a->GetColor();
            a->SetColor (b->GetColor ());
            b->SetColor (c);
          }
        }
	void Delete (Element* el)
	{
	  Element* y; // Node that will actually be spliced out
	  if ((el->treeLeft == 0) || (el->treeRight == 0))
	    y = el;
	  else
	    y = Successor (el);

	  Element* x; // Node we will put in y's place
	  if (y->treeLeft != 0)
	    x = y->treeLeft;
	  else
	    x = y->treeRight;
          Element* nilParent = 0;
	  if (x != 0) 
            x->SetParent (y->GetParent ());
          else
            nilParent = y->GetParent ();
	  if (y->GetParent () == 0)
	    root = x;
	  else
	  {
	    if (y == y->GetParent ()->treeLeft)
	      y->GetParent ()->treeLeft = x;
	    else
	      y->GetParent ()->treeRight = x;
	  }

          int yColor = y->GetColor();
	  if (yColor == Black)
          {
	    DeleteFixup (x, nilParent);
          }
          if (y != el)
          {
            /* We actually want to splice out 'el', so if we end up with
               another node instead, swap all fields.
             */
            CS_ASSERT((el->GetParent() != 0) || (root == el));
            CS_ASSERT((el->GetParent() == 0)
              || (el->GetParent()->treeLeft == el)
              || (el->GetParent()->treeRight == el));
            CS_ASSERT((el->treeLeft == 0) || (el->treeLeft->GetParent() == el));
            CS_ASSERT((el->treeRight == 0) || (el->treeRight->GetParent() == el));
            SwapElements (y, el);
            CS_ASSERT((y->GetParent() != 0) || (root == y));
            CS_ASSERT((y->GetParent() == 0)
              || (y->GetParent()->treeLeft == y)
              || (y->GetParent()->treeRight == y));
            CS_ASSERT((y->treeLeft == 0) || (y->treeLeft->GetParent() == y));
            CS_ASSERT((y->treeRight == 0) || (y->treeRight->GetParent() == y));
          }
	}
      };
      
      struct ElementList
      {
	Element* head;
	
        ElementList() : head (0) {}
	~ElementList() { Destroy(); }
	
	void Destroy ()
	{
	  Element* el = head;
	  while (el != 0)
	  {
	    Element* next = el->listNext;
	    delete el;
	    el = next;
	  }
	  head = 0;
	}
	
	void PushFront (Element* el)
	{
	  el->listNext = head;
	  el->listPrev = 0;
	  if (head)
	    head->listPrev = el;
	  head = el;
	}
	void Delete (Element *el)
	{
	  // Fix the pointers of the 2 surrounding elements
	  if (el->listPrev)
	    el->listPrev->listNext = el->listNext;
	  else
	    head = el->listNext;
	
	  if (el->listNext)
	    el->listNext->listPrev = el->listPrev;
	}
	
      };
      
      // Tree of available resources
      ElementTree availableResources;
      // List of active resources
      ElementList activeResources;
      
      TimeType currentTime;
      /**
       * The next time when the momentarily available resources should be 
       * scanned for "aged" (long time unused) resources that can be freed */
      TimeType nextPurgeAged;
      /// Whether a "clear" is pending.
      bool clearReq;
      
      void RecursivePurgeAged (Element* el)
      {
	if (el == 0) return;
	
        Element* l = el->treeLeft;
        Element* r = el->treeRight;
	RecursivePurgeAged (l);
	RecursivePurgeAged (r);
	
	if (currentTime > el->lastTimeUsed + purgeAge)
	{
	  availableResources.Delete (el);
	  delete el;
	}
      }
    public:
      /// Interval for the aged resource scan
      TimeType agedPurgeInterval;
      /**
       * Time interval since last use after which a resource is considered
       * "aged" and may be purged
       */
      TimeType purgeAge;
    
      GenericResourceCache() : currentTime (0), nextPurgeAged (0),
	clearReq (false), agedPurgeInterval (60),
	purgeAge (agedPurgeInterval)
      {
      }
      
      ~GenericResourceCache()
      {
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
	
	if (time >= nextPurgeAged)
	{
	  RecursivePurgeAged (availableResources.root);
	  nextPurgeAged = time + agedPurgeInterval;
	}
	
	Element* active = activeResources.head;
	while (active != 0)
	{
	  Element* next = active->listNext;
	  if (active->IsReusable (*this))
	  {
	    activeResources.Delete (active);
	    availableResources.Insert (active);
	  }
	  active = next;
	}
      }
      TimeType GetCurrentTime() const { return currentTime; }

      /// Query a resource. Returns 0 if none is available.
      T* Query (const typename ResourceSorting::KeyType& key = 
	ResourceSorting::KeyType(), bool exact = false)
      {
	Element* el;
	if (exact)
	  el = availableResources.Find (key);
	else
	  el = availableResources.FindSmallestGreaterEqual (key);
	if (el != 0)
	{
	  availableResources.Delete (el);
	  activeResources.PushFront (el);
	  el->lastTimeUsed = currentTime;
	  return &(el->data);
	}
	return 0;
      }
      /**
       * Add a resource as currently active. (But will be reused once 
       * possible.)
       */
      T* AddActive (const T& value, 
	const typename ReuseCondition::AddParameter& reuseParam = 
	  ReuseCondition::AddParameter ())
      {
	Element* el = new Element (value, 
          typename ReuseCondition::StoredAuxiliaryInfo (*this, reuseParam));
        el->lastTimeUsed = currentTime;
	activeResources.PushFront (el);
        return &(el->data);
      }

      /**
       * Request the auxiliary information data of the reuse condition
       * mixin for a cache entry.
       */
      typename ReuseCondition::StoredAuxiliaryInfo* GetReuseAuxiliary (
        T* entry)
      {
        typedef DataStorage<typename ReuseCondition::StoredAuxiliaryInfo> DS;
        DS* ds = 0;
        const size_t dataOffs = (uintptr_t)&(ds->data);
        return reinterpret_cast<DS*> ((char*)entry - dataOffs);
      }
    };
    
  } // namespace Utility
} // namespace CS

#endif // __CS_CSUTIL_GENERICRESOURCECACHE_H__
