/*
    Copyright (C) 2011 by Frank Richter

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

#ifndef __CSUTIL_WEAKKEYEDHASH_H__
#define __CSUTIL_WEAKKEYEDHASH_H__

#include "hash.h"

namespace CS
{
  namespace Container
  {
    /**
     * Hash type that allows (certain) changes of key objects.
     *
     * This hash type works with keys that may be 'invalidated' as a side
     * effect of some other operation - prime example is csWeakRef<>
     * which gets cleared if the referenced object is destroyed.
     * However, apart from invalidation, key objects may not change their
     * values.
     *
     * The validity of a key is tested by evaluating it to a \c bool.
     * Invalid keys should evaluate to \c false (conversely, valid keys
     * should evaluate to \c true).
     *
     * Invalid keys, and values associated with them, are occasionally
     * cleaned out as a part of normal operation (element retrieval,
     * element insertion ...), hence it is likely to occur that the value
     * associated with a key that got invalidated will stay allocated
     * for an unknown amount of time.
     */
    template <class T, class K,
      class ArrayMemoryAlloc = CS::Container::ArrayAllocDefault,
      class ArrayElementHandler = csArraySafeCopyElementHandler<
	CS::Container::HashElement<T, K> > > 
    class WeakKeyedHash :
      protected csHash<T, K, ArrayMemoryAlloc, ArrayElementHandler>
    {
      typedef csHash<T, K, ArrayMemoryAlloc, ArrayElementHandler> Superclass;
    public:
      
      /**
       * Get the first element matching the given key, or \a fallback if there is 
       * none.
       */
      const T& Get (const K& key, const T& fallback)
      {
	if (this->Elements.GetSize() == 0) return fallback;
	typename Superclass::ElementArray& values = 
	  this->Elements[csHashComputer<K>::ComputeHash (key) % this->Modulo];
	const size_t len = values.GetSize ();
	for (size_t i = 0; i < len; ++i)
	{
	  const typename Superclass::Element& v = values[i];
	  // Delete any elements with 'invalid' keys while searching
	  if (!v.GetKey())
	  {
	    values.DeleteIndexFast (i);
	    this->Size--;
	    i--;
	    continue;
	  }
	  if (csComparator<K, K>::Compare (v.GetKey(), key) == 0)
	    return v.GetValue();
	}

	return fallback;
      }
      using Superclass::Get;
      
      /**
       * Add an element to the hash table.
       * \remarks If \a key is already present, does NOT replace the existing value,
       *   but merely adds \a value as an additional value of \a key. To retrieve all
       *   values for a given key, use GetAll(). If you instead want to replace an
       *   existing value for \a key, use PutUnique().
       */
      T& Put (const K& key, const T &value)
      {
	if (this->Elements.GetSize() == 0) this->Elements.SetSize (this->Modulo);
	typename Superclass::ElementArray& values = 
	  this->Elements[csHashComputer<K>::ComputeHash (key) % this->Modulo];
	// Delete any elements 'invalid' with keys while looking for a slot
	size_t idx = 0;
	while (idx < values.GetSize())
	{
	  if (!values[idx].GetKey())
	  {
	    break;
	  }
	  idx++;
	}
	if (idx >= values.GetSize())
	{
	  values.Push (typename Superclass::Element (key, value));
	  this->Size++;
	}
	if (values.GetSize () > this->Elements.GetSize () / this->GrowRate
	  && this->Elements.GetSize () < this->MaxSize)
	{
	  this->Grow ();
	  /* can't use 'values[idx]' since that is no longer the place where
	    the item is stored. */
	  return *(this->GetElementPointer (key));
	}
	return values[idx].GetValue();
      }
      
      using Superclass::DeleteAll;
      
      /// An iterator class for WeakKeyedHash.
      class GlobalIterator
      {
      private:
	typedef typename Superclass::GlobalIterator WrappedIterator;
	WrappedIterator iter;
	
	bool haveNext;
	K key;
	T* value;
      protected:
	void NextElement ()
	{
	  while (iter.HasNext ())
	  {
	    K key;
	    value = &(iter.Next (key));
	    if (key)
	    {
	      this->key = key;
	      this->value = value;
	      haveNext = true;
	      break;
	    }
	    // @@@ Would be nice to clear out invalid keys as well here
	    // like: ...->DeleteElement (*this);
	  }
	  haveNext = false;
	  key = K ();
	}
	
	GlobalIterator (const WrappedIterator& iter)
	 : iter (iter)
	{
	  NextElement ();
	}

	friend class WeakKeyedHash<T, K, ArrayMemoryAlloc, ArrayElementHandler>;
      public:
	/// Copy constructor.
	GlobalIterator (const GlobalIterator& o)
	 : iter (o.iter), haveNext (o.haveNext), key (o.key), value (o.value) {}

	/// Assignment operator.
	GlobalIterator& operator=(const GlobalIterator& o)
	{
	  iter = o.iter;
	  haveNext = o.haveNext;
	  key = o.key;
	  value = o.value;
	  return *this;
	}

	/// Returns a boolean indicating whether or not the hash has more elements.
	bool HasNext () const
	{
	  return haveNext;
	}

	/// Advance the iterator of one step
	void Advance ()
	{
	  NextElement ();
	}

	/// Get the next element's value, don't move the iterator.
	T& NextNoAdvance ()
	{
	  return *value;
	}

	/// Get the next element's value.
	T& Next ()
	{
	  T &ret = NextNoAdvance ();
	  Advance ();
	  return ret;
	}

	/// Get the next element's value and key, don't move the iterator.
	T& NextNoAdvance (K &key)
	{
	  key = this->key;
	  return NextNoAdvance ();
	}

	/// Get the next element's value and key.
	T& Next (K &key)
	{
	  key = this->key;
	  return Next ();
	}

	/// Return a tuple of the value and key.
	const csTuple2<T, K> NextTuple ()
	{
	  csTuple2<T, K> t (NextNoAdvance (),
	    this->key);
	  Advance ();
	  return t;
	}

	/// Move the iterator back to the first element.
	void Reset ()
	{
	  iter->Reset ();
	  NextElement ();
	}
      };
      friend class GlobalIterator;
      
      /**
       * Return an iterator for the hash, to iterate over all elements.
       * \warning Modifying the hash (except with DeleteElement()) while you have
       *   open iterators will result in undefined behaviour.
       */
      GlobalIterator GetIterator ()
      {
	return GlobalIterator (Superclass::GetIterator ());
      }

      // @@@ FIXME: More csHash<> methods (and iterators) need to be 'ported'
    };
  } // namespace Container
} // namespace CS

#endif // __CSUTIL_WEAKKEYEDHASH_H__
