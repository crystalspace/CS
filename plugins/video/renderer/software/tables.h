/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

#ifndef __TABLES_H__
#define __TABLES_H__

/**
 * These define the quality of bilinear filtering. The higher these values
 * are the higher is the quality of picture you'll see. However, memory
 * consumptions are also growing rapidly.
 */
#define LOG2_STEPS_X			6
#define LOG2_STEPS_Y			6
#define LOG2_NUM_LIGHT_INTENSITIES	5

/**
 * Do not change manually! This will break the code.
 */
#define NUM_LIGHT_INTENSITIES		(1<<LOG2_NUM_LIGHT_INTENSITIES)
#define X_AND_FILTER			(0xffff-(1<<(17-LOG2_STEPS_X))+1)
#define Y_AND_FILTER			(0xffff-(1<<(17-LOG2_STEPS_Y))+1)

/**
 * This class represents all precalculated tables that are needed.
 * Currently there are only some tables for FltLights.
 * Note: all angle measures in the tables are in degrees, NOT radians
 */
class csTables
{
public:
  /// Sinus table.
  float* sin_tab;
  /// Cosinus table.
  float* cos_tab;
  /// Arc-cosinus table.
  int* acos_tab;
  /// Arc-sinus table.
  int* asin_tab;
  /**
   * Combined cos(asin(x)) table,
   * which equals sin(acos(x)), and sqrt(1-x*x)
   */
  long* cos_asin_tab;	// F20:12
  /**
   * Exponent table used for fogging.
   * Index is dist*density in 8:8 fixed point format.
   * Result is 1-exp(-dist*density) between 0 and 255.
   */
  UByte* exp_table;

  /// Multiplication table (for fog). Added by Denis Dmitriev
  int* mul_table;
  int* exp_table_2_real;	// Real table (extra 20 overflow fields in front)
  int* exp_table_2;		// Points to exp_real_2_table+20.

  /*
   * These tables are used for bilinear filtering. This feature is still
   * (and IMHO, till Merced III-10GHz, will remain) experimental. - D.D.
   * @@@ To all familiar with MMX: implementation in MMX can be fast -- routine
   *     makes the same things for four pixels values. And quality could jump
   *     (for speed and memory consumption I'm using 16 grades).
   */
  /** This table incorporates values (1-x)*(1-y), (1-x)*y, x*(1-y), and x*y */
  int* another_mul_table;
  /**
   * This table is really two tables in one (they have the same size --
   * notice the 2 as a multiplier). It removes all the multiplications from
   * the function. Sorry for lack of details, but there's a lot to be said.
   * If you really want to know what is this table for, see "math3d.cpp" and
   * "scan16.cpp" -- generation of the table, and it's usage respectively.
   *                                                                  - D.D.
   */
  unsigned short* color_565_table;

  /**
   * Compute all of the values of the tables.
   * Note!  For global objects in a shared library, 'Initialize()' will
   * need to be invoked explicitly, because the constructor never gets
   * called.
   */
  csTables() : sin_tab (NULL), cos_tab (NULL), acos_tab (NULL), asin_tab (NULL),
  	cos_asin_tab (NULL), exp_table (NULL), mul_table (NULL), exp_table_2_real (NULL), exp_table_2 (NULL),
	another_mul_table (NULL), color_565_table (NULL)
	{ }

  /// Clean all tables.
  ~csTables ();

  /**
   * Compute all of the values of the tables.
   * Note! Call this fuction explicitly for a global objects.  See the
   * description for the constructor for more info.
   */
  void Initialize ();
};

extern csTables tables;

#endif /*__TABLES_H__*/

