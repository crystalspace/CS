/*
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

#include "csgeom/vector3.h"
#include "csgeom/math3d_d.h"

/**
 * Test csVector3 operations.
 *
 * Excluded: standard constructor, csVector3::Description(),
 *           operators resulting in csDVector3
 */
class csVector3Test : public CppUnit::TestFixture
{
private:
  csVector3 v1;
  csVector3 v2;
  csVector3 temp;

public:
  void setUp();

  void testConstructors();
  void testAccessors();
  void testManip();
  void testOp();
  void testCond();
  void testMisc();

  CPPUNIT_TEST_SUITE(csVector3Test);
    CPPUNIT_TEST(testConstructors);
    CPPUNIT_TEST(testAccessors);
    CPPUNIT_TEST(testManip);
    CPPUNIT_TEST(testOp);
    CPPUNIT_TEST(testCond);
    CPPUNIT_TEST(testMisc);
  CPPUNIT_TEST_SUITE_END();
};

void csVector3Test::setUp()
{
  v1.x = 1;
  v1.y = 2;
  v1.z = 3;
  
  v2.x = 4;
  v2.y = 5;
  v2.z = 6;  
}

void csVector3Test::testConstructors()
{
  csVector3 one_value(3);
  CPPUNIT_ASSERT(one_value.x == 3 && one_value.y == 3 && one_value.z == 3);
  
  csVector3 two_values(4, 5);
  CPPUNIT_ASSERT(two_values.x == 4 && two_values.y == 5 && two_values.z == 0);
  
  csVector3 three_values(3, 4, 5);
  CPPUNIT_ASSERT(three_values.x == 3 && 
                 three_values.y == 4 && 
                 three_values.z == 5);

  csVector3 copy (one_value);
  CPPUNIT_ASSERT(copy.x == 3 && copy.y == 3 && copy.z == 3);
  
  csVector3 from_double(csDVector3 (1, 2, 3));
  CPPUNIT_ASSERT(from_double.x == 1 && 
                 from_double.y == 2 && 
                 from_double.z == 3);
}

void csVector3Test::testAccessors()
{
  temp.Set(v1);
  const float f = temp[0];
  float g = temp[0];
  CPPUNIT_ASSERT_EQUAL(1.0f, f);
  CPPUNIT_ASSERT_EQUAL(f, g);

  temp[1] = 3.0f;
  CPPUNIT_ASSERT_EQUAL(3.0f, temp.y);
}

void csVector3Test::testManip()
{
  float v[3] = {7, 8, 9};
  CPPUNIT_ASSERT(v1.x == 1 && v1.y == 2 && v1.z == 3);
  
  temp.Set(5,6, 7);
  CPPUNIT_ASSERT(temp.x == 5 && temp.y == 6 && temp.z == 7);
  
  temp.Set(v2);
  CPPUNIT_ASSERT(temp.x == 4 && temp.y == 5 && temp.z == 6);
  
  temp.Set(v);
  CPPUNIT_ASSERT(temp.x == 7 && temp.y == 8 && temp.z == 9);
  
  temp.Set(0.0);
  CPPUNIT_ASSERT(temp.x == 0 && temp.y == 0 && temp.z == 0);
  
  temp.Get(v);
  CPPUNIT_ASSERT(v[0] == 0 && v[1] == 0 && v[2] == 0);
  
  CPPUNIT_ASSERT_DOUBLES_EQUAL(v2.Norm(), 8.774964f, 0.00001);
  
  CPPUNIT_ASSERT_DOUBLES_EQUAL(v2.Norm(), csVector3::Norm(v2), EPSILON);
  
  CPPUNIT_ASSERT_EQUAL(77.0f, v2.SquaredNorm());
  
  temp.Set(1.0f);
  temp.Normalize();
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.Norm(), 1.0f, EPSILON);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.x, 0.577350, 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.y, 0.577350, 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.z, 0.577350, 0.0001);
  
  temp = csVector3::Unit(csVector3(1.0f));
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.Norm(), 1.0f, EPSILON);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.x, 0.577350, 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.y, 0.577350, 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.z, 0.577350, 0.0001);
  
  temp.Cross (v1, v2);
  CPPUNIT_ASSERT(temp.x == -3 && temp.y == 6 && temp.z == -3);
}

void csVector3Test::testOp()
{
  temp.Set(v1);
  
  temp += v2; CPPUNIT_ASSERT(temp.x == 5 && temp.y == 7 && temp.z == 9);
  temp -= v1; CPPUNIT_ASSERT(temp.x == 4 && temp.y == 5 && temp.z == 6);
  temp *= 2.0f; CPPUNIT_ASSERT(temp.x == 8 && temp.y == 10 && temp.z == 12);
  temp /= 2.0f; CPPUNIT_ASSERT(temp.x == 4 && temp.y == 5 && temp.z == 6);
  temp = +v1; CPPUNIT_ASSERT(temp.x == 1 && temp.y == 2 && temp.z == 3);
  temp = -v2; CPPUNIT_ASSERT(temp.x == -4 && temp.y == -5 && temp.z == -6);
  
  temp = v1 + v2; CPPUNIT_ASSERT(temp.x == 5 && temp.y == 7 && temp.z == 9);
  temp = v2 - v1; CPPUNIT_ASSERT(temp.x == 3 && temp.y == 3 && temp.z == 3);
  temp = v1 * 2.0f; CPPUNIT_ASSERT(temp.x == 2 && temp.y == 4 && temp.z == 6);
  temp = 2.0f * v1; CPPUNIT_ASSERT(temp.x == 2 && temp.y == 4 && temp.z == 6);
  temp = 2 * v1; CPPUNIT_ASSERT(temp.x == 2 && temp.y == 4 && temp.z == 6);
  temp = temp / 2; CPPUNIT_ASSERT(temp.x == 1 && temp.y == 2 && temp.z == 3);
  
  CPPUNIT_ASSERT_DOUBLES_EQUAL(v1 * v2, 32.0f, EPSILON);
  CPPUNIT_ASSERT_EQUAL(v1.SquaredNorm(), v1 * v1);

  temp = v1 >> v2;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.x, 1.662337, 0.00001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.y, 2.077922, 0.00001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.z, 2.493506, 0.00001);
  
  temp = v2 << v1;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.x, 1.662337, 0.00001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.y, 2.077922, 0.00001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.z, 2.493506, 0.00001);
  
  temp.Cross (v1, v2);
  CPPUNIT_ASSERT (temp == v1 % v2);
  CPPUNIT_ASSERT (temp != v2 % v1);
  CPPUNIT_ASSERT (temp == -(v2 % v1));
}

void csVector3Test::testCond()
{
  temp.Set(v1);
  CPPUNIT_ASSERT(temp == v1);
  CPPUNIT_ASSERT(temp != v2);
  CPPUNIT_ASSERT(v1 < 3.1f);
  CPPUNIT_ASSERT(3.1f > v1);
}

void csVector3Test::testMisc()
{
  CPPUNIT_ASSERT(v1.DominantAxis() == CS_AXIS_Z);

  temp.Set(0.0f);
  CPPUNIT_ASSERT(temp.IsZero());
  
  temp = v1.Unit();
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.x, 0.267261, 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.y, 0.534522, 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.z, 0.801783, 0.0001);
  
  temp = v1.UnitAxisClamped();
  CPPUNIT_ASSERT(temp.x == 0 && temp.y == 0 && temp.z == 1);
}
