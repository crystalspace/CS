#ifndef LEXAN_H
#define LEXAN_H

#include "cssysdef.h"
#include "isys/plugin.h"

SCF_VERSION (iRegExp, 0, 0, 1);

/// Interface definition for a bytecode regular expression
struct iRegExp
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
struct iRegExpCompiler
{
  /// Compiles a character string into a regular expression.  The compiled RE is placed in "re"
  virtual bool Compile(char *regexp, iRegExp &re, unsigned int start=0, unsigned int end=0)=0;
  
  /// Returns the last error.  Only valid when Compile returns false.
  virtual unsigned int GetError()=0;
};


SCF_VERSION (iLexicalAnalyzer, 0, 0, 1);

/// The interface to the lexical analyzer / regular expression virtual machine
struct iLexicalAnalyzer : public iPlugIn
{
  /// Initializes the bytecode machine
  virtual bool Initialize (iSystem *sys) = 0;

};

#endif

