/*
    Copyright (C) 2005 by Adam D. Bradley

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

#include "cppunit/Message.h"

#include "csutil/csstring.h"
#include "csutil/list.h"
#include "csutil/partialorder.h"
#include "csutil/hash.h"

/**
 * Test csPartialOrder class.
 */
class csPartialOrderTest : public CppUnit::TestFixture
{
public:
  void setUp();
  void testSmokeTest();
  void testTransitive();
  void testStringSets();
  void testDelete();
  void testDupInserts();

  CPPUNIT_TEST_SUITE(csPartialOrderTest);
    CPPUNIT_TEST(testSmokeTest);
    CPPUNIT_TEST(testTransitive);
    CPPUNIT_TEST(testStringSets);
    CPPUNIT_TEST(testDelete);
    CPPUNIT_TEST(testDupInserts);
  CPPUNIT_TEST_SUITE_END();
};

void csPartialOrderTest::setUp()
{
}

void csPartialOrderTest::testSmokeTest()
{
	csPartialOrder<csString> SmokeSet;
	csString one("1");
	csString two("2");
	SmokeSet.Add (one);
	SmokeSet.Add (two);
	SmokeSet.AddOrder (one, two);
	csList<const csString> Solution;
	SmokeSet.Solve (Solution);
	CPPUNIT_ASSERT (Solution.Front() == one);
	Solution.PopFront();
	CPPUNIT_ASSERT (Solution.Front() == two);
}

void csPartialOrderTest::testTransitive()
{
	csPartialOrder<csString> TransSet;
	csString one ("1");
	csString two ("2");
	csString three ("3");
	csString four ("4");
	csString five ("5");
	csString six ("6");
	TransSet.Add (one);
	TransSet.Add (three);
	TransSet.Add (five);
	TransSet.Add (four);
	TransSet.Add (two);
	TransSet.Add (six);
	TransSet.AddOrder (four, five);
	TransSet.AddOrder (one, two);
	TransSet.AddOrder (five, six);
	TransSet.AddOrder (two, three);
	TransSet.AddOrder (three, four);
	csList<const csString> Solution;
	TransSet.Solve (Solution);
	CPPUNIT_ASSERT (Solution.Front() == one);
	Solution.PopFront();
	CPPUNIT_ASSERT (Solution.Front() == two);
	Solution.PopFront();
	CPPUNIT_ASSERT (Solution.Front() == three);
	Solution.PopFront();
	CPPUNIT_ASSERT (Solution.Front() == four);
	Solution.PopFront();
	CPPUNIT_ASSERT (Solution.Front() == five);
	Solution.PopFront();
	CPPUNIT_ASSERT (Solution.Front() == six);
}

void csPartialOrderTest::testStringSets()
{
	csPartialOrder<csString> OrderSets;
	csString one_1 ("1 (1)");
	csString one_2 ("1 (2)");
	csString one_3 ("1 (3)");

	csString two_1 ("2 (1)");
	csString two_2 ("2 (2)");
	csString two_3 ("2 (3)");

	csString three ("3");
	
	csString four_1 ("4 (1)");
	csString four_2 ("4 (2)");
	csString four_3 ("4 (3)");

	csString five ("5");

	csString six_1 ("6 (1)");
	csString six_2 ("6 (2)");
	csString six_3 ("6 (3)");

	csString seven ("7");

	csPartialOrder<csString> Set;
	Set.Add (one_1);
	Set.Add (one_2);
	Set.Add (one_3);
	Set.Add (two_1);
	Set.Add (two_2);
	Set.Add (two_3);
	Set.Add (three);
	Set.Add (four_1);
	Set.Add (four_2);
	Set.Add (four_3);
	Set.Add (five);
	Set.Add (six_1);
	Set.Add (six_2);
	Set.Add (six_3);
	Set.Add (seven);
	Set.AddOrder (six_1, seven);
	Set.AddOrder (six_2, seven);
	Set.AddOrder (six_3, seven);
	Set.AddOrder (three, five);
	Set.AddOrder (five, seven);
	Set.AddOrder (one_3, two_3);
	Set.AddOrder (two_1, three);
	Set.AddOrder (one_1, three);
	Set.AddOrder (one_1, two_1);
	Set.AddOrder (one_1, two_2);
	Set.AddOrder (one_2, two_3);
	Set.AddOrder (one_3, two_1);
	Set.AddOrder (one_3, two_2);
	Set.AddOrder (two_2, three);
	Set.AddOrder (two_3, three);
	Set.AddOrder (three, four_1);
	Set.AddOrder (three, four_2);
	Set.AddOrder (four_3, five);
	Set.AddOrder (five, six_1);
	Set.AddOrder (five, six_2);
	Set.AddOrder (five, six_3);
	Set.AddOrder (one_1, two_3);
	Set.AddOrder (one_2, two_1);
	Set.AddOrder (one_2, two_2);
	Set.AddOrder (three, four_3);
	Set.AddOrder (four_1, five);
	Set.AddOrder (four_2, five);

	csList<const csString> Solution;
	Set.Solve (Solution);

	for (int i=0 ; i<3 ; i++) {
		CPPUNIT_ASSERT(Solution.Front().StartsWith("1 "));
		Solution.PopFront();
	}
	for (int i=0 ; i<3 ; i++) {
		CPPUNIT_ASSERT(Solution.Front().StartsWith("2 "));
		Solution.PopFront();
	}
	CPPUNIT_ASSERT(Solution.Front().StartsWith("3"));
	Solution.PopFront();
	for (int i=0 ; i<3 ; i++) {
		CPPUNIT_ASSERT(Solution.Front().StartsWith("4 "));
		Solution.PopFront();
	}
	CPPUNIT_ASSERT(Solution.Front().StartsWith("5"));
	Solution.PopFront();
	for (int i=0 ; i<3 ; i++) {
		CPPUNIT_ASSERT(Solution.Front().StartsWith("6 "));
		Solution.PopFront();
	}
	CPPUNIT_ASSERT(Solution.Front().StartsWith("7"));
	Solution.PopFront();
}

void csPartialOrderTest::testDelete()
{
	csPartialOrder<csString> Set;

	csString one ("1");
	csString two ("2");
	csString three ("3");
	csString four ("4");
	csString five ("5");
	csString six ("6");

	Set.Add (one);
	Set.Add (two);
	Set.Add (three);
	Set.Add (four);
	Set.Add (five);
	Set.Add (six);

	CPPUNIT_ASSERT (Set.Size() == 6);

	Set.Delete(one);
	Set.Delete(two);
	Set.Delete(three);

	CPPUNIT_ASSERT (Set.Size() == 3);

	Set.Add(three);
	Set.Add(two);
	Set.Add(one);

	CPPUNIT_ASSERT (Set.Size() == 6);
	
	Set.Delete(six);
	Set.Delete(five);
	Set.Delete(four);
	Set.Delete(three);
	Set.Delete(two);
	Set.Delete(one);

	CPPUNIT_ASSERT (Set.Size() == 0);
}

void csPartialOrderTest::testDupInserts()
{
	csPartialOrder<csString> Set;

	csString one ("1");
	csString two ("2");
	csString three ("3");
	csString four ("4");
	csString five ("5");
	csString six ("6");

	Set.Add (one);
	Set.Add (two);
	Set.Add (three);
	Set.Add (four);
	Set.Add (five);
	Set.Add (six);

	CPPUNIT_ASSERT_EQUAL (6, (int) Set.Size());

	Set.Add (three);
	Set.Add (three);

	CPPUNIT_ASSERT_EQUAL (6, (int) Set.Size());

	Set.Delete(one);
	Set.Delete(three);

	CPPUNIT_ASSERT_EQUAL (4, (int) Set.Size());

	Set.Delete (two);
	Set.Delete (four);
	Set.Delete (five);
	Set.Delete (six);

	CPPUNIT_ASSERT_EQUAL (0, (int) Set.Size());
}
