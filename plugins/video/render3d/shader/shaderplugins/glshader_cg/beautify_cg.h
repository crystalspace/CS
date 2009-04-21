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

    // Indentation level
    int indent;
    /* Keep track of the "statement number" of the current line and current
       statement. If they're equal at the time of a line break it means a
       statement spans multiple lines - use an extra indent then. */
    int lineStatement;
    int currentStatement;

    int state;
    enum
    {
      /* "Between" two statements - after a ;, {, or }, but before some
         other character was encountered. Skips whitespace and indents when
         (basically) a non-whitespace was found. */
      stateBetween = 0,
      /* In a statement - just output chars until the end (with some 
         indentation when a newline is encountered). */
      stateStatement = 1,
      /* In a preprocessor statement - just copy chars until the end of the
         line. */
      statePreprocessor = 2,

      /* Comment flags, when currently in a comment. They're flags so
         the state before the comment can be restored.
       */
      stateCommentLine = 0x10,
      stateCommentBlock = 0x20,
      stateCommentMask = stateCommentLine | stateCommentBlock,

      /* Set when a newline was encountered between statements; used to 
         properly emit a newline before the start of the next statement. */
      stateNeedNewline = 0x100
    };
    int HandleCommentChar (const char*& str, int commentType, char ch);
    int HandleStatementChar (const char*& str, char ch);
    int HandleBetweenChar (const char*& str, char ch);
    int HandlePreprocessorChar (const char*& str, char ch);

    void BeginComment (char ch, char next, const char*& str, int& newState);
    /* Insert newline, if needed. \a keepStatementCounter can be set to avoid
       keep the special indentation level for multi-line statements. */
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
    // Apply indentation according to current indent level, plus minus delta.
    void ApplyIndentation (int delta = 0);
  public:
    CgBeautifier (csString& dest);
  
    void Append (const char* str);
  };
}
CS_PLUGIN_NAMESPACE_END(GLShaderCg)

#endif // __BEAUTIFY_CG_H__
