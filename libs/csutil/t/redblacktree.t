/*
    Copyright (C) 2010 by Frank Richter

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

#include "csutil/redblacktree.h"
#include "csutil/set.h"

#define ARRAY_SIZE(x)		(sizeof((x))/sizeof((x)[0]))

/**
 * Test csRedBlackTree operations.
 */
class RedBlackTreeTest : public CppUnit::TestFixture
{
private:
  class RBTree : public csRedBlackTree<int>
  {
  public:
    using csRedBlackTree<int>::root;
    
    const int* TestLocateNodeExact (const int* p)
    {
      csRedBlackTree<int>::Node* n = LocateNodeExact (root.p, p);
      return n ? &(n->GetKey()) : 0;
    }
  };
  
  void InsertSequence (RBTree& rbtree, const int* seq, size_t n)
  {
    for (size_t i = 0; i < n; i++)
    {
      rbtree.Insert (seq[i]);
    }
  }
  
  typedef csRedBlackTreeMap<int, int> RBMap;
public:
  void testInsert ();
  void testTraverse ();
  void testDelete ();
  
  void testIterator ();
  void testIteratorConst ();
  void testIteratorConstRev ();
  
  void testInsertDupes ();
  void testTraverseDupes ();
  void testDupesFindExact ();

  void testGSE ();
  void testSGE ();
  void testGSE2 ();
  void testSGE2 ();
  
  void testMapInsert ();
  void testMapDelete ();
  void testMapIterator ();
  void testMapIteratorConst ();
  void testMapIteratorRev ();
  void testMapIteratorConstRev ();
  
  CPPUNIT_TEST_SUITE(RedBlackTreeTest);
    CPPUNIT_TEST(testInsert);
    CPPUNIT_TEST(testTraverse);
    CPPUNIT_TEST(testDelete);
    CPPUNIT_TEST(testIterator);
    CPPUNIT_TEST(testIteratorConst);
    CPPUNIT_TEST(testIteratorConstRev);
    CPPUNIT_TEST(testInsertDupes);
    CPPUNIT_TEST(testTraverseDupes);
    CPPUNIT_TEST(testDupesFindExact);
    CPPUNIT_TEST(testGSE);
    CPPUNIT_TEST(testSGE);
    CPPUNIT_TEST(testGSE2);
    CPPUNIT_TEST(testSGE2);
    CPPUNIT_TEST(testMapInsert);
    CPPUNIT_TEST(testMapDelete);
    CPPUNIT_TEST(testMapIterator);
    CPPUNIT_TEST(testMapIteratorConst);
    CPPUNIT_TEST(testMapIteratorRev);
    CPPUNIT_TEST(testMapIteratorConstRev);
  CPPUNIT_TEST_SUITE_END();
};

static const int sequenceUnique[] = {23, 42, -10, 2, 3, 1, 70, 69, 5, 0};
static const int sequenceUniqueSorted[] = {-10, 0, 1, 2, 3, 5, 23, 42, 69, 70};
static const int sequenceUniqueSortedRev[] = {70, 69, 42, 23, 5, 3, 2, 1, 0, -10};

void RedBlackTreeTest::testInsert()
{  
  RBTree rbtree;
  InsertSequence (rbtree, sequenceUnique, ARRAY_SIZE(sequenceUnique));
  
  for (size_t i = 0; i < ARRAY_SIZE(sequenceUnique); i++)
  {
    CPPUNIT_ASSERT(rbtree.Contains (sequenceUnique[i]));
  }
}

namespace
{
  class SortTester
  {
    size_t remaining;
    const int* seqPtr;
  public:
    SortTester (const int* sequence, size_t n)
    : remaining (n), seqPtr (sequence) {}
    
    void operator() (int v)
    {
      CPPUNIT_ASSERT_ASSERTION_PASS_MESSAGE(
	"sorted sequence smaller than number of values in tree",
	CPPUNIT_ASSERT (remaining > 0));
      if (remaining > 0)
      {
	CPPUNIT_ASSERT_EQUAL(*seqPtr, v);
	seqPtr++;
	remaining--;
      }
    }
    
    void CheckRemaining ()
    {
      CPPUNIT_ASSERT_ASSERTION_PASS_MESSAGE(
	"sorted sequence larger than number of values in tree",
	CPPUNIT_ASSERT (remaining == 0));
    }
  };
}

void RedBlackTreeTest::testTraverse()
{  
  RBTree rbtree;
  InsertSequence (rbtree, sequenceUnique, ARRAY_SIZE(sequenceUnique));
  
  SortTester sortTest (sequenceUniqueSorted, ARRAY_SIZE(sequenceUniqueSorted));
  rbtree.TraverseInOrder (sortTest);
  sortTest.CheckRemaining ();
}

void RedBlackTreeTest::testDelete()
{  
  for (size_t d = 0; d < ARRAY_SIZE(sequenceUnique); d++)
  {
    RBTree rbtree;
    InsertSequence (rbtree, sequenceUnique, ARRAY_SIZE(sequenceUnique));
    
    rbtree.Delete (sequenceUnique[d]);
    
    for (size_t i = 0; i < ARRAY_SIZE(sequenceUnique); i++)
    {
      bool contains = rbtree.Contains (sequenceUnique[i]);
      CPPUNIT_ASSERT_EQUAL((i == d) ? false : true, contains);
    }
  }
}

void RedBlackTreeTest::testIterator()
{  
  RBTree rbtree;
  InsertSequence (rbtree, sequenceUnique, ARRAY_SIZE(sequenceUnique));

  {
    SortTester sortTest (sequenceUniqueSorted, ARRAY_SIZE(sequenceUniqueSorted));
    RBTree::Iterator iter (rbtree.GetIterator ());
    while (iter.HasNext ())
    {
      sortTest (iter.Next ());
    }
    sortTest.CheckRemaining ();
  }
}

void RedBlackTreeTest::testIteratorConst()
{  
  RBTree rbtree;
  InsertSequence (rbtree, sequenceUnique, ARRAY_SIZE(sequenceUnique));

  {
    SortTester sortTest (sequenceUniqueSorted, ARRAY_SIZE(sequenceUniqueSorted));
    RBTree::ConstIterator iter (
      const_cast<const RBTree&> (rbtree).GetIterator ());
    while (iter.HasNext ())
    {
      sortTest (iter.Next ());
    }
    sortTest.CheckRemaining ();
  }
}

void RedBlackTreeTest::testIteratorConstRev()
{  
  RBTree rbtree;
  InsertSequence (rbtree, sequenceUnique, ARRAY_SIZE(sequenceUnique));

  {
    SortTester sortTest (sequenceUniqueSortedRev, ARRAY_SIZE(sequenceUniqueSortedRev));
    RBTree::ConstReverseIterator iter (
      const_cast<const RBTree&> (rbtree).GetReverseIterator ());
    while (iter.HasNext ())
    {
      sortTest (iter.Next ());
    }
    sortTest.CheckRemaining ();
  }
}

static const int sequenceDupes[] = {4, 3, 4, -1, 4, 2, 4, 12, 2, 5};
static const int sequenceDupesSorted[] = {-1, 2, 2, 3, 4, 4, 4, 4, 5, 12};

void RedBlackTreeTest::testInsertDupes()
{  
  RBTree rbtree;
  InsertSequence (rbtree, sequenceDupes, ARRAY_SIZE(sequenceDupes));
  
  for (size_t i = 0; i < ARRAY_SIZE(sequenceDupes); i++)
  {
    CPPUNIT_ASSERT(rbtree.Contains (sequenceDupes[i]));
  }
}

void RedBlackTreeTest::testTraverseDupes()
{  
  RBTree rbtree;
  InsertSequence (rbtree, sequenceDupes, ARRAY_SIZE(sequenceDupes));
  
  SortTester sortTest (sequenceDupesSorted, ARRAY_SIZE(sequenceDupesSorted));
  rbtree.TraverseInOrder (sortTest);
  sortTest.CheckRemaining ();
}

void RedBlackTreeTest::testDupesFindExact ()
{
  RBTree rbtree;
  InsertSequence (rbtree, sequenceDupes, ARRAY_SIZE(sequenceDupes));
  
  RBTree::Iterator iter (rbtree.GetIterator ());
  while (iter.HasNext ())
  {
    const int* p = &(iter.Next ());
    const int* n = rbtree.TestLocateNodeExact (p);
    CPPUNIT_ASSERT_EQUAL(p, n);
  }
}

static const int sequenceSparse[] = {-20, -10, 0, 10, 20};

void RedBlackTreeTest::testGSE ()
{
  RBTree rbtree;
  InsertSequence (rbtree, sequenceSparse, ARRAY_SIZE(sequenceDupes));
  
  const int* p;
  p = rbtree.FindGreatestSmallerEqual (10);
  CPPUNIT_ASSERT (p != 0);
  CPPUNIT_ASSERT_EQUAL (10, *p);
  p = rbtree.FindGreatestSmallerEqual (11);
  CPPUNIT_ASSERT (p != 0);
  CPPUNIT_ASSERT_EQUAL (10, *p);
  p = rbtree.FindGreatestSmallerEqual (21);
  CPPUNIT_ASSERT (p != 0);
  CPPUNIT_ASSERT_EQUAL (20, *p);
  p = rbtree.FindGreatestSmallerEqual (-21);
  CPPUNIT_ASSERT (p == 0);
}

void RedBlackTreeTest::testSGE ()
{
  RBTree rbtree;
  InsertSequence (rbtree, sequenceSparse, ARRAY_SIZE(sequenceDupes));
  
  const int* p;
  p = rbtree.FindSmallestGreaterEqual (10);
  CPPUNIT_ASSERT (p != 0);
  CPPUNIT_ASSERT_EQUAL (10, *p);
  p = rbtree.FindSmallestGreaterEqual (11);
  CPPUNIT_ASSERT (p != 0);
  CPPUNIT_ASSERT_EQUAL (20, *p);
  p = rbtree.FindSmallestGreaterEqual (21);
  CPPUNIT_ASSERT (p == 0);
  p = rbtree.FindSmallestGreaterEqual (-21);
  CPPUNIT_ASSERT (p != 0);
  CPPUNIT_ASSERT_EQUAL (-20, *p);
}

void RedBlackTreeTest::testGSE2 ()
{
  RBTree rbtree;
  for (int i = 0; i < 2; i++)
    rbtree.Insert (4);
  
  const int* p;
  p = rbtree.FindGreatestSmallerEqual (4);
  CPPUNIT_ASSERT (p != 0);
  CPPUNIT_ASSERT_EQUAL (4, *p);
  p = rbtree.FindGreatestSmallerEqual (5);
  CPPUNIT_ASSERT (p != 0);
  CPPUNIT_ASSERT_EQUAL (4, *p);
  p = rbtree.FindGreatestSmallerEqual (3);
  CPPUNIT_ASSERT (p == 0);
}

void RedBlackTreeTest::testSGE2 ()
{
  RBTree rbtree;
  for (int i = 0; i < 2; i++)
    rbtree.Insert (4);
  
  const int* p;
  p = rbtree.FindSmallestGreaterEqual (4);
  CPPUNIT_ASSERT (p != 0);
  CPPUNIT_ASSERT_EQUAL (4, *p);
  p = rbtree.FindSmallestGreaterEqual (5);
  CPPUNIT_ASSERT (p == 0);
  p = rbtree.FindSmallestGreaterEqual (3);
  CPPUNIT_ASSERT (p != 0);
  CPPUNIT_ASSERT_EQUAL (4, *p);
}

static const int sequenceMapUnique[][2] = {{23, 6}, {42, 7}, {-10, 0}, {2, 3},
  {3, 4}, {1, 2}, {70, 9}, {69, 8}, {5, 5}, {0, 1}};

void RedBlackTreeTest::testMapInsert()
{
  RBMap rbmap;
  
  for (size_t i = 0; i < ARRAY_SIZE(sequenceMapUnique); i++)
  {
    rbmap.Put (sequenceMapUnique[i][0], sequenceMapUnique[i][1]);
  }
  
  for (size_t i = 0; i < ARRAY_SIZE(sequenceMapUnique); i++)
  {
    int* t = rbmap.GetElementPointer(sequenceMapUnique[i][0]);
    CPPUNIT_ASSERT (t);
    CPPUNIT_ASSERT (*t == sequenceMapUnique[i][1]);
  }
}

void RedBlackTreeTest::testMapDelete()
{
  for (size_t d = 0; d < ARRAY_SIZE(sequenceMapUnique); d++)
  {
    RBMap rbmap;
    
    for (size_t i = 0; i < ARRAY_SIZE(sequenceMapUnique); i++)
    {
      rbmap.Put (sequenceMapUnique[i][0], sequenceMapUnique[i][1]);
    }
    
    CPPUNIT_ASSERT (rbmap.Delete (sequenceMapUnique[d][0]));
    
    for (size_t i = 0; i < ARRAY_SIZE(sequenceMapUnique); i++)
    {
      int* t = rbmap.GetElementPointer(sequenceMapUnique[i][0]);
      if (i == d)
      {
        CPPUNIT_ASSERT (!t);
      }
      else
      {
        CPPUNIT_ASSERT (t);
        CPPUNIT_ASSERT (*t == sequenceMapUnique[i][1]);
      }
    }
  }
}

void RedBlackTreeTest::testMapIterator ()
{
  RBMap rbmap;
  
  for (size_t i = 0; i < ARRAY_SIZE(sequenceMapUnique); i++)
  {
    rbmap.Put (sequenceMapUnique[i][0], sequenceMapUnique[i][1]);
  }
  
  RBMap::Iterator iter (rbmap.GetIterator ());
  int seq = 0;
  while (iter.HasNext ())
  {
    int v = iter.Next();
    CPPUNIT_ASSERT_EQUAL (seq, v);
    seq++;
  }
  
}

void RedBlackTreeTest::testMapIteratorConst ()
{
  RBMap rbmap;
  
  for (size_t i = 0; i < ARRAY_SIZE(sequenceMapUnique); i++)
  {
    rbmap.Put (sequenceMapUnique[i][0], sequenceMapUnique[i][1]);
  }
  
  RBMap::ConstIterator iter (const_cast<const RBMap&> (rbmap).GetIterator ());
  int seq = 0;
  while (iter.HasNext ())
  {
    int v = iter.Next();
    CPPUNIT_ASSERT_EQUAL (seq, v);
    seq++;
  }
  
}

void RedBlackTreeTest::testMapIteratorRev ()
{
  RBMap rbmap;
  
  for (size_t i = 0; i < ARRAY_SIZE(sequenceMapUnique); i++)
  {
    rbmap.Put (sequenceMapUnique[i][0], sequenceMapUnique[i][1]);
  }
  
  RBMap::ReverseIterator iter (rbmap.GetReverseIterator ());
  int seq = ARRAY_SIZE(sequenceMapUnique);
  while (iter.HasNext ())
  {
    seq--;
    int v = iter.Next();
    CPPUNIT_ASSERT_EQUAL (seq, v);
  }
  
}

void RedBlackTreeTest::testMapIteratorConstRev ()
{
  RBMap rbmap;
  
  for (size_t i = 0; i < ARRAY_SIZE(sequenceMapUnique); i++)
  {
    rbmap.Put (sequenceMapUnique[i][0], sequenceMapUnique[i][1]);
  }
  
  RBMap::ConstReverseIterator iter (const_cast<const RBMap&> (rbmap).GetReverseIterator ());
  int seq = ARRAY_SIZE(sequenceMapUnique);
  while (iter.HasNext ())
  {
    seq--;
    int v = iter.Next();
    CPPUNIT_ASSERT_EQUAL (seq, v);
  }
  
}
