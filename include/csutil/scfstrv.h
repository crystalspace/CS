/*
    Crystal Space Virtual File System SCF interface
    Copyright (C) 1999 by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __CS_SCFSTRV_H_
#define __CS_SCFSTRV_H_

#include "istrvec.h"
#include "csutil/csstrvec.h"

class scfStrVector : public csStrVector, public iStrVector
{
  typedef csStrVector superclass;
public:
  DECLARE_IBASE;

  scfStrVector (int ilimit = 64, int ithreshold = 64);
  virtual int Length() const;
  virtual char* Get (int n) const;
  virtual int Find (const char*) const;
  virtual int Push (char*);
  virtual char* Pop();
};

#endif // __CS_SCFSTRV_H_
