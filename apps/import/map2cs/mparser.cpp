/*
    Map2cs: a convertor to convert the frequently used MAP format, into
    something, that can be directly understood by Crystal Space.

    Copyright (C) 1999 Thomas Hieber (thieber@gmx.net)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"
#include "mapstd.h"
#include "mparser.h"

CMapParser::CMapParser()
{
  m_fd = 0;
  m_Eof = false;
  m_CurrentLine = 0;
  memset(m_NextChars, 0, sizeof(m_NextChars));
}

CMapParser::~CMapParser()
{
  if (m_fd)
  {
    fclose(m_fd);
  }
}

bool CMapParser::Open(const char* filename)
{
  m_fd  = fopen(filename, "rb");
  if (!m_fd)
  {
    ReportError("Error opening map \"%s\"", filename);
    return false;
  }

  m_CurrentLine = 1;
  m_Eof = false;
  int i;
  for (i=0;i<ReadAhead;i++)
  {
    GetNextChar(); //Ensure m_NextChars is inited;
  }

  if (!ReadNextToken (m_NextToken))
  {
    m_NextToken.Clear();
  }

  return true;
}

bool CMapParser::GetNextChar()
{
  int val = EOF;
  if (!m_Eof)
  {
    val = getc(m_fd);
    if (val == EOF)
    {
      m_Eof = true;
    }
  }

  memmove(m_NextChars, &m_NextChars[1], ReadAhead-1);
  if (m_NextChars[0] == '\n')
  {
    m_CurrentLine++;
  }

  if (m_Eof)
  {
    m_NextChars[ReadAhead-1] = 0;
    return m_NextChars[0] != 0;
  }
  else
  {
    m_NextChars[ReadAhead-1] = val;
    return true;
  }
}

bool CMapParser::SkipWhitespace ()
{
  char ch;
  while ((ch = m_NextChars[0]) <= ' ')
  {
    if (!GetNextChar())
    {
      return false;
    }
  }
  return true;
}

bool CMapParser::SkipToNextLine()
{
  while (m_NextChars[0] != '\n')
  {
    if (!GetNextChar())
    {
      return false;
    }
  }
  return true;
}

bool CMapParser::GetNextToken (csString& str)
{
  str.Replace (m_NextToken);
  if (m_NextToken.IsEmpty() && !emptinessHack) return false;
  ReadNextToken (m_NextToken);
  return true;
}

bool CMapParser::PeekNextToken (csString& str)
{
  str.Replace (m_NextToken);
  if (m_NextToken.IsEmpty() && !emptinessHack) return false;
  return true;
}

bool CMapParser::ReadNextToken (csString& str)
{
  //char *strStart = str;

  //*str = 0; 
  str.Clear(); //clear the string first
  emptinessHack = false;

  if (!SkipWhitespace ()) return false;

  //Check for Comments, that start by "//" and end at the end of the line
  if (m_NextChars[0] == '/' && m_NextChars[1] == '/')
  {
    if ( m_NextChars[2] == 'T' &&
         m_NextChars[3] == 'X' &&
        (m_NextChars[4] == '1' || m_NextChars[4] == '2'))
    {
      char Num = m_NextChars[4];

      //This is a special texture alignment info, provided by QuArK,
      //that is a little more powerfull than the standard map format
      //can provide.
      //For more info on QuArK visit: http://www.planetquake.com/quark
      //
      //For compatibility reasons, these extensions are hidden in a
      //comment. Normally we will remove all comments in the parser,
      //but this comment is important!
      if (!GetNextChar())    return false;
      if (!GetNextChar())    return false;
      if (!SkipToNextLine()) return false;
      if (!SkipWhitespace()) return false;
      str.Replace ("//TX1");
      str.SetAt (4, Num);
      return true;
    }

    if (!GetNextChar())    return false;
    if (!GetNextChar())    return false;
    if (!SkipToNextLine()) return false;
    if (!SkipWhitespace()) return false;

    //Call recursively, to eliminate multiple commented lines.
    return ReadNextToken (str);
  }

  //Quoted strings are returned directly
  if (m_NextChars[0] == '\"')
  {
    do
    {
      //drop the '\"' character
      if (!GetNextChar())
      {
        //*str = 0;     //Terminate the string anyway
        return false; //if we fail now, the token is not complete!
      }

      if (m_NextChars[0] == '\"')
      {
        //The end of the string has been reached

        GetNextChar(); //remove the end of string token (We dont care for success here!)
	emptinessHack = str.IsEmpty();
        //*str = 0;
        return true;
      }
      //*str++ = m_NextChars[0];
      str << m_NextChars[0];
    } while(1); //while (str<(strStart+maxlen));
  }

  // Check for special single character tokens
  if (m_NextChars[0]=='{' ||
      m_NextChars[0]=='}' ||
      m_NextChars[0]=='(' ||
      m_NextChars[0]==')' ||
      m_NextChars[0]=='\'' ||
      m_NextChars[0]==':')
  {
    //These special single character tokens must be followed by a whitespace
    //to be accepted. (otherwise there are problems with special texture names)
    if (m_NextChars[1] < ' ')
    {
      //*str++ = m_NextChars[0];
      //*str   = 0;
      str << m_NextChars[0];
      GetNextChar(); //Remove this token (We don't care for success here!)
      return true;
    }
  }

  //parse a regular word
  do
  {
    //*str++ = m_NextChars[0];
    str << m_NextChars[0];
    if (!GetNextChar())
    {
      //if there are no more characters, we claim the token
      //ended here in a regular way.
      //*str = 0;
      return true;
    }

    if (m_NextChars[0]=='{' ||
        m_NextChars[0]=='}' ||
        m_NextChars[0]=='(' ||
        m_NextChars[0]==')' ||
        m_NextChars[0]=='\'' ||
        m_NextChars[0]==':' ||
        m_NextChars[0]<=' ')
    {
      break;
    }
  } while (1); //(str<(strStart+maxlen));

  /*if (str>=(strStart+maxlen)) 
  {
    char tokenBegin[16];

    strncpy(tokenBegin, strStart, sizeof(tokenBegin)-1);
    tokenBegin[sizeof(tokenBegin)] = '\0';

    ReportError("Token (\"%s\"...) too long, max. length %d", tokenBegin, maxlen);
    return false;
  }

  *str = 0;*/
  return true;
}

bool CMapParser::GetIntToken  (int& val)
{
  csString Buffer;
  if (!GetNextToken(Buffer))
  {
    ReportError("Unexpected end of file. (expected an int number)");
    return false;
  }
  char dummy;
  if (sscanf(Buffer, "%d%c", &val, &dummy)!=1)
  {
    ReportError("Invalid Numeric format. Expected int, found \"%s\"", 
      Buffer.GetData());
    return false;
  }
  return true;
}

bool CMapParser::GetFloatToken(double& val)
{
  csString Buffer;
  if (!GetNextToken(Buffer))
  {
    ReportError("Unexpected end of file. (expected a double number)");
    return false;
  }
  char dummy;
  if (sscanf(Buffer, "%lf%c", &val, &dummy)!=1)
  {
    ReportError("Invalid Numeric format. Expected double, found \"%s\"", 
      Buffer.GetData());
    return false;
  }
  return true;
}

bool CMapParser::ExpectToken(const char* ExpectedToken)
{
  csString Buffer;
  if (!GetNextToken(Buffer))
  {
    ReportError("Unexpected end of file. (expected \"%s\")", ExpectedToken);
    return false;
  }
  if (strcmp(Buffer, ExpectedToken) != 0)
  {
    ReportError("Unexpected Token found. Expected \"%s\", found \"%s\"",
                ExpectedToken, Buffer.GetData());
    return false;
  }
  return true;
}

bool CMapParser::GetTextToken (csString& text)
{
  if (!GetNextToken(text))
  {
    ReportError("Unexpected end of file. (expected a text)");
    return false;
  }
  return true;
}

bool CMapParser::GetSafeToken (csString& str)
{
  if (!GetNextToken(str))
  {
    ReportError("Unexpected end of file.");
    return false;
  }
  return true;
}

void CMapParser::ReportError(const char* message, ...)
{
  printf("Error in line %d: ", m_CurrentLine);
  va_list args;
  va_start (args, message);
  vprintf (message, args);
  va_end (args);
  printf("\n");
}
