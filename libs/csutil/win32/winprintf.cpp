/*
    Copyright (C) 2003 by Frank Richter

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
#include "csutil/ansiparse.h"
#include "csutil/csunicode.h"
#include "csutil/util.h"
#include "csutil/snprintf.h"
#include "csutil/sysfunc.h"

#include <windows.h>
#include "csutil/win32/wintools.h"

#ifdef __CYGWIN__
#define _fileno(x) fileno(x)
#define _isatty(x) isatty(x)
#define _get_osfhandle(x) get_osfhandle(x)
#endif

static const WORD foregroundMask = FOREGROUND_BLUE | FOREGROUND_GREEN
  | FOREGROUND_RED;
static const WORD backgroundMask = BACKGROUND_BLUE | BACKGROUND_GREEN
  | BACKGROUND_RED;
static const WORD colorMask = foregroundMask | backgroundMask 
  | FOREGROUND_INTENSITY | BACKGROUND_INTENSITY;
static const WORD ansiToWindows[8] = 
{
  0,
  FOREGROUND_RED,
  FOREGROUND_GREEN,
  FOREGROUND_GREEN | FOREGROUND_RED,
  FOREGROUND_BLUE,
  FOREGROUND_BLUE | FOREGROUND_RED,
  FOREGROUND_BLUE | FOREGROUND_GREEN,
  FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED
};

static int _cs_fputs (const char* string, FILE* stream)
{
  UINT cp;
  HANDLE hCon; 
  WORD textAttr, oldAttr;
  bool isTTY = _isatty (_fileno (stream));
  if (isTTY) 
  {
    cp = GetConsoleOutputCP ();
    hCon = (HANDLE)_get_osfhandle (_fileno (stream));
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo (hCon, &csbi);
    oldAttr = textAttr = csbi.wAttributes;
  }

  size_t ret = 0;
  csString tmp;
  size_t ansiCommandLen;
  csAnsiParser::CommandClass cmdClass;
  size_t textLen;
  // Check for ANSI codes
  while (csAnsiParser::ParseAnsi (string, ansiCommandLen, cmdClass, textLen))
  {
    ret += textLen;
    tmp.Replace (string + ansiCommandLen, textLen);
    
    if (isTTY && (cmdClass == csAnsiParser::classFormat))
    {
      ret += ansiCommandLen;
      // Apply ANSI format
      const char* cmd = string;
      size_t cmdLen = ansiCommandLen;
      csAnsiParser::Command command;
      csAnsiParser::CommandParams commandParams;
      while (csAnsiParser::DecodeCommand (cmd, cmdLen, command,	commandParams))
      {
	switch (command)
	{
	  case csAnsiParser::cmdFormatAttrReset:
	    textAttr = (textAttr & ~colorMask) | FOREGROUND_BLUE 
	      | FOREGROUND_GREEN | FOREGROUND_RED;
	    break;
	  case csAnsiParser::cmdFormatAttrEnable:
	    {
	      switch (commandParams.attrVal)
	      {
		case csAnsiParser::attrBold:
		  textAttr |= FOREGROUND_INTENSITY;
		  break;
		case csAnsiParser::attrBlink:
		  textAttr |= BACKGROUND_INTENSITY;
		  break;
		/* // Proper Reverse and Invisible support is non-trivial
		case csAnsiParser::attrReverse:
		  break;
		case csAnsiParser::attrInvisible:
		  break;
		*/
	      }
	    }
	    break;
	  case csAnsiParser::cmdFormatAttrDisable:
	    {
	      switch (commandParams.attrVal)
	      {
		case csAnsiParser::attrBold:
		  textAttr &= ~FOREGROUND_INTENSITY;
		  break;
		case csAnsiParser::attrBlink:
		  textAttr &= ~BACKGROUND_INTENSITY;
		  break;
		/* // Proper Reverse and Invisible support is non-trivial
		case csAnsiParser::attrReverse:
		  break;
		case csAnsiParser::attrInvisible:
		  break;
		*/
	      }
	    }
	    break;
	  case csAnsiParser::cmdFormatAttrForeground:
	    textAttr = (textAttr & ~foregroundMask) 
	      | ansiToWindows[commandParams.colorVal];
	    break;
	  case csAnsiParser::cmdFormatAttrBackground:
	    textAttr = (textAttr & ~backgroundMask) 
	      | (ansiToWindows[commandParams.colorVal] << 4);
	    break;
	}
      }
    }

    if (textLen > 0)
    {
      if (isTTY && (oldAttr != textAttr))
      {
	SetConsoleTextAttribute (hCon, textAttr);
	oldAttr = textAttr;
      }
      int rc;
      if (isTTY && (cp != CP_UTF8))
      {
	/* We're writing to a console, the UTF-8 text has to be converted to the 
	* output codepage. */
	rc = fputs (cswinCtoA (tmp, cp), stream);
      }
      else
      {
	/* Not much to do - the text can be dumped to the stream,
	* Windows will care about proper output. */
	rc = fputs (tmp.GetDataSafe(), stream);
      }

      if (rc == EOF)
	return EOF;
    }

    string += ansiCommandLen + textLen;
  }
  return (int)ret;
}

static int _cs_vfprintf (FILE* stream, const char* format, va_list args)
{
  int rc;

  int bufsize, newsize = 128;
  char* str = 0;
  
  do
  {
    delete[] str;
    bufsize = newsize + 1;
    str = new char[bufsize];
    // use cs_vsnprintf() for consistency across all platforms.
    newsize = cs_vsnprintf (str, bufsize, format, args);
  }
  while (bufsize < (newsize + 1));

  // _cs_fputs() will do codepage convertion, if necessary
  rc = _cs_fputs (str, stream);
  delete[] str;
  // On success fputs() returns a value >= 0, but
  // printf() should return the number of chars written.
  return ((rc >= 0) ? (newsize - 1) : -1);
}

// Replacement for printf(); exact same prototype/functionality as printf()
int csPrintf (char const* str, ...)
{
  va_list args;
  va_start(args, str);
  int const rc = _cs_vfprintf (stdout, str, args);
  va_end(args);
  fflush (stdout);
  return rc;
}

// Replacement for vprintf()
int csPrintfV(char const* str, va_list args)
{
  int ret = _cs_vfprintf (stdout, str, args);
  fflush (stdout);
  return ret;
}

int csFPutErr (const char* str)
{
  int ret = _cs_fputs (str, stderr);
  fflush (stderr);
  return ret;
}

int csPrintfErr (const char* str, ...)
{
  va_list args;
  va_start (args, str);
  int const rc = csPrintfErrV (str, args);
  va_end (args);
  return rc;
}

int csPrintfErrV (const char* str, va_list args)
{
  int ret = _cs_vfprintf (stderr, str, args);
  fflush (stderr);
  return ret;
}
