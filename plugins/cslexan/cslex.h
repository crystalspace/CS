/*
    Copyright (C) 2001 by Christopher Nelson
  
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

#ifndef __CS_CSLEX_H__
#define __CS_CSLEX_H__

#include "ivaria/lexan.h"
#include "isys/plugin.h"
#include "csutil/csdllist.h"
#include "csutil/csstring.h"

/**************************** Explanation of each State Code ************************

  1. Each opcode is an 8-bit unsigned integer to conserve table space
  2. All opcodes 1-254 equate precisely with their UTF-8 or ASCII equivalents
  3. All extended opcodes are also 8-bit unsigned quantities
  
  Each opcode is formed with:  op,mod
  Each extended opcode is formed with 0,op,mod,[possible extra data]
  
  When the engine encounters a MOD with an OP_MATCH set, it repeats the execution of the
   associated OP.  The one exception to this is the execution of an OP_EXT_NOP with an OP_MATCH
   set in it's MOD.  In this case, the engine checks the previous matching state.  If the state
   is compatible with continued matching specified in the OP_MATCH, the engine resets the
   instruction pointer to the value on top of the stack.  If the pattern fails to continue
   matching, then it will pop the stack and continue on.
    
 ***************************************************************************************/       

const unsigned char OP_MATCH_ONE_OR_NONE  = 1;		// '?'
const unsigned char OP_MATCH_ONE_OR_MORE  = 2;		// '+'
const unsigned char OP_MATCH_NONE_OR_MORE = 3;		// '*'
const unsigned char OP_PUSH_ADDRESS	  = 1<<3;	// used for repeat matching

const unsigned char OP_EXT_NOP		  = 0;		// op does nothing
const unsigned char OP_EXT_CUSTOM_TABLE   = 1;		// op is a custom table, length follows
const unsigned char OP_EXT_ALPHA_TABLE	  = 2;		// op matches isalpha
const unsigned char OP_EXT_DIGIT_TABLE	  = 3;		// op matches isdigit
const unsigned char OP_EXT_ALNUM_TABLE	  = 4;		// op matches isalnum
const unsigned char OP_EXT_PUNCT_TABLE	  = 5;		// op matches ispunct
const unsigned char OP_EXT_SPACE_TABLE	  = 6;		// op matches isspace
const unsigned char OP_EXT_CNTRL_TABLE	  = 7;		// op matches iscntrl
const unsigned char OP_EXT_GRAPH_TABLE	  = 8;		// op matches isgraph
const unsigned char OP_EXT_LOWER_TABLE	  = 9;		// op matches islower
const unsigned char OP_EXT_UPPER_TABLE	  = 10;		// op matches isupper
const unsigned char OP_EXT_PRINT_TABLE	  = 11;		// op matches isprint
const unsigned char OP_EXT_XDIGIT_TABLE	  = 12;		// op matches isxdigit
const unsigned char OP_EXT_LOGICAL_OR     = 13;		// turns on the OR flag in the VM

const unsigned char OP_ESC = 0;				// means that this is an extended op
const unsigned char OP_END = 255;			// this opcode occurs at the end of every sequence.  it has no modifier byte

const unsigned int RE_COMP_ERR_MISSING_RIGHT_PAREN   = 1;
const unsigned int RE_COMP_ERR_MISSING_LEFT_PAREN    = 2;
const unsigned int RE_COMP_ERR_MISSING_RIGHT_BRACKET = 3;
const unsigned int RE_COMP_ERR_MISSING_LEFT_BRACKET  = 4;
const unsigned int RE_COMP_ERR_UNKNOWN_CHAR_CLASS    = 5;

const unsigned int RE_EXEC_ERR_KEY_DOES_NOT_EXIST    = 1;


class csRegExp : public iRegExp
{
  /// The actual bytecode for the regular expression
  unsigned char *code;
  
  /// The length of the buffer for the regular expression
  unsigned length;

public:
  SCF_DECLARE_IBASE;

  csRegExp(iBase* = 0);
  virtual ~csRegExp();
  
public:
  /// Gets an opcode at the given index.  Returns false if the index is invalid
  virtual bool GetOp(unsigned index, unsigned char &op);
 	
  /** Sets the opcode at the given index.  Grows the code buffer if the index
   * is greater than the length.  Returns true on success.  Returns false if
   * index is an absurd value (currently > 0xfff)
   */
  virtual bool SetOp(unsigned index, unsigned char op);
  
  /** Fixes the code buffer so that it fits the size of the regular expression
   * exactly. Returns true on success
   */
  virtual bool Compact();
};


/* ---------------------- Compiler ------------------------------------------------------------*/

class csRegExpCompiler  : public iRegExpCompiler
{
 /// The current parentheses recursion depth
 unsigned int paren_recursion_depth;
 
 /// The string index that parenthesis recursion ended at
 unsigned int paren_recursion_ended_at;
   
 /// The current code segment index
 unsigned int op_index;
 
 /// The current regexp string index
 unsigned int re_index;
 
 /// The last error, only valid if compile returns false
 unsigned int compile_error;
 
public:
  SCF_DECLARE_IBASE;

  /// Intializes the compiler
  csRegExpCompiler(iBase* = 0);
  virtual ~csRegExpCompiler() {};
  
public:
  /// Compiles a character string into a regular expression.  The compiled RE is placed in "re"
  virtual bool Compile(char *regexp, iRegExp &re, unsigned int start=0, unsigned int end=0);
  
  /// Returns the last error.  Only valid when Compile returns false.
  virtual unsigned int GetError()
  { return compile_error; }
};


/* ---------------------- VM ------------------------------------------------------------*/

class csLexicalAnalyzer  : public iLexicalAnalyzer
{
  /// Data object which manages the key/RE relationship
  struct key_re_pair
  {
    iRegExp 		&re;
    unsigned int	key;
    
    key_re_pair(iRegExp &ire, unsigned int ikey) : re(ire), key(ikey) {};
  };
  
  /// Data object which manages stream state
  struct stream_state
  {
    iDataBuffer		*buf;
    unsigned int	pos;
  };
  
  /// Data object which manages execution state
  struct execution_state
  {
    unsigned int 	ip;
    execution_state(unsigned int iip) : ip(iip) {};
  };
  
  /// The list of registered regular expressions
  csDLinkList re_list;
  
  /// The list of buffers.  The one on the front is the top of the stack
  csDLinkList ss_list;
  
  /// The list of execution states.  Used as a stack
  csDLinkList es_list;
  
  /// The key counter (used for fetching new keys when registering RegExp's)
  unsigned int next_key;

  /// The last error encountered.
  unsigned int exec_error;
  
  /// The key of the last RE successfully matched
  unsigned int last_matched_key;
  
  /// The last matched text
  csString     last_matched_text;
    
public:
 SCF_DECLARE_IBASE;

 /// constructs the machine
 csLexicalAnalyzer(iBase* = 0);
 
 /// destroys the machine
 virtual ~csLexicalAnalyzer();
 
public:
 /// Initializes the bytecode machine (plugin derived)
 virtual bool Initialize (iObjectRegistry *object_reg);

 /// Allows an object to register a regular expression with a key used in a production.
 virtual bool RegisterRegExp(unsigned int key, iRegExp &re);
 
 /// Requests the removal of a regular expression given it's key.
 virtual bool UnregisterRegExp(unsigned int key);
 
protected:
 /// Execute the given regular expression on the current stream (internal fucntion)
 virtual bool Exec(iRegExp &re);
  
public: 
 /// Requests a new (free) key
 virtual unsigned int GetFreeKey()
 { return next_key++; }
 
 /// Returns the last error.  Value is only valid when a member function has returned false.
 unsigned int GetError()
 { return exec_error; }
 
public:
  /// Returns the key of the last RE matched
  virtual unsigned int GetMatchedKey();
  
  /**
   * Returns a pointer to the last matched text.  The reference count for the
   * returned object has already been incremented.  It is the caller's
   * responsibility to call DecRef() when finished with the returned object.
   */
  virtual iString *GetMatchedText();
 
public:
  /// Saves the current input stream and state, then resets the current state and uses stream from buf
  virtual bool PushStream(iDataBuffer &buf);
  
  /// Replaces the current input stream and state with the next stream and state on the stack
  virtual bool PopStream();
  
  /// Scans the input stream for a match by one of the RegularExpression's returns 0 on failure, otherwise returns the key of the matched RE.
  virtual unsigned int Match();

  // Implement iPlugin interface.
  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csLexicalAnalyzer);
    virtual bool Initialize(iObjectRegistry* p)
    { return scfParent->Initialize(p); }
    virtual bool HandleEvent(iEvent&) { return false; }
  } scfiPlugin;
};

#endif // __CS_CSLEX_H__
