/*
    Copyright (C) 2007 by Marten Svanfeldt

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

#ifndef __CS_CSUTIL_FIXEDSIZECACHE_H__
#define __CS_CSUTIL_FIXEDSIZECACHE_H__

#include "csutil/metautils.h"
#include "csutil/compileassert.h"
#include "csutil/hashcomputer.h"

#include "csutil/custom_new_disable.h"

namespace CS
{
  namespace Utility
  {
    namespace Implementation
    {

      /**
       * Represent one element in the cache with key and data.
       * The element can be in one of three states:
       *  - Invalid
       *  - Key valid
       *  - Key and data valid
       *
       * However, for efficiency reasons it does not keep track of its current
       * state, that must be done by the user.
       */
      template<typename K, typename T>
      class FixedSizeCacheElement
      {
      public:
        /// Set key, assuming its invalid
        void SetKey(const K& k)
        {
          new (key) K (k);
        }

        /// Get current value of the key
        const K& GetKey() const
        {
          return *reinterpret_cast<const K*>(&key);
        }

        /// Invalidate the key
        void InvalidateKey()
        {
          reinterpret_cast<K*>(&key)->~K();
        }

        /// Set the data item, assuming its invalid
        void SetData(const T& t)
        {
          new (data) T (t);
        }

        /// Get current value of the data
        const T& GetData() const
        {
          return *reinterpret_cast<const T*>(&data);
        }

        /// Invalidate the data
        void InvalidateData()
        {
          reinterpret_cast<T*>(&data)->~T();
        }

      protected:
      private:
        /// Packed storage of key and data
        union
        {
          uint8 key[sizeof(K)];
          typename CS::Meta::TypeWithAlignment<CS::Meta::AlignmentOf<K>::value >::Type _k_align;
        };
        union
        {
          uint8 data[sizeof(T)];
          typename  CS::Meta::TypeWithAlignment<CS::Meta::AlignmentOf<T>::value >::Type _t_align;
        };
      };


      /**
       * One set in an associative cache. Holds a power-of-two number of elements,
       * subject to replacement using a LRU policy.
       *       
       * \param K Key type used to index the cache. Must be hashable and comparable.
       * \param T Data type stored in cache. Must be copy-constructable.
       * \param SetSize Number of elements in set.
       * \param LRUPolicy Template policy for the cache eviction LRU.
       */
      template<typename K, typename T, unsigned int SetSize, typename LRUPolicy>
      class FixedSizeCacheSet
      {
      public:
        typedef FixedSizeCacheSet<K, T, SetSize, LRUPolicy> ThisType;
        typedef FixedSizeCacheElement<K, T> ElementType;
        typedef typename LRUPolicy::template LRU<SetSize>::Type LRUType;

        /// Initialize an empty cache set
        FixedSizeCacheSet()
        {
          for (size_t i = 0; i < auxDataSize; ++i)
            auxData[i] = 0;
        }

        /// Destroy any remaning items in cache set
        ~FixedSizeCacheSet()
        {
          for (size_t i = 0; i < SetSize; ++i)
          {
            if(IsElementKeyValid (i))
            {
              elements[i].InvalidateKey ();
            }
            if(IsElementDataValid (i))
            {
              elements[i].InvalidateData ();
            }
          }
        }

        /**
         * Insert an element and key into the set.
         * If key already exists, no update will happen
         *
         * \param key Cache item key
         * \param data Cache item data
         * \return true if insert is successful
         */
        bool Insert (const K& key, const T& data)
        {
          // Check if data exists in set
          ElementType* e = Find(key);
          if(e)
          {
            return false;
          }

          // Find a spot to put it
          e = GetEmptyElement();
          e->SetKey (key);
          e->SetData (data);
          size_t i = e - elements;
          lru.Update (i);

          return true;
        }

        /**
         * Update an existing element in the set.
         * If the key is non-existent it will not be added
         *
         * \param key Cache item key
         * \param data New data for cached item
         */
        void Update (const K& key, const T& data)
        {
          ElementType* e = Find(key);
          if(e)
          {
            size_t i = e - elements;
            if (IsElementDataValid (i))
            {
              e->InvalidateData ();
            }
            
            e->SetData (data);
            lru.Update (i);
          }
        }

        /**
         * Update an existing element in the cache, inserting it if needed.
         *
         * \param key Cache item key
         * \param data New data for cached item
         */
        void InsertOrUpdate (const K& key, const T& data)
        {
          size_t i = 0;
          ElementType* e = Find(key);
          if(e)
          {
            i = e - elements;
            if (IsElementDataValid (i))
            {
              e->InvalidateData ();
            }

            e->SetData (data);
          }
          else
          {
            // Find a spot to put it
            e = GetEmptyElement();
            e->SetKey (key);
            e->SetData (data);
            i = e - elements;
          }
          lru.Update (i);          

          return;
        }

        /**
         * Find an element with given key in set
         *
         * \param key Cache item key
         * \return the element if found, otherwise 0
         */
        ElementType* Find (const K& key)
        {
          // Use linear probing for now
          for (size_t i = 0; i < SetSize; ++i)
          {
            if (IsElementKeyValid (i) &&
              csComparator<K, K>::Compare (elements[i].GetKey (), key) == 0)
            {
              return &elements[i];
            }
          }
          return 0;
        }

        /**
         * Get a possibly existing item in cache.
         *
         * \param key Cache item key
         * \param data Cached data. Updated on successful retrieval from cache.
         * \return true if item exists in cache.
         */
        bool Get (const K& key, T& data)
        {
          ElementType* e = Find (key);
          size_t i = e - elements;
          if (e && IsElementDataValid (i))
          {
            data = e->GetData ();
            lru.Update (i);
            return true;
          }

          return false;
        }

        /// Invalidate the data associated with given key, without remving the key itself
        void Invalidate (const K& key)
        {
          ElementType* e = Find (key);
          size_t i = e - elements;
          if (e && IsElementDataValid (i))
          {
            e->InvalidateData ();
          }
        }
             
      private:
        /// Get if one element has valid key
        bool IsElementKeyValid(size_t index) const
        {
          // Get bit 2*index
          index *= 2;
          const uint8 a = (SetSize < 8) ? auxData[0] : auxData[index >> 3];
          return (a & (1 << (index & 0x7))) != 0;
        }

        /// Set element to have valid key
        void SetElementKeyValid(size_t index, bool valid)
        {
          // Get bit 2*index
          index *= 2;
          uint8& a = auxData[index >> 3];
          if(valid)
            a |= (1 << (index & 0x7));
          else
            a &= ~(1 << (index & 0x7));
        }

        /// Get if one element has valid data
        bool IsElementDataValid(size_t index) const
        {
          // Get bit 2*index+1
          index = 2*index + 1;
          const uint8 a = auxData[index >> 3];
          return (a & (1 << (index & 0x7))) != 0;
        }

        /// Set element to have valid data
        void SetElementDataValid(size_t index, bool valid)
        {
          // Get bit 2*index+1
          index = 2*index + 1;
          uint8& a = auxData[index >> 3];
          if(valid)
            a |= (1 << (index & 0x7));
          else
            a &= ~(1 << (index & 0x7));
        }

        /// Get an element that can be reused for a new object.
        ElementType* GetEmptyElement()
        {           
          size_t i = 0;
          bool foundEmpty = false;

          for (i = 0; i < SetSize; ++i)
          {
            if (!IsElementDataValid (i))
            {
              foundEmpty = true;
              break;
            }
          }

          if(!foundEmpty)
          {
            i = lru.GetVictim ();
          }

          ElementType& e = elements[i];

          if (IsElementKeyValid (i))
          {
            e.InvalidateKey ();
          }
          if (IsElementDataValid (i))
          {
            e.InvalidateData ();
          }
          return &e;
        }

        /// The storage itself        
        ElementType elements[SetSize];

        /**
         * Auxiliary data needed for each element
         * Data is encoded as:
         * Aux   Key      Data
         * 00    Invalid  Invalid
         * 01    Valid    Invalid
         * 10    *        *
         * 11    Valid    Valid
         */
        static const size_t auxDataSize = ((SetSize*2)+7) / 8;
        uint8 auxData[auxDataSize];

        /// LRU to use
        LRUType lru;

        // Check that some invariants are met
        // Require that size is log2
        CS_COMPILE_ASSERT(CS::Meta::IsLog2<SetSize>::value);
      };

      /// Helper to compute the number of sets needed for a given hash size and associativity
      template<unsigned int Associativity>
      struct SetNumberComputer
      {
        template<size_t CacheSize>
        struct Helper
        {
          static const unsigned int value = CacheSize / Associativity;
        };
      };

      /// Special case, associativity 0 means fully associative
      template<>
      struct SetNumberComputer<0>
      {
        template<size_t CacheSize>
        struct Helper
        {
          static const unsigned int value = 1;
        };
      };


      /**
       * LRU implemented as a fixed size array storing the indices in order
       * of reference.
       * Performs best for relatively small sizes.
       *
       * \param Size LRU size
       */
      template<size_t Size>
      class FixedSizeLRU
      {
      public:
        /// Initialize the LRU array
        FixedSizeLRU()
        {
          for (size_t i = 0; i < Size; ++i)
          {
            lruStorage[i] = (LRUType)i;
          }
        }

        /// Set index as last accessed
        void Update (size_t index)
        {
          size_t i = 0;

          while(lruStorage[i] != index)
            i++;

          while(i > 0)
          {
            lruStorage[i] = lruStorage[i-1];
            --i;
          }

          lruStorage[0] = (LRUType)index;
        }

        /// Return last recently used item
        size_t GetVictim () const
        {
          return lruStorage[Size-1];
        }

      private:
        /// Number of bytes needed to store number < Size
        static const size_t Log2Size = CS::Meta::Log2<Size>::value;
        static const size_t MinBytesInSize = (Log2Size + 7) / 8;
        typedef typename CS::Meta::TypeOfSize<MinBytesInSize>::Type LRUType;

        /// Storage
        LRUType lruStorage[Size];
      };

      /**
       * Specialized LRU for one item sets. Item 0 is always the LRU item.
       */
      template<>
      class FixedSizeLRU<1>
      {
      public:
        /// Set index as last accessed
        void Update (size_t index)
        {}

        /// Return last recently used item
        size_t GetVictim () const
        {
          return 0;
        }
      };

      /**
       * Specialized LRU for two item sets.
       */
      template<>
      class FixedSizeLRU<2>
      {
      public:
        FixedSizeLRU()
          : lruStorage(0)
        {}

        /// Set index as last accessed
        void Update (size_t index)
        {
          lruStorage = (uint8)(index & 0xff);
        }

        /// Return last recently used item
        size_t GetVictim () const
        {
          return 1-lruStorage;
        }

      private:
        uint8 lruStorage;
      };

      /**
       * LRU policy using the tree/pseudo-LRU algorithm for general power-of-two
       * sized sets.
       *
       * \param Size number of elements in set
       */
      template<size_t Size>
      class FixedSizePseudoLRU
      {
      public:
        /// Initialize
        FixedSizePseudoLRU ()
        {
          for (size_t i = 0; i < TreeNodeQW; ++i)
          {
            lruTree[i] = 0;
          }
        }

        /// Set index as last accessed
        void Update (size_t index)
        {
          // Magic...
          size_t currNode = 0;
          size_t currStep = Size/2;

          while (currStep > 0)
          {
            // Check if we go left or right at current node
            if (index < currStep)
            {
              // Left
              SetBit (currNode);
              currNode = 2*currNode + 1;
            }
            else
            {
              // Right
              ClearBit (currNode);
              currNode = 2*currNode + 2;
              index -= currStep;
            }

            currStep /= 2;
          }
        }

        /// Return last recently used item
        size_t GetVictim () const
        {
          // Traverse tree until we find what we want
          size_t currNode = 0;
          size_t currVictim = 0;
          size_t currStep = Size/2;

          while (currStep > 0)
          {
            if (IsBitSet (currNode))
            {
              // Go right
              currVictim += currStep;
              currNode = 2*currNode + 2;
            }
            else
            {
              // Go left
              currNode = 2*currNode + 1;
            }

            currStep /= 2;
          }

          return currVictim;
        }


      private:
        void SetBit (size_t index)
        {
          uint32& a = lruTree[index >> 0x5];
          a |= 1 << (index & 0x1F);
        }

        void ClearBit (size_t index)
        {
          uint32& a = lruTree[index >> 0x5];
          a &= ~(1 << (index & 0x1F));
        }

        bool IsBitSet (size_t index) const
        {
          const uint32& a = lruTree[index >> 0x5];
          return (a & (1 << (index & 0x1F))) != 0;
        }

        static const size_t TreeNodeCount = Size-1;
        static const size_t TreeNodeQW = (TreeNodeCount+31) / 32;
        uint32 lruTree[TreeNodeQW];

        // Check that some invariants are met
        // Require that size is log2
        CS_COMPILE_ASSERT(CS::Meta::IsLog2<Size>::value);
      };

      /**
       * Specialized pseudo-LRU for set size 4. Use direct bit-patterns instead
       * of generic tree algorithm.
       */
      template<>
      class FixedSizePseudoLRU<4>
      {
      public:
        FixedSizePseudoLRU()
          : lruStorage (0)
        {}

        /// Set index as last accessed
        void Update (size_t index)
        {
          switch(index)
          {
          case 0:
            lruStorage = (lruStorage & 0x1) | 0x5;
            break;
          case 1:
            lruStorage = (lruStorage & 0x1) | 0x4;
            break;
          case 2:
            lruStorage = (lruStorage & 0x2) | 0x1;
            break;
          case 3:
            lruStorage = (lruStorage & 0x2);
            break;
          default:
            CS_ASSERT(false);
          }
        }

        /// Return last recently used item
        size_t GetVictim () const
        {
          if (lruStorage & 0x4)
          {
            return (lruStorage & 0x1) ? 3 : 2;
          }
          else
          {
            return (lruStorage & 0x2) ? 1 : 0;
          }
        }

      private:
        uint8 lruStorage;
      };

      /**
       * Specialized pseudo-LRU for set size 8. Use direct bit-patterns instead
       * of generic tree algorithm.
       */
      template<>
      class FixedSizePseudoLRU<8>
      {
      public:
        FixedSizePseudoLRU()
          : lruStorage (0)
        {}

        /// Set index as last accessed
        void Update (size_t index)
        {
          static const uint8 value[] = 
          {
            0xB, 0x3,
            0x11, 0x1,
            0x24, 0x4,
            0x40, 0x0
          };

          static const uint8 mask[] = 
          {
            0xB, 0xB,
            0x13, 0x13,
            0x25, 0x25,
            0x45, 0x45
          };

          lruStorage = (lruStorage & ~mask[index]) | value[index];
        }

        /// Return last recently used item
        size_t GetVictim () const
        {       
          static const uint8 value[] = 
          {
            0x0, 0x8,
            0x2, 0x12,
            0x1, 0x21,
            0x5, 0x45
          };

          static const uint8 mask[] = 
          {
            0xB, 0xB,
            0x13, 0x13,
            0x25, 0x25,
            0x45, 0x45
          };

          for (size_t i = 0; i < 8; ++i)
          {
            if ((lruStorage & mask[i]) == value[i])
              return i;
          }

          CS_ASSERT(false);
          return 0;
        }

      private:
        static const uint8 mask[8];

        uint8 lruStorage;
      };      

      /// Helper for FixedSizeBestChoiceLRUPolicy
      template<unsigned int Size>
      struct FixedSizeBestChoiceLRU
      {
        typedef FixedSizePseudoLRU<Size> Type;
      };

      /// Helper for FixedSizeBestChoiceLRUPolicy
      template<>
      struct FixedSizeBestChoiceLRU<1>
      {
        typedef FixedSizeLRU<1> Type;
      };

      /// Helper for FixedSizeBestChoiceLRUPolicy
      template<>
      struct FixedSizeBestChoiceLRU<2>
      {
        typedef FixedSizeLRU<2> Type;
      };

    } // namespace Implementation


    /**
     * LRU policy using an array based fixed-size LRU.
     * Useful mostly for small set sizes.
     */
    class FixedSizeLRUPolicy
    {
    public:
      template<size_t SetSize>
      struct LRU
      {
        typedef Implementation::FixedSizeLRU<SetSize> Type;
      };
    };

    /**
     * LRU policy using bit-tree base pseudo-LRU.
     * Performs better than FixedSizeLRUPolicy for set sizes equal to or larger
     * than 4 or 8.
     */
    class FixedSizePseudoLRUPolicy
    {
    public:
      template<size_t SetSize>
      struct LRU
      {
        typedef Implementation::FixedSizePseudoLRU<SetSize> Type;
      };
    };
    
    /**
     * LRU policy deciding between array LRU and pseudo-LRU based on set size.
     * Current switch-over point is 4 entries.
     */
    class FixedSizeBestChoiceLRUPolicy
    {
    public:
      template<size_t SetSize>
      struct LRU
      {
        typedef typename Implementation::FixedSizeBestChoiceLRU<SetSize>::Type Type;
      };
    };


    /**
     * Templated fixed size cache class.
     * The cache can be organized as a 1-way, N-way (where N is a power of two)
     * or fully associative cache. 
     * 
     * \param K Key type used to index the cache. Must be hashable and comparable.
     * \param T Data type stored in cache. Must be copy-constructable.
     * \param CacheSize Number of elements in total in cache
     * \param Associativity Associativity of cache. 0 means fully associative.
     * \param LRUPolicy Template policy for the per set cache eviction LRU.
     */
    template<
      typename K, 
      typename T, 
      unsigned int CacheSize,
      unsigned int Associativity = 1, 
      typename LRUPolicy = FixedSizeBestChoiceLRUPolicy,
      typename HashFold = CS::Utility::HashFoldingFNV1>
    class FixedSizeCache
    {
    public:
      // Some computed constants
      typedef Implementation::SetNumberComputer<Associativity> SetComputer;
      static const unsigned int NumberOfSets = 
        (SetComputer::template Helper<CacheSize>::value);
      static const unsigned int SetSize = CacheSize / NumberOfSets;
      
      typedef FixedSizeCache<K, T, CacheSize, Associativity, LRUPolicy> ThisType;
      typedef Implementation::FixedSizeCacheSet<K, T, SetSize, LRUPolicy> SetType;

      /**
       * Insert an element and key into the cache.
       * If key already exists, no update will happen
       *
       * \param key Cache item key
       * \param data Cache item data
       * \return true if insert is successful
       */
      bool Insert (const K& key, const T& data)
      {
        // Find which set
        uint h = csHashComputer<K>::ComputeHash (key) & (NumberOfSets-1);
        h = HashFold::FoldHash(h);

        SetType& set = sets[h];

        return set.Insert (key, data);
      }

      /**
       * Update an existing element in the cache.
       * If the key is non-existent it will not be added
       *
       * \param key Cache item key
       * \param data New data for cached item
       */
      void Update (const K& key, const T& data)
      {
        // Find which set
        uint h = csHashComputer<K>::ComputeHash (key) & (NumberOfSets-1);
        h = HashFold::FoldHash(h);

        SetType& set = sets[h];

        set.Update (key, data);
      }

      /**
       * Update an existing element in the cache, inserting it if needed.
       *
       * \param key Cache item key
       * \param data New data for cached item
       */
      void InsertOrUpdate (const K& key, const T& data)
      {
        // Find which set
        uint h = csHashComputer<K>::ComputeHash (key) & (NumberOfSets-1);
        h = HashFold::FoldHash(h);

        SetType& set = sets[h];

        set.InsertOrUpdate (key, data);
      }

      /**
       * Get a possibly existing item in cache.
       *
       * \param key Cache item key
       * \param data Cached data. Updated on successful retrieval from cache.
       * \return true if item exists in cache.
       */
      bool Get (const K& key, T& data)
      {
        // Find which set
        uint h = csHashComputer<K>::ComputeHash (key) & (NumberOfSets-1);
        h = HashFold::FoldHash(h);

        SetType& set = sets[h];

        return set.Get (key, data);
      }
    
    private:
      /// The cache itself
      SetType sets[NumberOfSets];

      // Check that some invariants are met
      // Require that number of sets is log2
      CS_COMPILE_ASSERT(CS::Meta::IsLog2<NumberOfSets>::value);
    };

  }
}


#include "csutil/custom_new_enable.h"
#endif
