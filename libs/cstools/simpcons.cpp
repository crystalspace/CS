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

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "sysdef.h"
#include "cssys/system.h"
#include "cssys/sysdriv.h"
#include "cstools/simpcons.h"
#include "csinput/csinput.h"
#include "csutil/csrect.h"
#include "csutil/inifile.h"
#include "igraph2d.h"
#include "itxtmgr.h"

#define SIZE_LINE	256
#define SIZE_HISTORY	32

#define Gfx2D System->G2D

IMPLEMENT_IBASE(csSimpleConsole)
  IMPLEMENTS_INTERFACE(iConsole)
IMPLEMENT_IBASE_END

void GfxWrite (int x, int y, int fg, int bg, char *str, ...)
{
  va_list arg;
  char buf[256];

  va_start (arg, str);
  vsprintf (buf, str, arg);
  va_end (arg);

  Gfx2D->Write (x, y, fg, bg, buf);
}

csSimpleConsole::csSimpleConsole(iBase *base)
{
  CONSTRUCT_IBASE(base);
}

csSimpleConsole::csSimpleConsole (csIniFile *iConfig, csSimpleCommand* pc) : command_handler(pc)
{
  CONSTRUCT_IBASE(NULL);
  config = iConfig;
  LineMessageMax = config->GetInt ("SimpleConsole", "LINEMAX", 4);
  HistoryMax = config->GetInt ("SimpleConsole", "LINEHISTORY", SIZE_HISTORY);
  console_transparent_bg = config->GetYesNo ("SimpleConsole", "TRANSPBG", 1);
  char *buf = config->GetStr ("SimpleConsole", "CONFG", "255,255,255");
  sscanf (buf, "%d,%d,%d", &console_fg_r, &console_fg_g, &console_fg_b);
  buf = config->GetStr ("SimpleConsole", "CONBG", "0,0,0");
  sscanf (buf, "%d,%d,%d", &console_bg_r, &console_bg_g, &console_bg_b);
  buf = config->GetStr ("SimpleConsole", "CONFONT", "auto");
  if (!strcasecmp (buf, "auto"))
  {
    // choose a font that allows at least 80 columns of text
    if (System->FrameWidth <= 560)
      console_font = csFontTiny;
    else if (System->FrameWidth <= 640)
      console_font = csFontCourier;
    else
      console_font = csFontPolice;
  }
  else if (!strcasecmp (buf, "tiny"))
    console_font = csFontTiny;
  else if (!strcasecmp (buf, "courier"))
    console_font = csFontCourier;
  else if (!strcasecmp (buf, "police"))
    console_font = csFontPolice;
  else
  {
    System->Printf (MSG_FATAL_ERROR, "Bad value for CONFONT in configuration "
      "file.\nUse 'auto', 'tiny', 'courier', or 'police'\n");
    fatal_exit (0, false);
  }

  int i = System->G2D->GetTextHeight (console_font);
  i += 2;
  LineMax = (System->FrameHeight / i) - 2;
  LineSize = (System->FrameWidth / 4) + 1;

  if (LineMessageMax <= 0)
    LineMessageMax = 1;
  else if (LineMessageMax >= LineMax)
    LineMessageMax = LineMax-1;

  CHK (LineMessage = new char * [LineMessageMax]);
  CHK (LinesChanged = new bool [LineMessageMax]);
  for (i = 0; i < LineMessageMax; i++)
  {
    CHK (LineMessage [i] = new char [SIZE_LINE]);
    LineMessage [i][0] = '\0';
    LinesChanged [i] = true;
  }
  LineMessageNumber = 0;

  CHK (Line = new char * [LineMax]);
  for (i = 0; i < LineMax; i++)
  {
    CHK (Line [i] = new char [SIZE_LINE]);
    Line [i][0] = '\0';
  }
  LineNumber = 0;

  CHK (LineCommand = new char [SIZE_LINE]);
  LineCommand [0] = '\0';
  LineCommandMax = SIZE_LINE - 1;
  LineCommandCount = 0;

  CHK (History = new char * [HistoryMax]);
  for (i = 0; i < HistoryMax; i++)
  {
    CHK (History [i] = new char[SIZE_LINE]);
    History [i][0] = '\0';
  }
  HistoryCount = 0;
  HistoryCurrent = 0;

  LineTime = System->Time ();
  ConsoleMode = MESSAGE_MODE;
  CursorState = false;
  CursorTime = System->Time ();
}

csSimpleConsole::~csSimpleConsole ()
{
  int i;

  for (i = 0; i < LineMessageMax; i++)
    CHKB (delete [] LineMessage [i]);
  CHK (delete [] LineMessage);

  if (Line)
  {
    for (i = 0; i < LineMax; i++)
      CHKB (delete [] Line [i]);
    CHK (delete [] Line);
  }

  if (History)
  {
    for (i = 0; i < HistoryMax; i++)
      CHKB (delete [] History [i]);
    CHK (delete [] History);
  }

  CHK (delete [] LineCommand);
  CHK (delete [] LinesChanged);
}

void csSimpleConsole::SetTransparent (int t)
{
  if (t == -1)
    console_transparent_bg = config->GetYesNo ("SimpleConsole", "TRANSPBG", 1);
  else
    console_transparent_bg = t;
}

void csSimpleConsole::SetMaxLines (int ml)
{
  int i;

  // First remove the old messages
  for (i = 0; i < LineMessageMax; i++)
    CHKB (delete [] LineMessage[i]);
  CHK (delete [] LineMessage);

  CHK (delete [] LinesChanged);

  if (ml == -1)
    LineMessageMax = config->GetInt ("SimpleConsole", "LINEMAX", 4);
  else
    LineMessageMax = ml;
  if (LineMessageMax >= LineMax)
    LineMessageMax = LineMax-1;

  // Allocate new messages.
  CHK (LineMessage = new char * [LineMessageMax]);
  CHK (LinesChanged = new bool [LineMessageMax]);
  for (i = 0; i < LineMessageMax; i++)
  {
    CHK (LineMessage [i] = new char [SIZE_LINE]);
    LineMessage [i][0] = '\0';
    LinesChanged[i] = true;
  }
  LineMessageNumber = 0;
}

void csSimpleConsole::Show ()
{
  ConsoleMode = CONSOLE_MODE;
}

void csSimpleConsole::Hide ()
{
  Gfx2D->ClearAll (0);
  ConsoleMode = MESSAGE_MODE;
}

void csSimpleConsole::PutMessage (bool advance, const char *str)
{
  if (LineMessageNumber >= LineMessageMax)
  {
    for (int i = 1; i < LineMessageMax; i++)
    {
      strcpy (LineMessage [i - 1], LineMessage [i]);
      LinesChanged [i - 1] = true;
    }
    LineMessageNumber--;
  }

  strncpy (LineMessage [LineMessageNumber], str, SIZE_LINE - 1);
  LinesChanged [LineMessageNumber] = true;

  LineTime = System->Time () + 4000;
  if (advance)
    LineMessageNumber++;
}

void csSimpleConsole::PutText (const char *str)
{
  if (str == 0 || *str == 0)
    return;

  int len = strlen (Line[LineNumber]);
  char* dst = Line[LineNumber] + len;
  char const* src = str;
  for (char c = *src; c != '\0'; c = *++src)
  {
    if (c == '\b')
    {
      if (len != 0)
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
        LineNumber++;
      else
      {
        for (int i = 1; i < LineMax; i++)
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

  if (len != 0)
  {
    *dst = '\0';
    PutMessage (false, Line[LineNumber]);
  }
}

void csSimpleConsole::ExecuteCommand (char *command)
{
  PutText ("cs# "); PutText (command); PutText ("\n");
  if (command && command[0] != '\0')
  {
    if (command_handler == 0 || !command_handler->PerformLine (command))
    { PutText ("Unknown command: "); PutText (command); PutText ("\n"); }

    if (HistoryCount > HistoryMax - 2)
    {
      for (int i = 0; i < (HistoryMax - 2); i++)
        strcpy (History[i], History[i + 1]);
      strncpy (History[HistoryMax - 2], command, SIZE_LINE - 1);
    } else
    {
      strncpy (History[HistoryCount], command, SIZE_LINE - 1);
      HistoryCount++;
    }
  }
}

void csSimpleConsole::Clear (bool)
{
  LineCommandCount = 0;
  LineMessageNumber = 0;
  LineNumber = 0;
  Line [LineNumber][0] = '\0';

  for (int i = 0; i < LineMessageMax; i++)
  {
    LineMessage [i][0] = '\0';
    LinesChanged [i] = true;
  }
}

void csSimpleConsole::Print (csRect* area)
{
  int i;
  time_t CurrentTime = System->Time ();

  Gfx2D->SetFontID (console_font);

#define WRITE(x,y,fc,bc,s,changed)				\
  {								\
    Gfx2D->Write (x, y, fc, bc, s);				\
    if ((changed) && area)					\
    {								\
      int tw = System->G2D->GetTextWidth (console_font, s);	\
      area->Union (x, y, x + tw, y + th);			\
    }								\
  }

#define WRITE2(x,y,fc,bc,s,changed)				\
  {								\
    Gfx2D->Write (x + 1, y + 1, bc, -1, s);			\
    Gfx2D->Write (x, y, fc, -1, s);				\
    if ((changed) && area)					\
    {								\
      int tw = System->G2D->GetTextWidth (console_font, s);	\
      area->Union (x, y, x + 1 + tw, y + 1 + th);		\
    }								\
  }

  // text height
  int th = System->G2D->GetTextHeight (console_font);
  th += 2;

  bool dblbuff = Gfx2D->GetDoubleBufferState ();

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
        LineTime = System->Time () + 4000;
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
        CursorTime = System->Time () + 333;
      }

      char buf [256];
      sprintf (buf, CursorState ? "cs# %s_" : "cs# %s",  LineCommand);

      int const lastline = strlen (Line[LineNumber]) == 0 ?
	 LineNumber : LineNumber + 1; // Account for partial lines.

      if (console_transparent_bg)
      {
        for (i = 0; i < lastline; i++)
          WRITE2 (1, th * i, console_fg, console_bg, Line [i], dblbuff);
        WRITE2 (1, th * lastline, console_fg, console_bg, buf, dblbuff);
      }
      else
      {
        Gfx2D->Clear (console_bg);
        if (dblbuff && area)
          area->Union (0, 0, System->FrameWidth - 1, System->FrameHeight - 1);
        for (i = 0; i < lastline; i++)
          WRITE (1, th * i, console_fg, -1, Line [i], false);
        WRITE (1, th * lastline, console_fg, -1, buf, false);
      }
      break;
    }
  }
}

void csSimpleConsole::AddChar (int c)
{
  switch (c)
  {
    case CSKEY_TAB:
      Hide ();
      break;

    case CSKEY_ENTER:
      ExecuteCommand (LineCommand);
      LineCommand[0] = '\0';
      LineCommandCount = 0;
      HistoryCurrent = HistoryCount;
      break;

    case CSKEY_BACKSPACE:
      if (LineCommandCount >= 0)
        LineCommand[--LineCommandCount] = '\0';
      break;

    case CSKEY_DOWN:
      if (HistoryCurrent < HistoryCount)
        HistoryCurrent++;
      strcpy (LineCommand, History[HistoryCurrent]);
      LineCommandCount = strlen (LineCommand);
      break;

    case CSKEY_UP:
      if (HistoryCurrent > 0)
        HistoryCurrent--;
      strcpy (LineCommand, History[HistoryCurrent]);
      LineCommandCount = strlen (LineCommand);
      break;

    default:
      if (c >= ' ' && c < 256)
        if (LineCommandCount < LineCommandMax)
        {
          LineCommand[LineCommandCount++] = c;
          LineCommand[LineCommandCount] = '\0';
        }
      break;
  }
}

void csSimpleConsole::SetupColors (iTextureManager* txtmgr)
{
  console_fg = txtmgr->FindRGB (console_fg_r, console_fg_g, console_fg_b);
  console_bg = txtmgr->FindRGB (console_bg_r, console_bg_g, console_bg_b);
}
