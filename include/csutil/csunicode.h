/*
    Copyright (C) 2003 by Frank Richter

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

#ifndef __CS_CSUNICODE_H__
#define __CS_CSUNICODE_H__

/**\file 
 * Unicode types and macros.
 */
 
/**\addtogroup util
 * @{ */

#if defined(CS_HAS_WCHAR_H)
#include <wchar.h>
#endif

#if defined(CS_USE_FAKE_WCHAR_TYPE)
typedef uint16 wchar_t;
#define CS_WCHAR_T_SIZE 2
#endif

#if !defined(CS_WCHAR_T_SIZE)
#  error Unknown wchar_t size.
#elif (CS_WCHAR_T_SIZE != 1) && (CS_WCHAR_T_SIZE != 2) && (CS_WCHAR_T_SIZE != 4)
#  error Unsupported wchar_t size.
#endif

/// A single char in a UTF8 encoded string.
typedef uint8 utf8_char;
/// A single char in a UTF16 encoded string.
typedef uint16 utf16_char;
/// A single char in a UTF32 encoded string.
typedef uint32 utf32_char;
 
/**\name Special character codes
 * @{ */
/// Replacer char
#define CS_UC_CHAR_REPLACER		0xFFFD
/// Invalid char
#define CS_UC_INVALID			0xFFFF
/// Byte-order mark
#define CS_UC_BOM			0xFFFE
/// Highest valid Unicode codepoint
#define CS_UC_LAST_CHAR			0x10FFFF

/// Check whether a code is in the "high" or "low surrogate" range.
#define CS_UC_IS_SURROGATE(C)		(((C) & 0xFFFFF800) == 0x0000D800)
/// Check whether a code is in the "high surrogate" range.
#define CS_UC_IS_HIGH_SURROGATE(C)	(((C) & 0xFFFFFC00) == 0x0000DC00)
/// Check whether a code is in the "low surrogate" range.
#define CS_UC_IS_LOW_SURROGATE(C)	(((C) & 0xFFFFFC00) == 0x0000D800)

/// Test whether a code point is a Unicode noncharacter
#define CS_UC_IS_NONCHARACTER(C)	(((C) == CS_UC_INVALID) || ((C) == CS_UC_BOM) \
					 || (((C) >= 0xFDD0) && ((C) <= 0xFDEF))      \
					 || ((C) > CS_UC_LAST_CHAR))

/// First char in the "high surrogate" range.
#define CS_UC_CHAR_HIGH_SURROGATE_FIRST		0xD800
/// Last char in the "high surrogate" range.
#define CS_UC_CHAR_HIGH_SURROGATE_LAST 		0xDBFF
/// First char in the "low surrogate" range.
#define CS_UC_CHAR_LOW_SURROGATE_FIRST		0xDC00
/// Last char in the "low surrogate" range.
#define CS_UC_CHAR_LOW_SURROGATE_LAST 		0xDFFF

/** @} */

/** @} */

#endif
