/*
    Copyright (C) 2003 by Frank Richter
              (C) 2006 by Marten Svanfeldt

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
#include "csgeom/math.h"
#include "csutil/ansiparse.h"
#include "csutil/csstring.h"
#include "csutil/csunicode.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/snprintf.h"
#include "csutil/sysfunc.h"
#include "csutil/util.h"

#define WIN32_LEAN_AND_MEAN
#include <wincon.h>
#include "csutil/win32/wintools.h"

#if defined(__CYGWIN__) || defined(CS_COMPILER_BCC)
#define _fileno(x) fileno(x)
#define _isatty(x) isatty(x)
#endif
#if defined(__CYGWIN__)
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

static int WriteConsoleUTF8 (HANDLE hCon, const utf8_char* inStr, size_t inLen)
{
  // According to MSDN, WriteConsole() takes at most 64K characters
  static const DWORD maxConsoleWrite = 64*1024;

  wchar_t wideBuf[maxConsoleWrite];
  size_t remaining = inLen;
  while (remaining > 0)
  {
    DWORD wideBufRemain = maxConsoleWrite;
    wchar_t* wideBufOut = wideBuf;

    while (wideBufRemain > 0)
    {
      utf32_char ch;
      int srcEncSize = csUnicodeTransform::UTF8Decode (inStr, remaining, ch);
      utf16_char ch_wide[CS_UC_MAX_UTF16_ENCODED];
      int dstEncSize = csUnicodeTransform::EncodeUTF16 (ch, ch_wide, CS_UC_MAX_UTF16_ENCODED);
      if (size_t (dstEncSize) <= wideBufRemain)
      {
        memcpy (wideBufOut, ch_wide, dstEncSize * sizeof (wchar_t));
        wideBufOut += dstEncSize;
        wideBufRemain -= dstEncSize;
        inStr += srcEncSize;
        remaining -= srcEncSize;
        if (remaining == 0) break;
      }
      else
        break;
    }

    DWORD charsToWrite = maxConsoleWrite - wideBufRemain;
    DWORD charsWritten = 0;
    if (!WriteConsoleW (hCon, wideBuf, charsToWrite, &charsWritten, nullptr)
	|| (charsWritten != charsToWrite))
      return EOF;
  }
  return 0;
}

static int _cs_fputs (const char* string, FILE* stream)
{
  HANDLE hCon = 0; 
  WORD textAttr = 0, oldAttr = 0;
  bool isTTY = (_isatty (_fileno (stream)) != 0);
  if (isTTY) 
  {
    hCon = (HANDLE)_get_osfhandle (_fileno (stream));
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo (hCon, &csbi);
    oldAttr = textAttr = csbi.wAttributes;
  }

  size_t ret = 0;
  size_t ansiCommandLen;
  csAnsiParser::CommandClass cmdClass;
  size_t textLen;
  // Check for ANSI codes
  while (csAnsiParser::ParseAnsi (string, ansiCommandLen, cmdClass, textLen))
  {
    ret += textLen;
    const char* textPtr = string + ansiCommandLen;
    
    if (isTTY && (cmdClass != csAnsiParser::classNone && 
                  cmdClass != csAnsiParser::classUnknown))
    {
      ret += ansiCommandLen;
      // Apply ANSI string
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
		default:
		  break;
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
		default:
		  break;
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
          case csAnsiParser::cmdClearScreen:
            {
              CONSOLE_SCREEN_BUFFER_INFO csbi;
              COORD homeCoord = {0, 0};

              GetConsoleScreenBufferInfo (hCon, &csbi);
              DWORD conSize = csbi.dwSize.X * csbi.dwSize.Y;
              DWORD charsWritten;
              //Fill with blanks
              FillConsoleOutputCharacter (hCon, ' ', conSize, homeCoord, 
                &charsWritten);

              //Fill attributes
              FillConsoleOutputAttribute (hCon, csbi.wAttributes, conSize,
                homeCoord, &charsWritten);

              //Put cursor at 0,0
              SetConsoleCursorPosition (hCon, homeCoord);
            }
            break;
          case csAnsiParser::cmdClearLine:
            {
              CONSOLE_SCREEN_BUFFER_INFO csbi;
              COORD homeCoord;

              GetConsoleScreenBufferInfo (hCon, &csbi);
              
              homeCoord = csbi.dwCursorPosition;

              DWORD conSize = csbi.dwSize.X - csbi.dwCursorPosition.X;
              DWORD charsWritten;
              //Fill with blanks
              FillConsoleOutputCharacter (hCon, ' ', conSize, homeCoord, 
                &charsWritten);

              //Fill attributes
              FillConsoleOutputAttribute (hCon, csbi.wAttributes, conSize,
                homeCoord, &charsWritten);

              //Put cursor at starting position
              SetConsoleCursorPosition (hCon, homeCoord);
            }
            break;
          case csAnsiParser::cmdCursorSetPosition:
            {
              //Limit to screen
              CONSOLE_SCREEN_BUFFER_INFO csbi;
              COORD coord;

              GetConsoleScreenBufferInfo (hCon, &csbi);

              coord.X = csClamp<int> (commandParams.cursorVal.x-1, csbi.dwSize.X, 0);
              coord.Y = csClamp<int> (commandParams.cursorVal.y-1, csbi.dwSize.Y, 0);

              //Set position
              SetConsoleCursorPosition (hCon, coord);
            }
            break;
          case csAnsiParser::cmdCursorMoveRelative:
            {
              //Limit to screen
              CONSOLE_SCREEN_BUFFER_INFO csbi;
              COORD coord;

              GetConsoleScreenBufferInfo (hCon, &csbi);


              coord.X = csClamp<int> (csbi.dwCursorPosition.X + commandParams.cursorVal.x, 
                csbi.dwSize.X, 0);

              coord.Y = csClamp<int> (csbi.dwCursorPosition.Y + commandParams.cursorVal.y, 
                csbi.dwSize.Y, 0);

              //Set position
              SetConsoleCursorPosition (hCon, coord);
            }
            break;
	  default:
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
      if (isTTY)
      {
	/* We're writing to a console - give Windows the string in Unicode
           and let it do the conversion */
        rc = WriteConsoleUTF8 (hCon, (utf8_char*)textPtr, textLen);
      }
      else
      {
	/* Not much to do - dump raw text to the stream. */
	rc = int (fwrite (textPtr, 1, textLen, stream));
      }

      if (rc == EOF)
	return EOF;
    }

    string += ansiCommandLen + textLen;
  }
  if (isTTY && (oldAttr != textAttr))
    SetConsoleTextAttribute (hCon, textAttr);
  return (int)ret;
}

static int _cs_vfprintf (FILE* stream, const char* format, va_list args)
{
  int rc;

  const size_t initialBufSize = 128;
  char localBuf[initialBufSize];
  char* str = localBuf;
  
  // use cs_vsnprintf() for consistency across all platforms.
  size_t newsize = cs_vsnprintf (str, initialBufSize, format, args);
  if (newsize >= initialBufSize)
  {
    size_t bufsize = newsize+1;
    str = (char*)cs_malloc (bufsize);
    cs_vsnprintf (str, bufsize, format, args);
  }

  // _cs_fputs() will do codepage conversion, if necessary
  rc = _cs_fputs (str, stream);
  if (str != localBuf) cs_free (str);
  // On success _cs_fputs() returns a value >= 0, but
  // _cs_vfprintf() should return the number of chars written.
  return ((rc >= 0) ? int (newsize) : -1);
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

int csFPrintf (FILE* file, const char* str, ...)
{
  va_list args;
  va_start (args, str);
  int const rc = _cs_vfprintf (file, str, args);
  va_end (args);
  return rc;
}

int csFPrintfV (FILE* file, const char* str, va_list args)
{
  return _cs_vfprintf (file, str, args);
}

