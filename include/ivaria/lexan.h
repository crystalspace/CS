/*
    Copyright (C) 2001 by Christopher Nelson
  
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

#ifndef __IVARIA_LEXAN_H__
#define __IVARIA_LEXAN_H__

#include "iutil/databuff.h"
#include "iutil/string.h"

SCF_VERSION (iRegExp, 0, 0, 1);

/// Interface definition for a bytecode regular expression
struct iRegExp : public iBase
{
  /// Gets an opcode at the given index.  Returns false if the index is invalid
  virtual bool GetOp(unsigned index, unsigned char &op)=0;
 	
  /** Sets the opcode at the given index.  Grows the code buffer if the index
   * is greater than the length.  Returns true on success.  Returns false if
   * index is an absurd value (currently > 0xfff)
   */
  virtual bool SetOp(unsigned index, unsigned char op)=0;
  
  /** Fixes the code buffer so that it fits the size of the regular expression
   * exactly. Returns true on success
   */
  virtual bool Compact()=0;
};


SCF_VERSION (iRegExpCompiler, 0, 0, 1);

/// Interface definition for a regular expression compiler
struct iRegExpCompiler : public iBase
{
  /// Compiles a character string into a regular expression.  The compiled RE is placed in "re"
  virtual bool Compile(char *regexp, iRegExp &re, unsigned int start=0, unsigned int end=0)=0;
  
  /// Returns the last error.  Only valid when Compile returns false.
  virtual unsigned int GetError()=0;
};


SCF_VERSION (iLexicalAnalyzer, 0, 0, 1);

/// The interface to the lexical analyzer / regular expression virtual machine
struct iLexicalAnalyzer : public iBase
{
  /// Register a regular expression for matching
  virtual bool RegisterRegExp(unsigned int key, iRegExp &re)=0;

  /// Unregister a previously registered expression
  virtual bool UnregisterRegExp(unsigned int key)=0;

  /// Requests a new (free) key
  virtual unsigned int GetFreeKey()=0;
  
  /// Returns the last error.  Value is only valid when a member function has returned false.
  virtual unsigned int GetError() = 0;
  
  /// Returns the key of the last RE matched
  virtual unsigned int GetMatchedKey()=0;
  
  /// Returns a pointer to the last matched text
  virtual iString *GetMatchedText()=0;
  
  /// Saves the current input stream and state, then resets the current state and uses stream from buf
  virtual bool PushStream(iDataBuffer &buf)=0;
  
  /// Replaces the current input stream and state with the next stream and state on the stack
  virtual bool PopStream()=0;
  
  /// Scans the input stream for a match by one of the RegularExpression's returns 0 on failure, otherwise returns the key of the matched RE.
  virtual unsigned int Match() = 0;
};

#endif // __IVARIA_LEXAN_H__
