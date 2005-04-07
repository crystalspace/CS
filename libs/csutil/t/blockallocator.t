/*
    Copyright (C) 2005 by Eric Sunshine

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

#include "csutil/blockallocator.h"
#include "csutil/hash.h"

/**
 * Test csBlockAllocator operations.
 */
class csBlockAllocatorTest : public CppUnit::TestFixture
{
private:
  class SideEffect
  {
  private:
    int id;
    csSet<int>* registry;
  public:
    SideEffect() : id(0), registry(0) {}
    ~SideEffect() { if (registry != 0) registry->Delete(id); }
    void Register(int i, csSet<int>* r) { id = i, registry = r; r->Add(i); }
  };

  template <typename T>
  class Allocator : public csBlockAllocator<T>
  {
  private:
    typedef csBlockAllocator<T> S;
  public:
    Allocator(size_t granularity) : S(granularity) {}
    size_t get_granularity() const { return S::size; }
    size_t get_element_size() const { return S::elsize; }
    size_t get_block_size() const { return S::blocksize; }
    size_t get_block_count() const { return S::blocks.GetSize(); }
    csBitArray get_allocation_map() const { return S::GetAllocationMap(); }
  };

  template <typename T>
  void countBits(Allocator<T> const& a, size_t expect_1, size_t expect_0) const
  {
    csBitArray m(a.get_allocation_map());
    size_t n = m.Length();
    size_t t = 0, f = 0;
    CPPUNIT_ASSERT_EQUAL(n, expect_1 + expect_0);
    while (n-- > 0)
      if (m.IsBitSet(n))
	t++;
      else
	f++;
    CPPUNIT_ASSERT_EQUAL(t, expect_1);
    CPPUNIT_ASSERT_EQUAL(f, expect_0);
  }

public:
  void testDestroy();
  void testRecycle();
  void testMap();
  void testCompact();

  CPPUNIT_TEST_SUITE(csBlockAllocatorTest);
    CPPUNIT_TEST(testDestroy);
    CPPUNIT_TEST(testRecycle);
    CPPUNIT_TEST(testMap);
    CPPUNIT_TEST(testCompact);
  CPPUNIT_TEST_SUITE_END();
};

void csBlockAllocatorTest::testDestroy()
{
  csSet<int> r;

  { // SCOPE
    csBlockAllocator<SideEffect> b(3);
    SideEffect* e1 = b.Alloc();
    SideEffect* e2 = b.Alloc();
    SideEffect* e3 = b.Alloc();
    SideEffect* e4 = b.Alloc();
    e1->Register(1,&r);
    e2->Register(2,&r);
    e3->Register(3,&r);
    e4->Register(4,&r);
    CPPUNIT_ASSERT(r.Contains(1));
    CPPUNIT_ASSERT(r.Contains(2));
    CPPUNIT_ASSERT(r.Contains(3));
    CPPUNIT_ASSERT(r.Contains(4));

    b.Free(e2);
    b.Free(e4);
    CPPUNIT_ASSERT( r.Contains(1));
    CPPUNIT_ASSERT(!r.Contains(2));
    CPPUNIT_ASSERT( r.Contains(3));
    CPPUNIT_ASSERT(!r.Contains(4));
  }

  CPPUNIT_ASSERT(!r.Contains(1));
  CPPUNIT_ASSERT(!r.Contains(3));
  CPPUNIT_ASSERT(r.IsEmpty());
}

void csBlockAllocatorTest::testRecycle()
{
  csBlockAllocator<int> b(3);
  int* o1 = b.Alloc();
  int* o2 = b.Alloc();
  int* o3 = b.Alloc();
  int* o4 = b.Alloc();
  b.Free(o3);
  b.Free(o2);
  b.Free(o4);
  b.Free(o1);
  int* n1 = b.Alloc();
  int* n2 = b.Alloc();
  int* n3 = b.Alloc();
  int* n4 = b.Alloc();
  CPPUNIT_ASSERT(n1 == o1 || n1 == o2 || n1 == o3 || n1 == o4);
  CPPUNIT_ASSERT(n2 == o1 || n2 == o2 || n2 == o3 || n2 == o4);
  CPPUNIT_ASSERT(n3 == o1 || n3 == o2 || n3 == o3 || n3 == o4);
  CPPUNIT_ASSERT(n4 == o1 || n4 == o2 || n4 == o3 || n4 == o4);
}

void csBlockAllocatorTest::testMap()
{
  Allocator<int> b(2);
  int* i1 = b.Alloc(); countBits(b, 1, 1);
  int* i2 = b.Alloc(); countBits(b, 2, 0);
  int* i3 = b.Alloc(); countBits(b, 3, 1);
  int* i4 = b.Alloc(); countBits(b, 4, 0);
  b.Free(i3);          countBits(b, 3, 1);
  b.Free(i4);          countBits(b, 2, 2);
  b.Free(i2);          countBits(b, 1, 3);
  b.Free(i1);          countBits(b, 0, 4);
  i1 = b.Alloc();      countBits(b, 1, 3);
  i2 = b.Alloc();      countBits(b, 2, 2);
  b.Empty();           countBits(b, 0, 0);
}

void csBlockAllocatorTest::testCompact()
{
  Allocator<int> b(2);
  int* i1 = b.Alloc();
  int* i2 = b.Alloc();
  int* i3 = b.Alloc();
  int* i4 = b.Alloc();
  CPPUNIT_ASSERT_EQUAL(b.get_block_count(), (size_t)2);

  b.Free(i3);
  b.Free(i2);
  b.Compact();
  CPPUNIT_ASSERT_EQUAL(b.get_block_count(), (size_t)2);
  countBits(b, 2, 2);

  b.Free(i1);
  b.Compact();
  CPPUNIT_ASSERT_EQUAL(b.get_block_count(), (size_t)1);
  countBits(b, 1, 1);

  b.Empty();
  CPPUNIT_ASSERT_EQUAL(b.get_block_count(), (size_t)0);

  i1 = b.Alloc();
  i2 = b.Alloc();
  i3 = b.Alloc();
  i4 = b.Alloc();
  CPPUNIT_ASSERT_EQUAL(b.get_block_count(), (size_t)2);

  b.Free(i3);
  b.Free(i4);
  b.Compact();
  CPPUNIT_ASSERT_EQUAL(b.get_block_count(), (size_t)1);
  countBits(b, 2, 0);
}
