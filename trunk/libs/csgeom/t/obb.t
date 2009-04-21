/*
    Copyright (C) 2002 by Jorrit Tyberghein
    Copyright (C) 2005 by Eric Sunshine <sunshine@sunshineco.com>

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

#include "csgeom/obb.h"

/**
 * Test csOBB operations.
 */
class csOBBTest : public CppUnit::TestFixture
{
private:
  static void expect(csOBB const&, float const expected[8][3]);
  static void defaultTable(csVector3 vertex_table[4]);

public:
  void testFind();
  void testFindAccurate();
  void testFindComplex();

  CPPUNIT_TEST_SUITE(csOBBTest);
    CPPUNIT_TEST(testFind);
    CPPUNIT_TEST(testFindAccurate);
    CPPUNIT_TEST(testFindComplex);
  CPPUNIT_TEST_SUITE_END();
};

void csOBBTest::expect(csOBB const& obb, float const expected[8][3])
{
  for (size_t i = 0; i < 8; i++)
  {
    csVector3 const v = obb.GetCorner(i);
    for (size_t j = 0; j < 3; j++)
      CPPUNIT_ASSERT_DOUBLES_EQUAL(v[j], expected[i][j], EPSILON);
  }
}

void csOBBTest::defaultTable(csVector3 vertex_table[4])
{
  vertex_table[0].Set(10, 10, 10);
  vertex_table[1].Set( 0,  0,  2);
  vertex_table[2].Set( 1,  0,  3);
  vertex_table[3].Set( 2,  0,  0);
}

void csOBBTest::testFind()
{
  static float const expected[8][3] =
    {
      {  9.827,  9.940, 10.290 },
      { 10.318,  9.318, 10.454 },
      { 11.357, 10.470,  7.715 },
      { 11.848,  9.848,  7.878 },
      { -0.172, -0.059,  2.290 },
      {  0.318, -0.681,  2.454 },
      {  1.357,  0.470, -0.284 },
      {  1.848, -0.151, -0.121 }
    };

  csVector3 vertex_table[4];
  defaultTable(vertex_table);

  csOBB obb;
  obb.FindOBB(vertex_table, 4);
  expect(obb, expected);
}

void csOBBTest::testFindAccurate()
{
  static float const expected[8][3] =
    {
      {  10,    10,     10     },
      {  9.443, 10.630,  9.814 },
      {  8.333,  9.166, 12.166 },
      {  7.776,  9.797, 11.981 },
      {  2,     -0.000, -0.000 },
      {  1.443,  0.630, -0.185 },
      {  0.333, -0.833,  2.166 },
      { -0.223, -0.202,  1.981 }
    };

  csVector3 vertex_table[4];
  defaultTable(vertex_table);

  csOBB obb;
  obb.FindOBBAccurate(vertex_table, 4);
  expect(obb, expected);
}

void csOBBTest::testFindComplex()
{
  static float const expected[8][3] =
    {
      { -3,-3,-3 },
      { -3,-3, 3 },
      { -3, 3,-3 },
      { -3, 3, 3 },
      {  3,-3,-3 },
      {  3,-3, 3 },
      {  3, 3,-3 },
      {  3, 3, 3 }
    };

  csVector3 vertex_table[22];
  vertex_table[ 0].Set(-2, -2,  2);
  vertex_table[ 1].Set( 2, -2,  2);
  vertex_table[ 2].Set(-2, -2, -2);
  vertex_table[ 3].Set( 2, -2, -2);
  vertex_table[ 4].Set(-2,  2,  2);
  vertex_table[ 5].Set( 2,  2,  2);
  vertex_table[ 6].Set(-2,  2, -2);
  vertex_table[ 7].Set( 2,  2, -2);
  vertex_table[ 8].Set( 0,  0, -3);
  vertex_table[ 9].Set( 3,  0,  0);
  vertex_table[10].Set( 0,  0,  3);
  vertex_table[11].Set(-3,  0,  0);
  vertex_table[12].Set( 0,  3,  0);
  vertex_table[13].Set( 0, -3,  0);
  vertex_table[14].Set(-3, -3, -3);
  vertex_table[15].Set(-3, -3,  3);
  vertex_table[16].Set(-3,  3, -3);
  vertex_table[17].Set(-3,  3,  3);
  vertex_table[18].Set( 3, -3, -3);
  vertex_table[19].Set( 3, -3,  3);
  vertex_table[20].Set( 3,  3, -3);
  vertex_table[21].Set( 3,  3,  3);

  csOBB obb;
  obb.FindOBB(vertex_table, 22);
  expect(obb, expected);
}
