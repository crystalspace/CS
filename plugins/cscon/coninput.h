/*
    Copyright (C) 2000 by Michael Dale Long

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

#ifndef __CS_CONINPUT_H__
#define __CS_CONINPUT_H__

#include "iconinp.h"

struct iSystem;
struct iConsole;
class csString;
class csEvent;
class csConsoleBuffer;

class csConsoleInput : public iConsoleInput
{
public:
  DECLARE_IBASE;
  csConsoleInput(iBase *base);
  virtual ~csConsoleInput();
  virtual bool Initialize(iSystem *system);
  virtual bool HandleEvent(csEvent &event);
  virtual const csString *GetInput(int line = -1) const;
  virtual int GetCurLine() const;
  virtual void NewLine();
  virtual int GetBufferSize() const;
  virtual void SetBufferSize(int size);
  virtual void Clear();
  virtual bool GetEcho() const;
  virtual void SetEcho(bool echo, iConsole *console = NULL);

protected:
  iSystem *piSystem;
  iConsole *piConsole;
  csConsoleBuffer *buffer;
};

#endif // ! __CS_CONINPUT_H__
