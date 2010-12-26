/*
    Copyright (C) 2010 by Matthieu Kraus

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

#include "csgeom/math.h"
#include <limits>

/**
 * Test math functionality.
 */
class csMathTest : public CppUnit::TestFixture
{
public:
  void setUp();

  void testQuietNaN();
  void testSignalingNaN();

  CPPUNIT_TEST_SUITE(csMathTest);
    CPPUNIT_TEST(testQuietNaN);
    CPPUNIT_TEST(testSignalingNaN);
  CPPUNIT_TEST_SUITE_END();
};

void csMathTest::setUp()
{
}

typedef std::numeric_limits<float> float_limits;
typedef std::numeric_limits<double> double_limits;

void csMathTest::testQuietNaN()
{
  float zero_f = 0.f;
  double zero_d = 0.f;

  float qNaN_f;
  float qNaN_d;

  if(float_limits::has_quiet_NaN)
  {
    qNaN_f = float_limits::quiet_NaN();
  }
  else
  {
    union floatQuietNaN
    {
      float f;
      int32 v;
    } quietNaN;
    quietNaN.v = 0x7FC0;
    qNaN_f = quietNaN.f;
  }

  if(float_limits::has_quiet_NaN)
  {
    qNaN_d = float_limits::quiet_NaN();
  }
  else
  {
    union doubleQuietNaN
    {
      double f;
      int64 v;
    } quietNaN;
    quietNaN.v = 0x7FE00000;
    qNaN_d = quietNaN.f;
  }

  CPPUNIT_ASSERT(CS::IsNaN(qNaN_f));
  CPPUNIT_ASSERT(!CS::IsNaN(zero_f));
  CPPUNIT_ASSERT(CS::IsNaN(qNaN_d));
  CPPUNIT_ASSERT(!CS::IsNaN(zero_d));
}

void csMathTest::testSignalingNaN()
{
  float zero_f = 0.f;
  double zero_d = 0.f;

  float sNaN_f;
  float sNaN_d;

  if(float_limits::has_signaling_NaN)
  {
    sNaN_f = float_limits::signaling_NaN();
  }
  else
  {
    union floatSignalingNaN
    {
      float f;
      int32 v;
    } signalingNaN;
    signalingNaN.v = 0x7F80;
    sNaN_f = signalingNaN.f;
  }

  if(float_limits::has_signaling_NaN)
  {
    sNaN_d = float_limits::signaling_NaN();
  }
  else
  {
    union doubleSignalingNaN
    {
      double f;
      int64 v;
    } signalingNaN;
    signalingNaN.v = 0x7FC00000;
    sNaN_d = signalingNaN.f;
  }

  CPPUNIT_ASSERT(CS::IsNaN(sNaN_f));
  CPPUNIT_ASSERT(!CS::IsNaN(zero_f));
  CPPUNIT_ASSERT(CS::IsNaN(sNaN_d));
  CPPUNIT_ASSERT(!CS::IsNaN(zero_d));
}

