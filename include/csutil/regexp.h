/*
    Copyright (C) 2004 by Frank Richter

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

#ifndef __CS_UTIL_REGEXP_H__
#define __CS_UTIL_REGEXP_H__

/**\file
 * Regular expressions support.
 */

#include "csextern.h"
#include "csutil/array.h"

/**
 * Possible errors that can occur during matching.
 */
enum csRegExpMatchError
{
  /// No problems during matching.
  csrxNoError,
  /// The pattern didn't match the string.
  csrxNoMatch,
  /**
   * There was an invalid \c \\{...\\} construct in the regular expression. 
   * A valid \c \\{...\\} construct must contain either a single number, or 
   * two numbers in increasing order separated by a comma.
   */
  csrxBadBraces,
  /**
   * There was a syntax error in the regular expression. 
   */
  csrxBadPattern,
  /**
   * A repetition operator such as ? or * appeared in a bad position (with no 
   * preceding subexpression to act on). 
   */
  csrxBadRepetition,
  /**
   * The regular expression referred to an invalid collating element (one not 
   * defined in the current locale for string collation).
   */
  csrxErrCollate,
  /**
   * The regular expression referred to an invalid character class name. 
   */
  csrxErrCharType,
  /** 
   * The regular expression ended with \c \\. 
   */
  csrxErrEscape,
  /**
   * There was an invalid number in the \c \\digit construct. 
   */
  csrxErrSubReg,
  /**
   * There were unbalanced square brackets in the regular expression. 
   */
  csrxErrBrackets,
  /**
   * An extended regular expression had unbalanced parentheses, or a basic 
   * regular expression had unbalanced \c \\( and \c \\).
   */
  csrxErrParentheses,
  /**
   * The regular expression had unbalanced \c \\{ and \c \\}. 
   */
  csrxErrBraces,
  /**
   * One of the endpoints in a range expression was invalid. 
   */
  csrxErrRange,
  /**
   * Out of memory.
   */
  csrxErrSpace,
  /**
   * Unknown error.
   */
  csrxErrUnknown
};

/**
 * Flags for regular expression matching
 */
enum csRegExpMatchFlags
{
  /**
   * Ignore case when matching letters.
   */
  csrxIgnoreCase = 1,
  /**
   * Treat a newline in string as dividing string into multiple lines, so that 
   * \c $ can match before the newline and \c ^ can match after. Also, don't 
   * permit \c . to match a newline, and don't permit \c [^...] to match a 
   * newline.
   *
   * Otherwise, newline acts like any other ordinary character. 
   */
  csrxNewLine = 2,
  /**
   * Do not regard the beginning of the specified string as the beginning of a 
   * line; more generally, don't make any assumptions about what text might 
   * precede it. 
   */
  csrxNotBOL = 4,
  /**
   * Do not regard the end of the specified string as the end of a line; more 
   * generally, don't make any assumptions about what text might follow it.
   */
  csrxNotEOL = 8
};

/**
 * Information about (sub)expression matches.
 */
struct CS_CRYSTALSPACE_EXPORT csRegExpMatch
{
  /// Offset to the match from the beginning of the string to match against.
  size_t startOffset;
  /** 
   * Offset to the end of the match from the beginning of the string to match 
   * against.
   */
  size_t endOffset;
};

/**
 * Matcher for regular expressions.
 * \note Implementation note: if the platform supports it, this class wraps
 *  the runtime libraries POSIX.2 regular expression interface; otherwise,
 *  it uses a built-in implementation of POSIX.2 regular expression (borrowed
 *  from glibc.) Both of them utiltize "compilation" of pattern for faster
 *  repeated matching of the same pattern. Although compilation is transparent,
 *  you should be aware that calls to Match() with different flags than 
 *  previous calls cause a recompilation of the pattern. Thus, to improve 
 *  performance, same \p flags should be used for subsequent calls to Match().
 */
class CS_CRYSTALSPACE_EXPORT csRegExpMatcher
{
  void* regex;
  char* pattern;
  int compiledFlags;
  csRegExpMatchError compileError;
  bool extendedRE;
  
  bool Compile (int flags, bool nosub);

public:
  /**
   * Create a new RE matcher.
   * \param pattern Pattern to match against.
   * \param extendedRE Treat the pattern as an extended regular expression, 
   *   rather than as a basic regular expression. 
   */
  csRegExpMatcher (const char* pattern, bool extendedRE = false);

  /// Copy constructor
  csRegExpMatcher (const csRegExpMatcher& other);

  /// Destructor.
  ~csRegExpMatcher ();
  
  /// Assignment operator
  csRegExpMatcher& operator= (const csRegExpMatcher &other);

  /**
   * Match a string against the pattern.
   * \param string String against which to attempt match.
   * \param flags One or more of csRegExpMatchFlags.  Flags are combined using
   *   the bitwise-or \p | operator.
   * \return csrxNoError in case of success, else an error code.
   * \note Also check the "Implementation note" in the csRegExpMatcher
   *  description.
   */
  csRegExpMatchError Match (const char* string, int flags = 0);

  /**
   * Match a string against the pattern.
   * \param string String against which to attempt match.
   * \param matches Array receiving the locations of individual
   *  (sub)expression matches.
   * \param flags One or more of csRegExpMatchFlags.  Flags are combined using
   *   the bitwise-or \p | operator.
   * \return csrxNoError in case of success, else an error code.
   * \note Also check the "Implementation note" in the csRegExpMatcher
   *  description.
   */
  csRegExpMatchError Match (const char* string, 
    csArray<csRegExpMatch>& matches, int flags = 0);
};

#endif // __CS_UTIL_REGEXP_H__
