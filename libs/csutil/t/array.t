/*
    Copyright (C) 2010 by Stefano Angeleri

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

#include <csutil/array.h>

/**
 * Test array functionality.
 */
class csArrayTest : public CppUnit::TestFixture
{
public:
  void setUp();

  void testArrayPut();
  void testArrayPush();

  CPPUNIT_TEST_SUITE(csArrayTest);
    CPPUNIT_TEST(testArrayPush);
    CPPUNIT_TEST(testArrayPut);
  CPPUNIT_TEST_SUITE_END();
};

void csArrayTest::setUp()
{
}

void csArrayTest::testArrayPush()
{
  csArray<size_t> testArr;
  for(size_t i = 1; i < 1000; i++)
  {
    testArr.Push(i);
    CPPUNIT_ASSERT(testArr.GetSize() == i);
  }
  for(size_t i = 1; i < 1000; i++)
  {
    CPPUNIT_ASSERT(testArr.Get(i-1) == i);
  }
}

void csArrayTest::testArrayPut()
{
  csArray<int> testArr;
  testArr.Push(0);
  testArr.Push(0);
  testArr.Push(0);

  for(int i = 0; i < 1000; i++)
  {
    CPPUNIT_ASSERT(testArr.GetSize() == 3);
    testArr.Put(0, 500);    
    CPPUNIT_ASSERT(testArr.GetSize() == 3);
    CPPUNIT_ASSERT(testArr.Get(0) == 500);
    testArr.Put(1, 500);
    CPPUNIT_ASSERT(testArr.Get(1) == 500);
    CPPUNIT_ASSERT(testArr.GetSize() == 3);
    testArr.Put(2, 500);
    CPPUNIT_ASSERT(testArr.Get(2) == 500);
  }
}

