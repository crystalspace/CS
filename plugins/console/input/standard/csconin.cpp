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
#include "csutil/csuctransform.h"
#include "csutil/util.h"
#include "csutil/objreg.h"
#include "ivaria/conout.h"
#include "ivaria/reporter.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "csutil/event.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csConsoleInput)
  SCF_IMPLEMENTS_INTERFACE (iConsoleInput)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iConsoleWatcher)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csConsoleInput::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csConsoleInput::eiConsoleWatcher)
  SCF_IMPLEMENTS_INTERFACE (iConsoleWatcher)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csConsoleInput)


csConsoleInput::csConsoleInput (iBase *iParent) : History (16, 16)
{
  SCF_CONSTRUCT_IBASE (iParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiConsoleWatcher);
  Console = 0;
  Prompt = 0;
  strCursorPos = 0;
  vCursorPos = 0;
  Prompt = csStrNew ("# ");
  PromptLen = strlen(Prompt);
  HistoryPos = 0;
  History.Push ("");
  line.Replace ("");
  InsertMode = true;
  MaxLines = 50;
}

csConsoleInput::~csConsoleInput ()
{
  delete [] Prompt;

  if (Console)
  {
    Console->RegisterWatcher (0);
    Console->DecRef ();
  }

  SCF_DESTRUCT_EMBEDDED_IBASE(scfiConsoleWatcher);
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csConsoleInput::Initialize (iObjectRegistry *object_reg)
{
  // It is not necessary to call iEventQueue::RegisterListener() since
  // application will usually pass events to us directly.

  csRef<iKeyboardDriver> currentKbd = 
    CS_QUERY_REGISTRY (object_reg, iKeyboardDriver);
  if (currentKbd == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.console.input.standard", "No iKeyboardDriver!");
    return false;
  }
  keyLogicator = currentKbd->CreateKeyComposer ();

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
    case csevKeyboard:
      if (csKeyEventHelper::GetEventType (&Event) == csKeyEventTypeDown)
      {
	utf32_char codeCooked = csKeyEventHelper::GetCookedCode (&Event);
	switch (codeCooked)
	{
	  case CSKEY_UP:
	    {
	      if (HistoryPos > 0)
		HistoryPos--;
	      else
		HistoryPos = History.Length () - 1;
	      line.Replace (History.Get (HistoryPos));

	      // Set cursor to end of line
	      strCursorPos = 0;
	      vCursorPos = 0;
	      size_t slen = line.Length ();
	      const char* ch = line.GetData ();
	      while (*ch)
	      {
		size_t skip = csUnicodeTransform::UTF8Skip ((utf8_char*)ch, 
		  slen - strCursorPos);
		strCursorPos += skip;
		vCursorPos++;
		ch += skip;
	      }
	    }
	    break;
	  case CSKEY_DOWN:
	    {
	      if (HistoryPos < History.Length () - 1)
		HistoryPos++;
	      else
		HistoryPos = 0;
	      line.Replace (History.Get (HistoryPos));

	      // Set cursor to end of line
	      strCursorPos = 0;
	      vCursorPos = 0;
	      size_t slen = line.Length ();
	      const char* ch = line.GetData ();
	      while (*ch)
	      {
		size_t skip = csUnicodeTransform::UTF8Skip ((utf8_char*)ch, 
		  slen - strCursorPos);
		strCursorPos += skip;
		vCursorPos++;
		ch += skip;
	      }
	    }
	    break;
	  case CSKEY_LEFT:
	    if ((strCursorPos > 0) && (vCursorPos > 0))
	    {
	      vCursorPos--;
	      strCursorPos -= csUnicodeTransform::UTF8Rewind (
		(utf8_char*)line.GetData () + strCursorPos, strCursorPos);
	    }
	    break;
	  case CSKEY_RIGHT:
	    {
	      size_t slen = line.Length ();
	      if (strCursorPos < slen)
	      {
		vCursorPos++;
		strCursorPos += csUnicodeTransform::UTF8Skip (
		  (utf8_char*)line.GetData () + strCursorPos, 
		  slen - strCursorPos);
	      }
	    }
	    break;
	  case CSKEY_HOME:
	    strCursorPos = 0;
	    vCursorPos = 0;
	    break;
	  case CSKEY_END:
	    {
	      // Set cursor to end of line
	      strCursorPos = 0;
	      vCursorPos = 0;
	      const char* ch = line.GetData ();
	      size_t slen = line.Length ();
	      while (*ch)
	      {
		size_t skip = csUnicodeTransform::UTF8Skip ((utf8_char*)ch, 
		  slen - strCursorPos);
		strCursorPos += skip;
		vCursorPos++;
		ch += skip;
	      }
	    }
	    break;
	  case CSKEY_ESC:
	    line.Replace ("");
	    strCursorPos = 0;
	    vCursorPos = 0;
	    break;
	  case CSKEY_INS:
	    InsertMode = !InsertMode;
	    break;
	  case CSKEY_ENTER:
	    Console->PutText("\n");
	    if (Callback)
	      Callback->Execute (line);
	    if (line.Length () > 0)
	    {
	      HistoryPos = History.Push (csStrNew (line)) + 1;
	      while (History.Length () > MaxLines)
		History.DeleteIndex (0);
  	      if(HistoryPos >= History.Length ())
    	        HistoryPos = History.Length ();
	    }
	    line.Replace ("");
	    strCursorPos = 0;
	    vCursorPos = 0;
	    break;
	  case CSKEY_BACKSPACE:
	    if ((vCursorPos > 0) && (strCursorPos > 0))
	    {
	      vCursorPos--;
	      size_t cs = csUnicodeTransform::UTF8Rewind (
		(utf8_char*)line.GetData () + strCursorPos, 
		strCursorPos);

	      line.DeleteAt (strCursorPos - cs, cs);
	      strCursorPos -= cs;
	    }
	    break;
	  case CSKEY_DEL:
	    {
	      size_t sl = line.Length ();
	      size_t cs = csUnicodeTransform::UTF8Skip (
		(utf8_char*)line.GetData () + strCursorPos, 
		sl - strCursorPos);
	      if (strCursorPos < sl)
		line.DeleteAt (strCursorPos, cs);
	    }
	    break;
	  case CSKEY_PGUP:
	    if (Console)
	    {
	      csKeyModifiers m;
	      csKeyEventHelper::GetModifiers (&Event, m);
	      Console->ScrollTo ((m.modifiers[csKeyModifierTypeCtrl] != 0) ?
		csConVeryTop : csConPageUp, true);
	    }
	    break;
	  case CSKEY_PGDN:
	    if (Console)
	    {
	      csKeyModifiers m;
	      csKeyEventHelper::GetModifiers (&Event, m);
	      Console->ScrollTo ((m.modifiers[csKeyModifierTypeCtrl] != 0) ?
		csConVeryBottom : csConPageDown, true);
	    }
	    break;
	  default:
	    if ((codeCooked >= ' ') && !CSKEY_IS_SPECIAL(codeCooked))
	    {
	      utf32_char newChars[3];
	      int newCharCount;

	      csKeyEventData eventData;
	      csKeyEventHelper::GetEventData (&Event, eventData);
	      if (keyLogicator->HandleKey (eventData, newChars,
		sizeof (newChars) / sizeof (utf32_char), &newCharCount) ==
		csComposeNoChar)
		break;

	      if (newCharCount > 0)
	      {
		newChars[newCharCount] = 0;

		size_t sl = line.Length ();
		utf8_char ch[CS_UC_MAX_UTF8_ENCODED*2 + 1];

		size_t chSize = csUnicodeTransform::UTF32to8 (ch, 
		  sizeof (ch) / sizeof (utf8_char), newChars) - 1;
		  // Subtract 1 as the 0 terminator is included

		if ((!InsertMode) && (strCursorPos < sl))
		  line.DeleteAt (strCursorPos, csUnicodeTransform::UTF8Skip (
		    (utf8_char*)line.GetData () + strCursorPos, 
		    sl - strCursorPos));
		line.Insert (strCursorPos, (char*)ch);
		strCursorPos += chSize;
		vCursorPos += newCharCount;
	      }
	    }
	    break;
	}

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
    Console->RegisterWatcher (0);
  }
  Console = iCon;
  if (Console)
  {
    Console->IncRef ();
    Console->RegisterWatcher (&scfiConsoleWatcher);
  }
  line.Replace ("");
  Refresh ();
}

const char *csConsoleInput::GetText (int iLine) const
{
  return ((iLine >= -1) && ((size_t)(iLine + 1) <= History.Length ())) ?
         History.Get (iLine == -1 ? History.Length () - 1 : iLine) : 0;
}

int csConsoleInput::GetCurLine () const
{
  return (int)History.Length () - 1;
}

void csConsoleInput::SetBufferSize (int iSize)
{
  MaxLines = (iSize >= 0) ? iSize : 0;
  while (History.Length () > MaxLines)
    History.DeleteIndex (0);
  if(HistoryPos >= History.Length ())
    HistoryPos = History.Length () - 1;
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
  Console->PutText ("\r");
  Console->PutText ("%s", Prompt);
  Console->PutText ("%s", line.GetData ());
  Console->SetCursorPos ((int)(PromptLen + vCursorPos));
  if (InsertMode)
    Console->SetCursorStyle (csConInsertCursor);
  else
    Console->SetCursorStyle (csConNormalCursor);
}
