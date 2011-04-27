/*
  Copyright (C) 2006 by Frank Richter
	    (C) 2006 by Jorrit Tyberghein

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CS_TEMPHEAP_H__
#define __CS_TEMPHEAP_H__

#include "csutil/csstring.h"
#include "csutil/memheap.h"
#include "csutil/ref.h"
#include "csutil/refcount.h"

#include "csutil/custom_new_disable.h"

#define DECLARE_STATIC_CLASSVAR_DIRECT(getterFunc,Type,Kill, InitParam)   \
enum { _##getterFunc##StorageUnits = (sizeof (Type) + sizeof (void*) - 1)/sizeof (char) };   \
static char _##getterFunc##Store[_##getterFunc##StorageUnits]; 	\
static CS_FORCEINLINE_TEMPLATEMETHOD 			       	\
char* getterFunc##GetStoreAligned()				\
{								\
  uintptr_t p = reinterpret_cast<uintptr_t> (_##getterFunc##Store); \
  static const int align = sizeof(void*);			\
  p = (p + align - 1) & ~(align - 1);				\
  return reinterpret_cast<char*> (p);				\
}								\
static void getterFunc##Init()                                 	\
{                                                              	\
  new (getterFunc##GetStoreAligned()) Type InitParam;           \
  csStaticVarCleanup (Kill);                                   	\
}                                                              	\
static CS_FORCEINLINE_TEMPLATEMETHOD Type& getterFunc()        	\
{                                                              	\
  return *reinterpret_cast<Type*> (getterFunc##GetStoreAligned());\
}                                                              	\
static void getterFunc ## _kill ();

#define IMPLEMENT_STATIC_CLASSVAR_DIRECT(Class,getterFunc) \
char Class::_##getterFunc##Store[Class::_##getterFunc##StorageUnits];

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{
  /**
   * Allocator for the "temporary heap". It should be used for data that
   * is only needed during parsing of a conditionalized shader. The idea
   * is that if all that temporary data would be allocated on the main
   * heap it might lead to significant fragmentation and waste of unused
   * heap memory (ie blocks cannot returned to the system due the 
   * fragmentation). But if the temporary data is allocated onto a separate
   * heap, essentially the memory for the whole separate heap can be returned
   * to the system after the temp data was freed.
   * Note that this means that *no* data that is used beyond the processing
   * should be allocated here; otherwise, the very same fragmentation issue
   * arises with the temp heap, and the whole scheme falls apart.
   */
  class TempHeap
  {
    DECLARE_STATIC_CLASSVAR_DIRECT(TheHeap,CS::Memory::Heap,HeapKill,);
    
    static void HeapKill()
    {
      TheHeap().~Heap();
    }
  public:
    static void Init()
    {
      TheHeapInit();
    }
  
    static void* Alloc (const size_t n)
    {
      return TheHeap().Alloc (n);
    }
    static void Free (void* p)
    {
      TheHeap().Free (p);
    }
    static void* Realloc (void* p, size_t newSize)
    {
      return TheHeap().Realloc (p, newSize);
    }
    static void Trim ()
    {
      TheHeap().Trim ();
    }
  };

  typedef CS::Memory::AllocatorHeapBase<TempHeap> TempHeapAlloc;

  class TempStringBase : public csStringBase, protected TempHeapAlloc
  {
  protected:
    TempStringBase()
    {
    #ifdef CS_MEMORY_TRACKER
      TempHeapAlloc::SetMemTrackerInfo (typeid(*this).name());
    #endif
    }

    void SetCapacityInternal (size_t NewSize, bool soft)
    {
      NewSize++; // GLOBAL NOTE *1*
      if (soft)
        NewSize = ComputeNewSize (NewSize);
      MaxSize = NewSize;
      char* buff = (char*)TempHeapAlloc::Alloc (MaxSize);
      if (Data == 0 || Size == 0)
        buff[0] = '\0';
      else
        memcpy(buff, Data, Size + 1);

      TempHeapAlloc::Free (Data);
      Data = buff;
    }

  public:
    virtual void ShrinkBestFit ()
    {
      if (Size == 0)
        Free();
      else
      {
        CS_ASSERT(Data != 0);
        MaxSize = Size + 1; // GLOBAL NOTE *1*
        char* s = (char*)TempHeapAlloc::Alloc (MaxSize);
        memcpy(s, Data, MaxSize);
        TempHeapAlloc::Free (Data);
        Data = s;
      }
    }

    virtual ~TempStringBase() { Free (); }
    virtual void Free () { TempHeapAlloc::Free (Data); Data = 0; }

    using csStringBase::Trim;
  };

  /**
   * String that works like csStringFast, but allocates additional memory
   * via TempHeap.
   */
  template<int LEN = 36>
  class TempString : public TempStringBase
  {
  protected:
    /// Internal buffer; used when capacity fits within LEN bytes.
    char minibuff[LEN];
    /**
     * Amount of minibuff allocated by SetCapacityInternal(); not necessarily
     * same as Size. This is analogous to MaxSize in csStringBase. We need it to
     * determine if minibuff was ever used in order to return NULL if not (to
     * emulate the NULL returned by csStringBase when no buffer has been
     * allocated). We also use minibuff to emulate GetCapacity(), which is a
     * predicate of several memory management methods, such as ExpandIfNeeded().
     */
    size_t miniused;

    virtual void SetCapacityInternal (size_t NewSize, bool soft)
    {
      if (Data != 0) // If dynamic buffer already allocated, just re-use it.
        TempStringBase::SetCapacityInternal(NewSize, soft);
      else
      {
        NewSize++; // Plus one for implicit null byte.
        if (NewSize <= LEN)
	  miniused = NewSize;
        else
        {
	  CS_ASSERT(MaxSize == 0);
	  if (soft)
	    NewSize = ComputeNewSize (NewSize);
          Data = (char*)TempHeapAlloc::Alloc (NewSize);
	  MaxSize = NewSize;
	  if (Size == 0)
	    Data[0] = '\0';
	  else
	    memcpy(Data, minibuff, Size + 1);
        }
      }
    }

    virtual char* GetDataMutable ()
    { return (miniused == 0 && Data == 0 ? 0 : (Data != 0 ? Data : minibuff)); }

  public:
    /**
     * Create an empty csStringFast object.
     */
    TempString () : TempStringBase(), miniused(0) { }
    /**
     * Create a csStringFast object and reserve space for at least Length 
     * characters.
     */
    TempString (size_t Length) : TempStringBase(), miniused(0)
    { SetCapacity (Length); }
    /**
     * Copy constructor.
     */
    TempString (const TempStringBase& copy) : TempStringBase (), miniused(0) 
    { Append (copy); }
    /**
     * Copy constructor.
     */
    TempString (const TempString& copy) : TempStringBase (), miniused(0)
    { Append (copy); }
    /**
     * Create a TempString object from a null-terminated C string.
     */
    TempString (const char* src) : TempStringBase(), miniused(0)
    { Append (src); }
    /**
     * Create a TempString object from a C string, given the length.
     */
    TempString (const char* src, size_t _length) : TempStringBase(), miniused(0)
    { Append (src, _length); }

    TempString (const csString& src) : TempStringBase(), miniused(0)
    { Append (src); }
    
    /// Create a TempString object from a single signed character.
    TempString (char c) : TempStringBase(), miniused(0)
    { Append (c); }
    /// Create a TempString object from a single unsigned character.
    TempString (unsigned char c) : TempStringBase(), miniused(0)
    { Append ((char)c); }
    /// Destroy the TempString.
    virtual ~TempString() { }

    /// Assign a value to this string.
    const TempString& operator = (const TempStringBase& copy)
    { Replace(copy); return *this; }
    const TempString& operator = (const TempString& copy)
    { Replace(copy); return *this; }

    /// Assign a formatted value to this string.
    template<typename T>
    const TempString& operator = (T const& s) { Replace (s); return *this; }

    virtual char const* GetData () const
    { return (miniused == 0 && Data == 0 ? 0 : (Data != 0 ? Data : minibuff)); }

    virtual size_t GetCapacity() const
    { return Data != 0 ? TempStringBase::GetCapacity() : miniused - 1; }

    virtual void ShrinkBestFit ()
    {
      if (Size == 0)
      {
        TempStringBase::ShrinkBestFit();
        miniused = 0;
      }
      else
      {
        size_t needed = Size + 1;
        if (needed > LEN)
	  TempStringBase::ShrinkBestFit();
        else
        {
	  miniused = needed;
	  if (Data != 0)
	  {
	    memcpy(minibuff, Data, needed); // Includes implicit null byte.
	    Free();
	  }
        }
      }
    }

    virtual void Free () { miniused = 0; TempStringBase::Free(); }
  };
}
CS_PLUGIN_NAMESPACE_END(XMLShader)

#include "csutil/custom_new_enable.h"

#endif // __CS_TEMPHEAP_H__
