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
  history = 0;
}

csConsoleInput::~csConsoleInput()
{
  if (piSystem)
    piSystem->DecRef ();
  if (piConsole)
    piConsole->DecRef ();
  delete buffer;
}

bool csConsoleInput::Initialize(iSystem *system)
{
  (piSystem = system)->IncRef ();
  return true;
}

bool csConsoleInput::HandleEvent(csEvent &event)
{
  if(event.Type==csevKeyDown) {
    csString *line;
    int cx, cy;

    switch(event.Key.Code) {
    case CSKEY_ESC:
      // The ignored keys
      break;
    case CSKEY_LEFT:
	if(cursor>0) {
	  cursor--;
	  if(piConsole) {
	    piConsole->GetCursorPos(cx, cy); 
	    piConsole->SetCursorPos(cx-1, cy);
	  }
	}
	break;
    case CSKEY_RIGHT:
      if( buffer->GetLine(history) && cursor<buffer->GetLine(history)->Length()) {
	cursor++;
	if(piConsole) {
	  piConsole->GetCursorPos(cx, cy);
	  piConsole->SetCursorPos(cx+1, cy);
	}
      }
      break;
    case CSKEY_UP:
      {
	//	printf("Cursor")
	int ancient_history = history;
	// If we're at the top of the list, cycle down to the bottom
	if(history==0)
	  history = buffer->GetCurLine();
	else
	  history--;
	// Update the console
	if(piConsole) {
	  const csString *consoleText = piConsole->GetText(), *bufferText = buffer->GetLine(ancient_history);
	  // Make sure neither the console line nor the buffer line is NULL
	  if(!(consoleText==NULL||bufferText==NULL)) {
	    int start = consoleText->Length() - bufferText->Length();
	    cursor -= consoleText->Length();
	    piConsole->DeleteText(start > 0 ? start : 0);
	  }
	  bufferText = buffer->GetLine(history);
	  if(bufferText&&(!bufferText->IsEmpty())) {
	    piConsole->PutText(bufferText->GetData());
	    cursor += bufferText->Length();
	  }
	}
      }
      break;
    case CSKEY_DOWN:
      {
	int ancient_history = history;
	// If we are at the bottom, cycle to the top
	if(history==0)
	  history = 0;
	else
	  history++;
	// Update the console
	if(piConsole) {
	  const csString *consoleText = piConsole->GetText(), *bufferText = buffer->GetLine(ancient_history);
	  // Make sure neither the console line nor the buffer line is NULL
	  if(!(consoleText==NULL||bufferText==NULL)) {
	    int start = consoleText->Length() - bufferText->Length();
	    cursor -= consoleText->Length();
	    piConsole->DeleteText(start > 0 ? start : 0);
	  }
	  bufferText = buffer->GetLine(history);
	  if(bufferText&&(!bufferText->IsEmpty())) {
	    piConsole->PutText(bufferText->GetData());
	    cursor += bufferText->Length();
	  }
	}
      }
      break;
    default:
      if(event.Key.Code < CSKEY_FIRST) {

	// Make sure that this isn't the current line or an unmodified newline
	if(!((history==buffer->GetCurLine())||(event.Key.Code==CSKEY_ENTER))) {
	  // Copy the line to the current line
	  buffer->DeleteLine(buffer->GetCurLine());
	  line = buffer->WriteLine();
	  if (buffer->GetLine(history))
            line->Append(*buffer->GetLine(history));
	  history = buffer->GetCurLine();
	}

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
	    echo = false;
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
#ifdef CS_DEBUG
	  else if(cursor>line->Length())
	    piSystem->Printf(MSG_FATAL_ERROR, "csConsoleInput:  Cursor past end of line! cursor at %d, line len = %d\n", cursor,line->Length());
#endif
	  else
	    line->Insert(cursor, (char) event.Key.Code);
	  // Increment cursor position
	  cursor++;
	  break;
	}
	if(piConsole&&echo) {
	  csString put((char) event.Key.Code);
//	  char p[5]; sprintf(p, "%d", event.Key.Code );
	  piConsole->PutText(put.GetData());
//	  piConsole->PutText(p);
	}
      }
    }
  }
#ifdef CS_DEBUG
  else {
    piSystem->Printf(MSG_WARNING, "csConsoleInput:  Received an unknown event!\n");
  }
#endif // CS_DEBUG
 
  // Just in case the application adds us into the input loop
  return false;

}

const csString *csConsoleInput::GetInput(int line) const
{
  if(line<0)
    return buffer->GetLine(history);
  else
    return buffer->GetLine(line);
}

int csConsoleInput::GetCurLine() const
{
  return buffer->GetCurLine();
}

void csConsoleInput::NewLine()
{
  if(!buffer->IsLineEmpty(history)) {
    buffer->NewLine();
    cursor = 0;
  }
  history = buffer->GetCurLine();
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
