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

#include "sysdef.h"
#include "csutil/csstring.h"
#include "conbuffr.h"

csConsoleBuffer::csConsoleBuffer(int length, int size)
{
  buffer = NULL;
  SetLength(length);
  SetPageSize(size);
}

csConsoleBuffer::~csConsoleBuffer()
{
  if(buffer) {
    // Clear out the current buffer and delete it
    Clear();
    delete buffer;
    delete dirty;
  }
}

void csConsoleBuffer::NewLine(bool snap)
{
  // Create an "empty" display string to avoid NULL pointer ugliness
  if(buffer[current_line]==NULL)
    buffer[current_line] = new csString(' ');
  
  // Increment the current line and account for going past the end of the buffer
  current_line++;
  if(current_line>=len) {
    // Delete the topmost line
    delete buffer[0];
    // Shift all the lines up by one
    memmove(buffer, &buffer[1], (len-1) * sizeof(csString *));
    current_line = len - 1;
    // We don't want the now second to last string deleted!
    buffer[current_line] = NULL;

    // Invalidate all the lines now visible to scroll properly
    int i;
    for(i=display_top;i<=display_bottom;i++)
      dirty[i] = true;
  }

  // Clear the new current line and mark it dirty
  if(buffer[current_line]!=NULL) {
    delete buffer[current_line];
    buffer[current_line] = NULL;
    dirty[current_line] = true;
  }

  if(snap&&((current_line>display_bottom)||(current_line<=display_top))) {
    display_top = (current_line - page_size) % len;
    if(display_top<0) {
      display_top = 0;
      display_bottom = page_size;
    } else
      display_bottom = current_line;

    // Invalidate all the lines now visible to scroll properly
    int i;
    for(i=display_top;i<=display_bottom;i++)
      dirty[i] = true;
  }

}

csString *csConsoleBuffer::WriteLine()
{
  // Prevent any NULL pointer ugliness
  if(buffer[current_line]==NULL)
    buffer[current_line] = new csString();
  dirty[current_line] = true;
  return buffer[current_line];
}

const csString *csConsoleBuffer::GetLine(int line, bool &dirty_line)
{
  // Calculate the position of the line
  int pos = (display_top + line) % len;

  // Tsk tsk...
  if(pos>display_bottom)
    return NULL;

  // Return the current dirty flag
  dirty_line = dirty[pos];

  // Clear the dirty flag
  dirty[pos] = false;

  // Return the csString
  return buffer[pos];
}

void csConsoleBuffer::SetLength(int length)
{
  int i;

  if(buffer) {
    // Clear out the current buffer and delete it
    Clear();
    delete buffer;
    delete dirty;
  }

  // Set up the new buffer
  len = length; 
  buffer = new csString*[length];
  dirty = new bool[length];

  // Clear out any stray values in the new array
  for(i = 0; i<len; i++) {
    if(buffer[i]) {
      buffer[i] = NULL;
      dirty[i] = false;
    }
  }
}

void csConsoleBuffer::Clear()
{
  int i;

  // Go through each line and delete it's csString
  for(i = 0; i<len; i++) {
    if(buffer[i]) {
      delete buffer[i];
      buffer[i] = NULL;
      dirty[i] = true;
    }
  }

  // Reset the state variables
  current_line = display_top = 0;
  display_bottom = page_size;

}

void csConsoleBuffer::SetPageSize(int size)
{
  // Set the page size and force a snap to the current line
  page_size = size;
  display_top = current_line - size;
  if(display_top<0) {
    display_top = 0;
    display_bottom = page_size;
  } else
    display_bottom = current_line;
}

void csConsoleBuffer::SetTopLine(int line)
{
  // Make sure we don't go past the end of the buffer
  if(line + page_size > len) {
    display_top = len - page_size;
    display_bottom = len;
  } else
    display_top = line;
}

void csConsoleBuffer::SetCurLine(int line)
{
  // Make sure we don't go past the end of the buffer
  if(line>len)
    line = len;
  else
    current_line = line;
}
