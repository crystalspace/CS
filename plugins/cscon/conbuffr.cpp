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

#include "cssysdef.h"
#include "csutil/csstring.h"
#include "conbuffr.h"

csConsoleBuffer::csConsoleBuffer (int length, int size)
{
  buffer = NULL;
  SetLength (length);
  SetPageSize (size);
  empty = new csString (" ");

  // Initialize the state variables
  current_line = display_top = 0;
  display_bottom = page_size;
}

csConsoleBuffer::~csConsoleBuffer ()
{
  if (buffer)
  {
    // Clear out the current buffer and delete it
    Clear ();
    delete buffer;
    delete dirty;
    delete empty;
  }
}

void csConsoleBuffer::NewLine (bool snap)
{
//    printf("*** top = %d, bottom = %d, size = %d\n", display_top, display_bottom, GetPageSize());
  // Assign the empty display string to avoid NULL pointer ugliness
  if (buffer[current_line] == NULL)
    buffer [current_line] = empty;

  // Increment the current line and account for going past the end of the buffer
  current_line++;
  if (current_line >= len)
  {
    // Delete the topmost line
    delete buffer [0];
    // Shift all the lines up by one
    memmove (buffer, &buffer [1], (len - 1) * sizeof (csString *));
    current_line = len - 1;
    // We don't want the now second to last string deleted!
    buffer [current_line] = NULL;

    // Invalidate all the lines now visible to scroll properly
    int i;
    for (i = display_top; i <= display_bottom; i++)
      dirty [i] = true;
  }

  // Clear the new current line and mark it dirty
  if (buffer [current_line] != NULL)
  {
    delete buffer [current_line];
    buffer [current_line] = NULL;
    dirty [current_line] = true;
  }

  if (snap
   && ((current_line >= display_bottom)
    || (current_line <= display_top)))
  {
    display_top++;
    display_bottom++;
    
    /*
    display_top = current_line - page_size + 1;
    if (display_top < 0)
    {
      display_top = 0;
      display_bottom = page_size;
    }
    else
      display_bottom = current_line + 1;
    */
    // Invalidate all the lines now visible to scroll properly
    int i;
    for (i = display_top; i <= display_bottom; i++)
      dirty [i] = true;
  }

}

csString *csConsoleBuffer::WriteLine ()
{
  // Prevent any NULL pointer ugliness
  if (buffer [current_line] == NULL
   || buffer [current_line] == empty)
    buffer [current_line] = new csString ();
  dirty [current_line] = true;
  return buffer [current_line];
}

const csString *csConsoleBuffer::GetLine (int line, bool *dirty_line)
{
  // Tsk tsk...
  if (line > display_bottom)
    return NULL;

  // Calculate the position of the line
  int pos = display_top + line;

  // Return the current dirty flag
  if (dirty_line)
    *dirty_line = dirty [pos];

  // Clear the dirty flag
  dirty [pos] = false;

  // Return the csString
  return buffer [pos];
}

void csConsoleBuffer::SetLength (int length)
{
  int i;

  if (buffer)
  {
    // Clear out the current buffer and delete it
    Clear ();
    delete buffer;
    delete dirty;
  }

  // Set up the new buffer
  len = length; 
  buffer = new csString * [length];
  dirty = new bool [length];

  // Clear out any stray values in the new array
  for (i = 0; i < len; i++)
    if (buffer [i])
    {
      buffer [i] = NULL;
      dirty [i] = false;
    }
}

void csConsoleBuffer::Clear ()
{
  int i;

  // Go through each line and delete it's csString
  for (i = 0; i < len; i++)
    if (buffer [i])
    {
      if (buffer [i] != empty)
	delete buffer [i];
      buffer [i] = NULL;
      dirty [i] = true;
    }

  // Reset the state variables
  current_line = display_top = 0;
  display_bottom = page_size;
}

void csConsoleBuffer::SetPageSize (int size)
{
  // Set the page size and force a snap to the current line
  display_bottom = display_top + size;
  if (current_line > display_bottom)
  {
    display_bottom = current_line;
    display_top = display_bottom - size;
    if (display_top < 0)
    {
      display_top = 0;
      display_bottom = size;
    }
  }
  page_size = size;
  /*
  display_top = current_line - size + 1;
  if (display_top < 0)
  {
    display_top = 0;
    display_bottom = page_size;
  }
  else
    display_bottom = current_line + 1;
  */
}

void csConsoleBuffer::SetTopLine (int line)
{
  // Make sure we don't go "above" the top of the buffer
  if (line < 0)
  {
    display_top = 0;
    display_bottom = page_size;
  } // Make sure we don't go past the end of the buffer
  else if (line + page_size > len)
  {
    display_top = len - page_size;
    display_bottom = len;
  }
  else if (line > current_line)
  {
    display_top = current_line;
    display_bottom = current_line + page_size;
  }
  else
  {
    display_top = line;
    display_bottom = line + page_size;
  }
}

void csConsoleBuffer::SetCurLine (int line)
{
  // Make sure we don't go past the end of the buffer
  if (line > len)
    line = len;
  else
    current_line = line;
}

void csConsoleBuffer::DeleteLine(int line)
{
  int pos;

  // Mix for disaster?
  if (line <= display_bottom)
    pos = display_top + line;
  else
    pos = line;

  // Delete the given line
  delete buffer [pos];
  buffer [pos] = NULL;

  // Set the dirty flag
  dirty [pos] = true;
}
