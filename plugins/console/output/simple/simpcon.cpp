/*
    Simple Console
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
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "csutil/sysfunc.h"
#include "csutil/csuctransform.h"
#include "simpcon.h"
#include "csutil/util.h"
#include "csgeom/csrect.h"
#include "csutil/cfgacc.h"
#include "csutil/csevent.h"
#include "csutil/event.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"
#include "ivaria/reporter.h"
#include "iutil/plugin.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/evdefs.h"
#include "iutil/eventq.h"
#include "iutil/cfgmgr.h"
#include "iutil/objreg.h"

#define SIZE_LINE 256

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csSimpleConsole)
  SCF_IMPLEMENTS_INTERFACE (iConsoleOutput)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSimpleConsole::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csSimpleConsole::EventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_FACTORY (csSimpleConsole)


csSimpleConsole::csSimpleConsole (iBase *iParent)
{
  SCF_CONSTRUCT_IBASE (iParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  scfiEventHandler = 0;
  LineMessage = 0;
  Line = 0;
  LinesChanged = 0;
  CursorStyle = csConNoCursor;
  Update = true;
  SystemReady = false;
  object_reg = 0;
  CursorPos = -1;
  ClearInput = false;
  Client = 0;
  ConsoleMode = CONSOLE_MODE;
  CursorState = false;
  InvalidAll = true;
  mutex = csMutex::Create (true);
  putTextLevel = 0;
}

csSimpleConsole::~csSimpleConsole ()
{
  if (scfiEventHandler)
  {
    csRef<iEventQueue> q (CS_QUERY_REGISTRY(object_reg, iEventQueue));
    if (q != 0)
      q->RemoveListener (scfiEventHandler);
    scfiEventHandler->DecRef ();
  }
  FreeLineMessage ();
  FreeBuffer ();

  // @@@@@@@@@@@@@@@@@@@@@@@@@ NEEDS DEBUGGING
  // If console_font = 0 is moved AFTER the other two assignments
  // then there will be a crash.
  console_font = 0;
  G3D = 0;
  G2D = 0;

  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csSimpleConsole::Initialize (iObjectRegistry *object_reg)
{
  csSimpleConsole::object_reg = object_reg;

  G3D = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  if (G3D == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.console.output.simple",
	"Console: Unable to locate 3D renderer plugin!");
    return false;
  }

  G2D = G3D->GetDriver2D ();
  if (G2D == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.console.output.simple",
	"Console: Unable to locate 2D canvas plugin!");
    return false;
  }

  // @@@ Not really good. G2D may not have been opened yet
  FrameWidth = G2D->GetWidth ();
  FrameHeight = G2D->GetHeight ();

  csConfigAccess Config(object_reg, "/config/simpcon.cfg");
  console_transparent_bg = Config->GetBool ("SimpleConsole.TranspBG", false);
  console_transparent_bg = Config->GetBool ("SimpleConsole.TranspBG", 1);
  const char *buf = Config->GetStr ("SimpleConsole.ConFG", "255,255,255");
  sscanf (buf, "%d,%d,%d", &console_fg_r, &console_fg_g, &console_fg_b);
  buf = Config->GetStr ("SimpleConsole.ConBG", "0,0,0");
  sscanf (buf, "%d,%d,%d", &console_bg_r, &console_bg_g, &console_bg_b);
  buf = Config->GetStr ("SimpleConsole.ConFont", "auto");
  mingap = Config->GetInt ("SimpleConsole.MinimumLineGap", 2);

  const char *fontname;
  if (strcasecmp (buf, "auto") != 0)
    fontname = buf;
  else
  {
    // choose a font that allows at least 80 columns of text
    if (FrameWidth <= 560)
      fontname = CSFONT_SMALL;
    else if (FrameWidth <= 640)
      fontname = CSFONT_COURIER;
    else
      fontname = CSFONT_LARGE;
  }

  iFontServer *fs = G2D->GetFontServer ();
  if (!fs)
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.console.output.simple",
	"Console: Unable to locate font server plugin!");
  else
    console_font = fs->LoadFont (fontname);

  if (!console_font)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.console.output.simple",
        "Failed to load font CONFONT=%s defined in configuration file.\n"
        "Try '*large', '*courier', '*italic' or '*small'", buf);
    return false;
  }

  int fw, fh;
  console_font->GetMaxSize (fw, fh);
  int i = fh;
  if (console_font->GetDescent() < mingap)
    i += mingap - console_font->GetDescent();
  LineSize = (FrameWidth / 4) + 1;
  SetBufferSize ((FrameHeight / i) - 2);
  SetLineMessages (Config->GetInt ("SimpleConsole.LineMax", 4));

  LineTime = csGetTicks ();
  CursorTime = csGetTicks ();

  // We want to see broadcast events
  if (!scfiEventHandler)
    scfiEventHandler = new EventHandler (this);
  csRef<iEventQueue> q (CS_QUERY_REGISTRY(object_reg, iEventQueue));
  if (q != 0)
    q->RegisterListener (scfiEventHandler, CSMASK_Broadcast);

  return true;
}

void csSimpleConsole::SetTransparency (bool iTransp)
{
  console_transparent_bg = iTransp;
}

void csSimpleConsole::FreeLineMessage ()
{
  int i;

  if (LineMessage)
  {
    for (i = 0; i < LineMessageMax; i++)
      delete [] LineMessage [i];
    delete [] LineMessage;
  }

  delete [] LinesChanged;
}

void csSimpleConsole::FreeBuffer ()
{
  if (Line)
  {
    int i;
    for (i = 0; i < LineMax; i++)
      delete [] Line [i];
    delete [] Line;
  }
}

void csSimpleConsole::SetBufferSize (int iCount)
{
  csScopedMutexLock lock (mutex);

  FreeBuffer ();

  LineMax = iCount;
  if (LineMax <= 0)
    LineMax = 1;
  Line = new char * [LineMax];
  int i;
  for (i = 0; i < LineMax; i++)
  {
    Line [i] = new char [SIZE_LINE];
    Line [i][0] = '\0';
  }
  LineNumber = 0;
}

void csSimpleConsole::SetLineMessages (int iCount)
{
  FreeLineMessage ();

  LineMessageMax = iCount;
  if (LineMessageMax <= 0)
    LineMessageMax = 1;
  else if (LineMessageMax >= LineMax)
    LineMessageMax = LineMax - 1;

  // Allocate new messages.
  LineMessage = new char * [LineMessageMax];
  LinesChanged = new bool [LineMessageMax];
  int i;
  for (i = 0; i < LineMessageMax; i++)
  {
    LineMessage [i] = new char [SIZE_LINE];
    LineMessage [i][0] = '\0';
    LinesChanged[i] = true;
  }
  LineMessageNumber = 0;
}

void csSimpleConsole::PutMessage (bool advance, const char *iText)
{
  if (LineMessageNumber >= LineMessageMax)
  {
    int i;
    for (i = 1; i < LineMessageMax; i++)
    {
      strcpy (LineMessage [i - 1], LineMessage [i]);
      LinesChanged [i - 1] = true;
    }
    LineMessageNumber--;
  }

  size_t len = strlen (iText);
  len = MIN (len, SIZE_LINE-1);
  memcpy (LineMessage [LineMessageNumber], iText, len);
  LineMessage [LineMessageNumber][len] = 0;
  LinesChanged [LineMessageNumber] = true;

  LineTime = csGetTicks () + 4000;
  if (advance)
    LineMessageNumber++;
}

CS_IMPLEMENT_STATIC_VAR(GetTextBuf, csString, ())

void csSimpleConsole::PutTextV (const char *iText2, va_list args)
{
  csScopedMutexLock lock (mutex);

  putTextLevel++;

  size_t len;
  char *dst;
  const char *src;
  char c;

  if (iText2 == 0 || *iText2 == 0)
    goto Done;

  {
    csString& iText = *GetTextBuf();
    iText.FormatV (iText2, args);

    len = strlen (Line [LineNumber]);
    dst = Line [LineNumber] + len;
    src = iText;
  }
  for (c = *src; c != '\0'; c = *++src)
  {
    if (ClearInput)
    {
      CursorPos = -1;
      *(dst = Line [LineNumber]) = '\0';
      ClearInput = false;
    }

    if (c == '\r')
      ClearInput = true;
    else if (c == '\b')
    {
      if (len > 0)
      {
        dst--;
        len--;
      }
    }
    else if (c == '\n')
    {
      *dst = '\0';
      PutMessage (true, Line[LineNumber]);
      if (LineNumber + 1 < LineMax)
      {
        // Messages that are written in one shot go to the message display
        if (!len) PutMessage (false, Line [LineNumber]);
        LineNumber++;
      }
      else
      {
	int i;
        for (i = 1; i < LineMax; i++)
          strcpy (Line[i - 1], Line[i]);
      }
      dst = Line[LineNumber];
      *dst = '\0';
      len = 0;
    }
    else if (len < SIZE_LINE - 1)
    {
      *dst++ = c;
      len++;
    } /* endif */
  } /* endfor */

  // Put the ending null character
  *dst = '\0';

Done:
  if (Update && SystemReady && (putTextLevel == 1))
  {
    csRect rect;
    G2D->BeginDraw ();
    G2D->Clear (console_bg);
    Draw2D (&rect);
    G2D->FinishDraw ();
    G2D->Print (&rect);
  }
  putTextLevel--;
}

void csSimpleConsole::SetCursorPos (int iCharNo)
{
  CursorPos = iCharNo;
}

void csSimpleConsole::Clear (bool)
{
  csScopedMutexLock lock (mutex);

  LineMessageNumber = 0;
  LineNumber = 0;
  Line [LineNumber][0] = '\0';
  ClearInput = false;

  int i;
  for (i = 0; i < LineMessageMax; i++)
  {
    LineMessage [i][0] = '\0';
    LinesChanged [i] = true;
  }
}

void csSimpleConsole::Draw2D (csRect* area)
{
  csScopedMutexLock lock (mutex);

  int i;
  csTicks CurrentTime = csGetTicks ();

#define WRITE(x,y,fc,bc,s,changed)				\
  {								\
    G2D->Write (console_font, x, y, fc, bc, s);			\
    if ((changed) && area)					\
    {								\
      int tw, th;						\
      console_font->GetDimensions (s, tw, th);			\
      area->Union (x, y, x + tw, y + th + 2);			\
    }								\
  }

#define WRITE2(x,y,fc,bc,s,changed)				\
  {								\
    G2D->Write (console_font, x + 1, y + 1, bc, -1, s);		\
    G2D->Write (console_font, x, y, fc, -1, s);			\
    if ((changed) && area)					\
    {								\
      int tw, th;						\
      console_font->GetDimensions (s, tw, th);			\
      area->Union (x, y, x + 1 + tw, y + 1 + th + 2);		\
    }								\
  }

  if (area && InvalidAll)
    area->Set (0, 0, FrameWidth, FrameHeight);

  int tw, th;
  console_font->GetMaxSize (tw, th);
  if (console_font->GetDescent () < mingap)
    th += mingap - console_font->GetDescent ();

  bool dblbuff = G2D->GetDoubleBufferState ();

  switch (ConsoleMode)
  {
    case MESSAGE_MODE:
    {
      if (CurrentTime > LineTime)
      {
        // Scroll all lines up once per four seconds
        for (i = 1; i < LineMessageMax; i++)
        {
          strcpy (LineMessage [i - 1], LineMessage [i]);
          LinesChanged [i - 1] = true;
        }
        if (LineMessageNumber > 0)
          LineMessageNumber--;
        LineMessage [LineMessageMax - 1][0] = '\0';
        LinesChanged [LineMessageMax - 1] = true;
        LineTime = csGetTicks () + 4000;
      }
      for (i = 0; i < LineMessageMax; i++)
      {
        WRITE2 (10, 10 + th * i, console_fg, console_bg, LineMessage [i],
	  dblbuff || LinesChanged [i]);
	LinesChanged [i] = false;
      }
      break;
    }
    case CONSOLE_MODE:
    {
      if (CurrentTime > CursorTime)
      {
        CursorState = !CursorState;
        CursorTime = csGetTicks () + 333;
      }

      utf32_char cursorCh;
      if (CursorState && (CursorStyle != csConNoCursor))
        cursorCh = (CursorStyle == csConNormalCursor) ? 0x2588 : '_';
      else
        cursorCh = ' ';

      utf8_char cursor [CS_UC_MAX_UTF8_ENCODED + 1];
      size_t cursorSize = csUnicodeTransform::EncodeUTF8 (cursorCh, cursor,
	sizeof (cursor) / sizeof (utf8_char));
      cursor[cursorSize] = 0;

      char *tmp = csStrNew (Line [LineNumber]);
      size_t curPosX = strlen (tmp);
      if ((CursorPos >= 0) && ((size_t)CursorPos < curPosX))
      {
	size_t tl = curPosX;
	size_t cp = CursorPos;
	size_t tp = 0;
	while (cp > 0)
	{
	  int skip = csUnicodeTransform::UTF8Skip ((utf8_char*)&tmp[tp],
	    tl - tp);
	  tp += skip;
	  cp--;
	}
	tmp[tp] = 0;
      }

      //if ((CursorPos >= 0) && (CursorPos < curx))
      //  tmp [CursorPos] = 0;
      int temp_h, curx;
      console_font->GetDimensions (tmp, curx, temp_h);
      delete [] tmp;

      if (console_transparent_bg)
      {
        for (i = 0; i <= LineNumber; i++)
          WRITE2(1, th * i, console_fg, console_bg, Line [i], dblbuff);
        WRITE2(1 + curx, th * LineNumber, console_fg, -1, (char*)cursor,
	  dblbuff);
      }
      else
      {
        G2D->Clear (console_bg);
        if (dblbuff && area)
          area->Union (0, 0, FrameWidth - 1, FrameHeight - 1);
        for (i = 0; i <= LineNumber; i++)
          WRITE(1, th * i, console_fg, -1, Line [i], false);
        WRITE(1 + curx, th * LineNumber, console_fg, -1, (char*)cursor, false);
      }
      break;
    }
  }
}

void csSimpleConsole::CacheColors ()
{
  console_fg = G2D->FindRGB (console_fg_r, console_fg_g, console_fg_b);
  console_bg = G2D->FindRGB (console_bg_r, console_bg_g, console_bg_b);
}

void csSimpleConsole::GfxWrite (int x, int y, int fg, int bg, char *iText, ...)
{
  va_list arg;
  csString buf;

  va_start (arg, iText);
  buf.FormatV (iText, arg);
  va_end (arg);

  G2D->Write (console_font, x, y, fg, bg, buf);
}

void csSimpleConsole::SetVisible (bool iShow)
{
  ConsoleMode = iShow ? CONSOLE_MODE : MESSAGE_MODE;
  if (Client)
    Client->ConsoleVisibilityChanged(this, iShow);
  InvalidAll = true;
}

const char *csSimpleConsole::GetLine (int iLine) const
{
  csScopedMutexLock lock (mutex);
  return Line [iLine < 0 ? LineNumber : iLine];
}

bool csSimpleConsole::HandleEvent (iEvent &Event)
{
  switch (Event.Type)
  {
    case csevBroadcast:
      switch (csCommandEventHelper::GetCode(&Event))
      {
        case cscmdSystemOpen:
          SystemReady = true;
          CacheColors ();
          return true;
        case cscmdSystemClose:
          SystemReady = false;
          return true;
      }
      break;
  }
  return false;
}
