/*
    Copyright (C) 2003 by Andrew Topp <designa@users.sourceforge.net>

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

#ifndef __CS_GFX_SHADEREXP_H__
#define __CS_GFX_SHADEREXP_H__

/**\file
 * An evaluable expression attached to a shader variable.
 */

#include "csextern.h"

#include "csutil/strhash.h"
#include "csutil/array.h"
#include "csutil/leakguard.h"
#include "csgeom/vector4.h"
#include "ivideo/shader/shader.h"

struct iObjectRegistry;
class csShaderVariable;
struct iShaderVariableContext;
struct iStringSet;
struct iDocumentNode;
struct cons;

/** 
 * An evaluable expression attached to a shader variable.
 */
class CS_CRYSTALSPACE_EXPORT csShaderExpression 
{
public:
  CS_LEAKGUARD_DECLARE (csShaderExpression);

  struct oper_arg 
  { 
    uint8 type;
    
    union 
    {
      float num;
      csStringID var;
      
      // Illegal outside of a cons cell
      int oper;
      cons * cell;

      // Special internal values
      int acc;
    };
    
    csVector4 vec4;
  };

  struct oper 
  {
    uint8 opcode, acc;
    oper_arg arg1, arg2;
  };

  typedef csArray<oper> oper_array;
  typedef csArray<oper_arg> arg_array;

private:
  iObjectRegistry * obj_reg;
  /// Indicates whether the token- and type-tables have been loaded.
  static bool loaded;
  //@{
  /**
   * Various internal hash tables
   */
  static csStringHash xmltokens;
  static csStringHash sexptokens;
  static csStringHash xmltypes;
  static csStringHash mnemonics;
  //@}
  /// Variables used for evaluation
  csShaderVarStack* stacks;
  /// String set for producing String IDs
  csRef<iStringSet> strset;
  /// Compiled array of opcodes for evaluation
  oper_array opcodes;
  /**
   * Used during compilation, set to the maximum allocated dimensions of
   * the accstack.
   */
  int accstack_max;
  /**
   * The accumulator stack.
   * Set to a static size after compilation, to save on allocation costs,
   * according to accstack_max.
   */
  arg_array accstack;

  /// Parse an XML X-expression
  bool parse_xml(cons *, iDocumentNode *);
  /// Parse a single X-expression data atom
  bool parse_xml_atom(oper_arg &, csStringID, const char *, const char *);
  /// Parse a Lisplike S-expression
  bool parse_sexp(cons *, iDocumentNode *);
  /** Parse an S-expression form
      Used to recurse down the S-expression's nested lists. */
  bool parse_sexp_form(const char *& text, cons *);
  /// Parse an S-expression atom 
  bool parse_sexp_atom(const char *& text, cons *);

  /// Compile a cons list into the oper_array.
  bool compile_cons(const cons *, int & acc_top);
  /// Compile a MAKE-VECTOR pseudo-op
  bool compile_make_vector(const cons *, int & acc_top, int acc);

  /// Evaluate away constant values 
  bool eval_const(cons *&);

  /// Evaluate an operator and 2 arguments
  bool eval_oper(int oper, oper_arg arg1, oper_arg arg2, oper_arg & output);
  /// Evaluate an operator with a single argument
  bool eval_oper(int oper, oper_arg arg1, oper_arg & output);
  /// Evaluate an operator without any arguments
  bool eval_oper(int oper, oper_arg & output);

  /// Add operator
  bool eval_add(const oper_arg & arg1, const oper_arg & arg2,
  	oper_arg & output) const;
  /// Subtraction operator
  bool eval_sub(const oper_arg & arg1, const oper_arg & arg2,
  	oper_arg & output) const;
  /// Multiplication operator
  bool eval_mul(const oper_arg & arg1, const oper_arg & arg2,
  	oper_arg & output) const;
  /// Division operator
  bool eval_div(const oper_arg & arg1, const oper_arg & arg2,
  	oper_arg & output) const;
  
  /// Element1 operator
  bool eval_elt1(const oper_arg & arg1, oper_arg & output) const;
  /// Element2 operator
  bool eval_elt2(const oper_arg & arg1, oper_arg & output) const;
  /// Element3 operator
  bool eval_elt3(const oper_arg & arg1, oper_arg & output) const;
  /// Element4 operator
  bool eval_elt4(const oper_arg & arg1, oper_arg & output) const;

  /// Sine operator
  bool eval_sin(const oper_arg & arg1, oper_arg & output) const;
  /// Cosine operator
  bool eval_cos(const oper_arg & arg1, oper_arg & output) const;
  /// Tangent operator
  bool eval_tan(const oper_arg & arg1, oper_arg & output) const;
  bool eval_dot(const oper_arg & arg1, const oper_arg & arg2,
  	oper_arg & output) const;
  bool eval_cross(const oper_arg & arg1, const oper_arg & arg2,
  	oper_arg & output) const;
  bool eval_vec_len(const oper_arg & arg1, oper_arg & output) const;
  bool eval_normal(const oper_arg & arg1, oper_arg & output) const;

  bool eval_pow(const oper_arg & arg1, const oper_arg & arg2,
  	oper_arg & output) const;
  bool eval_min(const oper_arg & arg1, const oper_arg & arg2,
  	oper_arg & output) const;
  bool eval_max(const oper_arg & arg1, const oper_arg & arg2,
  	oper_arg & output) const;

  /// Time function
  bool eval_time(oper_arg & output) const;
  /// Frame function
  bool eval_frame(oper_arg & output) const;

  /// Internal set vector element 1 and 2
  bool eval_selt12(const oper_arg & arg1, const oper_arg & arg2, oper_arg & output) const;
  /// Internal set vector element 3 and 4
  bool eval_selt34(const oper_arg & arg1, const oper_arg & arg2, oper_arg & output) const;
  /// Internal load operator
  bool eval_load(const oper_arg & arg1, oper_arg & output) const;

  /// Evaluate a variable into an oper_arg
  bool eval_variable(csShaderVariable *, oper_arg & out);
  /// Evaluate an oper_arg into a variable
  bool eval_argument(const oper_arg &, csShaderVariable *);

  /// Free all the memory used by a cons list
  void destruct_cons(cons *) const;
  /// Dump the contents of a cons list
  void print_cons(const cons *) const;
  /// Dump the opcode list
  void print_ops(const oper_array &) const;

  inline const char * get_type_name(csStringID id) const
  {
    return xmltypes.Request(id);
  }

  csShaderVariable* ResolveVar (csStringID name);

  csString* parseError;
  csString* evalError;
  csString errorMsg;
  void ParseError (const char* message, ...) const;
  void EvalError (const char* message, ...) const;
public:
  csShaderExpression(iObjectRegistry *);
  ~csShaderExpression();

  /// Parse in the XML in the context of a symbol table.
  bool Parse(iDocumentNode *);
  /**
   * Evaluate this expression into a variable.
   * It will use the symbol table it was initialized with.
   */
  bool Evaluate(csShaderVariable *, csShaderVarStack& stacks);

  /// Retrieve the error message if the evaluation or parsing failed.
  const char* GetError() const { return errorMsg; }
};

#endif
