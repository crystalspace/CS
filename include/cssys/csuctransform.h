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
  if (chUsed == strlen)				\
  {						\
    FAIL(chUsed);				\
  }						\
  next = *str++;				\
  if (next == 0)				\
  {						\
    FAIL(chUsed);				\
  }						\
  chUsed++; 					
  
  inline static int UTF8Decode (const utf8_char* str, size_t strlen, 
    utf32_char& ch, bool* isValid)
  {
    if (str == 0)
    {
      FAIL(0);
    }
    size_t chUsed = 0;
    
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
      else if ((ch < 0x4000000) && (n > 5))
      {
        FAIL(chUsed);
      }
      else if ((ch < 0x80000000) && (n > 6))
      {
        FAIL(chUsed);
      }
      
      if (CS_UC_IS_INVALID(ch) || CS_UC_IS_SURROGATE(ch))
	FAIL(chUsed);
      SUCCEED;
    }
  }
  
  inline static int UTF16Decode (const utf16_char* str, size_t strlen, 
    utf32_char& ch, bool* isValid)
  {
    if (str == 0)
    {
      FAIL(0);
    }
    size_t chUsed = 0;
    
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
      ch = (curCh & 0x03ff) << 10;
      GET_NEXT(curCh);
      // Invalid code
      if (!CS_UC_IS_LOW_SURROGATE (curCh))
      {
        // Fail with 1 so the char is handled upon the next Decode.
	FAIL(1);
      }
      ch |= (curCh & 0x3ff);
      // Check for "overlong" codes
      if ((ch == 0) || (ch < 0x10000))
	FAIL(chUsed);
    }
    else
    {
      ch = curCh;
    }
    if (CS_UC_IS_INVALID(ch))
      FAIL(chUsed);
    SUCCEED;
  }
  
  inline static int UTF32Decode (const utf32_char* str, size_t strlen, 
    utf32_char& ch, bool* isValid)
  {
    if (str == 0)
    {
      FAIL(0);
    }
    size_t chUsed = 0;
    
    GET_NEXT(ch);
    if (CS_UC_IS_INVALID(ch))
      FAIL(chUsed);
    SUCCEED;
  }
#undef FAIL
#undef SUCCEED
#undef GET_NEXT

#define _OUTPUT_CHAR(buf, chr)				\
  if (bufRemaining > 0)					\
  {							\
    if(buf) *buf++ = chr;				\
    bufRemaining--;					\
  }							\
  encodedLen++;

#define OUTPUT_CHAR(chr) _OUTPUT_CHAR(buf, chr)
  
  inline static int EncodeUTF8 (const utf32_char ch, utf8_char* buf, 
    size_t bufsize)
  {
    if ((CS_UC_IS_INVALID(ch)) || (CS_UC_IS_SURROGATE(ch))) 
      return 0;
    size_t bufRemaining = bufsize, encodedLen = 0;
    
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
    else if (ch < 0x4000000)
    {
      OUTPUT_CHAR ((utf8_char)(0xf8 | (ch >> 24)));
      OUTPUT_CHAR ((utf8_char)(0x80 | ((ch >> 18) & 0x3f)));
      OUTPUT_CHAR ((utf8_char)(0x80 | ((ch >> 11) & 0x3f)));
      OUTPUT_CHAR ((utf8_char)(0x80 | ((ch >> 6) & 0x3f)));
      OUTPUT_CHAR ((utf8_char)(0x80 | (ch & 0x3f)));
    }
    else if (ch < 0x80000000)
    {
      OUTPUT_CHAR ((utf8_char)(0xfc | (ch >> 32)));
      OUTPUT_CHAR ((utf8_char)(0x80 | ((ch >> 24) & 0x3f)));
      OUTPUT_CHAR ((utf8_char)(0x80 | ((ch >> 18) & 0x3f)));
      OUTPUT_CHAR ((utf8_char)(0x80 | ((ch >> 11) & 0x3f)));
      OUTPUT_CHAR ((utf8_char)(0x80 | ((ch >> 6) & 0x3f)));
      OUTPUT_CHAR ((utf8_char)(0x80 | (ch & 0x3f)));
    }
    return encodedLen;
  }
    
  inline static EncodeUTF16 (const utf32_char ch, utf16_char* buf, 
    size_t bufsize)
  {
    if ((CS_UC_IS_INVALID(ch)) || (CS_UC_IS_SURROGATE(ch))) 
      return 0;
    size_t bufRemaining = bufsize, encodedLen = 0;
    
    if (ch < 0x10000)
    {
      OUTPUT_CHAR((utf16_char)ch);
    }
    else if (ch < 0x100000)
    {
      OUTPUT_CHAR((utf16_char)((ch >> 10) | CS_UC_CHAR_HIGH_SURROGATE_FIRST));
      OUTPUT_CHAR((utf16_char)((ch & 0x3ff) | CS_UC_CHAR_LOW_SURROGATE_FIRST));
    }
    else
      return 0;
    
    return encodedLen;
  }

  inline static EncodeUTF32 (const utf32_char ch, utf32_char* buf, 
    size_t bufsize)
  {
    if ((CS_UC_IS_INVALID(ch)) || (CS_UC_IS_SURROGATE(ch))) 
      return 0;
    size_t bufRemaining = bufsize, encodedLen = 0;
    
    OUTPUT_CHAR(ch);
    
    return encodedLen;
  }
#undef OUTPUT_CHAR
  
#define OUTPUT_CHAR(chr) _OUTPUT_CHAR(dest, chr)
  
#define UCTF_CONVERTER(funcName, fromType, decoder, toType, encoder)	\
  inline static size_t funcName (toType* dest, const fromType* source,	\
    size_t destSize, size_t srcSize)					\
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
      if (dcnt >= bufRemaining) 					\
      {									\
        bufRemaining = 0;						\
	if (dest && (destSize > 0)) dest += bufRemaining;		\
      }									\
      else								\
      {									\
        bufRemaining -= dcnt;						\
	if (dest && (destSize > 0)) dest += dcnt;			\
      }									\
      encodedLen += dcnt;						\
      if (scnt >= srcChars) break;					\
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
   * \param source Source buffer.
   * \param destSize Number of widechars the destination buffer can hold.
   * \return Number of widechars in the complete UTF-16 string, including null 
   *  terminator.
   * \remark If the complete converted string wouldn't fit the destination 
   *  buffer, it is truncated. However, it'll also be null-terminated.
   *  The returned value is the number of characters needed for the *whole* 
   *  converted string.
   */
  UCTF_CONVERTER (UTF8to16, utf8_char, UTF8Decode, utf16_char, EncodeUTF16);
  UCTF_CONVERTER (UTF8to32, utf8_char, UTF8Decode, utf32_char, EncodeUTF32);

  /**
   * Convert UTF-16 to UTF-8.
   * \param dest Destination buffer.
   * \param source Source buffer.
   * \param destSize Number of chars the destination buffer can hold.
   * \return Number of chars in the complete UTF-8 string, including null 
   *  terminator.
   * \remark If the complete converted string wouldn't fit the destination 
   *  buffer, it is truncated. However, it'll also be null-terminated.
   *  The returned value is the number of characters needed for the *whole* 
   *  converted string.
   */
  UCTF_CONVERTER (UTF16to8, utf16_char, UTF16Decode, utf8_char, EncodeUTF8);
  UCTF_CONVERTER (UTF16to32, utf16_char, UTF16Decode, utf32_char, EncodeUTF32);
  
  UCTF_CONVERTER (UTF32to8, utf32_char, UTF32Decode, utf8_char, EncodeUTF8);
  UCTF_CONVERTER (UTF32to16, utf32_char, UTF32Decode, utf16_char, EncodeUTF16);
  
#undef UCTF_CONVERTER
#undef OUTPUT_CHAR

#if (CS_WCHAR_T_SIZE == 1)
  inline static size_t UTF8toWC (wchar_t* dest, const utf8_char* source,
    size_t destSize, size_t srcSize)
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

  inline static size_t UTF16toWC (wchar_t* dest, const utf16_char* source,
    size_t destSize, size_t srcSize)
  {
    return UTF16to8 ((utf8_char*)dest, source, destSize, srcSize);
  };

  inline static size_t UTF32toWC (wchar_t* dest, const utf32_char* source,
    size_t destSize, size_t srcSize)
  {
    return UTF32to8 ((utf8_char*)dest, source, destSize, srcSize);
  };
  
  inline static size_t WCtoUTF8 (utf8_char* dest, const wchar_t* source,
    size_t destSize, size_t srcSize)
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
      memcpy (dest, source, size * sizeof (wchar_t));
      *(dest + len) = 0;
    }
    return srcChars + 1;
  };

  inline static size_t WCtoUTF16 (utf16_char* dest, const wchar_t* source,
    size_t destSize, size_t srcSize)
  {
    return UTF8to16 (dest, source, destSize, srcSize);
  };

  inline static size_t WCtoUTF32 (utf32_char* dest, const wchar_t* source,
    size_t destSize, size_t srcSize)
  {
    return UTF8to32 (dest, source, destSize, srcSize);
  };
#elif (CS_WCHAR_T_SIZE == 2)
  inline static size_t UTF8toWC (wchar_t* dest, const utf8_char* source,
    size_t destSize, size_t srcSize)
  {
    return UTF8to16 ((utf16_char*)dest, source, destSize, srcSize);
  };

  inline static size_t UTF16toWC (wchar_t* dest, const utf16_char* source,
    size_t destSize, size_t srcSize)
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

  inline static size_t UTF32toWC (wchar_t* dest, const utf32_char* source,
    size_t destSize, size_t srcSize)
  {
    return UTF32to16 ((utf16_char*)dest, source, destSize, srcSize);
  };
  
  inline static size_t WCtoUTF8 (utf8_char* dest, const wchar_t* source,
    size_t destSize, size_t srcSize)
  {
    return UTF16to8 (dest, source, destSize, srcSize);
  };

  inline static size_t WCtoUTF16 (utf16_char* dest, const wchar_t* source,
    size_t destSize, size_t srcSize)
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

  inline static size_t WCtoUTF32 (utf32_char* dest, const wchar_t* source,
    size_t destSize, size_t srcSize)
  {
    return UTF16to32 (dest, source, destSize, srcSize);
  };
#elif (CS_WCHAR_T_SIZE == 4)
  inline static size_t UTF8toWC (wchar_t* dest, const utf8_char* source,
    size_t destSize, size_t srcSize)
  {
    return UTF8to32 ((utf32_char*)dest, source, destSize, srcSize);
  };

  inline static size_t UTF16toWC (wchar_t* dest, const utf16_char* source,
    size_t destSize, size_t srcSize)
  {
    return UTF16to32 ((utf32_char*)dest, source, destSize, srcSize);
  };

  inline static size_t UTF32toWC (wchar_t* dest, const utf32_char* source,
    size_t destSize, size_t srcSize)
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
      memcpy (dest, source, size * sizeof (wchar_t));
      *(dest + len) = 0;
    }
    return srcChars + 1;
  };
  
  inline static size_t WCtoUTF8 (utf8_char* dest, const wchar_t* source,
    size_t destSize, size_t srcSize)
  {
    return UTF32to8 (dest, source, destSize, srcSize);
  };

  inline static size_t WCtoUTF16 (utf16_char* dest, const wchar_t* source,
    size_t destSize, size_t srcSize)
  {
    return UTF32to16 (dest, source, destSize, srcSize);
  };

  inline static size_t WCtoUTF32 (utf32_char* dest, const wchar_t* source,
    size_t destSize, size_t srcSize)
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
      memcpy (dest, source, size * sizeof (wchar_t));
      *(dest + len) = 0;
    }
    return srcChars + 1;
  };
#else
  #error Odd-sized, unsupported wchar_t!
#endif

};

#endif

