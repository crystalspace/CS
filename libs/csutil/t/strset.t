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

#include "csutil/strset.h"

/**
 * Test csStringSet operations.
 */
class csStringSetTest : public CppUnit::TestFixture
{
public:
  void testRequest();
  void testContains();
  void testDelete();
  void testSizing();
  void testIterator();

  CPPUNIT_TEST_SUITE(csStringSetTest);
    CPPUNIT_TEST(testRequest);
    CPPUNIT_TEST(testContains);
    CPPUNIT_TEST(testDelete);
    CPPUNIT_TEST(testSizing);
    CPPUNIT_TEST(testIterator);
  CPPUNIT_TEST_SUITE_END();
};

void csStringSetTest::testRequest()
{
  csStringSet s;
  CPPUNIT_ASSERT_EQUAL(s.Request(34), (char const*)0);
  CPPUNIT_ASSERT_EQUAL(s.Request(csInvalidStringID), (char const*)0);
  csStringID const bar = s.Request("bar");
  CPPUNIT_ASSERT(bar != csInvalidStringID);
  csStringID const foo1 = s.Request("foo");
  CPPUNIT_ASSERT(foo1 != bar);
  csStringID const foo2 = s.Request("foo");
  CPPUNIT_ASSERT_EQUAL(foo1, foo2);
}

void csStringSetTest::testContains()
{
  csStringSet s;
  CPPUNIT_ASSERT(!s.Contains("foo"));
  CPPUNIT_ASSERT(!s.Contains(34));
  CPPUNIT_ASSERT(!s.Contains(csInvalidStringID));
  csStringID const foo = s.Request("foo");
  CPPUNIT_ASSERT(s.Contains("foo"));
  CPPUNIT_ASSERT(s.Contains(foo));
  CPPUNIT_ASSERT(!s.Contains("bar"));
  CPPUNIT_ASSERT(!s.Contains(34));
}

void csStringSetTest::testDelete()
{
  csStringSet s;
  csStringID const foo = s.Request("foo");
  csStringID const bar = s.Request("bar");
  CPPUNIT_ASSERT(s.Delete("foo"));
  CPPUNIT_ASSERT(!s.Delete(foo));
  CPPUNIT_ASSERT(s.Delete(bar));
  CPPUNIT_ASSERT(!s.Delete("bar"));
  CPPUNIT_ASSERT(s.IsEmpty());
}

void csStringSetTest::testSizing()
{
  csStringSet s;
  CPPUNIT_ASSERT(s.IsEmpty());
  CPPUNIT_ASSERT_EQUAL(s.GetSize(), (size_t)0);
  s.Request("foo");
  s.Request("bar");
  CPPUNIT_ASSERT(!s.IsEmpty());
  CPPUNIT_ASSERT_EQUAL(s.GetSize(), (size_t)2);
  s.Delete("cow");
  CPPUNIT_ASSERT_EQUAL(s.GetSize(), (size_t)2);
  s.Delete("bar");
  CPPUNIT_ASSERT_EQUAL(s.GetSize(), (size_t)1);
  s.Delete("foo");
  CPPUNIT_ASSERT_EQUAL(s.GetSize(), (size_t)0);
  s.Request("foo");
  s.Request("bar");
  CPPUNIT_ASSERT(!s.IsEmpty());
  s.Empty();
  CPPUNIT_ASSERT(s.IsEmpty());
}

void csStringSetTest::testIterator()
{
  csStringSet s;
  csStringID const foo = s.Request("foo");
  csStringID const bar = s.Request("bar");
  csStringID const cow = s.Request("cow");
  csStringSet::GlobalIterator iter = s.GetIterator();
  int n = 0;
  while (iter.HasNext())
  {
    n++;
    char const* t;
    csStringID i = iter.Next(t);
    std::string x(t);
    CPPUNIT_ASSERT(x == "foo" || x == "bar" || x == "cow");
    CPPUNIT_ASSERT(i ==  foo  || i ==  bar  || i ==  cow );
    if (x == "foo")
      CPPUNIT_ASSERT_EQUAL(i, foo);
    else if (x == "bar")
      CPPUNIT_ASSERT_EQUAL(i, bar);
    else // (x == "cow")
      CPPUNIT_ASSERT_EQUAL(i, cow);
  }
  CPPUNIT_ASSERT_EQUAL(n, 3);
}
