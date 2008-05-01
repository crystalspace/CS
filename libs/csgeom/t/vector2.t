/*
    Copyright (C) 2005 by Eric Sunshine
    Copyright (C) 2008 by Lukas Erlinghagen

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

#include "csgeom/vector2.h"

/**
 * Test csVector2 operations.
 *
 * Excluded: standard constructor, csVector2::Description()
 */
class csVector2Test : public CppUnit::TestFixture
{
private:
  csVector2 v1;
  csVector2 v2;
  csVector2 temp;

public:
  void setUp();

  void testConstructors();
  void testAccessors();
  void testManip();
  void testOp();
  void testCond();
  void testMisc();

  CPPUNIT_TEST_SUITE(csVector2Test);
    CPPUNIT_TEST(testConstructors);
    CPPUNIT_TEST(testAccessors);
    CPPUNIT_TEST(testManip);
    CPPUNIT_TEST(testOp);
    CPPUNIT_TEST(testCond);
    CPPUNIT_TEST(testMisc);
  CPPUNIT_TEST_SUITE_END();
};

void csVector2Test::setUp()
{
  v1.x = 1;
  v1.y = 2;
  v2.x = 3;
  v2.y = 4;
}

void csVector2Test::testConstructors()
{
  csVector2 one_value(3);
  CPPUNIT_ASSERT(one_value.x == 3 && one_value.y == 3);
  
  csVector2 two_values(1, 2);
  CPPUNIT_ASSERT(two_values.x == 1 && two_values.y == 2);
  
  csVector2 copy(one_value);
  CPPUNIT_ASSERT(copy.x == 3 && copy.y == 3);
}

void csVector2Test::testAccessors()
{
  temp.Set(v1);
  const float f = temp[0];
  float g = temp[0];
  CPPUNIT_ASSERT_EQUAL(1.0f, f);
  CPPUNIT_ASSERT_EQUAL(f, g);

  temp[1] = 3.0f;
  CPPUNIT_ASSERT_EQUAL(3.0f, temp.y);
}

void csVector2Test::testManip()
{
  float v[2] = { 8,9 };
  CPPUNIT_ASSERT(v1.x == 1 && v1.y == 2);
  temp.Set(5,6); CPPUNIT_ASSERT(temp.x == 5 && temp.y == 6);
  temp.Set(v2);  CPPUNIT_ASSERT(temp.x == 3 && temp.y == 4);
  temp.Set(v);   CPPUNIT_ASSERT(temp.x == 8 && temp.y == 9);
  temp.Set(0.0); CPPUNIT_ASSERT(temp.x == 0 && temp.y == 0);
  temp.Get(v);   CPPUNIT_ASSERT(v[0]   == 0 && v[1]   == 0);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(v2.Norm(), 5.0f, EPSILON);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(v2.Norm(), csVector2::Norm(v2), EPSILON);
  CPPUNIT_ASSERT_EQUAL(25.0f, v2.SquaredNorm());
  temp.Set(v1);
  temp.Rotate(HALF_PI); // Rotates clock-wise (rather than counter clock-wise).
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 2, temp.x, EPSILON);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(-1, temp.y, EPSILON);
}

void csVector2Test::testOp()
{
  temp.Set(v1);
  temp += v2;        CPPUNIT_ASSERT(temp.x ==  4 && temp.y ==  6);
  temp -= v1;        CPPUNIT_ASSERT(temp.x ==  3 && temp.y ==  4);
  temp *= 2.0;       CPPUNIT_ASSERT(temp.x ==  6 && temp.y ==  8);
  temp /= 2.0;       CPPUNIT_ASSERT(temp.x ==  3 && temp.y ==  4);
  temp = -v1;        CPPUNIT_ASSERT(temp.x == -1 && temp.y == -2);
  temp = v1 + v2;    CPPUNIT_ASSERT(temp.x ==  4 && temp.y ==  6);
  temp = v2 - v1;    CPPUNIT_ASSERT(temp.x ==  2 && temp.y ==  2);
  temp = v1 * 2.0;   CPPUNIT_ASSERT(temp.x ==  2 && temp.y ==  4);
  temp = 2.0 * v2;   CPPUNIT_ASSERT(temp.x ==  6 && temp.y ==  8);
  temp = temp / 2.0; CPPUNIT_ASSERT(temp.x ==  3 && temp.y ==  4);
  CPPUNIT_ASSERT_EQUAL(11.0f, v1 * v2);
}

void csVector2Test::testCond()
{
  temp.Set(v1);
  CPPUNIT_ASSERT(temp == v1);
  CPPUNIT_ASSERT(temp != v2);
  CPPUNIT_ASSERT(v1 < 3.0);
  CPPUNIT_ASSERT(3.0 > v1);
}

void csVector2Test::testMisc()
{
  temp.Set(0.0f);  
  CPPUNIT_ASSERT(temp.IsLeft(v1, v2) < 0);
  CPPUNIT_ASSERT(temp.IsLeft(v2, v1) > 0);
  
  temp.Set(0, 1);
  CPPUNIT_ASSERT_EQUAL(0.0f, temp.IsLeft(v1, v2));
  
  temp.Set(2, 3);
  CPPUNIT_ASSERT_EQUAL(0.0f, temp.IsLeft(v2, v1));
}
