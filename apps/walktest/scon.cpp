/*
    Simple console example
    Copyright (C) 1998 by Jorrit Tyberghein

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
#include "cssys/common/system.h"
#include "cssys/common/sysdriv.h"
#include "csgeom/csrect.h"
#include "walktest/walktest.h"
#include "walktest/scon.h"
#include "support/command.h"
#include "csinput/csinput.h"
#include "csutil/inifile.h"
#include "igraph2d.h"
#include "itxtmgr.h"

#define SIZE_LINE	256
#define SIZE_HISTORY	32

#define Gfx2D System->piG2D

void GfxWrite (int x, int y, int fg, int bg, char *str, ...)
{
  va_list arg;
  char buf[256];

  va_start (arg, str);
  vsprintf (buf, str, arg);
  va_end (arg);

  Gfx2D->Write (x, y, fg, bg, buf);
}

SimpleConsole::SimpleConsole ()
{
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
    long screen_surface = FRAME_WIDTH * FRAME_HEIGHT;
    if (screen_surface <= 320*200)
      console_font = csFontTiny;
    else if (screen_surface <= 640*480)
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
    System->Printf (MSG_FATAL_ERROR, "Bad value for CONFONT in configuration file.\nUse 'auto', 'tiny', 'courier', or 'police'\n");
    fatal_exit (0, false);
  }

  int i;

  System->piGI->GetTextHeight (i, console_font);
  i += 2;
  LineMax = (FRAME_HEIGHT / i) - 2;
  LineSize = (FRAME_WIDTH / 4) + 1;

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
  LineMessageCount = 0;

  CHK (Line = new char * [LineMax]);
  for (i = 0; i < LineMax; i++)
  {
    CHK (Line [i] = new char [SIZE_LINE]);
    Line [i][0] = '\0';
  }
  LineCount = 0;

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

  CHK (buff_text = new char [SIZE_LINE]);
  buff_text [0] = '\0';

  LineTime = System->Time ();
  ConsoleMode = MESSAGE_MODE;
  CursorState = false;
  CursorTime = System->Time ();

  showing_work = current_work_done = 0;
}

SimpleConsole::~SimpleConsole ()
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
  CHK (delete [] buff_text);
  CHK (delete [] LinesChanged);
}

void SimpleConsole::SetTransparent (int t)
{
  if (t == -1)
    console_transparent_bg = config->GetYesNo ("SimpleConsole", "TRANSPBG", 1);
  else
    console_transparent_bg = t;
}

void SimpleConsole::SetMaxLines (int ml)
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
  LineMessageCount = 0;

  showing_work = current_work_done = 0;
}

void SimpleConsole::Show ()
{
  ConsoleMode = CONSOLE_MODE;
}

void SimpleConsole::Hide ()
{
  Gfx2D->ClearAll (0);
  ConsoleMode = MESSAGE_MODE;
}

void SimpleConsole::PutMessage (char *str,...)
{
  va_list arg;
  char buf[256];

  va_start (arg, str);
  vsprintf (buf, str, arg);
  va_end (arg);

  if (LineMessageCount >= LineMessageMax)
  {
    for (int i = 1; i < LineMessageMax; i++)
    {
      strcpy (LineMessage [i - 1], LineMessage [i]);
      LinesChanged [i - 1] = true;
    }
    LineMessageCount--;
  }

  strncpy (LineMessage [LineMessageCount], buf, SIZE_LINE - 1);
  LinesChanged [LineMessageCount] = true;

  LineMessageCount++;
  LineTime = System->Time () + 4000;
}

// @@@ Added for progress indicator
void SimpleConsole::ReplaceLastMessage(char *str,...)
{
  va_list arg;
  char buf[256];

  va_start (arg, str);
  vsprintf (buf, str, arg);
  va_end (arg);

  if (LineMessageCount >= LineMessageMax)
  {
    for (int i = 1; i < LineMessageMax; i++)
    {
      strcpy (LineMessage [i - 1], LineMessage [i]);
      LinesChanged [i - 1] = true;
    }
    LineMessageCount--;
  }

  strncpy (LineMessage [LineMessageCount], buf, SIZE_LINE - 1);
  LinesChanged [LineMessageCount] = true;

  LineTime = System->Time () + 4000;
}

void SimpleConsole::ShowWork ()
{
  // all character sets contain IBM PC pseudo-graphics characters ...
  char *wheel = "Ä\\³/";

  if (LineCount >= LineMax)
  {
    for (int i = 1; i < LineMax; i++)
      strcpy (Line[i - 1], Line[i]);
    LineCount--;
  }

  size_t wheel_pos = strlen (Line [LineCount]) - 1;
  if (!showing_work)
  {
    showing_work = 1;
    wheel_pos++;
    Line [LineCount] [wheel_pos + 1] = 0;
  }
  Line [LineCount] [wheel_pos] = wheel [current_work_done];

  current_work_done = (current_work_done + 1) & 3;
  ReplaceLastMessage ("%s", Line[LineCount]);
}

void SimpleConsole::PutText (char *str,...)
{
  va_list arg;
  char buf[256];

  va_start (arg, str);
  vsprintf (buf, str, arg);
  va_end (arg);

  if(!buf[0])
    return;

  showing_work=0;

  while (1)
  {
    char *crpos;
    if ((crpos = strchr (buf, '\n')) != NULL)
    {
      if (LineCount >= LineMax)
      {
        for (int i = 1; i < LineMax; i++)
          strcpy (Line[i - 1], Line[i]);
        LineCount--;
      }
      int lsize = SIZE_LINE - 1;
      strncpy (Line[LineCount], buff_text, lsize);
      lsize -= strlen (buff_text);
      if (lsize > crpos - buf)
        lsize = crpos - buf;
      strncat (Line[LineCount], buf, lsize);
      strcpy (buf, &crpos[1]);
      buff_text[0] = '\0';
      PutMessage ("%s", Line[LineCount]);
      LineCount++;
    } else
    {
      if (LineCount >= LineMax)
      {
        for (int i = 1; i < LineMax; i++)
          strcpy (Line[i - 1], Line[i]);
        LineCount--;
      }
      int lsize = SIZE_LINE - 1;
      strncpy (Line[LineCount], buff_text, lsize);
      lsize -= strlen (buff_text);
      if (lsize > (signed)strlen(buf))
        lsize = strlen(buf);
      strncat (Line[LineCount], buf, lsize);
      strncat (buff_text, buf, lsize);
      ReplaceLastMessage ("%s", Line[LineCount]);
      break;
    }
    /* endif */
  } /* endwhile */
}

void SimpleConsole::ExecuteCommand (char *command)
{
  PutText ("cs# %s\n", command);
  if (command && command[0] != '\0')
  {
    if (!Command::perform_line (command))
      PutText ("Unknown command :'%s'\n", command);

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

void SimpleConsole::Clear ()
{
  LineMessageCount = 0;
  LineCount = 0;
  LineCommandCount = 0;
  int i;

  for (i = 0; i < LineMessageMax; i++)
  {
    *LineMessage [i] = 0;
    LinesChanged [i] = true;
  }
}

void SimpleConsole::Print (csRect* area)
{
  int i;
  long CurrentTime = System->Time ();

  Gfx2D->SetFontID (console_font);

#define WRITE(x,y,fc,bc,s)					\
  {								\
    Gfx2D->Write (x, y, fc, bc, s);				\
    if (area)							\
    {								\
      int tw, th;						\
      System->piGI->GetTextWidth (tw, console_font, s);		\
      area->Union (x, y, x + tw, y + th);			\
    }								\
  }

#define WRITE2(x,y,fc,bc,s)					\
  {								\
    Gfx2D->Write (x + 1, y + 1, bc, -1, s);			\
    Gfx2D->Write (x, y, fc, -1, s);				\
    if (area)							\
    {								\
      int tw;							\
      System->piGI->GetTextWidth (tw, console_font, s);		\
      area->Union (x, y, x + 1 + tw, y + 1 + th);		\
    }								\
  }

  // text height
  int th;
  System->piGI->GetTextHeight (th, console_font);
  th += 2;

  bool dblbuff;
  Gfx2D->GetDoubleBufferState (dblbuff);

  switch (ConsoleMode)
  {
    case MESSAGE_MODE:
      if (CurrentTime > LineTime)
      {
        // Scroll all lines up once per four seconds
        for (i = 1; i < LineMessageMax; i++)
        {
          strcpy (LineMessage [i - 1], LineMessage [i]);
          LinesChanged [i - 1] = true;
        }
        if (LineMessageCount > 0)
          LineMessageCount--;
        LineMessage [LineMessageMax - 1][0] = '\0';
        LinesChanged [LineMessageMax - 1] = true;
        LineTime = System->Time () + 4000;
      }
      for (i = 0; i < LineMessageMax; i++)
      {
        if (LinesChanged [i] || dblbuff)
        {
          WRITE2 (10, 10 + th * i, console_fg, console_bg, LineMessage [i]);

	  // Only if the area is given do we enable the optimization that
	  // changed lines should not be redrawn. If area is not given we
	  // are running for full-screen printing anyway so we can't
	  // use that optimization.
          if (area)
            LinesChanged [i] = false;
        }
      }
      break;

    case CONSOLE_MODE:
      if (CurrentTime > CursorTime)
      {
        CursorState = !CursorState;
        CursorTime = System->Time () + 333;
      }

      char buf [256];
      sprintf (buf, CursorState ? "cs# %s_" : "cs# %s",  LineCommand);

      if (console_transparent_bg)
      {
        for (i = 0; i < LineCount; i++)
          WRITE2 (1, th * i, console_fg, console_bg, Line [i]);
        WRITE2 (1, th * LineCount, console_fg, console_bg, buf);
      }
      else
      {
        Gfx2D->Clear (console_bg);
        if (area && dblbuff)
          area->Union (0, 0, FRAME_WIDTH - 1, FRAME_HEIGHT - 1);
        for (i = 0; i < LineCount; i++)
          WRITE (1, th * i, console_fg, -1, Line [i]);
        WRITE (1, th * LineCount, console_fg, -1, buf);
      }
      break;
  }
}

void SimpleConsole::AddChar (int c)
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

void SimpleConsole::SetupColors (ITextureManager* txtmgr)
{
  txtmgr->FindRGB (console_fg_r, console_fg_g, console_fg_b, console_fg);
  txtmgr->FindRGB (console_bg_r, console_bg_g, console_bg_b, console_bg);
}
