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

#ifndef __CS_ISTRVEC_H_
#define __CS_ISTRVEC_H_

#include "csutil/scf.h"

SCF_INTERFACE (iStrVector, 0, 0, 1) : public iBase
{
  virtual int Length() const = 0;
  virtual char* Get (int n) const = 0;
  virtual int Find (const char*) const = 0;
  virtual int Push (char*) = 0;
  virtual char* Pop() = 0;
};

#endif // __CS_ISTRVEC_H_
