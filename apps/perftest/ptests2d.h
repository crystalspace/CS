/*
    Copyright (C) 2000 by Jorrit Tyberghein
    Written by Samuel Humphreys

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

#ifndef __PTESTS2D_H__
#define __PTESTS2D_H__

#include <stdarg.h>
#include "apps/perftest/perftest.h"

struct iGraphics3D;
struct iGraphics2D;
struct iFont;

class Tester2D : public Tester
{
protected:
  iGraphics2D *G2D;
  int max_w, max_h, inc_h;
  int colour[8];
public:
  virtual void Setup (iGraphics3D* g3d, PerfTest* perftest);
  virtual ~Tester2D () { }
};

class LineTester2D : public Tester2D
{

public:
  virtual void Draw (iGraphics3D* g3d);
  virtual void Description (char* dst)
  {
    sprintf (dst, "%d 2D lines... ", 5*(max_w-10));
  }
  virtual Tester* NextTester ();
};

class PixelTester : public Tester2D
{

public:
  virtual void Draw (iGraphics3D* g3d);
  virtual void Description (char* dst)
  {
    sprintf (dst, "%d Pixels... ", max_w*max_h);
  }
  virtual Tester* NextTester ();
};

class StringTester : public Tester2D
{
  char *line;
  int length, text_height, rows;
  csRef<iFont> font;

public:
  virtual void Setup (iGraphics3D* g3d, PerfTest* perftest);
  virtual void Draw (iGraphics3D* g3d);
  virtual void Description (char* dst)
  {
    sprintf (dst, "%d characters using Write... ", length*rows);
  }
  virtual Tester* NextTester ();
};

#endif // __PTESTS2D_H__
