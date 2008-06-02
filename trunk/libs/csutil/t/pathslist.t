/*
    Copyright (C) 2005 by Eric Sunshine
	      (C) 2005 by Frank Richter

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

#include "csutil/syspath.h"

/**
 * Test csPathsList operations.
 */
class csPathsListTest : public CppUnit::TestFixture
{
public:
  void testAddUnique();
  void testMultiply();

  CPPUNIT_TEST_SUITE(csPathsListTest);
    CPPUNIT_TEST(testAddUnique);
    CPPUNIT_TEST(testMultiply);
  CPPUNIT_TEST_SUITE_END();
};

void csPathsListTest::testAddUnique()
{
  csPathsList list;
  list.AddUnique ("bar");
  list.AddUnique ("baz");
  CPPUNIT_ASSERT_EQUAL(list.Length(), (size_t)2);
  list.AddUnique ("bar");
  CPPUNIT_ASSERT_EQUAL(list.Length(), (size_t)2);
  list.AddUnique ("baz/");
  CPPUNIT_ASSERT_EQUAL(list.Length(), (size_t)2);
}

void csPathsListTest::testMultiply()
{
  static const char* strings1[] = {
    "foo",
    "/",
    "",
    0
  };
  static const char* strings2[] = {
    "bar",
    "",
    0
  };
  csPathsList list1 (strings1);
  csPathsList list2 (strings2);
  
  csPathsList list (list1 * list2);

  csString catPath;
  catPath = "foo"; catPath += CS_PATH_SEPARATOR; catPath += "bar";
  csString sepPath;
  sepPath = CS_PATH_SEPARATOR; sepPath += "bar";
  
  CPPUNIT_ASSERT_EQUAL (list[0].path, catPath);
  CPPUNIT_ASSERT_EQUAL (list[1].path, csString ("foo"));
  CPPUNIT_ASSERT_EQUAL (list[2].path, sepPath);
  CPPUNIT_ASSERT_EQUAL (list[3].path, csString (CS_PATH_SEPARATOR));
  CPPUNIT_ASSERT_EQUAL (list[4].path, csString ("bar"));
  CPPUNIT_ASSERT_EQUAL (list[5].path, csString (""));
}
