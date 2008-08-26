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

#include "csgeom/vector4.h"
#include "csgeom/math3d_d.h"

/**
 * Test csVector4 operations.
 *
 * Excluded: standard constructor, csVector4::Description()
 */
class csVector4Test : public CppUnit::TestFixture
{
private:
  csVector4 v1;
  csVector4 v2;
  csVector4 temp;

public:
  void setUp();

  void testConstructors();
  void testAccessors();
  void testManip();
  void testOp();
  void testCond();
  void testMisc();

  CPPUNIT_TEST_SUITE(csVector4Test);
    CPPUNIT_TEST(testConstructors);
    CPPUNIT_TEST(testAccessors);
    CPPUNIT_TEST(testManip);
    CPPUNIT_TEST(testOp);
    CPPUNIT_TEST(testCond);
    CPPUNIT_TEST(testMisc);
  CPPUNIT_TEST_SUITE_END();
};

void csVector4Test::setUp()
{
  v1.x = 1.0f;
  v1.y = 2.0f;
  v1.z = 3.0f;
  v1.w = 4.0f;
  
  v2.x = 5.0f;
  v2.y = 6.0f;
  v2.z = 7.0f;  
  v2.w = 8.0f;
}

void csVector4Test::testConstructors()
{
  csVector4 one_value(3.0f);
  CPPUNIT_ASSERT(one_value.x == 3.0f && one_value.y == 3.0f && 
    one_value.z == 3.0f && one_value.w == 3.0f);
  
  csVector4 two_values(4.0f, 5.0f);
  CPPUNIT_ASSERT(two_values.x == 4.0f && two_values.y == 5.0f &&
    two_values.z == 0.0f && two_values.w == 1.0f);
  
  csVector4 three_values(3.0f, 4.0f, 5.0f);
  CPPUNIT_ASSERT(three_values.x == 3.0f && three_values.y == 4.0f && 
                 three_values.z == 5.0f && three_values.w == 1.0f);

  csVector4 four_values(1.0f, 2.0f, 3.0f, 4.0f);
  CPPUNIT_ASSERT(four_values.x = 1.0f && four_values.y == 2.0f &&
    four_values.z == 3.0f && four_values.w == 4.0f);

  csVector4 copy (one_value);
  CPPUNIT_ASSERT(copy.x == 3.0f && copy.y == 3.0f && copy.z == 3.0f &&
    copy.w == 3.0f);
}

void csVector4Test::testAccessors()
{
  temp.Set(v1);
  const float f = temp[0];
  float g = temp[0];
  CPPUNIT_ASSERT_EQUAL(1.0f, f);
  CPPUNIT_ASSERT_EQUAL(f, g);

  temp[3] = 3.0f;
  CPPUNIT_ASSERT_EQUAL(3.0f, temp.w);

  float h[4];
  temp.Get(h);
  CPPUNIT_ASSERT_EQUAL(2.0f, h[1]);

  float& i = temp[3];
  i = 5.0f;
  CPPUNIT_ASSERT_EQUAL(5.0f, temp.w);
}

void csVector4Test::testManip()
{
  float v[4] = {7.0f, 8.0f, 9.0f, 10.0f};
  CPPUNIT_ASSERT(v1.x == 1.0f && v1.y == 2.0f && v1.z == 3.0f &&
    v1.w == 4.0f);
  
  temp.Set(5.0f, 6.0f, 7.0f, 8.0f);
  CPPUNIT_ASSERT(temp.x == 5.0f && temp.y == 6.0f && temp.z == 7.0f &&
    temp.w == 8.0f);
  
  temp.Set(v2);
  CPPUNIT_ASSERT(temp.x == 5.0f && temp.y == 6.0f && temp.z == 7.0f &&
    temp.w == 8.0f);
  
  temp.Set(v);
  CPPUNIT_ASSERT(temp.x == 7.0f && temp.y == 8.0f && temp.z == 9.0f &&
    temp.w == 10.0f);
  
  temp.Set(0.0);
  CPPUNIT_ASSERT(temp.x == 0.0f && temp.y == 0.0f && temp.z == 0.0f && 
    temp.w == 0.0f);
  
  temp.Get(v);
  CPPUNIT_ASSERT(v[0] == 0.0f && v[1] == 0.0f && v[2] == 0.0f && 
    v[3] == 0.0f);
  
  CPPUNIT_ASSERT_DOUBLES_EQUAL(v2.Norm(), 13.190905f, 0.00001f);
  
  CPPUNIT_ASSERT_DOUBLES_EQUAL(v2.Norm(), csVector4::Norm(v2), EPSILON);
  
  CPPUNIT_ASSERT_EQUAL(174.0f, v2.SquaredNorm());
  
  temp.Set(1.0f);
  temp.Normalize();
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.Norm(), 1.0f, EPSILON);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.x, 0.5f, 0.0001f);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.y, 0.5f, 0.0001f);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.z, 0.5f, 0.0001f);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.w, 0.5f, 0.0001f);
  
  temp = csVector4::Unit(csVector4(1.0f));
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.Norm(), 1.0f, EPSILON);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.x, 0.5f, 0.0001f);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.y, 0.5f, 0.0001f);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.z, 0.5f, 0.0001f);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.w, 0.5f, 0.0001f);
  
  temp.Cross (v1, v2);
  CPPUNIT_ASSERT_EQUAL(-16.0f, temp.x);
  CPPUNIT_ASSERT_EQUAL(- 8.0f, temp.y);
  CPPUNIT_ASSERT_EQUAL(  0.0f, temp.z);
  CPPUNIT_ASSERT_EQUAL( 24.0f, temp.w);
}

void csVector4Test::testOp()
{
  temp.Set(v1);
  
  temp += v2;
  CPPUNIT_ASSERT(temp.x == 6.0f && temp.y == 8.0f && temp.z == 10.0f && 
    temp.w == 12.0f);
  
  temp -= v1;
  CPPUNIT_ASSERT(temp.x == 5.0f && temp.y == 6.0f && temp.z == 7.0f && 
    temp.w == 8.0f);
  
  temp *= 2.0f;
  CPPUNIT_ASSERT(temp.x == 10.0f && temp.y == 12.0f && temp.z == 14.0f && 
    temp.w == 16.0f);
  
  temp /= 2.0f;
  CPPUNIT_ASSERT(temp.x == 5.0f && temp.y == 6.0f && temp.z == 7.0f && 
    temp.w == 8.0f);
  
  temp = +v1;
  CPPUNIT_ASSERT(temp.x == 1.0f && temp.y == 2.0f && temp.z == 3.0f && 
    temp.w == 4.0f);
  
  temp = -v2;
  CPPUNIT_ASSERT(temp.x == -5.0f && temp.y == -6.0f && temp.z == -7.0f && 
    temp.w == -8.0f);
  
  
  temp = v1 + v2;
  CPPUNIT_ASSERT(temp.x == 6.0f && temp.y == 8.0f && temp.z == 10.0f && 
    temp.w == 12.0f);
  
  temp = v2 - v1;
  CPPUNIT_ASSERT(temp.x == 4.0f && temp.y == 4.0f && temp.z == 4.0f && 
    temp.w == 4.0f);
  
  temp = v1 * 2.0f;
  CPPUNIT_ASSERT(temp.x == 2.0f && temp.y == 4.0f && temp.z == 6.0f && 
    temp.w == 8.0f);
  
  temp = 2.0f * v1;
  CPPUNIT_ASSERT(temp.x == 2.0f && temp.y == 4.0f && temp.z == 6.0f && 
    temp.w == 8.0f);
  
  temp = 2 * v1;
  CPPUNIT_ASSERT(temp.x == 2.0f && temp.y == 4.0f && temp.z == 6.0f && 
    temp.w == 8.0f);
  
  temp = temp / 2;
  CPPUNIT_ASSERT(temp.x == 1.0f && temp.y == 2.0f && temp.z == 3.0f && 
    temp.w == 4.0f);
  
  CPPUNIT_ASSERT_DOUBLES_EQUAL(v1 * v2, 70.0f, EPSILON);
  CPPUNIT_ASSERT_EQUAL(v1.SquaredNorm(), v1 * v1);

  temp = v1 >> v2;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.x, 2.011494, 0.00001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.y, 2.413793, 0.00001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.z, 2.816092, 0.00001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.w, 3.218390, 0.00001);
  
  temp = v2 << v1;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.x, 2.011494, 0.00001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.y, 2.413793, 0.00001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.z, 2.816092, 0.00001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.w, 3.218390, 0.00001);
  
  temp.Cross (v1, v2);
  CPPUNIT_ASSERT (temp == v1 % v2);
  CPPUNIT_ASSERT (temp != v2 % v1);
  CPPUNIT_ASSERT (temp == -(v2 % v1));
}

void csVector4Test::testCond()
{
  temp.Set(v1);
  CPPUNIT_ASSERT(temp == v1);
  CPPUNIT_ASSERT(temp != v2);
  CPPUNIT_ASSERT(v1 < 4.1f);
  CPPUNIT_ASSERT(4.1f > v1);
}

void csVector4Test::testMisc()
{
  temp.Set(0.0f);
  CPPUNIT_ASSERT(temp.IsZero());
  
  temp = v1.Unit();
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.x, 0.182574, 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.y, 0.365148, 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.z, 0.547722, 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(temp.w, 0.730296, 0.0001);
}
