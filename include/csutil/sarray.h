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

#ifndef __SARRAY_H__
#define __SARRAY_H__

/**
 * Create a static array class with the given name that contains
 * elements of the given type.
 */
#define CS_DECLARE_STATIC_ARRAY(Name,Type)				\
  CS_PRIVATE_DECLARE_STATIC_ARRAY (Name, Type)

//----------------------------------------------------------------------------
//--- implementation of the above macro follows ------------------------------
//----------------------------------------------------------------------------

class csStaticArray
{
protected:
  void *Map;
  int Size;

  /*
   * Remove all elements and copy the contents from another array. If
   * 'DeleteOld' is true then the old contents are deleted.
   */
  void Copy (const csStaticArray *other, bool DeleteOld = true);
  /*
   * Remove all elements and copy the contents from another array. If
   * 'DeleteOld' is true then the old contents are deleted.
   */
  void Copy (void *NewData, int NewSize, bool DeleteOld = true);
  /*
   * Remove all elements (and delete them if 'DeleteOld is true), then
   * use the contents of another array. The other array will lose control
   * over its elements and be cleared to empty.
   */
  inline void TakeOver (csStaticArray *other, bool DeleteOld = true);
  /*
   * Use the given array and size for this array. They are taken over,
   * not copied!
   */
  inline void TakeOver (void *NewData, int NewSize, bool DeleteOld = true);

  // Allocate an array of the given number of elements
  virtual void *AllocateArray (int Size) const = 0;
  // Delete an array of elements
  virtual void DeleteArray (void *Array) const = 0;
  // Copy one array into another one
  virtual void CopyArray (void *Dest, void *src, int Count) const = 0;

public:
  // constructor
  csStaticArray (int Size = 0);
  // destructor
  virtual ~csStaticArray ();

  // Return the number of elements in the array
  inline int GetSize () const;
  /*
   * Remove all elements from the array. If 'DeleteOld' is true then the
   * elements are also deleted.
   */
  void Clear (bool DeleteOld = true);
  /*
   * Allocate the given number of elements. The old contents are removed,
   * and if 'DeleteOld' is true they are also deleted.
   */
  void Alloc (int s, bool DeleteOld = true);
  /*
   * Change the size of the array but copy the contents.
   */
  void ReAlloc (int s);
};

#define CS_PRIVATE_DECLARE_STATIC_ARRAY(Name,Type)			\
class Name : public csStaticArray					\
{									\
  typedef Type cont_type;						\
private:								\
  void *AllocateArray (int n) const {return new cont_type[n];}		\
  void DeleteArray (void *a) const {delete[] ((cont_type*)a);}		\
  void CopyArray (void *Dest, void *Src, int n) const 			\
    {memcpy (Dest, Src, n*sizeof (cont_type)); }			\
public:									\
  inline Name (int Size = 0) :						\
    csStaticArray (Size) {}						\
  inline cont_type *GetArray ()						\
    { return (cont_type*)Map; }						\
  inline const cont_type *GetArray () const				\
    { return (cont_type*)Map; }						\
  inline void Copy (cont_type *NewData, int NewSize, bool DeleteOld = true)	\
    { csStaticArray::Copy (NewData, NewSize, DeleteOld); }		\
  inline void Copy (const Name *other, bool DeleteOld = true)		\
    { csStaticArray::Copy (other, DeleteOld); }				\
  inline void TakeOver (cont_type *NewData, int NewSize, bool DeleteOld = true)	\
    { csStaticArray::TakeOver (NewData, NewSize, DeleteOld); }		\
  inline void TakeOver (Name *other, bool DeleteOld = true)		\
    { csStaticArray::TakeOver (other, DeleteOld); }			\
  inline cont_type &operator[] (int n)					\
    { return GetArray () [n]; }						\
  inline const cont_type &operator[] (int n) const			\
    { return GetArray () [n]; }						\
}

inline int csStaticArray::GetSize () const
  {
    return Size;
  }
inline void csStaticArray::TakeOver (csStaticArray *other, bool DeleteOld)
  {
    TakeOver (other->Map, other->Size, DeleteOld);
    other->Map = NULL;
    other->Size = 0;
  }
inline void csStaticArray::TakeOver (void *NewData, int NewSize, bool DeleteOld)
  {
    if (DeleteOld) Clear ();
    Map = NewData;
    Size = NewSize;
  }

#endif // __SARRAY_H__
