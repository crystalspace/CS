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

#ifndef __CS_CSCON_H__
#define __CS_CSCON_H__

#include "iconsole.h"
#include "csutil/scf.h"
#include "csutil/csstring.h"

struct iGraphics2D;
struct iGraphics3D;
class csRect;

class csConsole : public iConsole
{
public:
  DECLARE_IBASE;
  csConsole(iBase *base);
  virtual ~csConsole();
  virtual bool Initialize(iSystem *);
  virtual void Show();
  virtual void Hide();
  virtual bool IsActive();
  virtual void Clear();
  virtual void PutText(const char *text);
  virtual void Draw(csRect *rect);
  virtual void SetBufferSize(int lines);

protected:
  bool active;
  csString **buffer; // Line buffer array
  int line; // Current line in the buffer;
  int topline, maxlines; // Top and Maximum lines in buffer
  iGraphics2D *piG2D;
  iGraphics3D *piG3D;
  iSystem *piSystem;

  // Increments to the next column/line, and accounts for various limits
  void IncLine();

};

#endif // ! __CS_CSCON_H__
