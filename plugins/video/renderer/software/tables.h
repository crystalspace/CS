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

/// At this point QRound (255 * exp (-float (i) / 256.)) reaches zero
#define EXP_256_SIZE			1420
/// Same for QRound (32 * exp (-float (i) / 256.))
#define EXP_16_SIZE			1065

/**
 * This class represents all precalculated tables that are needed.
 */
class csTables
{
public:
  /**
   * These tables are used for bilinear filtering. This feature is still
   * (and IMHO, till Merced III-10GHz, will remain) experimental. - D.D.
   * <i>To all familiar with MMX: implementation in MMX can be fast -- routine
   *    makes the same things for four pixels values. And quality could jump
   *    (for speed and memory consumption I'm using 16 grades).</i>
   * <p>
   * This table incorporates values (1-x)*(1-y), (1-x)*y, x*(1-y), and x*y
   */
  int* filter_mul_table;

  /**
   * This table is really two tables in one (they have the same size --
   * notice the 2 as a multiplier). It removes all the multiplications from
   * the function. Sorry for lack of details, but there's a lot to be said.
   * If you really want to know what is this table for, see "math3d.cpp" and
   * "scan16.cpp" -- generation of the table, and it's usage respectively.
   */
  unsigned short *color_565_table;

  /**
   * A table of 4096 1/z values where z is in fixed-point 0.12 format
   * Used in fog routines to get the real value of Z. The result is
   * an 8.24 fixed-point number.
   */
  unsigned int *one_div_z;

  /**
   * A table of exp(x) in the range 0..255; x == 0..EXP_256_SIZE
   */
  unsigned char *exp_256;
  /**
   * Same in the range 0..31 for 8-bit fog
   */
  unsigned char *exp_16;

  /**
   * Compute all of the values of the tables.
   * Note!  For global objects in a shared library, 'Initialize()' will
   * need to be invoked explicitly, because the constructor never gets
   * called.
   */
  csTables() : filter_mul_table (NULL), color_565_table (NULL),
    one_div_z (NULL), exp_256 (NULL)
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

