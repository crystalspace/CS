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

#ifndef __SPARSER_H__
#define __SPARSER_H__

/**
  * Defines, how many bytes will be read ahead. This will allow you
  * to take a look on the coming characters, to decide what type
  * a token will be
  */
const int ReadAhead = 10;

/**
  * Defines the maximum length of a token.
  */
const int MaxTokenLen = 300;

/**
  * This class will parse the map file. It will translate the ASCII
  * stream into Tokens, and will also keep track of the current
  * line numbers.
  * It also contains simple error checking and reporting abilities,
  * so if you request a number and the map contains a text, you
  * will get a descriptive error message.
  * The parser is pretty general, so it could also be used to parse
  * a configuration file as well.
  */
class CScriptParser
{
public:
  /**
    * The constructor, does some basic intialisation as usual.
    */
  CScriptParser();

  /**
    * The destructor. Will close the file, if it is currently open.
    */
  virtual ~CScriptParser();

  /**
    * Opens a file. Will return false, if there was an error opening
    * the file. This method will display an error message by itself too.
    */
  bool Open(const char* filename);


  /**
    * Returns an integer value. If the next token was no integer value,
    * an error message is displayed, and false is being returned
    */
  bool GetIntToken  (int&   val);

  /**
    * Returns a double value. If the next token was no double value,
    * an error message is displayed, and false is being returned
    */
  bool GetFloatToken(double& val);

  /**
    * Returnd the next token as text. If there is no next token.
    * (because of end of file), an error message is displayed, and
    * false is being returned
    */
  bool GetTextToken (char*  text);

  //pm start
  /**
    * Returnd the next token as text. If there is no next token.
    * (because of end of file), an error message is displayed, and
    * false is being returned
    */
  bool GetLineToken (char*  text);
  //pm stop

  /**
    * Returns the next token. If there is no next token.
    * (because of end of file), an error message is displayed, and
    * false is being returned
    * It is about the same code as GetTextToken, but the error
    * message is more general, and will not mention "text".
    */
  bool GetSafeToken (char* str);

  /**
    * Gets the next token and compares it with the expected token.
    * If it doesn't match, or the file comes to an unexpected end,
    * an error message is displayed, and false is being returned.
    */
  bool ExpectToken  (const char* ExpectedToken);

  /**
    * Reports an error. Message may contain %s signs to indicate,
    * where to add the (optional) information stings into the
    * error message. The number of %s _must_ match the number
    * of given info. This is _not_ being checked!
    * This method is virtual, so it can be defined different
    * for different uses. a GUI application might derive a
    * class from this class, which will display a csMessageBox,
    * while a console application will just do a csPrintf.
    */
  virtual void ReportError(const char* message,
                           const char* info1=0,
                           const char* info2=0);


  /**
    * Returns the Next token from the file. If there is no next
    * token, or if there is some severe format problem, it will
    * return false, otherwise it will return true.
    * This method will not generate any error messages by itself!
    */
  bool GetNextToken(char* str);

  //pm start
  /**
    * Returns the Next token from the file. If there is no next
    * token, or if there is some severe format problem, it will
    * return false, otherwise it will return true.
    * This method will not generate any error messages by itself!
    */
  bool GetNextLineToken(char* str);
  //pm stop

  /**
    * Peeks at the Next token from in the file. If there is no next
    * token, or if there is some severe format problem, it will
    * return false, otherwise it will return true.
    * This method will not generate any error messages by itself!
    * This method can be called without changing the state of the
    * parser!
    */
  bool PeekNextToken(char* str);

  /**
    * Return the number of the line, that is currently being processed.
    */
  int  GetCurrentLine() {return m_CurrentLine;}
protected:

  /**
    * Reads the next token directly from the imputfile
    */
  bool ReadNextToken(char* str);

  //pm start
  /**
    * Reads the next token directly from the imputfile
    */
  bool ReadNextLineToken(char* str);
  //pm stop

  /**
    * Gets the next character into the m_NextChars array. If there
    * are no more characters, false is returned, and 0 is inserted
    * in the array.
    */
  bool  GetNextChar();

  /**
    * Skips all whitespace (characters smaller or equal than ' '
    * in the current input stream)
    */
  bool  SkipWhitespace();

  //pm start
  /**
    * Skips all whitespace (characters smaller or equal than ' '
    * in the current input stream)
    */
  bool SkipControlM();
  //pm stop

  /**
    * Skips any character till the next line has started.
    */
  bool  SkipToNextLine();


  /**
    * Here the current character is stored (at index 0), For convenience
    * you can also look ahead "ReadAhead-1" characters.
    */
  char m_NextChars[ReadAhead];

  /**
    * Here is the next token stored, so one can use PeekNextToken to take
    * a look at it, without causing problems.
    */
  char m_NextToken[MaxTokenLen];

  /**
    * True, if the file has ended. (Note, that there may still be
    * characters stored in m_NextChars, that have not been evaluated)
    */
  bool  m_Eof;

  /**
    * The filedescriptor of the input file.
    */
  FILE* m_fd;

  /**
    * The current linenumer. (Of m_NextChars[0])
    */
  int   m_CurrentLine;
};

#endif // __SPARSER_H__
