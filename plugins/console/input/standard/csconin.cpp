/*
    Simple Console input
    Copyright (C) 1998-2000 by Jorrit Tyberghein

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
#include "csconin.h"
#include "csutil/util.h"
#include "ivaria/conout.h"
#include "isys/system.h"
#include "isys/event.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csConsoleInput)
  SCF_IMPLEMENTS_INTERFACE (iConsoleInput)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iConsoleWatcher)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csConsoleInput::eiPlugin)
  SCF_IMPLEMENTS_INTERFACE (iPlugin)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csConsoleInput::eiConsoleWatcher)
  SCF_IMPLEMENTS_INTERFACE (iConsoleWatcher)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csConsoleInput)

SCF_EXPORT_CLASS_TABLE (csconin)
  SCF_EXPORT_CLASS (csConsoleInput, "crystalspace.console.input.standard",
    "Crystal Space standard input console")
SCF_EXPORT_CLASS_TABLE_END

csConsoleInput::csConsoleInput (iBase *iParent) : History (16, 16)
{
  SCF_CONSTRUCT_IBASE (iParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiPlugin);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiConsoleWatcher);
  Callback = NULL;
  Console = NULL;
  Prompt = NULL;
  CursorPos = 0;
  Prompt = csStrNew ("# ");
  PromptLen = strlen(Prompt);
  HistoryPos = 0;
  History.Push (csStrNew (""));
  line = new char [linemax = 80];
  InsertMode = true;
  MaxLines = 50;
}

csConsoleInput::~csConsoleInput ()
{
  delete [] Prompt;

  if (Console)
  {
    Console->RegisterWatcher (NULL);
    Console->DecRef ();
  }
  if (Callback) Callback->DecRef ();
}

bool csConsoleInput::Initialize (iSystem *iSys)
{
  // It is not necessary to call System->CallOnEvents since application
  // will usually pass events to us directly.
  (void)iSys;
  return true;
}

void csConsoleInput::eiConsoleWatcher::ConsoleVisibilityChanged(
  iConsoleOutput*, bool visible)
{
  if (visible)
    scfParent->Refresh();
}

bool csConsoleInput::HandleEvent (iEvent &Event)
{
  switch (Event.Type)
  {
    case csevKeyDown:
      switch (Event.Key.Code)
      {
        case CSKEY_UP:
          if (HistoryPos > 0)
	    HistoryPos--;
	  else
	    HistoryPos = History.Length () - 1;
          strcpy (line, History.Get (HistoryPos));
          CursorPos = strlen (line);
          break;
        case CSKEY_DOWN:
          if (HistoryPos < History.Length () - 1)
	    HistoryPos++;
	  else
	    HistoryPos = 0;
          strcpy (line, History.Get (HistoryPos));
          CursorPos = strlen (line);
          break;
        case CSKEY_LEFT:
          if (CursorPos > 0) CursorPos--;
          break;
        case CSKEY_RIGHT:
          if (CursorPos < (int)strlen (line)) CursorPos++;
          break;
        case CSKEY_HOME:
          CursorPos = 0;
          break;
        case CSKEY_END:
          CursorPos = strlen (line);
          break;
        case CSKEY_ESC:
          line [CursorPos = 0] = '\0';
          break;
        case CSKEY_INS:
          InsertMode = !InsertMode;
          break;
        case CSKEY_ENTER:
	  Console->PutText(CS_MSG_CONSOLE, "\n");
          if (Callback)
            Callback->Execute (line);
          if (line [0])
          {
            HistoryPos = History.Push (csStrNew (line)) + 1;
            while (History.Length () > MaxLines)
              History.Delete (0);
          }
          line [CursorPos = 0] = '\0';
          break;
        case CSKEY_BACKSPACE:
          if (CursorPos)
          {
            CursorPos--;
            int sl = strlen (line);
	    if (CursorPos + 1 == sl)
	      line[CursorPos] = '\0';
	    else
              memmove(line + CursorPos, line + CursorPos + 1,
	        sl - CursorPos + 1);
          }
          break;
        case CSKEY_DEL:
          {
            int sl = strlen (line);
	    if (CursorPos + 1 == sl)
	      line[CursorPos] = '\0';
	    else if (CursorPos < sl)
              memmove(
	        line + CursorPos, line + CursorPos + 1, sl - CursorPos + 1);
          }
          break;
        case CSKEY_PGUP:
          if (Console)
            Console->ScrollTo ((Event.Key.Modifiers & CSMASK_CTRL) ?
              csConVeryTop : csConPageUp, true);
          break;
        case CSKEY_PGDN:
          if (Console)
            Console->ScrollTo ((Event.Key.Modifiers & CSMASK_CTRL) ?
              csConVeryBottom : csConPageDown, true);
          break;
        default:
          if ((Event.Key.Char >= ' ') && (CursorPos < linemax))
          {
            int sl = strlen (line);
            if (InsertMode && CursorPos < sl)
              memmove(line + CursorPos + 1, line + CursorPos,
	        sl - CursorPos + 1);
            bool needeol = (line [CursorPos] == '\0');
            line [CursorPos++] = Event.Key.Char;
            if (needeol) line [CursorPos] = '\0';
          }
          break;
      }
      Refresh ();
      return true;
  }
  return false;
}

void csConsoleInput::Bind (iConsoleOutput *iCon)
{
  if (Console)
  {
    Console->DecRef ();
    Console->RegisterWatcher (NULL);
  }
  Console = iCon;
  if (Console)
  {
    Console->IncRef ();
    Console->RegisterWatcher (&scfiConsoleWatcher);
  }
  delete [] line;
  linemax = Console->GetMaxLineWidth ();
  line = new char [linemax + 1];
  line [0] = 0;
  Refresh ();
}

const char *csConsoleInput::GetText (int iLine) const
{
  return ((iLine >= -1) && (iLine < History.Length ())) ?
         History.Get (iLine == -1 ? History.Length () - 1 : iLine) : NULL;
}

int csConsoleInput::GetCurLine () const
{
  return History.Length () - 1;
}

void csConsoleInput::SetBufferSize (int iSize)
{
  MaxLines = (iSize >= 0) ? iSize : 0;
  while (History.Length () > MaxLines)
    History.Delete (0);
}

void csConsoleInput::Clear ()
{
  History.DeleteAll ();
  Refresh ();
}

void csConsoleInput::SetPrompt (const char *iPrompt)
{
  delete [] Prompt;
  Prompt = csStrNew (iPrompt);
  PromptLen = strlen (Prompt);
  Refresh ();
}

void csConsoleInput::Refresh ()
{
  if (!Console || !Console->GetVisible ()) return;
  Console->PutText (CS_MSG_CONSOLE, "\r");
  Console->PutText (CS_MSG_CONSOLE, Prompt);
  Console->PutText (CS_MSG_CONSOLE, line);
  Console->SetCursorPos (PromptLen + CursorPos);
  if (InsertMode)
    Console->SetCursorStyle (csConInsertCursor);
  else
    Console->SetCursorStyle (csConNormalCursor);
}
