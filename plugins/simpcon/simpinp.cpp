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
#include "csutil/util.h"
#include "simpinp.h"
#include "ivaria/iconsole.h"
#include "isys/isystem.h"
#include "isys/ievent.h"

IMPLEMENT_IBASE (csSimpleInput)
  IMPLEMENTS_INTERFACE (iPlugIn)
  IMPLEMENTS_INTERFACE (iConsoleInput)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csSimpleInput)

csSimpleInput::csSimpleInput (iBase *iParent) : History (16, 16)
{
  CONSTRUCT_IBASE (iParent);
  Callback = NULL;
  Console = NULL;
  Prompt = NULL;
  CursorPos = 0;
  Prompt = strnew ("# ");
  PromptLen = strlen(Prompt);
  HistoryPos = 0;
  History.Push (strnew (""));
  line = new char [linemax = 80];
  InsertMode = true;
  MaxLines = 50;
}

csSimpleInput::~csSimpleInput ()
{
  delete [] Prompt;
  if (Console)
  {
    Console->DecRef ();
    Console->RegisterPlugin (NULL);
  }
}

bool csSimpleInput::Initialize (iSystem *iSys)
{
  // It is not needed to call System->CallOnEvents since application
  // will usually pass events to us directly
  (void)iSys;

  return true;
}

bool csSimpleInput::HandleEvent (iEvent &Event)
{
  switch (Event.Type)
  {
    case csevBroadcast:
      if (Event.Command.Code == cscmdConsoleStatusChange)
        Refresh ();
      return true;
    case csevKeyDown:
      switch (Event.Key.Code)
      {
        case CSKEY_UP:
          if (HistoryPos > 0) HistoryPos--; else HistoryPos = History.Length () - 1;
          strcpy (line, History.Get (HistoryPos));
          CursorPos = strlen (line);
          break;
        case CSKEY_DOWN:
          if (HistoryPos < History.Length () - 1) HistoryPos++; else HistoryPos = 0;
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
          if (Callback)
            Callback (CallbackData, line);
          if (line [0])
          {
            HistoryPos = History.Push (strnew (line)) + 1;
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
            memmove (line + CursorPos, line + CursorPos + 1, sl - CursorPos + 1);
          }
          break;
        case CSKEY_DEL:
          {
            int sl = strlen (line);
            memmove (line + CursorPos, line + CursorPos + 1, sl - CursorPos + 1);
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
              memmove (line + CursorPos + 1, line + CursorPos, sl - CursorPos + 1);
            bool needeol = (line [CursorPos] == 0);
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

void csSimpleInput::Bind (iConsole *iCon)
{
  if (Console)
  {
    Console->DecRef ();
    Console->RegisterPlugin (NULL);
  }
  Console = iCon;
  if (Console)
  {
    Console->IncRef ();
    Console->RegisterPlugin (this);
  }
  delete [] line;
  linemax = Console->GetMaxLineWidth ();
  line = new char [linemax + 1];
  line [0] = 0;
  Refresh ();
}

const char *csSimpleInput::GetText (int iLine) const
{
  return ((iLine >= -1) && (iLine < History.Length ())) ?
         History.Get (iLine == -1 ? History.Length () - 1 : iLine) : NULL;
}

int csSimpleInput::GetCurLine () const
{
  return History.Length () - 1;
}

void csSimpleInput::SetBufferSize (int iSize)
{
  MaxLines = (iSize >= 0) ? iSize : 0;
  while (History.Length () > MaxLines)
    History.Delete (0);
}

void csSimpleInput::Clear ()
{
  History.DeleteAll ();
  Refresh ();
}

void csSimpleInput::SetPrompt (const char *iPrompt)
{
  delete [] Prompt;
  Prompt = strnew (iPrompt);
  PromptLen = strlen (Prompt);
  Refresh ();
}

void csSimpleInput::Refresh ()
{
  if (!Console || !Console->GetVisible ()) return;
  Console->PutText (MSG_CONSOLE, Prompt);
  Console->PutText (MSG_CONSOLE, line);
  Console->PutText (MSG_CONSOLE, "\r");
  Console->SetCursorPos (PromptLen + CursorPos);
  if (InsertMode)
    Console->SetCursorStyle (csConInsertCursor);
  else
    Console->SetCursorStyle (csConNormalCursor);
}
