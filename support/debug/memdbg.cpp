/*
    Copyright (C) 1999 by Andrew Zabolotny
    Cross-platform memory debugger: implementation

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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cssysdef.h"
#include "memdbg.h"

/*
 * This is a general memory debugger for C++. It tracks all "new" and "delete"
 * calls, collects the addresses from where these calls have been made and
 * also collects miscelaneous statistics during program execution. Upon exit
 * you can get a sheet describing which memory blocks were never freed, which
 * were freed more than once and so on.
 * The memory debugger is implemented in a single source file. This may look
 * somewhat clumsy but has the advantage that you have to include just one
 * extra file into your project to enable memory debugger.
 */

/**
 * The main work is performed automatically, but this variable controls
 * the way memory debugging is performed, the amount of output information
 * and so on.
 */
static int mdbFlags = MDF_DEFAULT;

/// Flag: is executable memory map already loaded?
static bool mdbMapOK = false;

/**
 * The following variable is used to collect memory activity statistics.
 */
struct
{
  // Total memory allocated during execution of program
  // @@ (can overflow for very big programs)
  size_t totalsize;
  // The current memory owned by the program
  size_t cursize;
  // Maximal amount of memory used (peak load)
  size_t peaksize;
  // Number of memory allocations
  unsigned long allocs;
  // Number of memory frees
  unsigned long frees;
  // Top ten memory allocations by size
  struct
  {
    // Address where memory block was allocated
    address alloc_addr;
    // The size of memory block
    size_t size;
  } topten [10];
} mdbStat =
{
  0
};

/**
 * This structure is used to keep track of all allocated and freed memory
 * blocks. Such a structure is allocated for each allocated block.
 */
struct mdbBlock
{
  // The memory block address
  address mem;
  // Block size (without interblock fences, if any)
  size_t size;
  // Address where block was allocated/freed
  address alloc_addr, free_addr;
  // The block is free already?
  bool free;
  // The block was allocated with malloc or with new?
  bool malloc;
} *mdbChain = NULL;
static int mdbChainLen = 0, mdbChainMaxLen = 0;
// A rudimentary lock to partially protect mdbChain in multithreaded environments
static int mdbLock = 0;

#define MDB_LOCK	while (mdbLock) ; mdbLock++;
#define MDB_UNLOCK	mdbLock--;

#define MDB_FILL_CHAR	'X'
#define MDB_FENCE	"INTERBLOCK_FENCE"
#define MDB_FENCE_SIZE	16

/// The output log file
static FILE *mdbLog = NULL;

/// Addresses where to break on alloc/free
static address *break_alloc = NULL, *break_alloc_addr = NULL;
static address *break_free = NULL, *break_free_addr = NULL;
static int break_alloc_count = -1, break_free_count = -1;
static int break_alloc_max = -1, break_free_max = -1;

#ifdef DEBUG_MALLOC
// Declare some simple prototypes for alternative malloc and company
extern "C" void *MALLOC (size_t);
extern "C" void *CALLOC (size_t, size_t);
extern "C" void *REALLOC (void *, size_t);
extern "C" void FREE (void *);
#endif

/// The function used to output to both console and log file
static void output (const char *format, ...)
{
  char outbuff [1024];
  va_list args;
  va_start (args, format);
  vsprintf (outbuff, format, args);
  va_end (args);

  if (mdbLog)
  {
    if (fprintf (mdbLog, "%s", outbuff) < 0)
    {
      // Some runtimes (e.g. DJGPP) closes all opened files before exiting
      mdbLog = fopen ("memdbg.log", "a");
      fprintf (mdbLog, "%s", outbuff);
    }
  }
  else
    printf ("MEMDBG: %s", outbuff);
}

static void *__new (address addr, size_t s, bool malloc)
{
  if (!mdbMapOK)
    mdbLoadMap ();

  if (mdbFlags & MDF_VERBOSE)
    output ("%s (%lu) at 0x%08lX (%s)\n", malloc ? "malloc" : "new",
      s, addr, mdbLocation (addr));

  if (mdbFlags & MDF_CHECKBOUNDS)
    s += MDB_FENCE_SIZE * 2;

  char *p = (char *)MALLOC (s);
  if (!p)
  {
    if (mdbFlags & MDF_DEBUGBREAK)
      DEBUG_BREAK;
    return NULL;
  }

  if (mdbFlags & MDF_CHECKBOUNDS)
  {
    s -= MDB_FENCE_SIZE * 2;
    memcpy (p, MDB_FENCE, MDB_FENCE_SIZE);
    p += MDB_FENCE_SIZE;
    memcpy (p + s, MDB_FENCE, MDB_FENCE_SIZE);
  }

  if (mdbFlags & MDF_ALLOCFILL)
    memset (p, MDB_FILL_CHAR, s);

  MDB_LOCK;
  if (mdbChainLen >= mdbChainMaxLen)
  {
    mdbChainMaxLen += 1024;
    mdbChain = (mdbBlock *)REALLOC (mdbChain, mdbChainMaxLen * sizeof (mdbBlock));
  }

  // Find the nearest memory block and insert the new block there
  mdbBlock *b = &mdbChain [mdbChainLen++];
  b->mem = p;
  b->size = s;
  b->alloc_addr = addr;
  b->free = false;
  b->malloc = malloc;

  // Now collect some statistics
  mdbStat.totalsize += s;
  mdbStat.cursize += s;
  mdbStat.allocs++;
  if (mdbStat.peaksize < mdbStat.cursize)
    mdbStat.peaksize = mdbStat.cursize;
  // Look if the alloc fits in top-ten
  int i;
  // First look if the alloc is not already registered in top-ten list
  for (i = 0; i < 10; i++)
    if (mdbStat.topten [i].alloc_addr == addr)
    {
      if (mdbStat.topten [i].size < s)
        mdbStat.topten [i].size = s;
      i = 666;
      break;
    }
  if (i != 666)
    for (i = 0; i < 10; i++)
    {
      if (mdbStat.topten [i].size < s)
        break;
    }
  if (i < 10)
  {
    memmove (&mdbStat.topten [i + 1], &mdbStat.topten [i], (9 - i) * sizeof (mdbStat.topten [0]));
    mdbStat.topten [i].alloc_addr = addr;
    mdbStat.topten [i].size = s;
  }
  MDB_UNLOCK;

  for (i = 0; i <= break_alloc_count; i++)
    if ((!break_alloc [i] || break_alloc [i] == p)
     && (!break_alloc_addr [i] || break_alloc_addr [i] == addr))
    {
      DEBUG_BREAK;
      break;
    }

  return p;
}

static void __delete (address addr, void *p, bool malloc)
{
  if (!mdbMapOK)
    mdbLoadMap ();

  // "delete NULL" ends here
  if (!p)
    return;

  int i;
  for (i = 0; i <= break_free_count; i++)
    if ((!break_free [i] || break_free [i] == p)
     && (!break_free_addr [i] || break_free_addr [i] == addr))
    {
      DEBUG_BREAK;
      break;
    }

  bool ext_info = false;
  address alloc_addr = 0;
  size_t size = 0;

  MDB_LOCK;
  // Find the memory block in our list
  int mbi;
  for (mbi = mdbChainLen - 1; mbi >= 0; mbi--)
    if (p == mdbChain [mbi].mem)
      break;
  mdbBlock *c = NULL;
  if (mbi >= 0)
    c = &mdbChain [mbi];

  if (!c)
  {
    output ("Trying to free unallocated memory, details:\n");
    if (mdbFlags & MDF_DEBUGBREAK)
      DEBUG_BREAK;
    goto info;
  }

  if (c->free)
  {
    output ("Trying to free memory block more than once, details:\n");
    if (mdbFlags & MDF_DEBUGBREAK)
      DEBUG_BREAK;
    ext_info = true;
    goto info;
  }

  if (c->malloc != malloc)
  {
    output ("Allocation and free operators do not match, details:\n");
    if (mdbFlags & MDF_DEBUGBREAK)
      DEBUG_BREAK;
    ext_info = true;
  }

  // Check if inter-block fence is intact
  if (mdbFlags & MDF_CHECKBOUNDS)
  {
    if (strncmp (((char *)p) - MDB_FENCE_SIZE, MDB_FENCE, MDB_FENCE_SIZE)
     || strncmp (((char *)p) + c->size, MDB_FENCE, MDB_FENCE_SIZE))
    {
      output ("Memory write outside block bounds detected, details:\n");
      ext_info = true;

      if (mdbFlags & MDF_DEBUGBREAK)
        DEBUG_BREAK;
    }
    p = ((char *)p) - MDB_FENCE_SIZE;
  }

  alloc_addr = c->alloc_addr;
  size = c->size;

  // Fill block that is being free with `garbage'
  memset (p, MDB_FILL_CHAR, size + ((mdbFlags & MDF_CHECKBOUNDS) ? MDB_FENCE_SIZE * 2 : 0));

  if (mdbFlags & MDF_FREEFILL)
  {
    c->free_addr = addr;
    c->free = true;
  }
  else
  {
    FREE (p);
    memmove (&mdbChain [mbi], &mdbChain [mbi + 1], (mdbChainLen - mbi - 1) * sizeof (mdbBlock));
    mdbChainLen--;
  }

  // And now some statistics
  mdbStat.cursize -= size;
  mdbStat.frees++;

  if (ext_info || (mdbFlags & MDF_VERBOSE))
  {
info:
    output ("%s (%p) at 0x%08lX (%s)\n", malloc ? "free" : "delete",
      p, addr, mdbLocation (addr));
    if (ext_info)
      output ("block was allocated at 0x%08lX (%s)\n",
        alloc_addr, mdbLocation (alloc_addr));
  }
  MDB_UNLOCK;
}

void *operator new (size_t s)
{
  GET_CALL_ADDRESS (s);
  return __new (addr, s, false);
}

void operator delete (void* p)
{
  GET_CALL_ADDRESS (p);
  __delete (addr, p, false);
}

void *operator new [] (size_t s)
{
  GET_CALL_ADDRESS (s);
  return __new (addr, s, false);
}

void operator delete [] (void* p)
{
  GET_CALL_ADDRESS (p);
  __delete (addr, p, false);
}

#ifdef DEBUG_MALLOC

void *malloc (size_t s)
{
  GET_CALL_ADDRESS (s);
  return __new (addr, s, true);
}

void *calloc (size_t n, size_t s)
{
  GET_CALL_ADDRESS (s);
  void *ret = __new (addr, n * s, true);
  memset (ret, 0, n * s);
  return ret;
}

void *realloc (void *p, size_t s)
{
  GET_CALL_ADDRESS (s);

  size_t oldsize = 0;
  if (p)
  {
    MDB_LOCK;
    // Find the memory block in our list
    int mbi;
    for (mbi = mdbChainLen - 1; mbi >= 0; mbi--)
      if (p == mdbChain [mbi].mem)
        break;
    mdbBlock *c = NULL;
    if (mbi >= 0)
      c = &mdbChain [mbi];

    if (!c)
    {
      output ("Trying to realloc unallocated memory, details:\n");
      if (mdbFlags & MDF_DEBUGBREAK)
        DEBUG_BREAK;
      output ("realloc (%p) at 0x%08lX (%s)\n", p, addr, mdbLocation (addr));
      return NULL;
    }
    if (!c->malloc)
    {
      output ("Trying to realloc new'd memory, details:\n");
      if (mdbFlags & MDF_DEBUGBREAK)
        DEBUG_BREAK;
      output ("realloc (%p) at 0x%08lX (%s)\n", p, addr, mdbLocation (addr));
      return NULL;
    }
    oldsize = c->size;
    MDB_UNLOCK;
  }

  void *ret = __new (addr, s, true);
  memcpy (ret, p, oldsize > s ? s : oldsize);
  __delete (addr, p, true);
  return ret;
}

void free (void *p)
{
  GET_CALL_ADDRESS (s);
  __delete (addr, p, true);
}

#endif // DEBUG_MALLOC

//---------------------------------// Executable memory map management //-----//

/**
 * Executable memory map structures.
 * The structures below are used to hold debug information about
 * the entire executable. The executable is split into source files,
 * source files are split into functions and functions are split into
 * lines. The hierarchical structure allows, on one side, for a effective
 * non-redundant data storage, and on other side, for very fast search
 * given an address on entry. All lists are sorted by address, thus we
 * can use a fast binary search algorithm for our purposes.
 */

/// The structure used to keep information about a source line
struct mdbLine
{
  /// The line number
  int Line;
  /// The starting address of this line
  address Start;
};

struct mdbFunction
{
  /// Function name
  char *Name;
  /// Starting and ending addresses
  address Start, End;
  /// Number of source code lines that we know about
  int MaxLines, Lines;
  /// The pointer to an array of mdbLine structures
  mdbLine *Line;
};

struct mdbSourceFile
{
  /// Source file name
  char *Name;
  /// Starting and ending addresses
  address Start, End;
  /// Number of functions in this file
  int MaxFunctions, Functions;
  /// A pointer to an array of mdbFunction structures
  mdbFunction *Function;
};

/// Memory map database variables
static mdbSourceFile *mdbMap;
/// Number of source files
static int mdbMaxFiles, mdbFiles;

void mdbLoadMap ()
{
  mdbMapOK = true;

  FILE *input = fopen ("memdbg.map", "r");
  if (!input)
  {
    printf ("MEMORY DEBUGGER ERROR:\n");
    printf ("Cannot read executable memory map from file memdbg.map\n");
    printf ("The file can be created by using the memdbg.sh script,\n");
    printf ("this way: ./memdbg.sh <executable>\n");
    exit (-1);
  }

  printf ("Parsing executable memory map ...");
  fflush (stdout);

  // The parser below expects an SysV-like nm output on entry.
  // Format of lines that the parser recognises (the '|' are used
  // to delimit fields; if field is empty it means that the field
  // contents are ignored; angle brackets <> denotes field descriptions):
  //
  // <filename> |             | |    SO |          | |
  // <function> | <startaddr> | |   FUN |   <size> | |
  //            | <lineaddr>  | | SLINE | <lineno> | |
  //

  // Reserve size for some number of source file descriptions
  mdbMap = (mdbSourceFile *)MALLOC ((mdbMaxFiles = 16) * sizeof (mdbSourceFile));
  // Current file we are working with
  int CurFile = -1;
  int CurFunc = -1;
  int CurLine = -1;

  int nFunctions = 0, nLines = 0;

  while (!feof (input))
  {
    char buff [1000];
    if (!fgets (buff, sizeof (buff), input))
      break;
    char *tmp = buff;
    char *eos = strchr (tmp, 0);
    while (eos > tmp && eos [-1] <= ' ')
      eos--;
    *eos = 0;

    char rectype = *tmp++;
    address addr = 0;
    if (rectype != 'O')
    {
      tmp += strspn (tmp, " \t");
      addr = (address)strtol (tmp, &tmp, 16);
      tmp += strspn (tmp, " \t");
    }

    switch (rectype)
    {
      case 'O':
        {
          mdbFlags = 0;
          while (*tmp)
            switch (*tmp++)
            {
              case ' ': case '\t':                   break;
              case 'a': mdbFlags |= MDF_ALLOCFILL;   break;
              case 'f': mdbFlags |= MDF_FREEFILL;    break;
              case 'd': mdbFlags |= MDF_DEBUGBREAK;  break;
              case 'v': mdbFlags |= MDF_VERBOSE;     break;
              case 's': mdbFlags |= MDF_SUMMARY;     break;
              case 'l': mdbFlags |= MDF_LISTUNFREED; break;
              case 'b': mdbFlags |= MDF_CHECKBOUNDS; break;
              case 'L': mdbFlags |= MDF_LOGFILE;     break;
              default: printf ("MEMDBG WARNING: unknown flag '%c'\n", tmp [-1]);
            }
          break;
        }
      case 'D':
        {
          address prog_addr = (address)strtol (tmp, &tmp, 16);
          tmp += strspn (tmp, " \t");
          switch (*tmp)
          {
            case 'A':
              if (++break_alloc_count > break_alloc_max)
              {
                break_alloc_max += 16;
                break_alloc = (address *)REALLOC (break_alloc, break_alloc_max * sizeof (address));
                break_alloc_addr = (address *)REALLOC (break_alloc_addr, break_alloc_max * sizeof (address));
              }
              break_alloc [break_alloc_count] = addr;
              break_alloc_addr [break_alloc_count] = prog_addr;
              break;
            case 'F':
              if (++break_free_count > break_free_max)
              {
                break_free_max += 16;
                break_free = (address *)REALLOC (break_free, break_free_max * sizeof (address));
                break_free_addr = (address *)REALLOC (break_free_addr, break_free_max * sizeof (address));
              }
              break_free [break_free_count] = addr;
              break_free_addr [break_free_count] = prog_addr;
              break;
            default:
              printf ("MEMDBG WARNING: unknown debug breakpoint type '%c'\n", tmp [-1]);
              break;
          }
          break;
        }
      case 'S':
        {
          // If we're processing a function, close it
          if (CurFile >= 0)
          {
            mdbMap [CurFile].End = addr;
            if (CurFunc >= 0)
              mdbMap [CurFile].Function [CurFunc].End = addr;
          }
          // Source file name
          if (CurFile < 0 || mdbMap [CurFile].Functions)
            CurFile++;
          // Enlarge the file array if required
          if (CurFile >= mdbMaxFiles)
            mdbMap = (mdbSourceFile *)REALLOC (mdbMap,
              (mdbMaxFiles += 16) * sizeof (mdbSourceFile));
          mdbSourceFile &mdbFile = mdbMap [CurFile];
          mdbFile.Name = (char *)MALLOC (strlen (tmp) + 1);
          strcpy (mdbFile.Name, tmp);
          mdbFile.Start = 0;
          mdbFile.MaxFunctions = mdbFile.Functions = 0;
          mdbFile.Function = NULL;
          CurFunc = CurLine = -1;
          break;
        }
      case 'F':
        if (CurFile >= 0)
        {
          mdbSourceFile &mdbFile = mdbMap [CurFile];

          // If we're processing a function, close it
          if (CurFunc >= 0)
          {
            mdbFile.End = addr;
            mdbFile.Function [CurFunc].End = addr;
          }

          // Function body
          if (CurFunc < 0 || mdbMap [CurFile].Function [CurFunc].Lines)
            CurFunc++, nFunctions++;
          mdbFile.Functions = CurFunc + 1;
          // Enlarge the function array if required
          if (CurFunc >= mdbFile.MaxFunctions)
            mdbFile.Function = (mdbFunction *)REALLOC (mdbFile.Function,
              (mdbFile.MaxFunctions += 16) * sizeof (mdbFunction));
          mdbFunction &mdbFunc = mdbFile.Function [CurFunc];
          mdbFunc.Name = (char *)MALLOC (strlen (tmp) + 1);
          strcpy (mdbFunc.Name, tmp);
          mdbFunc.Start = addr;
          mdbFunc.End = addr;
          mdbFunc.MaxLines = mdbFunc.Lines = 0;
          mdbFunc.Line = NULL;
          if (mdbFile.Start == 0)
          {
            mdbFile.Start = mdbFunc.Start;
            mdbFile.End = mdbFunc.End;
          }
          CurLine = -1;
          break;
        }
      case 'L':
        if (CurFile >= 0 && CurFunc >= 0)
        {
          // Line number
          CurLine++; nLines++;
          mdbSourceFile &mdbFile = mdbMap [CurFile];
          mdbFunction &mdbFunc = mdbFile.Function [CurFunc];
          mdbFunc.Lines = CurLine + 1;
          // Enlarge the line array if required
          if (CurLine >= mdbFunc.MaxLines)
            mdbFunc.Line = (mdbLine *)REALLOC (mdbFunc.Line,
              (mdbFunc.MaxLines += 64) * sizeof (mdbLine));
          mdbLine &mdbLN = mdbFunc.Line [CurLine];
          mdbLN.Line = atoi (tmp);
          mdbLN.Start = addr;
          break;
        }
      case ';':
        break;
      default:
        printf ("MEMDBG WARNING: unknown record type '%c'\n", rectype);
        break;
    } /* endswitch */
  } /* endwhile */

  // If we're processing a function, close it
  if (CurFile >= 0)
  {
    mdbMap [CurFile].End = MAX_ADDRESS;
    if (CurFunc >= 0)
      mdbMap [CurFile].Function [CurFunc].End = MAX_ADDRESS;
  }

  printf (" done.\n");

  // Open debugger log file
  if (mdbFlags & MDF_LOGFILE)
  {
    mdbLog = fopen ("memdbg.log", "w");
    fprintf (mdbLog, "-------------- Memory debugger session started --------------\n");
  }
  output ("Loaded info on %d source files, %d functions, %d lines of code.\n",
    mdbFiles = (CurFile + 1), nFunctions, nLines);
}

void mdbGetPos (address addr, const char *&file, const char *&func, int &line)
{
  file = NULL;
  func = NULL;
  line = -1;

  int minfile = 0, maxfile = mdbFiles - 1;
  int curfile = -1;
  while (minfile <= maxfile)
  {
    curfile = minfile + (maxfile - minfile) / 2;
    if (addr < mdbMap [curfile].Start)
      maxfile = curfile - 1;
    else if (addr > mdbMap [curfile].End)
      minfile = curfile + 1;
    else
      break;
  }
  if (minfile > maxfile || curfile < 0)
    return;

  mdbSourceFile &mdbFile = mdbMap [curfile];
  file = mdbFile.Name;
  int minfunc = 0, maxfunc = mdbFile.Functions - 1;
  int curfunc = -1;
  while (minfunc <= maxfunc)
  {
    curfunc = minfunc + (maxfunc - minfunc) / 2;
    if (addr < mdbFile.Function [curfunc].Start)
      maxfunc = curfunc - 1;
    else if (addr > mdbFile.Function [curfunc].End)
      minfunc = curfunc + 1;
    else
      break;
  }
  if (minfunc > maxfunc || curfunc < 0)
    return;

  mdbFunction &mdbFunc = mdbFile.Function [curfunc];
  func = mdbFunc.Name;
  int minline = 0, maxline = mdbFunc.Lines - 1;
  int curline = -1;
  while (minline <= maxline)
  {
    curline = minline + (maxline - minline) / 2;
    if (addr < mdbFunc.Line [curline].Start)
      maxline = curline - 1;
    else
      minline = curline + 1;
  }
  if (curline < 0)
    return;

  if (addr < mdbFunc.Line [curline].Start)
    curline--;

  line = mdbFunc.Line [curline].Line;
}

char *mdbLocation (address addr)
{
  static char buff [500];
  const char *file, *func; int line;
  mdbGetPos (addr, file, func, line);
  if (!file || line < 0)
    sprintf (buff, "unknown (%p)", addr);
  else
    sprintf (buff, "file %s:%d, func %s", file, line, func);
  return buff;
}

static bool memchk (void *p, size_t size, bool bounds)
{
  char *c = (char *)p;

  if (bounds)
  {
    c -= MDB_FENCE_SIZE;
    size += MDB_FENCE_SIZE * 2;
  }

  uint32 fc = MDB_FILL_CHAR | (MDB_FILL_CHAR << 8) |
    (MDB_FILL_CHAR << 16) | (MDB_FILL_CHAR << 24);
  while (size)
  {
    if (size > sizeof (uint32) * 4)
    {
      if (*(uint32 *)c != fc)
        return true;
      c += sizeof (uint32);
      if (*(uint32 *)c != fc)
        return true;
      c += sizeof (uint32);
      if (*(uint32 *)c != fc)
        return true;
      c += sizeof (uint32);
      if (*(uint32 *)c != fc)
        return true;
      c += sizeof (uint32);
      size -= sizeof (uint32) * 4;
    }
    else
    {
      if (*c++ != MDB_FILL_CHAR)
        return true;
      size--;
    }
  }
  return false;
}

void mdbFinish ()
{
  int i;
  mdbBlock *c;

  // Check "freed" blocks to remain intact
  if (mdbFlags & MDF_FREEFILL)
    for (i = 0; i < mdbChainLen; i++)
    {
      c = &mdbChain [i];
      if (c->free && memchk (c->mem, c->size, mdbFlags & MDF_CHECKBOUNDS))
      {
        if (mdbFlags & MDF_DEBUGBREAK)
          DEBUG_BREAK;
        output ("Memory has been overwritten after being freed! Details:\n");
        output ("size:%8lu allocated at %s\n", c->size, mdbLocation (c->alloc_addr));
      }
    }

  if (mdbFlags & MDF_LISTUNFREED)
  {
    output ("\n");
    output ("+--------+----------+- Unfreed memory blocks ---------------------------------\n");
    output ("|  size  |  address |location where block was allocated\n");
    output ("+--------+----------+---------------------------------------------------------\n");
    for (i = 0; i < mdbChainLen; i++)
    {
      c = &mdbChain [i];
      if (!c->free)
        output ("|%8lu|%10p|%s\n", c->size, c->mem, mdbLocation (c->alloc_addr));
    }
    output ("+--------+----------+---------------------------------------------------------\n");
  }

  if (mdbFlags & MDF_SUMMARY)
  {
    output ("\n-*- Memory allocation statistics -*-\n");
    output ("Total bytes allocated (may overflow): %lu bytes\n", mdbStat.totalsize);
    output ("Bytes still unfreed:                  %lu bytes\n", mdbStat.cursize);
    output ("Peak memory load:                     %lu bytes\n", mdbStat.peaksize);
    output ("Number of memory allocations:         %lu\n", mdbStat.allocs);
    output ("Number of memory frees:               %lu\n", mdbStat.frees);
    output ("\n");
    output ("+--------+- Top ten memory allocations ---------------------------------------\n");
    output ("|  size  |location where block was allocated\n");
    output ("+--------+--------------------------------------------------------------------\n");
    for (i = 0; i < 10; i++)
    {
      if (mdbStat.topten [i].alloc_addr)
        output ("|%8lu|%s\n", mdbStat.topten [i].size,
          mdbLocation (mdbStat.topten [i].alloc_addr));
    }
    output ("+--------+--------------------------------------------------------------------\n");
  }

  if (mdbLog)
    fclose (mdbLog);
  mdbLog = NULL;
}

// We need mdbFinish() to be called last function in executable
static struct __mdb_dummy
{
  ~__mdb_dummy ()
  { mdbFinish (); }
} __mdb_dummy_var;
