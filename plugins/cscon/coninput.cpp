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
#include "isystem.h"
#include "iconsole.h"
#include "iconinp.h"
#include "csutil/scf.h"
#include "csutil/csstring.h"
#include "csinput/csevent.h"
#include "coninput.h"
#include "conbuffr.h"

csConsoleInput::csConsoleInput(iBase *base)
{
  CONSTRUCT_IBASE(base);
  buffer = new csConsoleBuffer(4096, 4096);
  piConsole = NULL;
  cursor = 0;
}

csConsoleInput::~csConsoleInput()
{
  //@@@ This is disabled due to a bug in some implementations of iSystem
  //if(piSystem)
  //piSystem->DecRef();
  if(piConsole)
    piConsole->DecRef();
  delete buffer;
}

bool csConsoleInput::Initialize(iSystem *system)
{
  piSystem = system;
  return true;
}

bool csConsoleInput::HandleEvent(csEvent &event)
{

  if(event.Type==csevKeyDown) {
    csString *line;
    int cx, cy;
    bool dummy;

    switch(event.Key.Code) {
    case CSKEY_ESC:
      // The ignored keys
      break;
    case CSKEY_LEFT:
	if(cursor>0) {
	  cursor--;
	  piConsole->GetCursorPos(cx, cy);
	  piConsole->SetCursorPos(cx-1, cy);
	}
	break;
    case CSKEY_RIGHT:
      if(cursor<buffer->GetLine(buffer->GetCurLine(), dummy)->Length()) {
	cursor++;
	piConsole->GetCursorPos(cx, cy);
	piConsole->SetCursorPos(cx+1, cy);
      }
    case CSKEY_UP:
      //@@@ How do I handle this
      break;
    case CSKEY_DOWN:
      //@@@ How do I handle this
      break;
    default:
      if(event.Key.Code < CSKEY_FIRST) {

	bool echo = true;

	// Handle special cases
	switch(event.Key.Code) {
	case CSKEY_ENTER:
	  // New line
	  NewLine();
	  break;
	case CSKEY_BACKSPACE:
	  line = buffer->WriteLine();
	  // Delete the last character in the current line
	  if(cursor>1)
	    line->DeleteAt(cursor-1);
	  else if (cursor==1)
	    buffer->DeleteLine(buffer->GetCurLine());
	  else if (cursor==0) {
#if 0
	    if(buffer->GetCurLine()>0) {
	      buffer->SetCurLine(buffer->GetCurLine()-1);
	    }
#else
	    echo = false;
#endif
	    // This gets decremented to zero below
	    cursor = 1;
	  }
	  // Move the cursor back by one
	  cursor--;
	  break;
	default:
	  // Append the character to the current line
	  line = buffer->WriteLine();
	  if(cursor==line->Length())
	    line->Append((char) event.Key.Code);
#ifdef DEBUG
	  else if(cursor>line->Length())
	    piSystem->Print(MSG_FATAL_ERROR, "csConsoleInput:  Cursor past end of line!\n");
#endif
	  else
	    line->Insert(cursor, (char) event.Key.Code);
	  // Increment cursor position
	  cursor++;
	  break;
	}
	if(piConsole&&echo) {
	  csString put((char) event.Key.Code);
	  piConsole->PutText(put.GetData());
	}
      }
    }
  }
#ifdef DEBUG
  else {
    piSystem->Print(MSG_WARNING, "csConsoleInput:  Received an unknown event!\n");
  }
#endif // DEBUG
  
  // Just in case the application adds us into the input loop
  return false;

}

const csString *csConsoleInput::GetInput(int line) const
{
  bool dummy;

  if(line<0)
    return buffer->GetLine(buffer->GetCurLine(), dummy);
  else
    return buffer->GetLine(line, dummy);
}

int csConsoleInput::GetCurLine() const
{
  return buffer->GetCurLine();
}

void csConsoleInput::NewLine()
{
  buffer->NewLine();
  cursor = 0;
}

int csConsoleInput::GetBufferSize() const
{
  return buffer->GetLength();
}

void csConsoleInput::SetBufferSize(int size)
{
  /* Make sure the page size is the same as the buffer length so
   * the csConsoleBuffer is always trying to keep the display space
   * update to date.
   */
  buffer->SetLength(size);
  buffer->SetPageSize(size);
}

void csConsoleInput::Clear()
{
  buffer->Clear();
  cursor = 0;
}

bool csConsoleInput::GetEcho() const
{
  return piConsole!=NULL;
}

void csConsoleInput::SetEcho(bool echo, iConsole *console)
{
  if(echo) {
    if(console) {
      // if piConsole isn't NULL, we release it
      if(piConsole)
	piConsole->DecRef();
      piConsole = console;
      piConsole->IncRef();
    } else {
      // if piConsole isn't NULL, we just use what we've got
      if(piConsole==NULL)
	piConsole = QUERY_PLUGIN(piSystem, iConsole);
    }
  } else {
    // if piConsole isn't already NULL, we release it
    if(piConsole!=NULL) {
      piConsole->DecRef();
      piConsole = NULL;
    }
  }
}

IMPLEMENT_IBASE(csConsoleInput)
  IMPLEMENTS_INTERFACE(iConsoleInput)
  IMPLEMENTS_INTERFACE(iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY(csConsoleInput)
