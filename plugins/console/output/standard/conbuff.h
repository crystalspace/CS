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

#ifndef __CS_CONBUFF_H__
#define __CS_CONBUFF_H__

#include "csutil/csstring.h"

class csConsoleBuffer
{
public:
  csConsoleBuffer(int lenth, int size);
  ~csConsoleBuffer();

  void NewLine(bool snap = true);
  csString *WriteLine();
  const csString *GetLine(int line, bool *dirty = 0);
  void SetLength(int length);
  void Clear();
  void SetPageSize(int size);
  void SetTopLine(int line);
  void SetCurLine(int line);
  void DeleteLine(int line);

  inline int GetLength() const { return len; }
  inline int GetPageSize() const { return page_size; }
  inline int GetTopLine() const { return display_top; }
  inline int GetCurLine() const { return current_line; }
  inline bool IsLineEmpty(int line) const
  {
    return (buffer[line]==0 ||
    	    buffer[line]->IsEmpty() ||
	    buffer[line]==empty);
  }

private:
  int len;
  int page_size;
  int display_top;
  int display_bottom;
  int current_line;

  csString **buffer, *empty;
  bool *dirty;
};

#endif // __CS_CONBUFF_H__
