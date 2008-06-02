/*
  Copyright (C) 2007 by Frank Richter

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"

#include "beautify_cg.h"

CS_PLUGIN_NAMESPACE_BEGIN(GLShaderCg)
{

  CgBeautifier::CgBeautifier (csString& dest) : dest (dest), indent (0),
    lineStatement (-1), currentStatement (0), state (0)
  {
  }

  int CgBeautifier::HandleCommentChar (const char*& str, int commentType, 
                                       char ch)
  {
    int newState = state;
    switch (commentType)
    {
      case stateCommentLine:
        {
          if ((ch == '\r') || (ch == '\n'))
          {
            if ((ch == '\r') && (*(str+1) == '\n')) 
              str++;
            newState &= ~stateCommentMask;
            newState |= stateNeedNewline;
          }
          else
          {
            dest += ch;
          }
          str++;
        }
        break;
      case stateCommentBlock:
        {
          if ((ch == '*') && (*(str+1) == '/'))
          {
            newState &= ~stateCommentMask;
            dest += "*/";
            str += 2;
          }
          else
          {
            dest += ch;
            str++;
          }
        }
        break;
    }
    return newState;
  }
  
    
  int CgBeautifier::HandleStatementChar (const char*& str, char ch)
  {
    int newState = state;
    switch (ch)
    {
      case ';':
        dest += ch;
        str++;
        currentStatement++;
        newState = stateBetween;
        break;
      case '\r':
        if (*(str+1) == '\n') str++;
      case '\n':
        str++;
        newState |= stateNeedNewline;
        break;
      case '{':
        newState = stateBetween | (state & stateNeedNewline);
        break;
      case '}':
        newState = stateBetween;
        break;
      case '/':
        {
          char next = *(str+1);
          if ((next == '/') || (next == '*'))
          {
            BeginComment (ch, next, str, newState);
          }
          else
          {
            dest += ch;
            str++;
          }
        }
        break;
      default:
        dest += ch;
        str++;
        break;
    }
    return newState;
  }
  
  int CgBeautifier::HandleBetweenChar (const char*& str, char ch)
  {
    int newState = state;
    switch (ch)
    {
      case '{':
        currentStatement++;
        InsertNewlineIfNeeded (true);
        dest += ch;
        str++;
        indent++;
        newState = stateBetween;
        break;
      case '}':
        indent--;
        newState = stateBetween;
        InsertNewlineIfNeeded (true);
        dest += ch;
        str++;
        break;
      case '\r':
        if (*(str+1) == '\n') str++;
      case '\n':
        str++;
        if (state & stateNeedNewline) dest += "\n";
        newState |= stateNeedNewline;
        break;
      case '\t':
      case ' ':
        str++;
        break;
      case '#':
        if (state & stateNeedNewline)
        {
          dest += "\n";
          ApplyIndentation (-1);
          newState = statePreprocessor;
        }
        else
        {
          dest += ch;
          str++;
        }
        break;
      case '/':
        {
          char next = *(str+1);
          if ((next == '/') || (next == '*'))
          {
            BeginComment (ch, next, str, newState);
          }
          else
          {
            dest += ch;
            str++;
          }
        }
        break;
      default:
        newState = stateStatement;
        InsertNewlineIfNeeded ();
        break;
    }
    return newState;
  }

  int CgBeautifier::HandlePreprocessorChar (const char*& str, char ch)
  {
    int newState = state;
    switch (ch)
    {
      case '\r':
        if (*(str+1) == '\n') str++;
      case '\n':
        str++;
        newState = stateBetween | stateNeedNewline;
        break;
      default:
        dest += ch;
        str++;
        break;
    }
    return newState;
  }

  void CgBeautifier::Append (const char* str)
  {
    char ch;
    while ((ch = *str) != 0)
    {
      if ((state & stateCommentMask) != 0)
      {
        state = HandleCommentChar (str, state & stateCommentMask, ch);
      }
      else
      {
        switch (state & ~stateNeedNewline)
        {
          case stateBetween:
            state = HandleBetweenChar (str, ch);
            break;
          case stateStatement:
            state = HandleStatementChar (str, ch);
            break;
          case statePreprocessor:
            state = HandlePreprocessorChar (str, ch);
            break;
        }
      }
    }

  }

  void CgBeautifier::BeginComment (char ch, char next, const char*& str, 
                                   int& newState)
  {
    if (next == '/')
    {
      InsertNewlineIfNeeded (true);
      dest += "//";
      str += 2;
      newState |= stateCommentLine;
    }
    else if (next == '*')
    {
      InsertNewlineIfNeeded (true);
      dest += "/*";
      str += 2;
      newState |= stateCommentBlock;
    }
  }

  void CgBeautifier::ApplyIndentation (int delta)
  {
    for (int i = 0; i < indent+delta; i++)
      dest += "  ";
  }

}
CS_PLUGIN_NAMESPACE_END(GLShaderCg)
