/*
    The Crystal Space world file loader
    Copyright (C) 2000 by Andrew Zabolotny <bit@eltech.ru>

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

#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "sysdef.h"
#include "stdldr.h"
#include "stdparse.h"
#include "isystem.h"
#include "iworld.h"
#include "cssys/csendian.h"
#include "csutil/csvector.h"
#include "csutil/util.h"

//------------------------------------------------------- Helper functions -----

#define KEYWORD_PREFIX		"KW_"
#define KEYWORD_PREFIX_LEN	3

static const char *const *token_list;
static class csSortedTokens : public csVector
{
public:
  csSortedTokens () : csVector (64, 64)
  { }
  const char *GetStr (int idx) const
  { return token_list [(int)csVector::Get (idx)] + KEYWORD_PREFIX_LEN; }
  virtual int Compare (csSome Item1, csSome Item2, int Mode) const
  { return strcmp (token_list [(int)Item1] + KEYWORD_PREFIX_LEN,
                   token_list [(int)Item2] + KEYWORD_PREFIX_LEN); }
  virtual int CompareKey (csSome Item, csConstSome Key, int Mode) const
  { return strcmp (token_list [(int)Item] + KEYWORD_PREFIX_LEN, (char *)Key); }
} *token_idx = NULL;

// Initialize token table (pre-sort and such)
void init_token_table (const char *const *yytname)
{
  if (!token_idx)
    token_idx = new csSortedTokens ();

  token_list = yytname;
  int idx = 0;
  while (yytname [idx])
  {
    if (!strncmp (yytname [idx], KEYWORD_PREFIX, KEYWORD_PREFIX_LEN))
      token_idx->Push ((csSome)idx);
    idx++;
  }
  token_idx->QuickSort ();
}

//------------------------------------------------------------------------------

/*
    The tokenizer converts the plain ASCII text into a sequence of tokens.
    The tokens are packed the following way: first comes a byte that contains
    token type (in four lowest bits), the upper 4 bits contains a count
    (1-16) of sequential tokens of the same type.
    Also since the numbers always come with a ',' inbetween, the symbol ','
    is removed if present between two numbers (and added while decompressing
    in yylex()).
    Also we introduce special token types for often-used symbols '(' and ')'.
    Yet another trick: since numbers are the most numerous tokens in mesh
    files, we use 8/16-bit integers (mostly used for representing vertex
    indices) and also 32/16-bit floating-point format for storing non-integer
    numbers.
*/

enum csToken
{
  tokNUMBER8,	/* A 8-bit unsigned integer follows */
  tokNUMBER8N,	/* A 8-bit negative integer follows */
  tokNUMBER16,	/* A 16-bit unsigned integer follows */
  tokNUMBER16F,	/* A 16-bit floating-point number follows */
  tokNUMBER32,	/* A 32-bit portable float follows */
  tokKEYWORD,	/* An 8-bit keyword ID follows */
  tokSTRING8,	/* An 8-bit string index follows */
  tokSTRING16,	/* An 16-bit string index follows */
  tokSYMBOL,	/* An 8-bit symbol follows */
  tokSYMOPEN,	/* Replacement for tokSYMBOL '(' */
  tokSYMCLOSE,	/* Replacement for tokSYMBOL ')' */
  tokCOLON	/* Replacement for tokSYMBOL ':' */
};

#define TOKEN_IS_NUMBER(x)	((x) < tokKEYWORD)
#define TOKEN_MASK		0x0f
#define TOKEN_COUNT_GET(x)	((((unsigned char)x) >> 4) + 1)
#define TOKEN_COUNT_INC(x)	(x += (1 << 4))
#define TOKEN_COUNT_MAX		16
// Precision for 16-bit floats that is still acceptable
#define FLOAT_PRECISION		100

IMPLEMENT_IBASE (csStandardLoader)
  IMPLEMENTS_INTERFACE (iPlugIn)
  IMPLEMENTS_INTERFACE (iLoader)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csStandardLoader)

EXPORT_CLASS_TABLE (stdldr)
  EXPORT_CLASS_DEP (csStandardLoader, "crystalspace.loader.standard",
    "Standard Crystal Space geometry loader", "crystalspace.kernel.")
EXPORT_CLASS_TABLE_END

csStandardLoader::csStandardLoader (iBase *iParent) :
  strings (16, 16), lineoffs (1, 1)
{
  CONSTRUCT_IBASE (iParent);
  system = NULL; vfs = NULL; world = NULL;
  line = -1;
}

csStandardLoader::~csStandardLoader ()
{
}

bool csStandardLoader::Initialize (iSystem *iSys)
{
  system = iSys;
  return true;
}

void csStandardLoader::ClearAll ()
{
}

bool csStandardLoader::Load (const char *iName)
{
  return true;
}

bool csStandardLoader::Load (csTokenList iInput, size_t &iSize)
{
  yyinit (iInput, iSize);
  bool rc = (yyparse (this) == 0);
  yydone ();
  return rc;
}

bool csStandardLoader::Parse (char *iData)
{
  size_t size;
  csTokenList tl = Tokenize (iData, size);
  if (!tl) return false;

  yyinit (tl, size);
  bool rc = (yyparse (this) == 0);
  yydone ();

FILE *f = fopen ("plugins/csstdldr/test/world.bin", "wb");
fwrite (tl, size, 1, f);
fclose (f);

  free (tl);
  return rc;
}

csTokenList csStandardLoader::Tokenize (char *iData, size_t &oSize)
{
  csTokenList output = (csTokenList)malloc (oSize = 256);
  int cur = sizeof (long), prev = -1, prevprev = -1;

  lineoffs.DeleteAll ();
  strings.DeleteAll ();

#define ENSURE(size)					\
  if (cur + size > oSize)				\
  {							\
    while (cur + size > oSize) oSize += 256;		\
    output = (csTokenList)realloc (output, oSize);	\
  }

  line = 1;
  int prevline = -1;
  for (;;)
  {
    // Skip initial white spaces
    while (*iData && (*iData == ' ' || *iData == '\t' || *iData == '\r' || *iData == '\n'))
    {
      if (*iData == '\n')
        line++;
      iData++;
    }

    enum { ltkEOF, ltkNUMBER, ltkSTRING, ltkKEYWORD, ltkSYMBOL } token =
      (*iData == 0) ? ltkEOF :
      *iData == '+' || *iData == '-' || *iData == '.' || isdigit (*iData) ? ltkNUMBER :
      *iData == '\'' || *iData == '"' ? ltkSTRING :
      isalpha (*iData) || *iData == '/' ? ltkKEYWORD :
      ltkSYMBOL;

    if (token == ltkEOF)
      break;

    char *start = iData++;
    for (;;)
    {
      // Check for EOF
      if (!*iData)
        break;
      if (token == ltkNUMBER)
      {
        // Check for end of digit
        if (*iData != '.' && !isdigit (*iData))
          break;
      }
      else if (token == ltkSTRING)
      {
        // Check for end of string
        if (*iData == *start)
          break;
        // Replace CRs by space
        else if (*iData == '\r')
          *iData = ' ';
        else if (*iData == '\n')
          line++;
      }
      else if (token == ltkKEYWORD)
      {
        // Check for end of keyword
        if (!isalnum (*iData) && (*iData != '_') && (*iData != '/') && (*iData != '.'))
          break;
      }
      else
        // A single symbol
        break;
      iData++;
    }

    if (token == ltkSYMBOL && *start == ';')
    {
      // Comment detected, skip to EOL
      while (*iData && *iData != '\n')
        iData++;
      continue;
    }

    // Look if it is a valid number
    if (token == ltkNUMBER
     && (iData - start) <= 1
     && (*start == '+' || *start == '-' || *start == '.'))
      token = ltkSYMBOL;

    if (line != prevline)
    {
      lineoffs.Push ((csSome)line);
      lineoffs.Push ((csSome)cur);
      prevline = line;
    }

    // Temporarily replace last character with '0'
    char oldchar = *iData; *iData = 0;

    long tmp = 0;
    void *data = NULL;
    size_t datasize = 0;
    csToken outtoken;
    switch (token)
    {
      case ltkEOF:
        break;
      case ltkNUMBER:
      {
        float f = atof (start);
        tmp = QInt (f);
        if (f == tmp)
          if (tmp >= 0)
            outtoken = (tmp < 256) ? tokNUMBER8 : (tmp < 65536) ? tokNUMBER16 : tokNUMBER32;
          else
            outtoken = (tmp > -256) ? tokNUMBER8N : tokNUMBER32;
        else
          outtoken = tokNUMBER32;

        if (outtoken == tokNUMBER16)
          tmp = little_endian_short (tmp);
        else if (outtoken == tokNUMBER32)
        {
          // See if a 16-bit floating-point number would be enough
          tmp = float2short (f);
          float f1 = f;
          float f2 = short2float (tmp);
          while (fabs (f1) <= FLOAT_PRECISION)
          { f1 *= 10; f2 *= 10; }
          if (QRound (f1) == QRound (f2))
            tmp = little_endian_short (tmp), outtoken = tokNUMBER16F;
          else
            tmp = little_endian_long (float2long (f));
        }
        data = &tmp;
        datasize = (outtoken == tokNUMBER8 || outtoken == tokNUMBER8N) ? sizeof (char) :
                   (outtoken == tokNUMBER16 || outtoken == tokNUMBER16F) ? sizeof (short) :
                   sizeof (long);
        break;
      }
      case ltkSTRING:
        outtoken = tokSTRING8;
        data = start + 1;
        datasize = iData - start;
        break;
      case ltkKEYWORD:
        tmp = token_idx->FindSortedKey (start);
        if (tmp >= 0)
        {
          outtoken = tokKEYWORD;
          data = &tmp;
          datasize = sizeof (UByte);
          break;
        }

        // Interpret unknown tokens as non-quoted strings
        outtoken = tokSTRING8;
        data = start;
        datasize = iData - start + 1;
        break;
      case ltkSYMBOL:
        datasize = 0;
        if (*start == '(')
          outtoken = tokSYMOPEN;
        else if (*start == ')')
          outtoken = tokSYMCLOSE;
        else if (*start == ':')
          outtoken = tokCOLON;
        else
          outtoken = tokSYMBOL, datasize = sizeof (char);
        data = start;
        break;
    }

    // Strings are represented via an index into the string table
    if (outtoken == tokSTRING8)
    {
      tmp = strings.FindKey (data);
      if (tmp < 0)
        tmp = strings.Push (strnew ((char *)data));
      if (tmp > 255)
      {
        outtoken = tokSTRING16;
        tmp = little_endian_short (tmp);
      }
      data = &tmp;
      datasize = (outtoken == tokSTRING16) ? sizeof (short) : sizeof (char);
    }

    // Remove ',' symbols between numbers (they are added on decompression)
    if ((token == ltkNUMBER)
      && prev
      && ((output [prev] & TOKEN_MASK) == tokSYMBOL)
      && (output [prev + 1] == ',')
      && prevprev
      && TOKEN_IS_NUMBER (output [prevprev] & TOKEN_MASK))
    {
      cur = prev;
      prev = prevprev;
      prevprev = NULL;
    }

    // Compress all sequential equal tokens into one
    if (prev
     && ((output [prev] & TOKEN_MASK) == outtoken)
     && (TOKEN_COUNT_GET (output [prev]) < TOKEN_COUNT_MAX))
    {
      ENSURE (datasize);
      TOKEN_COUNT_INC (output [prev]);
    }
    else
    {
      ENSURE (1 + datasize);
      prevprev = prev;
      prev = cur;
      output [cur++] = outtoken;
    }

    memcpy (output + cur, data, datasize);
    cur += datasize;

    // Restore the ending character
    *iData = oldchar;

    // If we're at the closing quote of a string, skip it
    if (token == ltkSTRING)
      iData++;
  }

  // Put the string table at the end of token table
  set_le_long (output, cur);
  for (int i = 0; i < strings.Length (); i++)
  {
    size_t len = strlen (strings.Get (i)) + 1;
    ENSURE (len);
    memcpy (output + cur, strings.Get (i), len);
    cur += len;
  }
  strings.FreeAll ();

  line = -1;
  oSize = cur;

  return output;
}

void csStandardLoader::yyinit (csTokenList iInput, size_t iSize)
{
  curtoken = NULL;

  // Get the string table
  size_t stofs = get_le_long (iInput);
  bytesleft = stofs - sizeof (long);
  startinput = input = iInput + sizeof (long);
  if (stofs < iSize) iInput [iSize - 1] = 0;
  while (stofs < iSize)
  {
    int sl = strlen (iInput + stofs) + 1;
    strings.Push (iInput + stofs);
    stofs += sl;
  }
}

void csStandardLoader::yydone ()
{
  strings.DeleteAll ();
  lineoffs.DeleteAll ();
}

void csStandardLoader::yyerror (char *s)
{
  int l = line;
  if (l < 0)
  {
    // Find the line number
    int i = 0;
    while (i < lineoffs.Length () && ofs >= (int)lineoffs.Get (i + 1))
      i += 2;
    l = (int)lineoffs.Get (i - 2);
  }
  fprintf (stderr, "line %d%s: %s\n", l, l < 0 ? " (unknown)" : "", s);
}

int csStandardLoader::yylex ()
{
  // Remember the position of last token in the case we'll need to display error
  ofs = input - (startinput - sizeof (long));

  UByte th;
  if (curtoken && curtokencount)
  {
    curtokencount--;
    th = *curtoken;
    if (!curtokencount)
      curtoken = NULL;
  }
  else
  {
    if (bytesleft <= 0)
      return 0;

    th = *input;
    curtokencount = TOKEN_COUNT_GET (th) - 1;
    if (curtokencount > 0)
      curtoken = input;
    input++; bytesleft--;
  }

  int val;
  switch (csToken (th & TOKEN_MASK))
  {
    case tokNUMBER8:
      yylval.fval = (unsigned char)*input;
      input += sizeof (char);
      bytesleft -= sizeof (char);
      return NUMBER;
    case tokNUMBER8N:
      yylval.fval = -256 + (unsigned char)*input;
      input += sizeof (char);
      bytesleft -= sizeof (char);
      return NUMBER;
    case tokNUMBER16:
      yylval.fval = (unsigned short)get_le_short (input);
      input += sizeof (short);
      bytesleft -= sizeof (short);
      return NUMBER;
    case tokNUMBER16F:
      yylval.fval = get_le_float16 (input);
      input += sizeof (short);
      bytesleft -= sizeof (short);
      return NUMBER;
    case tokNUMBER32:
      yylval.fval = get_le_float32 (input);
      input += sizeof (long);
      bytesleft -= sizeof (long);
      return NUMBER;
    case tokSTRING8:
      yylval.string = strings.Get ((unsigned char)*input);
      input += sizeof (char);
      bytesleft -= sizeof (char);
      return STRING;
    case tokSTRING16:
      yylval.string = strings.Get ((unsigned short)get_le_short (input));
      input += sizeof (short);
      bytesleft -= sizeof (short);
      return STRING;
    case tokKEYWORD:
      val = 255 + (int)token_idx->Get ((unsigned char)*input);
      input += sizeof (char);
      bytesleft -= sizeof (char);
      return val;
    case tokSYMBOL:
      val = (unsigned char)*input;
      input += sizeof (char);
      bytesleft -= sizeof (char);
      return val;
    case tokSYMOPEN:
      return '(';
    case tokSYMCLOSE:
      return ')';
    case tokCOLON:
      return ':';
  }

  // It should never come here
  return 0;
}
