/*
    The Crystal Space map file loader
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

#define SYSDEF_ALLOCA
#include "cssysdef.h"
#include "stdldr.h"
#include "stdparse.h"
#include "qint.h"
#include "cssys/csendian.h"
#include "csutil/csvector.h"
#include "csutil/util.h"

#include "ivideo/itxtmgr.h"
#include "isys/isystem.h"
#include "iengine/iengine.h"
#include "isys/ivfs.h"
#include "iengine/ipolygon.h"
#include "iengine/iportal.h"

// This character, if encountered, is considered to start either a keyword
// or a non-quoted string. Having too much symbols here can hurt since these
// symbols will not be interpreted as terminal symbols, but rather as strings.
#define FIRST_KEYWORD_CHAR "_/!@$#~"
// These characters can be in the continuation of a keyword/unquoted string
// but not at the beginning of it
#define NEXT_KEYWORD_CHAR ".*+-"

//------------------------------------------------------ Helper functions -----

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
  { return strcmp (token_list [(int)Item] + KEYWORD_PREFIX_LEN, (char*)Key); }
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

//-----------------------------------------------------------------------------

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

csStandardLoader::csStandardLoader (iBase *iParent)
{
  CONSTRUCT_IBASE (iParent);
  system = NULL; vfs = NULL; engine = NULL;
  line = -1;
  strings = NULL;
  lineoffs = NULL;
  portals = NULL;
  recursion_depth = 0;
}

csStandardLoader::~csStandardLoader ()
{
  if (engine) engine->DecRef ();
  if (vfs) vfs->DecRef ();
  if (system) system->DecRef ();
}

bool csStandardLoader::Initialize (iSystem *iSys)
{
  (system = iSys)->IncRef ();
  if (!(vfs = QUERY_PLUGIN (system, iVFS)))
    return false;
  if (!(engine = QUERY_PLUGIN (system, iEngine)))
    return false;
  return true;
}

bool csStandardLoader::Load (const char *iName)
{
  if (!vfs) return false;

  // First of all, check if the pre-tokenized file exists
  char fnw [VFS_MAX_PATH_LEN + 1];
  size_t sl = strlen (iName);
  if (sl > sizeof (fnw) - 5) sl = sizeof (fnw) - 5;
  memcpy (fnw, iName, sl); fnw [sl] = 0;
  strcpy (fnw + sl, ".csw");
  csFileTime ftw;
  bool csw_valid = false;
  if (vfs->GetFileTime (fnw, ftw))
  {
    // Now check if pre-tokenized file is up-to-date
    csFileTime fts;
    if (vfs->GetFileTime (iName, fts))
    {
      unsigned long tw = ftw.sec + (ftw.min + (ftw.hour +
        (ftw.day + (ftw.mon + (ftw.year - 70) * 12) * 32) * 24) * 60) * 60;
      unsigned long ts = fts.sec + (fts.min + (fts.hour +
        (fts.day + (fts.mon + (fts.year - 70) * 12) * 32) * 24) * 60) * 60;
      csw_valid = (tw >= ts);
    }
    else
      // If we don't have the source file, read the pre-tokenized version
      csw_valid = true;
  }

  char *data;
  iDataBuffer *databuffer = NULL;
  if (csw_valid)
  {
    databuffer = vfs->ReadFile (fnw);
    if (!databuffer
     || !(data = **databuffer)
     || get_le_long (data) != PARSER_VERSION)
      csw_valid = false;
  }
  if (!csw_valid)
  {
    databuffer = vfs->ReadFile (iName);
    if (!databuffer)
    {
      system->Printf (MSG_FATAL_ERROR,
        "Cannot read geometry file '%s'!\n", iName);
      return false;
    }
    data = Tokenize (**databuffer, src->GetSize ());
    databuffer->DecRef ();
    databuffer = NULL;
    // Save the pre-tokenized version
    vfs->WriteFile (fnw, data, size);
  }

  // Finally, launch the parser
  if (!yyinit (data, size))
  {
    system->Printf (MSG_FATAL_ERROR, "Failed to load file '%s'!\n", iName);
    return false;
  }
  bool rc = (yyparse () == 0);
  yydone (rc);

  // Free the token data
  if (csw_valid)
    databuffer->DecRef ();
  else
  {
    free (data);
    // Kill the pre-tokenized data on error
    if (!rc) vfs->DeleteFile (fnw);
  }

  return rc;
}

bool csStandardLoader::Parse (char *iData)
{
  size_t size = strlen (iData);
  csTokenList tl = Tokenize (iData, size);
  if (!tl) return false;

  if (!yyinit (tl, size))
  {
    free (tl);
    return false;
  }
  bool rc = (yyparse () == 0);
  yydone (rc);

  free (tl);
  return rc;
}

csTokenList csStandardLoader::Tokenize (char *iData, size_t &ioSize)
{
  // Statistic says us that output is usually a bit less than
  // 1/2 of the original ASCII text size
  size_t inputSize = ioSize; ioSize /= 4;
  char *inputData = iData;
  csTokenList output = (csTokenList)malloc (ioSize);
  int cur = sizeof (long) * 2, prev = -1, prevprev = -1;

  if (!lineoffs) lineoffs = new csVector ();
  if (!strings) strings = new csStringList ();

#define ENSURE(size)							\
  if (cur + (size) > ioSize)						\
  {									\
    /* Try to predict how much we'll need */				\
    size_t predict_size = QRound ((iData <= inputData) ? 0 :		\
      (double (cur) * double (inputSize)) / double (iData - inputData));\
    size_t min_size = ioSize + (size);					\
    if (min_size < ioSize + 256)					\
      min_size = ioSize + 256;						\
    if (predict_size < min_size)					\
      predict_size = min_size;						\
    output = (csTokenList)realloc (output, ioSize = predict_size);	\
  }

  line = 1;
  int prevline = -1;
  for (;;)
  {
    // Skip initial white spaces
    while (*iData &&
      (*iData == ' ' || *iData == '\t' || *iData == '\r' || *iData == '\n'))
    {
      if (*iData == '\n')
        line++;
      iData++;
    }

    enum { ltkEOF, ltkNUMBER, ltkSTRING, ltkKEYWORD, ltkSYMBOL } token =
      (*iData == 0) ? ltkEOF :
      *iData == '+' || *iData == '-' || *iData == '.' ||
      isdigit (*iData) ? ltkNUMBER :
      *iData == '\'' || *iData == '"' ? ltkSTRING :
      isalpha (*iData) || strchr (FIRST_KEYWORD_CHAR, *iData) ? ltkKEYWORD :
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
        if (!isalnum (*iData)
         && !strchr (FIRST_KEYWORD_CHAR NEXT_KEYWORD_CHAR, *iData))
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
      lineoffs->Push ((csSome)line);
      lineoffs->Push ((csSome)cur);
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
            outtoken = (tmp < 256) ?
	      tokNUMBER8 : (tmp < 65536) ? tokNUMBER16 : tokNUMBER32;
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
          while (ABS (f1) <= FLOAT_PRECISION)
          { f1 *= 10; f2 *= 10; }
          if (QRound (f1) == QRound (f2))
            tmp = little_endian_short (tmp), outtoken = tokNUMBER16F;
          else
            tmp = little_endian_long (float2long (f));
        }
        data = &tmp;
        datasize = (outtoken == tokNUMBER8 || outtoken == tokNUMBER8N) ?
	  sizeof (char) :
          (outtoken == tokNUMBER16 || outtoken == tokNUMBER16F) ?
	  sizeof (short) : sizeof (long);
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
      tmp = strings->FindKey (data);
      if (tmp < 0)
        tmp = strings->Push (strnew ((char *)data));
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
    bool equal = (prev
     && ((output [prev] & TOKEN_MASK) == outtoken)
     && (TOKEN_COUNT_GET (output [prev]) < TOKEN_COUNT_MAX));

    ENSURE (datasize + ((equal & 1) ^ 1));

    if (equal)
      TOKEN_COUNT_INC (output [prev]);
    else
    {
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
  set_le_long (output, PARSER_VERSION);
  set_le_long (output + sizeof (long), cur);

  int i; size_t total_len = 0;
  for (i = 0; i < strings->Length (); i++)
    total_len += strlen (strings->Get (i)) + 1;

  ENSURE (total_len);

  for (i = 0; i < strings->Length (); i++)
  {
    size_t len = strlen (strings->Get (i)) + 1;
    memcpy (output + cur, strings->Get (i), len);
    cur += len;
  }
  delete strings;
  strings = NULL;

  line = -1;
  ioSize = cur;

  return output;
}

bool csStandardLoader::yyinit (csTokenList iInput, size_t iSize)
{
  if (!iInput || !engine || !system)
    return false;

  curtoken = NULL;
  storage.tex_prefix = NULL;

  if (!strings) strings = new csStringList ();
  if (!portals) portals = new csPortalList (64, 64);

  // Get the string table
  size_t stofs = get_le_long (iInput + sizeof (long));
  bytesleft = stofs - sizeof (long) * 2;
  startinput = input = iInput + sizeof (long) * 2;
  if (stofs < iSize) iInput [iSize - 1] = 0;
  while (stofs < iSize)
  {
    int sl = strlen (iInput + stofs) + 1;
    strings->Push (iInput + stofs);
    stofs += sl;
  }

  return true;
}

void csStandardLoader::yydone (bool iSuccess)
{
  // Resolve all the portals
  if (iSuccess)
    for (int i = portals->Length () - 1; i >= 0; i--)
    {
      csPPortal *port = portals->Get (i);
      iSector *isect = engine->FindSector (port->destsec);
      if (!isect)
      {
        system->Printf (MSG_WARNING,
	  "invalid sector `%s' for portal target!", port->destsec);
        continue;
      }

      iPortal *iport = polygon.object->CreatePortal (isect);
      iport->SetFlags (port->flags.Get (), -1U);
      switch (port->mode)
      {
        case csPPortal::pmNone:
          break;
        case csPPortal::pmWarp:
          iport->SetWarp (CSMATRIX3 (port->matrix), CSVECTOR3 (port->before),
            CSVECTOR3 (port->after));
          break;
        case csPPortal::pmMirror:
          iport->SetMirror (polygon.object);
          break;
      }
    }

  delete portals; portals = NULL;
  delete strings; strings = NULL;
  delete lineoffs; lineoffs = NULL;
}

void csStandardLoader::yyerror (char *s)
{
  int l = line;
  if (l < 0 && lineoffs)
  {
    // Find the line number
    int i = 0;
    while (i < lineoffs->Length () && ofs >= (int)lineoffs->Get (i + 1))
      i += 2;
    l = (int)lineoffs->Get (i - 2);
  }

  char lno [10];
  if (l < 0)
    strcpy (lno, "(unknown)");
  else
    sprintf (lno, "%d", l);

  char src [100];
  if (storage.cur_library)
    sprintf (src, "library %s", storage.cur_library);
  else
    strcpy (src, "input");

  fprintf (stderr, "%s line %s: %s\n", src, lno, s);
}

int csStandardLoader::yylex (void *lval)
{
  YYSTYPE *yylval = (YYSTYPE *)lval;
  // Remember position of last token in the case we'll need to display error
  ofs = input - (startinput - sizeof (long) * 2);

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
      yylval->fval = (unsigned char)*input;
      input += sizeof (char);
      bytesleft -= sizeof (char);
      return NUMBER;
    case tokNUMBER8N:
      yylval->fval = -256 + (unsigned char)*input;
      input += sizeof (char);
      bytesleft -= sizeof (char);
      return NUMBER;
    case tokNUMBER16:
      yylval->fval = (unsigned short)get_le_short (input);
      input += sizeof (short);
      bytesleft -= sizeof (short);
      return NUMBER;
    case tokNUMBER16F:
      yylval->fval = get_le_float16 (input);
      input += sizeof (short);
      bytesleft -= sizeof (short);
      return NUMBER;
    case tokNUMBER32:
      yylval->fval = get_le_float32 (input);
      input += sizeof (long);
      bytesleft -= sizeof (long);
      return NUMBER;
    case tokSTRING8:
      yylval->string = strings->Get ((unsigned char)*input);
      input += sizeof (char);
      bytesleft -= sizeof (char);
      return STRING;
    case tokSTRING16:
      yylval->string = strings->Get ((unsigned short)get_le_short (input));
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

bool csStandardLoader::RecursiveLoad (const char *iName)
{
  if (recursion_depth > MAX_RECURSION_DEPTH)
  {
    system->Printf (MSG_FATAL_ERROR,
      "Max recursive depth exceeded (cyclic LIBRARY statements?)\n");
    return false;
  }

  recursion_depth++;

  char *_tex_prefix = storage.tex_prefix;
  char *_cur_library = storage.cur_library;
  csStringList *_strings = strings; strings = NULL;
  csVector *_lineoffs = lineoffs; lineoffs = NULL;
  csTokenList _input = input;
  csTokenList _startinput = startinput;
  csTokenList _curtoken = curtoken;
  int _bytesleft = bytesleft;
  int _curtokencount = curtokencount;

  bool rc = Load (iName);

  curtokencount = _curtokencount;
  bytesleft = _bytesleft;
  curtoken = _curtoken;
  startinput = _startinput;
  input = _input;
  lineoffs = _lineoffs;
  strings = _strings;
  storage.cur_library = _cur_library;
  storage.tex_prefix = _tex_prefix;

  recursion_depth--;

  return rc;
}

//-------------------------------------- Geometry loader parser helpers -----//

void csStandardLoader::InitTexture (char *name)
{
  // Prepare all parameters to initial values
  storage.tex.name =
  storage.tex.filename = name;
  storage.tex.transp.Set (0, 0, 0);
  storage.tex.do_transp = false;
  storage.tex.flags = CS_TEXTURE_3D;
}

bool csStandardLoader::CreateTexture ()
{
  // See if we have to add some prefix to texture name
  char *new_name = storage.tex.name;
  if (storage.tex_prefix)
  {
    new_name = (char *)alloca (strlen (storage.tex_prefix) +
      strlen (storage.tex.name) + 2);
    strcat (strcat (strcpy (new_name, storage.tex_prefix), "_"),
      storage.tex.name);
  }

  // Now tell the engine to register the respective texture
  return engine->CreateTexture (new_name, storage.tex.filename,
    storage.tex.do_transp ? (csColor *)&storage.tex.transp : NULL,
    storage.tex.flags);
}

void csStandardLoader::InitCamera (char *name)
{
  storage.camera.name = name;
  storage.camera.sector = NULL;
  storage.camera.pos.Set (0, 0, 0);
  storage.camera.forward.Set (0, 0, 1);
  storage.camera.upward.Set (0, 1, 0);
}

bool csStandardLoader::CreateCamera ()
{
  if (!storage.camera.name || !storage.camera.sector)
  {
    if (!storage.camera.name)
      yyerror ("no camera name defined!");
    if (!storage.camera.sector)
      yyerror ("no camera sector defined!");
    return false;
  }

  return engine->CreateCamera (storage.camera.name, storage.camera.sector,
    (csVector3 &)storage.camera.pos, (csVector3 &)storage.camera.forward,
    (csVector3 &)storage.camera.upward);
}

static bool CheckFlags (int mode)
{
  // Check for incompatible combinations
  int x1 = mode & (pmORIGIN | pmMATRIX);
  int x2 = mode & (pmFIRSTSECOND | pmMATRIX | pmVECTORS | pmPLANEREF);

  if ((x1 && !IsPowerOf2 (x1))
   || (x2 && !IsPowerOf2 (x2)))
    return false;

  return true;
}

bool csStandardLoader::CreatePlane (const char *name)
{
  if (!name)
  {
    yyerror ("unnamed texture planes not allowed in global context");
    return false;
  }

  // Check for incompatible combinations
  if (!CheckFlags (storage.plane.mode))
  {
    yyerror ("invalid texture plane definition");
    return false;
  }

  csVector3 &o = (csVector3 &)storage.plane.origin;
  csVector3 &f = (csVector3 &)storage.plane.first;
  csVector3 &s = (csVector3 &)storage.plane.second;
  csMatrix3 &m = (csMatrix3 &)storage.plane.matrix;

  if (storage.plane.mode == pmNONE)
  {
    yyerror ("empty plane definition");
    return false;
  }

  if (storage.plane.mode & (pmFIRSTSECOND | pmVECTORS))
  {
    if (!(storage.plane.mode & pmORIGIN))
    {
      yyerror ("no texture plane origin defined");
      return false;
    }
    if (storage.plane.first_len == 0.0)
    {
      yyerror ("no FIRST vector defined");
      return false;
    }
    if (storage.plane.second_len == 0.0)
    {
      yyerror ("no SECOND vector defined");
      return false;
    }
    if (storage.plane.mode & pmFIRSTSECOND)
    {
      f -= o; s -= o;
      float fact = storage.plane.first_len / f.Norm ();
      f *= fact;
      fact = storage.plane.second_len / s.Norm ();
      s *= fact;
    }

    csVector3 z = f % s;
    m.Set (f.x, s.x, z.x,
           f.y, s.y, z.y,
           f.z, s.z, z.z);
  }

  return engine->CreatePlane (name, o, m);
}

bool csStandardLoader::CreateTexturePlane (iPolygon3D *iPolygon)
{
  // Check for incompatible combinations
  if (!CheckFlags (polygon.mode))
  {
    yyerror ("invalid texture plane definition");
    return false;
  }

  if (polygon.mode == pmNONE)
  {
    yyerror ("empty plane definition");
    return false;
  }

  if (polygon.mode & (pmFIRSTSECOND | pmMATRIX | pmVECTORS))
  {
    csVector3 &o = (csVector3 &)polygon.origin;
    csVector3 &f = (csVector3 &)polygon.first;
    csVector3 &s = (csVector3 &)polygon.second;
    csMatrix3 &m = (csMatrix3 &)polygon.matrix;

    if (polygon.mode & (pmFIRSTSECOND | pmVECTORS))
    {
      if (!(polygon.mode & pmORIGIN))
      {
        yyerror ("no texture plane origin defined");
        return false;
      }
      if (polygon.first_len == 0.0)
      {
        yyerror ("no FIRST vector defined");
        return false;
      }
      if (polygon.second_len == 0.0)
      {
        yyerror ("no SECOND vector defined");
        return false;
      }
      if (polygon.mode & pmFIRSTSECOND)
      {
        f -= o; s -= o;
        float fact = polygon.first_len / f.Norm ();
        f *= fact;
        fact = polygon.second_len / s.Norm ();
        s *= fact;
      }

      csVector3 z = f % s;
      m.Set (f.x, s.x, z.x,
             f.y, s.y, z.y,
             f.z, s.z, z.z);
    }
    iPolygon->CreatePlane (o, m);
  }
  else if (polygon.mode & pmPLANEREF)
    if (!iPolygon->SetPlane (polygon.planetpl))
    {
      yyerror ("unknown texture plane name");
      return false;
    }
  return true;
}
