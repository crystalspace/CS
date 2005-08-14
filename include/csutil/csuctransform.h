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

#ifndef __CS_CSUCTRANSFORM_H__
#define __CS_CSUCTRANSFORM_H__

#include "csunicode.h"

/**\file 
 * Converter between UTF encodings.
 */
 
/**\addtogroup util
 * @{ */

/**
 * An Unicode code point as UTF8 is at maximum encoded to this number of code 
 * units.
 */
#define CS_UC_MAX_UTF8_ENCODED		4  /* 6 to encode 32 bit */
/**
 * An Unicode code point as UTF16 is at maximum encoded to this number of code 
 * units.
 */
#define CS_UC_MAX_UTF16_ENCODED		2
/**
 * An Unicode code point as UTF32 is at maximum encoded to this number of code 
 * units.
 */
#define CS_UC_MAX_UTF32_ENCODED		1
/**
 * A complex mapping (uppercase, lowercase, fold) for a Unicode code point 
 * expands to at maximum this number of code points 
 */
#define CS_UC_MAX_MAPPED		3

/**
 * Flags influencing the behaviour of MapToUpper, MapToLower and MapToFold
 */
enum
{
  /**
   * Force 'simple' mappings, that is, at most one code point is returned.
   * The default 'complex' mappings can return more than one code point in
   * some cases. \sa #CS_UC_MAX_MAPPED
   */
  csUcMapSimple = (1 << 0)
};

/**
 * Contains functions to convert between several UTF encodings.
 */
class csUnicodeTransform
{
public:
#define FAIL(ret)				\
  {						\
    if (isValid) *isValid = false;		\
    ch = CS_UC_CHAR_REPLACER;			\
    return ret;					\
  }

#define SUCCEED					\
    if (isValid) *isValid = true;		\
    return chUsed;
  
#define GET_NEXT(next)	\
  if ((size_t)chUsed == strlen)			\
  {						\
    FAIL(chUsed);				\
  }						\
  next = *str++;				\
  if (next == 0)				\
  {						\
    FAIL(chUsed);				\
  }						\
  chUsed++; 					
  
  /**\name UTF Decoders
   * @{ */
  /**
   * Decode an Unicode code point encoded in UTF-8.
   * \param str Pointer to the encoded code point.
   * \param strlen Number of code units in the source string.
   * \param ch Decoded code point.
   * \param isValid When an error occured during decoding, \p ch contains the
   *  replacement character (#CS_UC_CHAR_REPLACER). In this case, the bool
   *  pointed to by \p isValid will be set to false. The parameter can be 0,
   *  but in this case the information whether the decoded char is the 
   *  replacement character because the source data is errorneous is lost.
   * \param returnNonChar Whether decoded non-character or high and low 
   *  surrogates are returned as such. Normally, those code points are replaced
   *  with #CS_UC_CHAR_REPLACER to signal an invalid encoded code point.
   * \return The number of code units in \a str that have to be skipped to 
   *  retrieve the next encoded code point.
   */
  inline static int UTF8Decode (const utf8_char* str, size_t strlen, 
    utf32_char& ch, bool* isValid = 0, bool returnNonChar = false)
  {
    if (str == 0)
    {
      FAIL(0);
    }
    int chUsed = 0;
    
    utf8_char curCh;
    GET_NEXT(curCh);
    if ((curCh & 0x80) == 0)
    {
      // easy case
      ch = curCh;
      SUCCEED;
    }
    else
    {
      // Count with how many bytes this char is encoded.
      int n = 0;
      while ((n < 7) && ((curCh & (1 << (7 - n))) != 0)) { n++; }

      if ((n < 2) || (n > 6))
      {
	// Invalid code: first char of a "sequence" must have
	// at least two and at most six MSBs set
	FAIL(1);
      }

      ch = (curCh & ((1 << (8 - n)) - 1));
      
      for (int i = 1; i < n; i++)
      {
	GET_NEXT(curCh);
	if ((curCh & 0xc0) != 0x80)
	{
	  FAIL(chUsed);
	}
	else
	{
	  ch <<= 6;
	  ch |= (curCh & 0x3f);
	}
      }
      
      // Check if in Unicode range.
      if (ch > CS_UC_LAST_CHAR)
      {
        FAIL(chUsed);
      }

      // Check for "overlong" codes.
      if ((ch < 0x80) && (n > 0))
      {
        FAIL(chUsed);
      }
      else if ((ch < 0x800) && (n > 2))
      {
        FAIL(chUsed);
      }
      else if ((ch < 0x10000) && (n > 3))
      {
        FAIL(chUsed);
      }
      else if ((ch < 0x200000) && (n > 4))
      {
        FAIL(chUsed);
      }
      /* 
      else if ((ch < 0x4000000) && (n > 5))
      {
        FAIL(chUsed);
      }
      else if ((ch < 0x80000000) && (n > 6))
      {
        FAIL(chUsed);
      }
      */
      
      if (!returnNonChar && (CS_UC_IS_NONCHARACTER(ch) 
	|| CS_UC_IS_SURROGATE(ch)))
	FAIL(chUsed);
      SUCCEED;
    }
  }
  
  /**
   * Decode an Unicode code point encoded in UTF-16.
   * \copydoc UTF8Decode(const utf8_char*,size_t,utf32_char&,bool*,bool)
   */
  inline static int UTF16Decode (const utf16_char* str, size_t strlen, 
    utf32_char& ch, bool* isValid = 0, bool returnNonChar = false)
  {
    if (str == 0)
    {
      FAIL(0);
    }
    int chUsed = 0;
    
    utf16_char curCh;
    GET_NEXT(curCh);
    // Decode surrogate
    if (CS_UC_IS_SURROGATE (curCh))
    {
      // Invalid code
      if (!CS_UC_IS_HIGH_SURROGATE (curCh))
      {
	FAIL(chUsed);
      }
      ch = 0x10000 + ((curCh & 0x03ff) << 10);
      GET_NEXT(curCh);
      // Invalid code
      if (!CS_UC_IS_LOW_SURROGATE (curCh))
      {
        // Fail with 1 so the char is handled upon the next Decode.
	FAIL(1);
      }
      ch |= (curCh & 0x3ff);
    }
    else
    {
      ch = curCh;
    }
    if (!returnNonChar && (CS_UC_IS_NONCHARACTER(ch) 
      || CS_UC_IS_SURROGATE(ch)))
      FAIL(chUsed);
    SUCCEED;
  }
  
  /**
   * Decode an Unicode code point encoded in UTF-32.
   * \copydoc UTF8Decode(const utf8_char*,size_t,utf32_char&,bool*,bool)
   */
  inline static int UTF32Decode (const utf32_char* str, size_t strlen, 
    utf32_char& ch, bool* isValid = 0, bool returnNonChar = false)
  {
    if (str == 0)
    {
      FAIL(0);
    }
    int chUsed = 0;
    
    GET_NEXT(ch);
    if ((!returnNonChar && (CS_UC_IS_NONCHARACTER(ch) 
      || CS_UC_IS_SURROGATE(ch))) || (ch > CS_UC_LAST_CHAR))
      FAIL(chUsed);
    SUCCEED;
  }

  /**
   * Decode an Unicode code point encoded in UTF-8.
   * \copydoc UTF8Decode(const utf8_char*,size_t,utf32_char&,bool*,bool)
   */
  inline static int Decode (const utf8_char* str, size_t strlen, 
    utf32_char& ch, bool* isValid = 0, bool returnNonChar = false)
  {
    return UTF8Decode (str, strlen, ch, isValid, returnNonChar);
  }
  /**
   * Decode an Unicode code point encoded in UTF-16.
   * \copydoc UTF8Decode(const utf8_char*,size_t,utf32_char&,bool*,bool)
   */
  inline static int Decode (const utf16_char* str, size_t strlen, 
    utf32_char& ch, bool* isValid = 0, bool returnNonChar = false)
  {
    return UTF16Decode (str, strlen, ch, isValid, returnNonChar);
  }
  /**
   * Decode an Unicode code point encoded in UTF-32.
   * \copydoc UTF8Decode(const utf8_char*,size_t,utf32_char&,bool*,bool)
   */
  inline static int Decode (const utf32_char* str, size_t strlen, 
    utf32_char& ch, bool* isValid = 0, bool returnNonChar = false)
  {
    return UTF32Decode (str, strlen, ch, isValid, returnNonChar);
  }

  /** @} */
#undef FAIL
#undef SUCCEED
#undef GET_NEXT

  /**\name UTF Encoders
   * @{ */
#define _OUTPUT_CHAR(buf, chr)				\
  if (bufRemaining > 0)					\
  {							\
    if(buf) *buf++ = chr;				\
    bufRemaining--;					\
  }							\
  encodedLen++;

#define OUTPUT_CHAR(chr) _OUTPUT_CHAR(buf, chr)
  
  /**
   * Encode an Unicode code point to UTF-8.
   * \param ch Code point to encode.
   * \param buf Pointer to the buffer receiving the encoded code point.
   * \param bufsize Number of code units that fit in \a buf.
   * \param allowNonchars Whether non-character or high and low surrogates
   *  are encoded. Normally, those code points are rejected to prevent the
   *  generation of invalid encoded strings.
   * \return The number of code units needed to encode \p ch.
   * \remark The buffer will be filled up as much as possible.
   *  Check the returned value whether the encoded code point completely fit 
   *  into the buffer.
   */
  inline static int EncodeUTF8 (const utf32_char ch, utf8_char* buf, 
    size_t bufsize, bool allowNonchars = false)
  {
    if ((!allowNonchars && ((CS_UC_IS_NONCHARACTER(ch)) 
      || (CS_UC_IS_SURROGATE(ch)))) || (ch > CS_UC_LAST_CHAR))
      return 0;
    size_t bufRemaining = bufsize;
    int encodedLen = 0;
    
    if (ch < 0x80)
    {
      OUTPUT_CHAR ((utf8_char)ch);
    }
    else if (ch < 0x800)
    {
      OUTPUT_CHAR ((utf8_char)(0xc0 | (ch >> 6)));
      OUTPUT_CHAR ((utf8_char)(0x80 | (ch & 0x3f)));
    }
    else if (ch < 0x10000)
    {
      OUTPUT_CHAR ((utf8_char)(0xe0 | (ch >> 12)));
      OUTPUT_CHAR ((utf8_char)(0x80 | ((ch >> 6) & 0x3f)));
      OUTPUT_CHAR ((utf8_char)(0x80 | (ch & 0x3f)));
    }
    else if (ch < 0x200000)
    {
      OUTPUT_CHAR ((utf8_char)(0xf0 | (ch >> 18)));
      OUTPUT_CHAR ((utf8_char)(0x80 | ((ch >> 12) & 0x3f)));
      OUTPUT_CHAR ((utf8_char)(0x80 | ((ch >> 6) & 0x3f)));
      OUTPUT_CHAR ((utf8_char)(0x80 | (ch & 0x3f)));
    }
    /*
    else if (ch < 0x4000000)
    {
      OUTPUT_CHAR ((utf8_char)(0xf8 | (ch >> 24)));
      OUTPUT_CHAR ((utf8_char)(0x80 | ((ch >> 18) & 0x3f)));
      OUTPUT_CHAR ((utf8_char)(0x80 | ((ch >> 12) & 0x3f)));
      OUTPUT_CHAR ((utf8_char)(0x80 | ((ch >> 6) & 0x3f)));
      OUTPUT_CHAR ((utf8_char)(0x80 | (ch & 0x3f)));
    }
    else if (ch < 0x80000000)
    {
      OUTPUT_CHAR ((utf8_char)(0xfc | (ch >> 30)));
      OUTPUT_CHAR ((utf8_char)(0x80 | ((ch >> 24) & 0x3f)));
      OUTPUT_CHAR ((utf8_char)(0x80 | ((ch >> 18) & 0x3f)));
      OUTPUT_CHAR ((utf8_char)(0x80 | ((ch >> 12) & 0x3f)));
      OUTPUT_CHAR ((utf8_char)(0x80 | ((ch >> 6) & 0x3f)));
      OUTPUT_CHAR ((utf8_char)(0x80 | (ch & 0x3f)));
    }
    */
    return encodedLen;
  }
    
  /**
   * Encode an Unicode code point to UTF-16.
   * \copydoc EncodeUTF8(const utf32_char,utf8_char*,size_t,bool)
   */
  inline static int EncodeUTF16 (const utf32_char ch, utf16_char* buf, 
    size_t bufsize, bool allowNonchars = false)
  {
    if ((!allowNonchars && ((CS_UC_IS_NONCHARACTER(ch)) 
      || (CS_UC_IS_SURROGATE(ch)))) || (ch > CS_UC_LAST_CHAR))
      return 0;
    size_t bufRemaining = bufsize;
    int encodedLen = 0;
    
    if (ch < 0x10000)
    {
      OUTPUT_CHAR((utf16_char)ch);
    }
    else if (ch < 0x100000)
    {
      utf32_char ch_shifted = ch - 0x10000;
      OUTPUT_CHAR((utf16_char)((ch_shifted >> 10) 
	| CS_UC_CHAR_HIGH_SURROGATE_FIRST));
      OUTPUT_CHAR((utf16_char)((ch_shifted & 0x3ff) 
	| CS_UC_CHAR_LOW_SURROGATE_FIRST));
    }
    else
      return 0;
    
    return encodedLen;
  }

  /**
   * Encode an Unicode code point to UTF-32.
   * \copydoc EncodeUTF8(const utf32_char,utf8_char*,size_t,bool)
   */
  inline static int EncodeUTF32 (const utf32_char ch, utf32_char* buf, 
    size_t bufsize, bool allowNonchars = false)
  {
    if ((!allowNonchars && ((CS_UC_IS_NONCHARACTER(ch)) 
      || (CS_UC_IS_SURROGATE(ch)))) || (ch > CS_UC_LAST_CHAR))
      return 0;
    size_t bufRemaining = bufsize;
    int encodedLen = 0;
    
    OUTPUT_CHAR(ch);
    
    return encodedLen;
  }

  /**
   * Encode an Unicode code point to UTF-8.
   * \copydoc EncodeUTF8(const utf32_char,utf8_char*,size_t,bool)
   */
  inline static int Encode (const utf32_char ch, utf8_char* buf, 
    size_t bufsize, bool allowNonchars = false)
  {
    return EncodeUTF8 (ch, buf, bufsize, allowNonchars);
  }
  /**
   * Encode an Unicode code point to UTF-16.
   * \copydoc EncodeUTF8(const utf32_char,utf8_char*,size_t,bool)
   */
  inline static int Encode (const utf32_char ch, utf16_char* buf, 
    size_t bufsize, bool allowNonchars = false)
  {
    return EncodeUTF16 (ch, buf, bufsize, allowNonchars);
  }
  /**
   * Encode an Unicode code point to UTF-32.
   * \copydoc EncodeUTF8(const utf32_char,utf8_char*,size_t,bool)
   */
  inline static int Encode (const utf32_char ch, utf32_char* buf, 
    size_t bufsize, bool allowNonchars = false)
  {
    return EncodeUTF32 (ch, buf, bufsize, allowNonchars);
  }
  /** @} */
#undef OUTPUT_CHAR
  
  /**\name Converters between strings in different UTF encodings
   * @{ */
#define OUTPUT_CHAR(chr) _OUTPUT_CHAR(dest, chr)
  
#define UCTF_CONVERTER(funcName, fromType, decoder, toType, encoder)	\
  inline static size_t funcName (toType* dest, size_t destSize, 	\
    const fromType* source, size_t srcSize = (size_t)-1)		\
  {									\
    if ((srcSize == 0) || (source == 0))				\
      return 0;								\
									\
    size_t bufRemaining = (destSize > 0) ? destSize - 1 : 0;		\
    size_t encodedLen = 0;						\
									\
    size_t srcChars = srcSize;						\
									\
    if (srcSize == (size_t)-1)						\
    {									\
      srcChars = 0;							\
      const fromType* sptr = source;					\
      while (*sptr++ != 0) srcChars++;					\
    }									\
									\
    while (srcChars > 0)						\
    {									\
      utf32_char ch;							\
      int scnt = decoder (source, srcChars, ch, 0);			\
      if (scnt == 0) break;						\
      int dcnt = encoder (ch, dest, bufRemaining);			\
      if (dcnt == 0) 							\
      {									\
      	dcnt = encoder (CS_UC_CHAR_REPLACER, dest, bufRemaining);	\
      }									\
									\
      if ((size_t)dcnt >= bufRemaining) 				\
      {									\
	if (dest && (destSize > 0)) dest += bufRemaining;		\
        bufRemaining = 0;						\
      }									\
      else								\
      {									\
        bufRemaining -= dcnt;						\
	if (dest && (destSize > 0)) dest += dcnt;			\
      }									\
      encodedLen += dcnt;						\
      if ((size_t)scnt >= srcChars) break;				\
      srcChars -= scnt;							\
      source += scnt;							\
    }									\
									\
    if (dest) *dest = 0;						\
									\
    return encodedLen + 1;						\
  }

  /**
   * Convert UTF-8 to UTF-16.
   * \param dest Destination buffer.
   * \param destSize Number of code units the destination buffer can hold.
   * \param source Source buffer.
   * \param srcSize Number of code units contained in the source buffer.
   *   If this is -1, the length will be determined automatically.
   * \return Number of code units in the complete converted string, including 
   *  null terminator.
   * \remark If the complete converted string wouldn't fit the destination 
   *  buffer, it is truncated. However, it'll also be null-terminated. Hence,
   *  if it has a size of 1, you get an empty string.
   *  The returned value is the number of code units needed for the *whole* 
   *  converted string.
   */
  UCTF_CONVERTER (UTF8to16, utf8_char, UTF8Decode, utf16_char, EncodeUTF16);
  /**
   * Convert UTF-8 to UTF-32.
   * \copydoc UTF8to16(utf16_char*,size_t,const utf8_char*,size_t)
   */
  UCTF_CONVERTER (UTF8to32, utf8_char, UTF8Decode, utf32_char, EncodeUTF32);

  /**
   * Convert UTF-16 to UTF-8.
   * \copydoc UTF8to16(utf16_char*,size_t,const utf8_char*,size_t)
   */
  UCTF_CONVERTER (UTF16to8, utf16_char, UTF16Decode, utf8_char, EncodeUTF8);
  /**
   * Convert UTF-16 to UTF-32.
   * \copydoc UTF8to16(utf16_char*,size_t,const utf8_char*,size_t)
   */
  UCTF_CONVERTER (UTF16to32, utf16_char, UTF16Decode, utf32_char, EncodeUTF32);
  
  /**
   * Convert UTF-32 to UTF-8.
   * \copydoc UTF8to16(utf16_char*,size_t,const utf8_char*,size_t)
   */
  UCTF_CONVERTER (UTF32to8, utf32_char, UTF32Decode, utf8_char, EncodeUTF8);
  /**
   * Convert UTF-32 to UTF-16.
   * \copydoc UTF8to16(utf16_char*,size_t,const utf8_char*,size_t)
   */
  UCTF_CONVERTER (UTF32to16, utf32_char, UTF32Decode, utf16_char, EncodeUTF16);
  /** @} */
  
#undef UCTF_CONVERTER
#undef OUTPUT_CHAR
#undef _OUTPUT_CHAR

#if (CS_WCHAR_T_SIZE == 1)
  inline static size_t UTF8toWC (wchar_t* dest, size_t destSize, 
    const utf8_char* source, size_t srcSize)
  {
    size_t srcChars = srcSize;						
    if (srcSize == (size_t)-1)						
    {									
      srcChars = 0;							
      const utf8_char* sptr = source;					
      while (*sptr++ != 0) srcChars++;					
    }				
    if ((dest != 0) && (destSize != 0))
    {
      size_t len = MIN (destSize - 1, srcChars);
      memcpy (dest, source, size * sizeof (wchar_t));
      *(dest + len) = 0;
    }
    return srcChars + 1;
  };

  inline static size_t UTF16toWC (wchar_t* dest, size_t destSize, 
    const utf16_char* source, size_t srcSize)
  {
    return UTF16to8 ((utf8_char*)dest, destSize, source, srcSize);
  };

  inline static size_t UTF32toWC (wchar_t* dest, size_t destSize, 
    const utf32_char* source, size_t srcSize)
  {
    return UTF32to8 ((utf8_char*)dest, destSize, source, srcSize);
  };
  
  inline static size_t WCtoUTF8 (utf8_char* dest, size_t destSize, 
    const wchar_t* source, size_t srcSize)
  {
    size_t srcChars = srcSize;						
    if (srcSize == (size_t)-1)						
    {									
      srcChars = 0;							
      const wchar_t* sptr = source;					
      while (*sptr++ != 0) srcChars++;					
    }				
    if ((dest != 0) && (destSize != 0))
    {
      size_t len = MIN (destSize - 1, srcChars);
      memcpy (dest, source, len * sizeof (wchar_t));
      *(dest + len) = 0;
    }
    return srcChars + 1;
  };

  inline static size_t WCtoUTF16 (utf16_char* dest, size_t destSize, 
    const wchar_t* source, size_t srcSize)
  {
    return UTF8to16 (dest, destSize, source, srcSize);
  };

  inline static size_t WCtoUTF32 (utf32_char* dest, size_t destSize, 
    const wchar_t* source, size_t srcSize)
  {
    return UTF8to32 (dest, destSize, source, srcSize);
  };

  inline static int Decode (const wchar_t* str, size_t strlen, 
    utf32_char& ch, bool* isValid = 0, bool returnNonChar = false)
  {
    return UTF8Decode ((utf8_char*)str, strlen, ch, isValid, returnNonChar);
  }
  inline static int Encode (const utf32_char ch, wchar_t* buf, 
    size_t bufsize, bool allowNonchars = false)
  {
    return EncodeUTF8 (ch, (utf8_char*)buf, bufsize, allowNonchars);
  }
#elif (CS_WCHAR_T_SIZE == 2)
  // Methods below for doxygen documentation are here as the size '2' is 
  // default.
  
  /**\name Converters UTF and platform-specific wchar_t
   * @{ */
  /**
   * Convert UTF-8 to platform-specific wide chars.
   * \copydoc UTF8to16(utf16_char*,size_t,const utf8_char*,size_t)
   */
  inline static size_t UTF8toWC (wchar_t* dest, size_t destSize, 
    const utf8_char* source, size_t srcSize)
  {
    return UTF8to16 ((utf16_char*)dest, destSize, source, srcSize);
  };

  /**
   * Convert UTF-16 to platform-specific wide chars.
   * \copydoc UTF8toWC(wchar_t*,size_t,const utf8_char*,size_t)
   */
  inline static size_t UTF16toWC (wchar_t* dest, size_t destSize, 
    const utf16_char* source, size_t srcSize)
  {
    size_t srcChars = srcSize;						
    if (srcSize == (size_t)-1)						
    {									
      srcChars = 0;							
      const utf16_char* sptr = source;					
      while (*sptr++ != 0) srcChars++;					
    }				
    if ((dest != 0) && (destSize != 0))
    {
      size_t len = MIN (destSize - 1, srcChars);
      memcpy (dest, source, len * sizeof (wchar_t));
      *(dest + len) = 0;
    }
    return srcChars + 1;
  };

  /**
   * Convert UTF-32 to platform-specific wide chars.
   * \copydoc UTF8toWC(wchar_t*,size_t,const utf8_char*,size_t)
   */
  inline static size_t UTF32toWC (wchar_t* dest, size_t destSize, 
    const utf32_char* source, size_t srcSize)
  {
    return UTF32to16 ((utf16_char*)dest, destSize, source, srcSize);
  };
  
  /**
   * Convert platform-specific wide chars to UTF-8.
   * \copydoc UTF8toWC(wchar_t*,size_t,const utf8_char*,size_t)
   */
  inline static size_t WCtoUTF8 (utf8_char* dest, size_t destSize, 
    const wchar_t* source, size_t srcSize)
  {
    return UTF16to8 (dest, destSize, (utf16_char*)source, srcSize);
  };

  /**
   * Convert platform-specific wide chars to UTF-16.
   * \copydoc UTF8toWC(wchar_t*,size_t,const utf8_char*,size_t)
   */
  inline static size_t WCtoUTF16 (utf16_char* dest, size_t destSize, 
    const wchar_t* source, size_t srcSize)
  {
    size_t srcChars = srcSize;						
    if (srcSize == (size_t)-1)						
    {									
      srcChars = 0;							
      const wchar_t* sptr = source;					
      while (*sptr++ != 0) srcChars++;					
    }				
    if ((dest != 0) && (destSize != 0))
    {
      size_t len = MIN (destSize - 1, srcChars);
      memcpy (dest, source, len * sizeof (wchar_t));
      *(dest + len) = 0;
    }
    return srcChars + 1;
  };

  /**
   * Convert platform-specific wide chars to UTF-32.
   * \copydoc UTF8toWC(wchar_t*,size_t,const utf8_char*,size_t)
   */
  inline static size_t WCtoUTF32 (utf32_char* dest, size_t destSize, 
    const wchar_t* source, size_t srcSize)
  {
    return UTF16to32 (dest, destSize, (utf16_char*)source, srcSize);
  };

#if !defined(CS_COMPILER_MSVC)
  /* @@@ For VC6, utf16_char == wchar_t, complains below. (Can be avoided on 
   * VC7 with "Builtin wchar_t" - but disabled for CS.) */
  /**
   * Decode an Unicode code point from wchar_t.
   * \copydoc UTF8Decode(const utf8_char*,size_t,utf32_char&,bool*,bool)
   */
  inline static int Decode (const wchar_t* str, size_t strlen, 
    utf32_char& ch, bool* isValid = 0, bool returnNonChar = false)
  {
    return UTF16Decode ((utf16_char*)str, strlen, ch, isValid, returnNonChar);
  }
  /**
   * Encode an Unicode code point to wchar_t.
   * \copydoc EncodeUTF8(const utf32_char,utf8_char*,size_t,bool)
   */
  inline static int Encode (const utf32_char ch, wchar_t* buf, 
    size_t bufsize, bool allowNonchars = false)
  {
    return EncodeUTF16 (ch, (utf16_char*)buf, bufsize, allowNonchars);
  }
#endif
  /** @} */
#elif (CS_WCHAR_T_SIZE == 4)
  inline static size_t UTF8toWC (wchar_t* dest, size_t destSize, 
    const utf8_char* source, size_t srcSize)
  {
    return UTF8to32 ((utf32_char*)dest, destSize, source, srcSize);
  };

  inline static size_t UTF16toWC (wchar_t* dest, size_t destSize, 
    const utf16_char* source, size_t srcSize)
  {
    return UTF16to32 ((utf32_char*)dest, destSize, source, srcSize);
  };

  inline static size_t UTF32toWC (wchar_t* dest, size_t destSize, 
    const utf32_char* source,  size_t srcSize)
  {
    size_t srcChars = srcSize;						
    if (srcSize == (size_t)-1)						
    {									
      srcChars = 0;							
      const utf32_char* sptr = source;					
      while (*sptr++ != 0) srcChars++;					
    }				
    if ((dest != 0) && (destSize != 0))
    {
      size_t len = MIN (destSize - 1, srcChars);
      memcpy (dest, source, len * sizeof (wchar_t));
      *(dest + len) = 0;
    }
    return srcChars + 1;
  };
  
  inline static size_t WCtoUTF8 (utf8_char* dest, size_t destSize, 
    const wchar_t* source, size_t srcSize)
  {
    return UTF32to8 (dest, destSize, (utf32_char*)source, srcSize);
  };

  inline static size_t WCtoUTF16 (utf16_char* dest, size_t destSize, 
    const wchar_t* source, size_t srcSize)
  {
    return UTF32to16 (dest, destSize, (utf32_char*)source, srcSize);
  };

  inline static size_t WCtoUTF32 (utf32_char* dest, size_t destSize, 
    const wchar_t* source, size_t srcSize)
  {
    size_t srcChars = srcSize;						
    if (srcSize == (size_t)-1)						
    {									
      srcChars = 0;							
      const wchar_t* sptr = source;					
      while (*sptr++ != 0) srcChars++;					
    }				
    if ((dest != 0) && (destSize != 0))
    {
      size_t len = MIN (destSize - 1, srcChars);
      memcpy (dest, source, len * sizeof (wchar_t));
      *(dest + len) = 0;
    }
    return srcChars + 1;
  };

  inline static int Decode (const wchar_t* str, size_t strlen, 
    utf32_char& ch, bool* isValid = 0, bool returnNonChar = false)
  {
    return UTF32Decode ((utf32_char*)str, strlen, ch, isValid, returnNonChar);
  }
  inline static int Encode (const utf32_char ch, wchar_t* buf, 
    size_t bufsize, bool allowNonchars = false)
  {
    return EncodeUTF32 (ch, (utf32_char*)buf, bufsize, allowNonchars);
  }
#else
  #error Odd-sized, unsupported wchar_t!
#endif

  /**\name Helpers to skip encoded code units in different UTF encodings
   * @{ */
   
  /**
   * Determine how many code units in an UTF-8 buffer need to be skipped to 
   * get to the next encoded char.
   * \param str Pointer to buffer with encoded code point.
   * \param maxSkip The number of code units to skip at max. Usually, this is
   *  the number of chars from \a str to the end of the buffer.
   * \return Number of chars to skip in the buffer. Returns 0 if \p maxSkip is
   *  0.
   */
  inline static int UTF8Skip (const utf8_char* str, size_t maxSkip)
  {
    if (maxSkip < 1) return 0;
  
    if ((*str & 0x80) == 0)
    {
      return 1;
    }
    else
    {
      int n = 0;
      while ((n < 7) && ((*str & (1 << (7 - n))) != 0)) { n++; }

      if ((n < 2) || (n > 6))
      {
	return 1;
      }

      int skip = 1;
      
      for (; skip < n; skip++)
      {
	if (((str[skip] & 0xc0) != 0x80) || ((size_t)skip > maxSkip))
	{
	  break;
	}
      }
      return skip;
    }
  }
  
  /**
   * Determine how many code units in an UTF-8 buffer need to skipped back to
   * get to the start of the previous encoded code point.
   * \param str Pointer to the encoded code point after the code point that is
   *  actually to be skipped back.
   * \param maxRew The number of code units to go back at max. Typically, this 
   *  is the number of chars from \a str to the start of the buffer.
   * \return Number of chars to skip back in the buffer. Returns 0 if 
   *  \p maxRew is 0.
   */
  inline static int UTF8Rewind (const utf8_char* str, size_t maxRew)
  {
    if (maxRew < 1) return 0;
    
    const utf8_char* pos = str - 1;
    
    if ((*pos & 0x80) == 0)
    {
      return 1;
    }
    
    // Skip backward to the first byte of the sequence.
    int skip = 1;
    while (((*pos & 0xc0) == 0x80) && ((size_t)skip < maxRew))
    {
      skip++;
      pos--;
    }
    
    return skip;
  }
  
  /**
   * Determine how many code units in an UTF-16 buffer need to be skipped to 
   * get to the next encoded char.
   * \copydoc UTF8Skip(const utf8_char*,size_t)
   */
  inline static int UTF16Skip (const utf16_char* str, size_t maxSkip)
  {
    if (CS_UC_IS_HIGH_SURROGATE (*str))
      return (int)(MIN(maxSkip, 2));
    else
      return (int)(MIN(maxSkip, 1));
  }
  
  /**
   * Determine how many code units in an UTF-16 buffer need to skipped back to
   * get to the start of the previous encoded code point.
   * \copydoc UTF8Rewind(const utf8_char*,size_t)
   */
  inline static int UTF16Rewind (const utf16_char* str, size_t maxRew)
  {
    if (maxRew < 1) return 0;
    
    const utf16_char* pos = str - 1;
    if (!CS_UC_IS_SURROGATE(*pos)) 
      return 1;
    else
    {
      if ((maxRew > 1) && (CS_UC_IS_HIGH_SURROGATE(*(pos - 1))))
        return 2;
      else
        return 1;
    }
  }
  
  /**
   * Determine how many code units in an UTF-32 buffer need to be skipped to 
   * get to the next encoded char.
   * \copydoc UTF8Skip(const utf8_char*,size_t)
   */
  inline static int UTF32Skip (const utf32_char* str, size_t maxSkip)
  {
    return (int)(MIN(maxSkip, 1));
  }

  /**
   * Determine how many code units in an UTF-32 buffer need to skipped back to
   * get to the start of the previous encoded code point.
   * \copydoc UTF8Rewind(const utf8_char*,size_t)
   */
  inline static int UTF32Rewind (const utf32_char* str, size_t maxRew)
  {
    if (maxRew < 1) return 0;
    return 1;
  }
  /** @} */

  /**\name Code point mappings
   * @{ */
   
  /**
   * Map a code point to its upper case equivalent(s).
   * \param ch Code point to be mapped.
   * \param dest Destination buffer.
   * \param destSize Number of code units the destination buffer can hold.
   * \param flags Flags to control the result of the mapping. Currently 
   *   supported is csUcMapSimple.
   * \return Number of code units the complete mapping result would require.
   */
  static size_t MapToUpper (const utf32_char ch, utf32_char* dest, 
    size_t destSize, uint flags = 0);
  /**
   * Map a code point to its lower case equivalent(s).
   * \copydoc MapToUpper(const utf32_char, utf32_char*, size_t, uint)
   */
  static size_t MapToLower (const utf32_char ch, utf32_char* dest, 
    size_t destSize, uint flags = 0);
  /**
   * Map a code point to its fold equivalent(s).
   * Fold mapping is useful for binary comparison of two Unicode strings.
   * \copydoc MapToUpper(const utf32_char, utf32_char*, size_t, uint)
   */
  static size_t MapToFold (const utf32_char ch, utf32_char* dest, 
    size_t destSize, uint flags = 0);
  /** @} */
};

/** @} */

#endif

