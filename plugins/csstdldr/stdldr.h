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

#ifndef __STDLDR_H__
#define __STDLDR_H__

#include "iloader.h"
#include "csgeom/math3d.h"
#include "csgeom/transfrm.h"
#include "csutil/csstrvec.h"

struct iWorld;
struct iVFS;

typedef char *csTokenList;

// Maximal number of recursive LIBRARY()'es
#define MAX_RECURSION_DEPTH	10

// We can't use csColor since it has a constructor :-(
struct csPColor
{
  float red, green, blue;
  void Set (float r, float g, float b)
  { red = r; green = g; blue = b; }
};

class csStandardLoader : public iLoader
{
  // The system driver (for error reporting and for queriyng other plugins)
  iSystem *system;
  // The Virtual File System
  iVFS *vfs;
  // The current world we are loading into
  iWorld *world;

public:
  DECLARE_IBASE;

  /// Initialize the loader object
  csStandardLoader (iBase *iParent);

  /// Destroy the loader object
  virtual ~csStandardLoader ();

  /// Set up the system driver and extract all other required plugins (VFS,...)
  virtual bool Initialize (iSystem *iSys);

  /**
   * Load the given file (from VFS) into engine.
   * This does not clear anything that could be already
   * loaded there; call ClearAll () if you need it.
   */
  virtual bool Load (const char *iName);

  /**
   * Parse a string and load the model from here.
   * It is not adviced to make heavy use of this feature;
   * some loaders (e.g. the standard one) can speed up loading
   * by preparsing files and storing them pre-tokenized data
   * somewhere near the original file; if you use this method
   * you will always go through the tokenizer first.
   * The caller should be prepared for iData to be modified.
   * This is not always the case (for example the standard loader
   * just temporarily modifies it and restores back) but in any case
   * for performance reasons this memory could be used.
   */
  virtual bool Parse (char *iData);

private:
  // The string list (all strings encountered in input)
  class csStringList : public csVector
  {
  public:
    csStringList (int ilimit = 64, int ithreshold = 64) :
      csVector (ilimit, ithreshold) {}
    void FreeAll ()
    { while (count--) delete [] Get (count); SetLength (0); }
    char *Get (int idx)
    { return (char *)csVector::Get (idx); }
  } *strings;

  // The token offsets for each tokenized line (for error reporting)
  csVector *lineoffs;
  // Input data (incremented as parser goes along)
  csTokenList input, startinput;
  // Current line for tokenizer
  int line, ofs;
  // Current token count offset
  csTokenList curtoken;
  // Total bytes in input stream left and number of similar tokens
  int bytesleft, curtokencount;
  // Current recursion depth
  int recursion_depth;

  // Prepare the parser
  bool yyinit (csTokenList iInput, size_t iSize);
  // Finalize the parser
  void yydone ();
  // The standard Bison error reporting function
  void yyerror (char *s);
  // The standard Bison tokenizer function
  int yylex (void *lval);
  // The parser function
  friend int yyparse (void *);

  // Temporary storage for parser
  struct yystorage
  {
    // A matrix
    csMatrix3 matrix;
    // Yet another matrix (used as scratch space by `matrix' non-terminal symbol)
    csMatrix3 matrix2;
    // Whenever the matrix has been defined
    bool matrix_valid;
    // A vector
    csVector3 vector;
    // Whenever the vector has been defined
    bool vector_valid;
    // Current texture prefix
    char *tex_prefix;
    // The name of currently loaded library/world
    char *cur_library;
    // A unnamed union containing miscelaneous parameters for different entities
    union
    {
      // Texture loader storage
      struct
      {
        // texture/file name
        char *name, *filename;
        // Transparent color
        csPColor transp;
        // Texture uses transparency?
        bool do_transp;
        // Texture flags
        int flags;
      } tex;
    };
  } storage;

  // Save the state of loader and call Load() recursively
  bool RecursiveLoad (const char *iName);

  // Tokenize a string and return a newly-allocated array of tokens
  csTokenList Tokenize (char *iData, size_t &oSize);

  // Initialize texture creation process
  void InitTexture (char *name);
  // Finish texture creation
  bool CreateTexture ();
};

#endif // __STDLDR_H__
