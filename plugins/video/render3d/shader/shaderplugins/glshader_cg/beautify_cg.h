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

#ifndef __BEAUTIFY_CG_H__
#define __BEAUTIFY_CG_H__

#include "csutil/csstring.h"

CS_PLUGIN_NAMESPACE_BEGIN(GLShaderCg)
{
  class CgBeautifier
  {
    csString& dest;

    int indent;
    /* Keep track of the "statement number" of the current line and current
       statement. If they're equal at the time of a line break it means a
       statement spans multiple lines - use an extra indent then. */
    int lineStatement;
    int currentStatement;

    int state;
    enum
    {
      stateBetween = 0,
      stateStatement = 1,
      statePreprocessor = 2,

      stateCommentLine = 0x10,
      stateCommentBlock = 0x20,
      stateCommentMask = stateCommentLine | stateCommentBlock,

      stateNeedNewline = 0x100
    };
    int HandleCommentChar (const char*& str, int commentType, char ch);
    int HandleStatementChar (const char*& str, char ch);
    int HandleBetweenChar (const char*& str, char ch);
    int HandlePreprocessorChar (const char*& str, char ch);

    void BeginComment (char ch, char next, const char*& str, int& newState);
    void InsertNewlineIfNeeded (bool keepStatementCounter = false)
    {
      if (state & stateNeedNewline)
      {
        dest += "\n";
        ApplyIndentation ((lineStatement != currentStatement) ? 0 : 1);
        if (!keepStatementCounter) lineStatement = currentStatement;
        state &= ~stateNeedNewline;
      }
    }
    void ApplyIndentation (int delta = 0);
    void SkipSpaces (const char*& str);
  public:
    CgBeautifier (csString& dest);
  
    void Append (const char* str);
  };
}
CS_PLUGIN_NAMESPACE_END(GLShaderCg)

#endif // __BEAUTIFY_CG_H__
