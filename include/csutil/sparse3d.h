/*
    Sparse 3-D matrix.
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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

#ifndef __CS_SPARSE3D_H__
#define __CS_SPARSE3D_H__

/**\file
 * 3D sparse matrix 
 */

#include "csextern.h"

/**
 * General 3D sparse matrix class. This is an abstract class.
 * Specific implementations are csWideSparse3D and csDenseSparse3D.
 */
class CS_CRYSTALSPACE_EXPORT csSparse3D
{
public:
  ///
  csSparse3D () { }
  ///
  virtual ~csSparse3D () { }

  /// 
  virtual void Clear () = 0;

  /// 
  virtual void Set (int x, int y, int z, void* obj) = 0;

  ///
  virtual void* Get (int x, int y, int z) = 0;

  ///
  virtual void Del (int x, int y, int z) = 0;
};

/**
 * This implementation of csSparse3D is very suited where the
 * accesses will be very widely spaced (for example: one element
 * at (-1000,0,0) and one at (1000,0,0)). Getting and setting
 * elements is not as efficient as with DenseSparse3D but it
 * consumes less memory.
 * @@@ NOTE! Current implementation is very naive. The list should
 * be sorted to make searching at least a little more efficient.
 * <p>
 * This implementation of Sparse3D is better suited when accesses
 * are close together. The speed of access is much better (no need
 * to scan lists).
 */
class CS_CRYSTALSPACE_EXPORT csWideSparse3D : public csSparse3D
{
private:
  ///
  struct SparseCell
  {
    int z;
    void* obj;
    SparseCell* next, * prev;
  };
  ///
  struct HdY
  {
    HdY () { first_z = 0; }
    int y;
    HdY* next, * prev;
    SparseCell* first_z;
  };
  ///
  struct HdX
  {
    HdX () { first_y = 0; }
    int x;
    HdX* next, * prev;
    HdY* first_y;
  };

  ///
  HdX* first_x;
  ///
  HdX* get_header_x (int x);
  ///
  HdY* get_header_y (HdX* y_list, int y);
  ///
  SparseCell* get_cell_z (HdY* z_list, int z);

public:
  ///
  csWideSparse3D ();
  ///
  virtual ~csWideSparse3D ();

  ///
  virtual void Clear ();

  ///
  virtual void Set (int x, int y, int z, void* obj);

  ///
  virtual void* Get (int x, int y, int z);

  ///
  virtual void Del (int x, int y, int z);
};

#endif // __CS_SPARSE3D_H__
