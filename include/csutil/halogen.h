/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Contributions made by Ivan Avramovic <ivan@avramovic.com>

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

#ifndef __CS_HALOGEN_H__
#define __CS_HALOGEN_H__

/**\file
 */

/**
 * Generate an halo alpha map given halo iSize (iSize x iSize) and
 * halo intensity factor (0..1) (this is NOT the intensity itself!).
 * The iCross argument is an 0..1 float that defines how much the
 * hallo ressembles a cross (0) or a circle (1).
 */
extern uint8 *csGenerateHalo (int iSize, float iFactor, float iCross);

/**
 * Generate an "super-nova" kind of halo alphamap given halo size,
 * initial random seed, number of spokes and a "roundness" coefficient
 * 0..1, with iRoundness == 1 you will get just a circle.
 */
extern uint8 *csGenerateNova (int iSize, int iSeed, int iNumSpokes,
  float iRoundness);

#endif // __CS_HALOGEN_H__
