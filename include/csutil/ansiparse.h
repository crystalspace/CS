/*
    Copyright (C) 2005 by Jorrit Tyberghein
	      (C) 2005 by Frank Richter

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

#ifndef __CS_CSUTIL_ANSIPARSE_H__
#define __CS_CSUTIL_ANSIPARSE_H__

/**\file
 * Helper to parse a string for ANSI codes.
 */
 
/**\addtogroup util
 * @{ */

/// Helper to parse a string for ANSI codes.
class csAnsiParser
{
public:
  /// Identifier for the ANSI command
  enum Command
  {
    /// Command was unrecognized
    cmdUnknown,
    /// Reset all attributes
    cmdFormatAttrReset,
    /// 'Enable attribute' 
    cmdFormatAttrEnable,
    /// 'Disable attribute'
    cmdFormatAttrDisable,
    /// 'Set foreground color'
    cmdFormatAttrForeground,
    /// 'Set background color'
    cmdFormatAttrBackground,
  };
  /// Classification of the command sequence
  enum CommandClass
  {
    /// No ANSI sequence was found
    classNone,
    /// An ANSI sequence was found, but not recognized.
    classUnknown,
    /// A formatting sequence was found.
    classFormat
  };
  /**
   * Types of attributes in the cmdFormatAttrEnable/cmdFormatAttrBackground 
   * command
   */
  enum FormatAttr
  {
    /// 'Bold' attribute
    attrBold,
    /// 'Italics' attribute
    attrItalics,
    /// 'Underline' attribute
    attrUnderline,
    /// 'Blink' attribute
    attrBlink,
    /// 'Reverse' attribute
    attrReverse,
    /// 'Strikethrough' attribute
    attrStrikethrough,
    /// 'Dim' attribute
    attrDim,
    /// 'Invisible' attribute
    attrInvisible,
  };
  /// Values for foreground/background color
  enum FormatColor
  {
    /// None specified
    colNone = -1,
    /// Black
    colBlack,
    /// Red
    colRed,
    /// Green
    colGreen,
    /// Yellow
    colYellow,
    /// Blue
    colBlue,
    /// Magenta
    colMagenta,
    /// Cyan
    colCyan,
    /// White
    colWhite
  };
  /// Parameters to ANSI command
  struct CommandParams
  {
    union
    {
      /// Color for cmdFormatAttrForeground and cmdFormatAttrBackground commands
      FormatColor colorVal;
      /// Attribute for cmdFormatAttrEnable and cmdFormatAttrDisable commands
      FormatAttr attrVal;
    };
  };
  /**
   * Parse a string for ANSI codes.
   * Looks if a string contains an ANSI code sequence at the beginning.
   * If yes, the ansiCommandLen parameter is filled with the length of the
   * sequence. textLen contains the number of chars up to the next ANSI
   * sequence or the end of the string of no sequence is found.
   */
  static bool CS_CSUTIL_EXPORT ParseAnsi (const char* str,
    size_t& ansiCommandLen, CommandClass& cmdClass, size_t& textLen);
  /**
   * Decode an ANSI code sequence.
   * Decodes a part of an ANSI code sequence, if known. Returns whether the
   * decoding was was successful. \a cmd is updated to point to the start
   * of the next sequence part.
   * \remark Multiple sequences might occur, repeated call this function
   *  until <tt>false</tt> is returned.
   */
  static bool CS_CSUTIL_EXPORT DecodeCommand (const char*& cmd,
    size_t& cmdLen, Command& command, CommandParams& commandParams);
};

/** @} */

#endif // __CS_CSUTIL_ANSIPARSE_H__
