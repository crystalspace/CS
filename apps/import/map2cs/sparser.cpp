/*
    Map2cs: a convertor to convert the frequently used MAP format, into
    something, that can be directly understood by Crystal Space.

    Copyright (C) 1999 Thomas Hieber (thieber@gmx.net)
	Modified by Pete Mistich for script handling

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
#include "sparser.h"

CScriptParser::CScriptParser()
{
  m_fd = 0;
  m_Eof = false;
  m_CurrentLine = 0;
  memset(m_NextChars, 0, sizeof(m_NextChars));
  memset(m_NextToken, 0, sizeof(m_NextToken));
}

CScriptParser::~CScriptParser()
{
  if (m_fd)
  {
    fclose(m_fd);
  }
}

bool CScriptParser::Open(const char* filename)
{
  m_fd  = fopen(filename, "rb");
  if (!m_fd)
  {
    ReportError("Error opening scriptfile \"%s\"", filename);
    return false;
  }

  m_CurrentLine = 1;
  m_Eof = false;
  int i;
  for (i=0;i<ReadAhead;i++)
  {
    GetNextChar(); //Ensure m_NextChars is inited;
  }

  if (!ReadNextToken(m_NextToken))
  {
    m_NextToken[0] = 0;
  }

  return true;
}

bool CScriptParser::GetNextChar()
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

bool CScriptParser::SkipWhitespace()
{
  while (m_NextChars[0] <= ' ')
  {
    if (!GetNextChar())
    {
      return false;
    }
  }
  return true;
}

bool CScriptParser::SkipToNextLine()
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

bool CScriptParser::GetNextToken(char* str)
{
  strcpy(str, m_NextToken);
  if (m_NextToken[0] == 0) return false;
  ReadNextToken(m_NextToken);
  return true;
}
/*
bool CScriptParser::PeekNextToken(char* str)
{
  strcpy(str, m_NextToken);
  if (m_NextToken[0] == 0) return false;
  return true;
}
*/
bool CScriptParser::ReadNextToken(char* str)
{
  *str = 0; //clear the string first

  if (!SkipWhitespace()) return false;

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
      strcpy(str, "//TX1");
      str[4] = Num;
      return true;
    }

    if (!GetNextChar())    return false;
    if (!GetNextChar())    return false;
    if (!SkipToNextLine()) return false;
    if (!SkipWhitespace()) return false;

    //Call recursively, to eliminate multiple commented lines.
    return ReadNextToken(str);
  }

  //Quoted strings are returned directly
  if (m_NextChars[0] == '\"')
  {
    do
    {
      //drop the '\"' character
      if (!GetNextChar())
      {
        *str = 0;     //Terminate the string anyway
        return false; //if we fail now, the token is not complete!
      }

      if (m_NextChars[0] == '\"')
      {
        //The end of the string has been reached

        GetNextChar(); //remove the end of string token (We dont care for success here!)
        *str = 0;
        return true;
      }
      *str++ = m_NextChars[0];
    } while (1);
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
      *str++ = m_NextChars[0];
      *str   = 0;
      GetNextChar(); //Remove this token (We don't care for success here!)
      return true;
    }
  }

  //parse a regular word
  do
  {
    *str++ = m_NextChars[0];
    if (!GetNextChar())
    {
      //if there are no more characters, we claim the token
      //ended here in a regular way.
      *str = 0;
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
  } while (1);

  *str = 0;
  return true;
}
/*
bool CScriptParser::GetIntToken  (int& val)
{
  char Buffer[1000];
  if (!GetNextToken(Buffer))
  {
    ReportError("Unexpected end of file. (expected an int number)");
    return false;
  }
  char dummy;
  if (sscanf(Buffer, "%d%c", &val, &dummy)!=1)
  {
    ReportError("Invalid Numeric format. Expected int, found \"%s\"", Buffer);
    return false;
  }
  return true;
}
*/
/*
bool CScriptParser::GetFloatToken(double& val)
{
  char Buffer[1000];
  if (!GetNextToken(Buffer))
  {
    ReportError("Unexpected end of file. (expected a double number)");
    return false;
  }
  char dummy;
  if (sscanf(Buffer, "%lf%c", &val, &dummy)!=1)
  {
    ReportError("Invalid Numeric format. Expected double, found \"%s\"", Buffer);
    return false;
  }
  return true;
}
*/
/*
bool CScriptParser::ExpectToken(const char* ExpectedToken)
{
  char Buffer[1000];
  if (!GetNextToken(Buffer))
  {
    ReportError("Unexpected end of file. (expected \"%s\")", ExpectedToken);
    return false;
  }
  if (strcmp(Buffer, ExpectedToken) != 0)
  {
    ReportError("Unexpected Token found. Expected \"%s\", found \"%s\"",
                ExpectedToken, Buffer);
    return false;
  }
  return true;
}
*/
bool CScriptParser::GetTextToken (char*  text)
{
  if (!GetNextToken(text))
  {
    ReportError("Unexpected end of file. (expected a text)");
    return false;
  }
  return true;
}
/*
bool CScriptParser::GetSafeToken (char* str)
{
  if (!GetNextToken(str))
  {
    ReportError("Unexpected end of file.");
    return false;
  }
  return true;
}
*/
void CScriptParser::ReportError(const char* message,
                             const char* info1,
                             const char* info2)
{
  csString Msg1;
  if (info1 && info2)
  {
    Msg1.Format (message, info1, info2);
  }
  else if (info1)
  {
    Msg1.Format (message, info1);
  }
  else
  {
    Msg1.Format (message);
  }
  csPrintf ("Error in line %d: %s", m_CurrentLine, Msg1.GetData());
}



//********************************************************************
bool CScriptParser::GetLineToken (char*  text)
{
  if (!GetNextLineToken(text))
  {
    ReportError("Unexpected end of file. (expected a text)");
    return false;
  }
  return true;
}

bool CScriptParser::GetNextLineToken(char* str)
{

  strcpy(str, m_NextToken);
  if (m_NextToken[0] == 0) return false;
  ReadNextLineToken(m_NextToken);
  return true;

}

bool CScriptParser::ReadNextLineToken(char* str)
{
  *str = 0; //clear the string first

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
      strcpy(str, "//TX1");
      str[4] = Num;
      return true;
    }

    if (!GetNextChar())    return false;
    if (!GetNextChar())    return false;
    if (!SkipToNextLine()) return false;
    if (!SkipWhitespace()) return false;

    //Call recursively, to eliminate multiple commented lines.
    return ReadNextLineToken(str);
  }

  //Quoted strings are returned directly
  if (m_NextChars[0] == '\"')
  {
    do
    {
      //drop the '\"' character
      if (!GetNextChar())
      {
        *str = 0;     //Terminate the string anyway
        return false; //if we fail now, the token is not complete!
      }

      if (m_NextChars[0] == '\"')
      {
        //The end of the string has been reached

        GetNextChar(); //remove the end of string token (We dont care for success here!)
        *str = 0;
        return true;
      }
      *str++ = m_NextChars[0];
    } while (1);
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
      *str++ = m_NextChars[0];
      *str   = 0;
      GetNextChar(); //Remove this token (We don't care for success here!)
      return true;
    }
  }

  //parse a regular word
  do
  {
	if ( (int) m_NextChars[0] == 13  )
	{
		m_NextChars[0] = ' ';
	}
	if ( (int) m_NextChars[0] == 10  )
	{
		m_NextChars[0] = '\n';
	}

	*str++ = m_NextChars[0];

    if (!GetNextChar())
    {
      //if there are no more characters, we claim the token
      //ended here in a regular way.
      *str = 0;
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
  } while (1);

  *str = 0;
  return true;
}

bool CScriptParser::SkipControlM()
{

  if ( (int) m_NextChars[0] == 9)
  {
    return true;
  }
  return false;
}

