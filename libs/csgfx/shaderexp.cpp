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

#include "cssysdef.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <ctype.h>

#include "csutil/scfarray.h"
#include "csutil/sysfunc.h"

#include "iutil/strset.h"
#include "iutil/document.h"
#include "iutil/objreg.h"

#include "csgfx/shaderexp.h"
#include "csgfx/shadervar.h"
#include "ivideo/shader/shader.h"

#if defined(CS_DEBUG)
#define DEBUG_PRINTF csPrintf
#else
#define DEBUG_PRINTF while(0) csPrintf
#endif

//#define SHADEREXP_DEBUG

enum 
{
  OP_INVALID = 0,

  // Arith operations
  /* @@@ These may be supplemented with type-specific ops for better
  optimizations, if types of variables can be guaranteed somehow. Types
  of constant expressions are not an issue, as they are evaluated once. */     
  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,

  // Vector operations
  OP_VEC_ELT1,
  OP_VEC_ELT2,
  OP_VEC_ELT3,
  OP_VEC_ELT4,

  // System functions...
  OP_FUNC_SIN,
  OP_FUNC_COS,
  OP_FUNC_TAN,
  OP_FUNC_DOT,
  OP_FUNC_CROSS,
  OP_FUNC_VEC_LEN, 
  OP_FUNC_NORMAL,

  OP_FUNC_ARCSIN,
  OP_FUNC_ARCCOS,
  OP_FUNC_ARCTAN,

  OP_FUNC_POW,
  OP_FUNC_MIN,
  OP_FUNC_MAX,

  OP_FUNC_TIME,
  OP_FUNC_FRAME,

  // Pseudo-ops, special case weird stuff
  OP_PS_MAKE_VECTOR,

  // Highest internal op number
  OP_LIMIT,

  // XML Special identifiers
  OP_XML_ATOM,
  OP_XML_SEXP,

  // Internal ops
  OP_INT_SELT12,
  OP_INT_SELT34,
  OP_INT_LOAD,
  
  OP_LAST
};

static const char* const opNames[OP_LAST] = {
  "!INVALID",
  "ADD",
  "SUB",
  "MUL",
  "DIV",
  "ELT1",
  "ELT2",
  "ELT3",
  "ELT4",
  "SIN",
  "COS",
  "TAN",
  "DOT",
  "CROSS",
  "VECLEN",
  "NORMAL",
  "ARCSIN",
  "ARCCOS",
  "ARCTAN",
  "POW",
  "MIN",
  "MAX",
  "TIME",
  "FRAME",
  "!MAKEVECTOR",
  "!LIMIT",
  "!ATOM",
  "!SEXP",
  "SELT12",
  "SELT34",
  "LOAD"
};

enum 
{
  TYPE_INVALID = 0,

  TYPE_NUMBER,
  TYPE_VECTOR2,
  TYPE_VECTOR3,
  TYPE_VECTOR4,
  TYPE_VARIABLE, // a shader variable

  TYPE_LIMIT,

  // Illegal anywhere but a cons cell
  TYPE_OPER,
  TYPE_CONS,

  // Special internal types
  TYPE_ACCUM,
  
  TYPE_LAST
};

static const char* const typeNames[TYPE_LAST] = {
  "!invalid",
  "num",
  "vec2",
  "vec3",
  "vec4",
  "var",
  "!limit",
  "!oper",
  "!cons",
  "!accum"
};

struct cons 
{
  csShaderExpression::oper_arg car;
  cons * cdr; // That's all it can be
  cons * cdr_rev; // Double-linked list for CDR.

  cons() : cdr(0), cdr_rev(0) {}
};

struct op_args_info 
{
  int min_args; /* -1 means no limit */
  int max_args;
  bool can_fold; /* if args < minimum, can this op be folded back into 
                 the parent cons list ? eg: (+ (+ 1) 2) == (+ 1 2) */
};

/*
Currently supported patterns:
0, 0  - No args
1, 1  - 1 arg
2, -1 - 2+ args, able to be parsed as (op 1 2 3 4) == (op 1 (op 2 (op 3 4))) 
x, y  - This will not do anything to atomic arguments. x <= y. Most of these will
require special handling during the compile stage.
*/
static const op_args_info optimize_arg_table[] = 
{
  { 0, 0, false }, //  OP_INVALID

  { 2, -1, true }, //  OP_ADD
  { 2, -1, true }, //  OP_SUB
  { 2, -1, true }, //  OP_MUL
  { 2, -1, true }, //  OP_DIV

  { 1, 1, false }, //  OP_VEC_ELT1
  { 1, 1, false }, //  OP_VEC_ELT2
  { 1, 1, false }, //  OP_VEC_ELT3
  { 1, 1, false }, //  OP_VEC_ELT4

  { 1, 1, false }, //  OP_FUNC_SIN
  { 1, 1, false }, //  OP_FUNC_COS
  { 1, 1, false }, //  OP_FUNC_TAN

  { 1, 1, false }, // OP_FUNC_DOT
  { 1, 1, false }, // OP_FUNC_CROSS
  { 1, 1, false }, // OP_FUNC_VEC_LEN
  { 1, 1, false }, // OP_FUNC_NORMAL

  { 1, 1, false }, //  OP_FUNC_ARCSIN
  { 1, 1, false }, //  OP_FUNC_ARCCOS
  { 1, 1, false }, //  OP_FUNC_ARCTAN

  { 2, 2, false }, // OP_FUNC_POW
  { 2, -1, true }, // OP_FUNC_MIN
  { 2, -1, true }, // OP_FUNC_MAX

  { 0, 0, false }, // OP_FUNC_TIME
  { 0, 0, false }, // OP_FUNC_FRAME

  { 2, 4, false }, // OP_PS_MAKE_VECTOR

  { 0, 0, false }, //  OP_LIMIT
};

/* Note on vector default values:
* - Constant vectors should default to x=0,y=0,z=0,w=1 for unspecified fields.
* - Variable vectors default to all 0. */

CS_LEAKGUARD_IMPLEMENT (csShaderExpression);

csShaderExpression::csShaderExpression(iObjectRegistry * objr) :
accstack_max(0)
{
  obj_reg = objr;

}

csShaderExpression::~csShaderExpression()
{

}

void csShaderExpression::ParseError (const char* message, ...) const
{
  if (!errorMsg.IsEmpty()) errorMsg << '\n';
  va_list args;
  va_start (args, message);
  errorMsg.AppendFmtV (message, args);
  va_end (args);
}

void csShaderExpression::EvalError (const char* message, ...) const
{
  if (!errorMsg.IsEmpty()) errorMsg << '\n';
  va_list args;
  va_start (args, message);
  errorMsg.AppendFmtV (message, args);
  va_end (args);
}

csShaderVariable* csShaderExpression::ResolveVar (csStringID name)
{
  if (!stacks) return 0;
  return csGetShaderVariableFromStack (stacks, name);
}

bool csShaderExpression::Parse(iDocumentNode * node)
{
  errorMsg.Empty();
  cons * head = new cons;

  strset = CS_QUERY_REGISTRY_TAG_INTERFACE (
    obj_reg, "crystalspace.shared.stringset", iStringSet);
  if (!strset) 
  {
    ParseError ("Can't find string registry.");

    return false;
  }

  if (!parse_xml(head, node)) 
  {
#ifdef SHADEREXP_DEBUG
    if (head)
      print_cons(head);
#endif
    destruct_cons(head);

    ParseError ("Failed to construct cons list.");

    return false;
  }

#ifdef SHADEREXP_DEBUG
  print_cons(head);
  csPrintf("\n***************\n");
#endif

  if (!eval_const(head))
  {
    destruct_cons(head);

    ParseError ("Failed to constant-eval cons list.");

    return false;
  }

#ifdef SHADEREXP_DEBUG
  print_cons(head);
  csPrintf("\n***************\n");
#endif

  int acc_top = 0;
  if (!compile_cons(head, acc_top))
  {
    destruct_cons(head);

    ParseError ("Failed to compile cons list to opcode array.");

    return false;
  }

#ifdef SHADEREXP_DEBUG
  print_ops(opcodes);
#endif

  opcodes.ShrinkBestFit();

  oper_arg tmp;
  tmp.type = TYPE_INVALID;
  tmp.vec4.Set(0.0f);
  accstack.SetLength(MAX(acc_top, accstack_max) + 1, tmp);

  destruct_cons(head);

  return true;
}

bool csShaderExpression::Evaluate(csShaderVariable* var, 
                                  csShaderVarStack& stacks)
{
  csRef<iShaderVarStack> wrapStacks;
  wrapStacks.AttachNew (new scfArrayWrap <iShaderVarStack, 
    csShaderVariable*, csShaderVarStack> (stacks));
  return Evaluate (var, wrapStacks);
}

bool csShaderExpression::Evaluate(csShaderVariable* var, 
                                  iShaderVarStack* stacks)
{
#ifdef SHADEREXP_DEBUG
  int debug_counter = 0;
#endif

  errorMsg.Empty();
  if (!opcodes.Length())
  {
    EvalError ("Empty expression");
    return false;
  }

  bool eval = true;
  this->stacks = stacks;

  oper_array::Iterator iter = opcodes.GetIterator();

  while (iter.HasNext())
  {
    const oper & op = iter.Next();

#ifdef SHADEREXP_DEBUG
    debug_counter++;
#endif

    if (op.arg1.type == TYPE_INVALID)
    {
      if (!eval_oper(op.opcode, accstack.Get(op.acc)))
      {
        eval = false;
        break;
      }
    } 
    else if (op.arg2.type == TYPE_INVALID)
    {
      if (!eval_oper(op.opcode, op.arg1, accstack.Get(op.acc)))
      {
        eval = false;
        break;
      }
    } 
    else 
    {
      if (!eval_oper(op.opcode, op.arg1, op.arg2, accstack.Get(op.acc)))
      {
        eval = false;
        break;
      }
    }

#ifdef SHADEREXP_DEBUG
    csPrintf("Eval result (op %3i): <ACC%i> <- ", debug_counter, op.acc);
    print_result(accstack.Get(op.acc));
    csPrintf("\n");
#endif    
  }

  bool ret = false;
  if (eval) ret = eval_argument(accstack.Get(0), var);

  this->stacks = 0;

  return ret;
}

bool csShaderExpression::eval_const(cons *& head)
{
  /* This pass is expected to do the following:
     - Ensure that arguments are correct. No arg-less ops or multiple-arg
       operators with single arguments.
     - In the case of single argument'd multi-arg ops, they are 
       inserted into the parent cons list, and the sub-cons 
       removed. This works nicely with the currently used constant 
       reduction algorithm. (+ (+ 1) 2) becomes (+ 1 2).
     - In the case of set # of argument ops, error if there aren't enough
       or too many.
     - (- <atom>) is resolved to (- 0 <atom>)
     - Sanity check, ensure that atom types are correct within 
       the cons tree (currently via CS_ASSERT).

     When modifying, please keep these in mind. The compiler and evaluator
     have limited error-checking for the above, and will produce incorrect
     results if they're not resolved. */

  cons * cell = head, * last = 0;
  int oper;

  if (cell->car.type <= TYPE_LIMIT) return true;

  if (cell->car.type != TYPE_OPER 
    && cell->cdr)
  {
    EvalError ("Cons head is not an operator, can't evaluate.");

    return false;
  }

  oper = head->car.oper;
  cell = head->cdr;

  if ((oper >= OP_LIMIT || oper <= OP_INVALID))
  {
    EvalError ("Unknown operator type in optimizer: %d.", oper);

    return false;
  }    

  if (!cell)
  {
    if (optimize_arg_table[oper].min_args != 0)
    {
      EvalError ("Operator with no arguments?");

      return false;
    } 
    else 
    {
      return true;
    }
  }

  /* Special case: (- 45) is functionally equiv to -45 or 0 - 45 */
  if (oper == OP_SUB &&
    !cell->cdr)
  {
    cons * zero = new cons;

    zero->car.type = TYPE_NUMBER;
    zero->car.num = 0.0;

    zero->cdr = cell;
    zero->cdr_rev = head;

    head->cdr = zero;
    cell->cdr_rev = zero;

    cell = zero;
  }

  /* @@@ this is begging to be broken up into functions */
  if (optimize_arg_table[oper].min_args == 1)
  {
    /* It is assumed in here that the output opcodes require only 1 argument. */

    if (cell->cdr)
    {
      EvalError ("Single argument operator \'%s\' has more than 1 argument.", 
        GetOperName (oper));

      return false;
    }

    switch (cell->car.type)
    {
    case TYPE_CONS: 
      {
        if (!eval_const(cell->car.cell))
          return false;

        cons * subcell = cell->car.cell;

        if (!subcell->cdr && subcell->car.type != TYPE_OPER)
        {
          CS_ASSERT(subcell->car.type == TYPE_NUMBER  ||
            subcell->car.type == TYPE_VECTOR2 ||
            subcell->car.type == TYPE_VECTOR3 ||
            subcell->car.type == TYPE_VECTOR4);

          cell->car = subcell->car;
          destruct_cons(subcell);

        } 
        else 
        {
          break;
        }

        CS_ASSERT(cell->car.type == TYPE_NUMBER  ||
          cell->car.type == TYPE_VECTOR2 ||
          cell->car.type == TYPE_VECTOR3 ||
          cell->car.type == TYPE_VECTOR4);
      } /* fallthrough */

    case TYPE_NUMBER:  
    case TYPE_VECTOR2:  
    case TYPE_VECTOR3:  
    case TYPE_VECTOR4: 
      {
        if (!eval_oper(oper, cell->car, head->car))
          return false;

        head->cdr = 0;
        destruct_cons(cell);
      } 
      break;

    case TYPE_VARIABLE:
      break;

    default:
      EvalError ("Unknown type in optimizer for argument: %" PRIu8 ".", 
        cell->car.type);

      return false;
    }

  } 
  else if (optimize_arg_table[oper].min_args > 1 && optimize_arg_table[oper].max_args < 0)
  {
    /* No limit to arguments. It is assumed that (op 1 2 3 4) can be chained, 
    eg: (op 1 (op 2 (op 3 4))), and that opcodes require 2 arguments */

    while (cell)
    {
      switch (cell->car.type)
      {
      case TYPE_CONS: 
        {
          if (!eval_const(cell->car.cell))
            return false;

          cons * subcell = cell->car.cell;

          if (!subcell->cdr)
          {
            if (subcell->car.type == TYPE_OPER)
            {
              cell = cell->cdr;
              last = 0;
            } 
            else 
            {
              CS_ASSERT(subcell->car.type == TYPE_NUMBER  ||
                subcell->car.type == TYPE_VECTOR2 ||
                subcell->car.type == TYPE_VECTOR3 ||
                subcell->car.type == TYPE_VECTOR4);

              cell->car = subcell->car;
              destruct_cons(subcell);

              last = cell->cdr_rev;

              if (last->car.type == TYPE_OPER ||
                last->car.type == TYPE_VARIABLE ||
                last->car.type == TYPE_CONS)
              {
                last = 0;
              } 
            }
          } 
          else 
          {
            cell = cell->cdr;
          }

        } 
        break;

      case TYPE_OPER:
        EvalError ("Shouldn't be an operator anywhere but the cons head.");

        return false;

      case TYPE_NUMBER:  
      case TYPE_VECTOR2:
      case TYPE_VECTOR3: 
      case TYPE_VECTOR4:
        if (!last)
        {
          last = cell;
          cell = cell->cdr;
        } 
        else 
        {
          if (!eval_oper(oper, last->car, cell->car, last->car))
            return false;

          cons * cptr = cell;
          cell = cell->cdr;
          cptr->cdr = 0;
          destruct_cons(cptr);
          if (cell)
            cell->cdr_rev = last;
          last->cdr = cell;

        } 
        break;

      case TYPE_VARIABLE:
        last = 0;
        cell = cell->cdr;

        break;

      default:
        EvalError ("Unknown type-id: %" PRIu8 "", cell->car.type);

        return false;
      }
    }
  } 
  else if (optimize_arg_table[oper].min_args <= optimize_arg_table[oper].max_args)
  {
    int argcount = 0;

    /* x, x where both X's are equal. Will not reduce arguments using the parent op, 
    but will try to optimize arguments to constant atoms. */

    while (cell)
    {
      if (cell->car.type == TYPE_CONS)
      {
        if (!eval_const(cell->car.cell))
          return false;

        cons * subcell = cell->car.cell;

        if (!subcell->cdr && subcell->car.type != TYPE_OPER)
        {
          CS_ASSERT(subcell->car.type == TYPE_NUMBER  ||
            subcell->car.type == TYPE_VECTOR2 ||
            subcell->car.type == TYPE_VECTOR3 ||
            subcell->car.type == TYPE_VECTOR4);

          cell->car = subcell->car;
          destruct_cons(subcell);
        }	
      }

      argcount++;
      cell = cell->cdr;
    }

    if (argcount < optimize_arg_table[oper].min_args || argcount > optimize_arg_table[oper].max_args)
    {
      EvalError ("Incorrect # of args (%d) to operator %s.", argcount, 
        GetOperName (oper));

      return false;
    }
  }

  if (optimize_arg_table[oper].can_fold &&
    head && head->cdr && !head->cdr->cdr)
  { // operator + single argument
    head->car = head->cdr->car;
    destruct_cons(head->cdr);
    head->cdr = 0;
  }

  return true;
}

bool csShaderExpression::eval_variable(csShaderVariable * var, oper_arg & out)
{
  csShaderVariable::VariableType type = var->GetType();

  switch (type)
  {
  case csShaderVariable::INT: 
    {
      int tmp;

      out.type = TYPE_NUMBER;
      var->GetValue(tmp);
      out.num = (float)tmp;
    } 
    break;

  case csShaderVariable::FLOAT: 
    out.type = TYPE_NUMBER;
    var->GetValue(out.num);
    break;

  case csShaderVariable::VECTOR2:
    out.type = TYPE_VECTOR2;
    var->GetValue(out.vec4); // @@@ relies on the fact that csShaderVariables don't check type.
    out.vec4.z = 0;
    out.vec4.w = 0.0f; // standard value for w
    break;

  case csShaderVariable::VECTOR3:
    out.type = TYPE_VECTOR3;
    var->GetValue(out.vec4);
    out.vec4.w = 0.0f; // standard value for w
    break;

  case csShaderVariable::VECTOR4:
    out.type = TYPE_VECTOR4;
    var->GetValue(out.vec4);
    break;

  default:
    EvalError ("Unknown type %d in shader variable, not usable in an expression.", 
      (int)type);
    return false;
  }

  return true;
}

bool csShaderExpression::eval_argument(const oper_arg & arg, csShaderVariable * out)
{
  switch (arg.type)
  {
  case TYPE_NUMBER:
    out->SetValue(arg.num);
    break;

  case TYPE_VECTOR2: 
    {
      csVector2 tmp;

      tmp.x = arg.vec4.x;
      tmp.y = arg.vec4.y;

      out->SetValue(tmp);
    } 
    break;

  case TYPE_VECTOR3: 
    {
      csVector3 tmp;

      tmp.x = arg.vec4.x;
      tmp.y = arg.vec4.y;
      tmp.z = arg.vec4.z;

      out->SetValue(tmp);
    } 
    break;

  case TYPE_VECTOR4:
    out->SetValue(arg.vec4);
    break;

  default:
    EvalError ("Unknown type %" PRIu8 " when converting arg to shader variable.", 
      arg.type);
    return false;
  }

  return true;
}

bool csShaderExpression::eval_oper(int oper, oper_arg arg1, oper_arg arg2, oper_arg & output)
{
  if (arg1.type == TYPE_VARIABLE)
  {
    csShaderVariable* var = ResolveVar (arg1.var);
    if (!var)
    {
      EvalError ("Cannot resolve variable name %s in symbol table.", 
        strset->Request(arg1.var));

      return false;
    }

    if (!eval_variable(var, arg1))
      return false;
  } 
  else if (arg1.type == TYPE_ACCUM)
  {
    arg1 = accstack.Get(arg1.acc);
  }

  if (arg2.type == TYPE_VARIABLE)
  {
    csShaderVariable * var = ResolveVar (arg2.var);
    if (!var)
    {
      EvalError ("Cannot resolve variable name %s in symbol table.", 
        strset->Request(arg2.var));

      return false;
    }

    if (!eval_variable(var, arg2))
      return false;
  } 
  else if (arg2.type == TYPE_ACCUM)
  {
    arg2 = accstack.Get(arg2.acc);
  }

  switch (oper)
  {
  case OP_ADD:  return eval_add(arg1, arg2, output);
  case OP_SUB:  return eval_sub(arg1, arg2, output);
  case OP_MUL:  return eval_mul(arg1, arg2, output);
  case OP_DIV:  return eval_div(arg1, arg2, output);
  case OP_FUNC_DOT:  return eval_dot(arg1, arg2, output);
  case OP_FUNC_CROSS: return eval_cross(arg1, arg2, output);
  case OP_FUNC_POW: return eval_pow(arg1, arg2, output);
  case OP_FUNC_MIN: return eval_min(arg1, arg2, output);
  case OP_FUNC_MAX: return eval_max(arg1, arg2, output);
  case OP_INT_SELT12: return eval_selt12(arg1, arg2, output);
  case OP_INT_SELT34: return eval_selt34(arg1, arg2, output);

  default:
    EvalError ("Unknown multi-arg operator %s (%d).", GetOperName (oper), oper);
  }

  return false;
}

bool csShaderExpression::eval_oper(int oper, oper_arg arg1, oper_arg & output)
{
  if (arg1.type == TYPE_VARIABLE)
  {
    csShaderVariable * var = ResolveVar (arg1.var);
    if (!var)
    {
      EvalError ("Cannot resolve variable name '%s' in symbol table.", 
        strset->Request(arg1.var));

      return false;
    }

    if (!eval_variable(var, arg1))
      return false;
  } 
  else if (arg1.type == TYPE_ACCUM)
  {
    arg1 = accstack.Get(arg1.acc);
  }

  switch (oper)
  {
  case OP_VEC_ELT1:	  return eval_elt1(arg1, output);
  case OP_VEC_ELT2:	  return eval_elt2(arg1, output);
  case OP_VEC_ELT3:	  return eval_elt3(arg1, output);
  case OP_VEC_ELT4:	  return eval_elt4(arg1, output);
  case OP_FUNC_SIN:	  return eval_sin(arg1, output);
  case OP_FUNC_COS:	  return eval_cos(arg1, output);
  case OP_FUNC_TAN:	  return eval_tan(arg1, output);
  case OP_FUNC_ARCSIN:  return eval_arcsin(arg1, output);
  case OP_FUNC_ARCCOS:  return eval_arccos(arg1, output);
  case OP_FUNC_ARCTAN:  return eval_arctan(arg1, output);
  case OP_FUNC_VEC_LEN: return eval_vec_len(arg1, output);
  case OP_FUNC_NORMAL: return eval_normal(arg1, output);

  case OP_INT_LOAD: return eval_load(arg1, output);

  default:
    EvalError ("Unknown single-arg operator %s (%d).", GetOperName (oper), oper);
  }

  return false;
}

bool csShaderExpression::eval_oper(int oper, oper_arg & output)
{
  switch (oper)
  {
  case OP_FUNC_TIME: return eval_time(output);
  case OP_FUNC_FRAME: return eval_frame(output);

  default:
    EvalError ("Unknown single-arg operator %s (%d).", GetOperName (oper), oper);
  }

  return false;
}

bool csShaderExpression::eval_add(const oper_arg & arg1, const oper_arg & arg2, oper_arg & output) const 
{
  if (arg1.type == TYPE_NUMBER && arg2.type == TYPE_NUMBER)
  {
    output.type = TYPE_NUMBER;
    output.num = arg1.num + arg2.num;
  } 
  else if (arg1.type != TYPE_NUMBER && arg2.type != TYPE_NUMBER)
  {
    CS_ASSERT(arg1.type == TYPE_VECTOR2 ||
      arg1.type == TYPE_VECTOR3 ||
      arg1.type == TYPE_VECTOR4);

    CS_ASSERT(arg2.type == TYPE_VECTOR2 ||
      arg2.type == TYPE_VECTOR3 ||
      arg2.type == TYPE_VECTOR4);

    output.type = (arg1.type > arg2.type) ? arg1.type : arg2.type; // largest vector type, 
    output.vec4 = arg1.vec4 + arg2.vec4;                           // you can coerce a vector up, eg. vec2->vec3, using a dummy expr, but not down.
  } 
  else 
  {
    EvalError ("Invalid types for operator, %s(%" PRIu8 ") + %s(%" PRIu8 ").", 
      GetTypeName (arg1.type), arg1.type, GetTypeName (arg2.type), 
      arg2.type);

    return false;
  }

  return true;
}

bool csShaderExpression::eval_sub(const oper_arg & arg1, const oper_arg & arg2, oper_arg & output) const 
{
  if (arg1.type == TYPE_NUMBER && arg2.type == TYPE_NUMBER)
  {
    output.type = TYPE_NUMBER;
    output.num = arg1.num - arg2.num;
  } 
  else if (arg1.type != TYPE_NUMBER && arg2.type != TYPE_NUMBER)
  {
    CS_ASSERT(arg1.type == TYPE_VECTOR2 ||
      arg1.type == TYPE_VECTOR3 ||
      arg1.type == TYPE_VECTOR4);

    CS_ASSERT(arg2.type == TYPE_VECTOR2 ||
      arg2.type == TYPE_VECTOR3 ||
      arg2.type == TYPE_VECTOR4);

    output.type = (arg1.type > arg2.type) ? arg1.type : arg2.type;
    output.vec4 = arg1.vec4 - arg2.vec4;
  } 
  else 
  {
    EvalError ("Invalid types for operator, %s - %s.", 
      GetTypeName (arg1.type), GetTypeName (arg2.type));

    return false;
  }

  return true;
}

bool csShaderExpression::eval_mul(const oper_arg & arg1, const oper_arg & arg2, oper_arg & output) const 
{
  if (arg1.type == TYPE_NUMBER && arg2.type == TYPE_NUMBER)
  {
    output.type = TYPE_NUMBER;
    output.num = arg1.num * arg2.num;

  } 
  else if (arg2.type == TYPE_NUMBER)
  { // obviously, arg1 doesn't
    CS_ASSERT(arg1.type == TYPE_VECTOR2 ||
      arg1.type == TYPE_VECTOR3 ||
      arg1.type == TYPE_VECTOR4);

    output.type = arg1.type;
    output.vec4 = arg1.vec4 * arg2.num;

  } 
  else if (arg1.type == TYPE_NUMBER)
  { // obviously, arg2 doesn't :)
    CS_ASSERT(arg2.type == TYPE_VECTOR2 ||
      arg2.type == TYPE_VECTOR3 ||
      arg2.type == TYPE_VECTOR4);

    output.type = arg2.type;
    output.vec4 = arg2.vec4 * arg1.num;

  } 
  else 
  {
    EvalError ("Invalid types for operator, %s * %s.", GetTypeName (arg1.type), GetTypeName (arg2.type));

    return false;
  }

  return true;  
}

bool csShaderExpression::eval_div(const oper_arg & arg1, const oper_arg & arg2, oper_arg & output) const {
  if (arg1.type == TYPE_NUMBER && arg2.type == TYPE_NUMBER)
  {
    output.type = TYPE_NUMBER;
    output.num = arg1.num / arg2.num;

  } 
  else if (arg2.type == TYPE_NUMBER)
  { 
    CS_ASSERT(arg1.type == TYPE_VECTOR2 ||
      arg1.type == TYPE_VECTOR3 ||
      arg1.type == TYPE_VECTOR4);

    output.type = arg1.type;
    output.vec4 = arg1.vec4 / arg2.num;

  } 
  else 
  {
    EvalError ("Invalid types for operator, %s / %s.", GetTypeName (arg1.type), GetTypeName (arg2.type));

    return false;
  }

  return true;  
}

bool csShaderExpression::eval_elt1(const oper_arg & arg1, oper_arg & output) const 
{
  if (arg1.type < TYPE_VECTOR2 || arg1.type > TYPE_VECTOR4)
  {
    EvalError ("Invalid type for first argument to elt1, %s.", GetTypeName (arg1.type));

    return false;
  }

  output.type = TYPE_NUMBER;
  output.num = arg1.vec4.x;

  return true;  
}

bool csShaderExpression::eval_elt2(const oper_arg & arg1, oper_arg & output) const 
{
  if (arg1.type < TYPE_VECTOR2 || arg1.type > TYPE_VECTOR4)
  {
    EvalError ("Invalid type for first argument to elt2, %s.", GetTypeName (arg1.type));

    return false;
  }

  output.type = TYPE_NUMBER;
  output.num = arg1.vec4.y;

  return true;  
}

bool csShaderExpression::eval_elt3(const oper_arg & arg1, oper_arg & output) const 
{
  if (arg1.type < TYPE_VECTOR3 || arg1.type > TYPE_VECTOR4)
  {
    EvalError ("Invalid type for first argument to elt3, %s.", GetTypeName (arg1.type));

    return false;
  }

  output.type = TYPE_NUMBER;
  output.num = arg1.vec4.z;

  return true;  
}

bool csShaderExpression::eval_elt4(const oper_arg & arg1, oper_arg & output) const 
{
  if (arg1.type != TYPE_VECTOR4)
  {
    EvalError ("Invalid type for first argument to elt4, %s.", GetTypeName (arg1.type));

    return false;
  }

  output.type = TYPE_NUMBER;
  output.num = arg1.vec4.w;

  return true;  
}

bool csShaderExpression::eval_sin(const oper_arg & arg1, oper_arg & output) const 
{
  if (arg1.type != TYPE_NUMBER)
  {
    EvalError ("Invalid type for first argument to sin, %s.", GetTypeName (arg1.type));

    return false;
  }

  output.type = TYPE_NUMBER;
  output.num = sin(arg1.num);

  return true;  
}

bool csShaderExpression::eval_cos(const oper_arg & arg1, oper_arg & output) const 
{
  if (arg1.type != TYPE_NUMBER)
  {
    EvalError ("Invalid type for first argument to cos, %s.", GetTypeName (arg1.type));

    return false;
  }

  output.type = TYPE_NUMBER;
  output.num = cos(arg1.num);

  return true;  
}

bool csShaderExpression::eval_tan(const oper_arg & arg1, oper_arg & output) const 
{
  if (arg1.type != TYPE_NUMBER)
  {
    EvalError ("Invalid type for first argument to tan, %s.", GetTypeName (arg1.type));

    return false;
  }

  output.type = TYPE_NUMBER;
  output.num = tan(arg1.num);

  return true;  
}

bool csShaderExpression::eval_arcsin(const oper_arg & arg1, oper_arg & output) const 
{
  if (arg1.type != TYPE_NUMBER)
  {
    EvalError ("Invalid type for first argument to arcsin, %s.", GetTypeName (arg1.type));

    return false;
  }

  output.type = TYPE_NUMBER;
  output.num = asin(arg1.num);

  return true;  
}

bool csShaderExpression::eval_arccos(const oper_arg & arg1, oper_arg & output) const 
{
  if (arg1.type != TYPE_NUMBER)
  {
    EvalError ("Invalid type for first argument to arccos, %s.", GetTypeName (arg1.type));

    return false;
  }

  output.type = TYPE_NUMBER;
  output.num = acos(arg1.num);

  return true;  
}

bool csShaderExpression::eval_arctan(const oper_arg & arg1, oper_arg & output) const 
{
  if (arg1.type != TYPE_NUMBER)
  {
    EvalError ("Invalid type for first argument to arctan, %s.", GetTypeName (arg1.type));

    return false;
  }

  output.type = TYPE_NUMBER;
  output.num = atan(arg1.num);

  return true;  
}

bool csShaderExpression::eval_dot(const oper_arg & arg1, const oper_arg & arg2, oper_arg & output) const 
{
  if (arg1.type != TYPE_VECTOR2 ||
    arg1.type != TYPE_VECTOR3 ||
    arg1.type != TYPE_VECTOR4)
  {
    EvalError ("Argument 1 to dot is not a vector.");

    return false;
  }

  if (arg2.type != TYPE_VECTOR2 ||
    arg2.type != TYPE_VECTOR3 ||
    arg2.type != TYPE_VECTOR4)
  {
    EvalError ("Argument 2 to dot is not a vector.");

    return false;
  }

  output.type = TYPE_NUMBER;
  output.num = arg1.vec4 * arg2.vec4;

  return true;
}

bool csShaderExpression::eval_cross(const oper_arg & arg1, const oper_arg & arg2, oper_arg & output) const 
{
  if (arg1.type != TYPE_VECTOR2 ||
    arg1.type != TYPE_VECTOR3 ||
    arg1.type != TYPE_VECTOR4)
  {
    EvalError ("Argument 1 to cross is not a vector.");

    return false;
  }

  if (arg1.type != arg2.type)
  {
    EvalError ("Argument 2 to cross doesn't match type of arg 1.");

    return false;
  }

  output.type = arg1.type;
  output.vec4 = arg1.vec4 % arg2.vec4;

  if (arg1.type != TYPE_VECTOR4)
    output.vec4.w = 0;

  if (arg1.type == TYPE_VECTOR2)
    output.vec4.z = 0;

  return true;
}

bool csShaderExpression::eval_vec_len(const oper_arg & arg1, oper_arg & output) const 
{
  if (arg1.type != TYPE_VECTOR2 ||
    arg1.type != TYPE_VECTOR3 ||
    arg1.type != TYPE_VECTOR4)
  {
    EvalError ("Argument to vec-len is not a vector.");

    return false;
  }

  output.type = TYPE_NUMBER;
  output.num = arg1.vec4.Norm();

  return true;
}

bool csShaderExpression::eval_normal(const oper_arg & arg1, oper_arg & output) const 
{
  if (arg1.type != TYPE_VECTOR2 ||
    arg1.type != TYPE_VECTOR3 ||
    arg1.type != TYPE_VECTOR4)
  {
    EvalError ("Argument to norm is not a vector.");

    return false;
  }

  output.type = arg1.type;
  output.vec4 = csVector4::Unit(arg1.vec4);

  return true;
}

bool csShaderExpression::eval_pow(const oper_arg & arg1, const oper_arg & arg2, oper_arg & output) const 
{
  if (arg1.type == TYPE_NUMBER && arg2.type == TYPE_NUMBER)
  {
    output.type = TYPE_NUMBER;
    output.num = pow(arg1.num, arg2.num);
  } 
  else 
  {
    EvalError ("Invalid types for operator, pow(%s, %s).", 
      GetTypeName (arg1.type), GetTypeName (arg2.type));

    return false;
  }

  return true;
}

bool csShaderExpression::eval_min(const oper_arg & arg1, const oper_arg & arg2, oper_arg & output) const 
{
  if (arg1.type == TYPE_NUMBER && arg2.type == TYPE_NUMBER)
  {
    output.type = TYPE_NUMBER;
    output.num = MIN(arg1.num, arg2.num);
  } 
  else 
  {
    EvalError ("Invalid types for operator, min(%s, %s).", 
      GetTypeName (arg1.type), GetTypeName (arg2.type));

    return false;
  }

  return true;
}

bool csShaderExpression::eval_max(const oper_arg & arg1, const oper_arg & arg2, oper_arg & output) const 
{
  if (arg1.type == TYPE_NUMBER && arg2.type == TYPE_NUMBER)
  {
    output.type = TYPE_NUMBER;
    output.num = MAX(arg1.num, arg2.num);
  } 
  else 
  {
    EvalError ("Invalid types for operator, max(%s, %s).", 
      GetTypeName (arg1.type), GetTypeName (arg2.type));

    return false;
  }

  return true;
}

bool csShaderExpression::eval_time(oper_arg & output) const 
{
  output.type = TYPE_NUMBER;
  output.num = 128;

  return true;
}

bool csShaderExpression::eval_frame(oper_arg & output) const 
{
  output.type = TYPE_NUMBER;
  output.num = 256;

  return true;
}

bool csShaderExpression::eval_selt12(const oper_arg & arg1, const oper_arg & arg2, oper_arg & output) const 
{
  if (arg1.type != TYPE_NUMBER || arg2.type != TYPE_NUMBER)
  {
    EvalError ("Arguments to selt12 aren't numbers.");

    return false;
  }

  output.type = TYPE_VECTOR2;
  output.vec4.x = arg1.num;
  output.vec4.y = arg2.num;

  return true;
}

bool csShaderExpression::eval_selt34(const oper_arg & arg1, const oper_arg & arg2, oper_arg & output) const 
{
  if (arg1.type != TYPE_NUMBER)
  {
    EvalError ("Arguments to selt34 aren't numbers.");

    return false;
  }

  output.type = TYPE_VECTOR3;
  output.vec4.z = arg1.num;

  if (arg2.type == TYPE_INVALID) 
    return true;

  if (arg2.type != TYPE_NUMBER)
  {
    EvalError ("Arguments to selt34 aren't numbers.");

    return false;
  }

  output.type = TYPE_VECTOR4;
  output.vec4.w = arg2.num;

  return true;
}

bool csShaderExpression::eval_load(const oper_arg & arg1, oper_arg & output) const 
{
  /* I really hope this is optimized by the compiler. */

  output = arg1;

  return true;
}

bool csShaderExpression::parse_xml(cons * head, iDocumentNode * node)
{
  csRef<iDocumentNodeIterator> iter (node->GetNodes());
  cons * cptr = head;
  csStringID tok = GetXmlTokenOp (node->GetValue());

  if (tok == OP_XML_ATOM)
  {
    const char * type = node->GetAttributeValue("type"), 
      * val  = node->GetContentsValue();

    if (!parse_xml_atom(cptr->car, 
      GetXmlType (type), 
      type,
      val))
    {
      return false;
    }

    cptr->cdr = 0;
  }
  else if (tok == OP_XML_SEXP)
  {
    return parse_sexp(head, node);
  }
  else if (tok <= OP_INVALID || tok >= OP_LIMIT)
  {
    ParseError ("Invalid XML token: '%s'.", node->GetValue());

    return false;
  }
  else
  {
    cptr->car.type = TYPE_OPER;
    cptr->car.oper = tok;

    while (iter->HasNext())
    {

      csRef<iDocumentNode> next_node (iter->Next());
      if (next_node->GetType() != CS_NODE_ELEMENT) continue;
      csStringID sub_tok = GetXmlTokenOp (next_node->GetValue());

      cptr->cdr = new cons;
      cptr->cdr->cdr_rev = cptr;
      cptr = cptr->cdr;

      if (sub_tok != OP_XML_ATOM &&
	  sub_tok != OP_XML_SEXP)
      {
        cptr->car.type = TYPE_CONS;
        cptr->car.cell = new cons;

        if (!parse_xml(cptr->car.cell, next_node)) 
          return false;

      }
      else
      {
        if (!parse_xml(cptr, next_node))
          return false;
      }

    }
  }

  return true;
}

bool csShaderExpression::parse_sexp(cons * head, iDocumentNode * node)
{
  const char * text = node->GetContentsValue();
  cons * cptr = head;

  if (!text || !*text) return false;
  
  while (isspace (*text)) text++;

  if (text[0] == '(') 
    return parse_sexp_form(text, cptr);
  else
    return parse_sexp_atom(text, cptr);

  return true;
}

bool csShaderExpression::parse_sexp_form(const char *& text, cons * head) {
  cons * cptr = head;

  CS_ASSERT(text[0] == '(');
  text++;

  /* Function name first */
  const char * tmp = text;
  while (!isspace(*tmp))
    tmp++;

  if (!(*tmp)) {
    ParseError("End of string inside form");

    return false;
  }

  int size = tmp - text;
  CS_ALLOC_STACK_ARRAY(char, tmp2, size + 1);
  memcpy(tmp2, text, size);
  tmp2[size] = 0;

  csStringID func_name = GetSexpTokenOp (tmp2);
  if (func_name <= OP_INVALID || func_name >= OP_LIMIT) {
    ParseError ("Invalid S-EXP function-name: '%s'.", tmp2);

    return false;
  }

  cptr->car.type = TYPE_OPER;
  cptr->car.oper = func_name;
  
  text = tmp + 1;
  while (*text != ')') {
    while (isspace(*text)) 
      text++;

    if (!(*text)) {
      ParseError("End of string inside form at %s<Here>", text - 20);

      return false;
    }

    if (*text != ')') {
      cptr->cdr = new cons;
      cptr->cdr->car.type = TYPE_INVALID;
      cptr->cdr->cdr_rev = cptr;
      cptr = cptr->cdr;

      if (text[0] == '(') {
	cptr->car.type = TYPE_CONS;
	cptr->car.cell = new cons;

	if (!parse_sexp_form(text, cptr->car.cell))
	  return false;
      } else {
	if (!parse_sexp_atom(text, cptr))
	  return false;
      }
    }
  }
  
  text++;

  return true;
}

bool csShaderExpression::parse_sexp_atom (const char *& text, cons * head) {
  if (isdigit(*text) || 
    (*text == '-' && isdigit(text[1])) ||
    (*text == '+' && isdigit(text[1])) ||
    (*text == '.' && isdigit(text[1]))) 
  { /* TYPE_NUMBER */
    const char * tmp = text;
    char * tmp3 = 0;

    while (!isspace(*tmp) && *tmp)
      tmp++;

    int size = tmp - text;
    CS_ALLOC_STACK_ARRAY(char, tmp2, size + 1);
    memcpy(tmp2, text, size);
    tmp2[size] = 0;

    double n = strtod(text, &tmp3);
  
    head->car.type = TYPE_NUMBER;
    head->car.num = n;

    text = tmp3;
  }
  else if (*text == '#' && text[1] == '(')
  { /* TYPE_VECTOR* */ 
    int args = 0;
    float arg[4];
    char * tmp = 0;

    text += 2;

    errno = 0;

    while (args < 4) {
      arg[args++] = (float)strtod(text, &tmp);

      if (isspace(*tmp))
        tmp++;

      if (*tmp == ')') {
	text = tmp; 

        break;
      }

      if (*tmp == 0) {
        ParseError ("End of parse string inside atom.");
        return false;
      }

      text = tmp;
    }

    if (*text != ')') 
    {
      ParseError ("Vector doesn't terminate with ')', or too many elements in vector. Error at position: %s", text);
      return false;
    }

    if (args == 4)
      head->car.type = TYPE_VECTOR4;
    else if (args == 3)
      head->car.type = TYPE_VECTOR3;
    else if (args == 2)
      head->car.type = TYPE_VECTOR2;
    else 
    {
      ParseError ("Odd number of elements in parsed vector: %d.", args);

      return false;
    }

    switch (args) {    /* Everything falls through */
    case 4:
      head->car.vec4.w = arg[3];
    case 3:
      head->car.vec4.z = arg[2];
    case 2:
      head->car.vec4.y = arg[1];
    default:
      head->car.vec4.x = arg[0];
    }

    text++;
  } 
  else if (isalpha(*text) || (*text == '"')) 
  { 
    /* TYPE_VARIABLE */
    bool quoted = *text == '"';
    if (quoted) text++;
    const char * tmp = text;

    while (*tmp 
      && ((!quoted && !isspace(*tmp) && (*tmp != ')')) 
	|| (quoted && (*tmp != '"')))) 
      tmp++;

    size_t size = tmp - text;
    CS_ALLOC_STACK_ARRAY(char, tmp2, size + 1);
    memcpy (tmp2, text, size);
    tmp2[size] = 0;

    head->car.type = TYPE_VARIABLE;
    head->car.var = strset->Request (tmp2);

    text = tmp;
    if (quoted) text++;
  } 
  else 
  {
    csString strSnippet (text);
    if (strSnippet.Length() > 16)
    {
      strSnippet.Truncate (13);
      strSnippet << "...";
    }
    ParseError ("Unrecognized item in SEXP parse string: %s", strSnippet.GetData());

    return false;
  }

  head->cdr = 0;
  CS_ASSERT(head->car.type != TYPE_INVALID);

  return true;
}

bool csShaderExpression::parse_xml_atom(oper_arg & arg, csStringID type, const char * type_str, const char * val_str)
{
  arg.type = type;

  switch (type)
  {
  case TYPE_NUMBER:
    {
      return parse_num_atom(val_str, arg);
    }
    break;

  case TYPE_VECTOR2:
    {
      float v1, v2;

      if (sscanf(val_str, "%f,%f", &v1, &v2) < 2)
      {
        ParseError ("Couldn't parse vector2: %s.", val_str);

        return false;
      }

      arg.vec4.Set(v1, v2, 0, 0);

    } break;

  case TYPE_VECTOR3:
    {
      float v1, v2, v3;

      if (sscanf(val_str, "%f,%f,%f", &v1, &v2, &v3) < 3)
      {
        ParseError ("Couldn't parse vector3: %s.", val_str);

        return false;
      }

      arg.vec4.Set(v1, v2, v3, 0);

    } break;

  case TYPE_VECTOR4:
    {
      float v1, v2, v3, v4;

      if (sscanf(val_str, "%f,%f,%f,%f", &v1, &v2, &v3, &v4) < 4)
      {
        ParseError ("Couldn't parse vector4: %s.", val_str);

        return false;
      }

      arg.vec4.Set(v1, v2, v3, v4);

    } break;

  case TYPE_VARIABLE:
    arg.var = strset->Request(val_str);

    break;

  default:
    ParseError ("Invalid type in atom: %s.", type_str);

    return false;
  }

  return true;
}

bool csShaderExpression::parse_num_atom(const char *& text, oper_arg & arg) {
  char * tmp = 0;

  errno = 0;
  
  double n = strtod(text, &tmp);
  
  if (*tmp)
    {
      ParseError ("Error parsing float at position %td.",
		  tmp - text);
      
      return false;
    }       
  
  if (errno)
    {
#ifdef CS_DEBUG
      perror("Error parsing float");
#endif
      return false;
    }

  arg.num = (float)n;

  return true;
}

bool csShaderExpression::compile_cons(const cons * cell, int & acc_top)
{
  int this_acc = acc_top;

  if (cell->car.type < TYPE_LIMIT)
  {
    oper tmp;

    tmp.opcode = OP_INT_LOAD;
    tmp.acc = this_acc;
    acc_top++;

    tmp.arg1 = cell->car;
    tmp.arg2.type = TYPE_INVALID;

    opcodes.Push(tmp);

    return true;
  }
  else
  {
    CS_ASSERT(cell->car.type == TYPE_OPER); /* should be guaranteed by preceeding passes */
  }

  int op = cell->car.oper;
  const cons * cptr = cell->cdr;

  if (this_acc > accstack_max)
    accstack_max = this_acc;

  /* Special cases */
  if (op == OP_PS_MAKE_VECTOR)
    return compile_make_vector(cptr, acc_top, this_acc);

  if (!cptr)
  { /* zero arg func */
    oper tmp;

    tmp.opcode = op;
    tmp.acc = this_acc;
    acc_top++;

    tmp.arg1.type = TYPE_INVALID;
    tmp.arg2.type = TYPE_INVALID;

    opcodes.Push(tmp);

    return true;
  }

  while (cptr)
  {
    oper tmp;

    CS_ASSERT((cptr->car.type > TYPE_INVALID && cptr->car.type < TYPE_LIMIT) || cptr->car.type == TYPE_CONS);

    tmp.opcode = op;
    tmp.acc = this_acc;

    if (cptr->car.type == TYPE_CONS)
    {
      if (!compile_cons(cptr->car.cell, acc_top)) 
        return false;

      if (acc_top > this_acc + 1)
      {
        tmp.arg1.type = TYPE_ACCUM;
        tmp.arg1.acc = this_acc;
        tmp.arg2.type = TYPE_ACCUM;
        tmp.arg2.acc = this_acc + 1;

        acc_top--;
      }
      else if (!cptr->cdr)
      {
        CS_ASSERT(acc_top > this_acc);

        tmp.arg1.type = TYPE_ACCUM;
        tmp.arg1.acc = acc_top - 1;

        tmp.arg2.type = TYPE_INVALID;
      }
      else
      {
        /* don't want to emit any code yet, there is no 
        useful second argument */

        cptr = cptr->cdr;
        continue;
      }

    }
    else if (acc_top > this_acc)
    {
      tmp.arg1.type = TYPE_ACCUM;
      tmp.arg1.acc = this_acc;
      tmp.arg2 = cptr->car;
    }
    else
    {
      if (cptr->cdr)
      {
        if (cptr->cdr->car.type != TYPE_CONS)
        {
          tmp.arg1 = cptr->car;
          tmp.arg2 = cptr->cdr->car;

          cptr = cptr->cdr;
        }
        else
        {
          tmp.opcode = OP_INT_LOAD;
          tmp.arg1 = cptr->car;
          tmp.arg2.type = TYPE_INVALID;
        }
      }
      else
      {
        tmp.arg1 = cptr->car;
        tmp.arg2.type = TYPE_INVALID;
      }

      acc_top++;
    }

    opcodes.Push(tmp);
    cptr = cptr->cdr;
  }

  return true;
}

bool csShaderExpression::compile_make_vector(const cons * cptr, int & acc_top, int this_acc)
{
  oper tmp;

  tmp.opcode = OP_INT_SELT12;
  tmp.acc = this_acc;

  if (cptr->car.type == TYPE_CONS)
  {
    tmp.arg1.type = TYPE_ACCUM;
    tmp.arg1.acc = acc_top;

    if (!compile_cons(cptr->car.cell, acc_top))
      return false;
  }
  else
  {
    tmp.arg1 = cptr->car;
  }

  cptr = cptr->cdr;

  if (cptr->car.type == TYPE_CONS)
  {
    tmp.arg2.type = TYPE_ACCUM;
    tmp.arg2.acc = acc_top;

    if (!compile_cons(cptr->car.cell, acc_top))
      return false;
  }
  else
  {
    tmp.arg2 = cptr->car;
  }

  opcodes.Push(tmp);

  acc_top = this_acc + 1;

  cptr = cptr->cdr;
  if (!cptr)
  {
    return true;
  }

  tmp.opcode = OP_INT_SELT34;
  tmp.acc = this_acc;

  if (cptr->car.type == TYPE_CONS)
  {
    tmp.arg1.type = TYPE_ACCUM;
    tmp.arg1.acc = acc_top;

    if (!compile_cons(cptr->car.cell, acc_top))
      return false;
  }
  else
  {
    tmp.arg1 = cptr->car;
  }

  cptr = cptr->cdr;
  if (!cptr)
  {
    acc_top = this_acc + 1;

    tmp.arg2.type = TYPE_INVALID;
    opcodes.Push(tmp);

    return true;
  }

  if (cptr->car.type == TYPE_CONS)
  {
    tmp.arg2.type = TYPE_ACCUM;
    tmp.arg2.acc = acc_top;

    if (!compile_cons(cptr->car.cell, acc_top))
      return false;
  }
  else
  {
    tmp.arg2 = cptr->car;
  }

  acc_top = this_acc + 1;
  opcodes.Push(tmp);

  return true;
}

void csShaderExpression::destruct_cons(cons * cell) const
{
  if (!cell)
    return;

  if (cell->car.type == TYPE_CONS)
    destruct_cons(cell->car.cell);

  destruct_cons(cell->cdr);

  delete cell;
}

void csShaderExpression::print_cons(const cons * head) const
{
  const cons * cell = head;

  csPrintf ("(");

  while (cell)
  {
    switch (cell->car.type)
    {
    case TYPE_CONS:
      csPrintf (" ");
      print_cons(cell->car.cell);
      break;

    case TYPE_OPER:
      csPrintf ("%s", GetOperName ((csStringID)cell->car.oper));
      break;

    case TYPE_NUMBER:
      csPrintf (" %f", cell->car.num);
      break;

    case TYPE_VECTOR2:
      csPrintf (" #(%f %f)", cell->car.vec4.x, cell->car.vec4.y);
      break;

    case TYPE_VECTOR3:
      csPrintf (" #(%f %f %f)", cell->car.vec4.x, cell->car.vec4.y, 
        cell->car.vec4.z);
      break;

    case TYPE_VECTOR4:
      csPrintf (" #(%f %f %f %f)", cell->car.vec4.x, cell->car.vec4.y, 
        cell->car.vec4.z, cell->car.vec4.w);
      break;

    case TYPE_VARIABLE:
      csPrintf (" \"%s\"", strset->Request(cell->car.var));
      break;

    default:
      csPrintf (" #<unknown type>");
    }

    cell = cell->cdr;
  }

  csPrintf (")");
}

void csShaderExpression::print_ops(const oper_array & ops) const
{
  oper_array::ConstIterator iter = ops.GetIterator();

  while (iter.HasNext())
  {
    const oper & op = iter.Next();

    csPrintf (" %s", GetOperName (op.opcode));

    if (op.arg1.type != TYPE_INVALID)
    {
      switch (op.arg1.type)
      {
      case TYPE_NUMBER:
        csPrintf (" %f", op.arg1.num);
        break;

      case TYPE_VECTOR2:
        csPrintf (" #(%f %f)", op.arg1.vec4.x, op.arg1.vec4.y);
        break;

      case TYPE_VECTOR3:
        csPrintf (" #(%f %f %f)", op.arg1.vec4.x, op.arg1.vec4.y, op.arg1.vec4.z);
        break;

      case TYPE_VECTOR4:
        csPrintf (" #(%f %f %f %f)", op.arg1.vec4.x, op.arg1.vec4.y, op.arg1.vec4.z, op.arg1.vec4.w);
        break;

      case TYPE_VARIABLE:
        csPrintf (" %s", strset->Request(op.arg1.var));
        break;

      case TYPE_ACCUM:
        csPrintf (" ACC%d", op.arg1.acc);
        break;

      default:
        csPrintf (" #<unknown type %" PRIu8 ">", op.arg1.type);
      }

    }

    if (op.arg2.type != TYPE_INVALID)
    {
      switch (op.arg2.type)
      {
      case TYPE_NUMBER:
        csPrintf (",%f", op.arg2.num);
        break;

      case TYPE_VECTOR2:
        csPrintf (",#(%f %f)", op.arg2.vec4.x, op.arg2.vec4.y);
        break;

      case TYPE_VECTOR3:
        csPrintf (",#(%f %f %f)", op.arg2.vec4.x, op.arg2.vec4.y, op.arg2.vec4.z);
        break;

      case TYPE_VECTOR4:
        csPrintf (",#(%f %f %f %f)", op.arg2.vec4.x, op.arg2.vec4.y, op.arg2.vec4.z, op.arg2.vec4.w);
        break;

      case TYPE_VARIABLE:
        csPrintf (",%s", strset->Request(op.arg2.var));
        break;

      case TYPE_ACCUM:
        csPrintf (",ACC%d", op.arg2.acc);
        break;

      default:
        csPrintf (",#<unknown type %" PRIu8 ">", op.arg2.type);
      }
    }

    csPrintf (" -> ACC%d\n", op.acc);
  }
}

void csShaderExpression::print_result(const oper_arg & arg) const {
  switch (arg.type)
    {
    case TYPE_NUMBER:
      csPrintf ("#<NUMBER %f>", arg.num);
      break;
      
    case TYPE_VECTOR2:
      csPrintf ("#<VECTOR2 (%f %f)>", arg.vec4.x, arg.vec4.y);
      break;
      
    case TYPE_VECTOR3:
      csPrintf ("#<VECTOR3 (%f %f %f)>", arg.vec4.x, arg.vec4.y, arg.vec4.z);
      break;
      
    case TYPE_VECTOR4:
      csPrintf ("#<VECTOR4 (%f %f %f %f)>", arg.vec4.x, arg.vec4.y, arg.vec4.z, arg.vec4.w);
      break;
      
    case TYPE_VARIABLE:
      csPrintf ("#<VARIABLEREF \"%s\">", strset->Request(arg.var));
      break;
      
    case TYPE_ACCUM:
      csPrintf ("#<ACCUMREF ACC%d>", arg.acc);
      break;
      
    default:
      csPrintf ("#<unknown type %" PRIu8 ">", arg.type);
    }
  
}

const char* csShaderExpression::GetTypeName (csStringID id)
{
  CS_ASSERT (id < TYPE_LAST);
  return typeNames[id];
}

const char* csShaderExpression::GetOperName (csStringID id)
{
  CS_ASSERT (id < OP_LAST);
  return opNames[id];
}

struct TokenTabEntry
{
  const char* token;
  size_t tokenLen;
  csStringID id;
};

static csStringID GetTokenID (const TokenTabEntry* tokenTab,
  size_t tokenCount, const char* token)
{
  const char* p = token;
  const size_t tokenLen = strlen (token);
  size_t pos = 0;
  size_t l = 0, h = tokenCount;
  while (l < h)
  {
    size_t m = (l+h) / 2;
    if (pos > tokenTab[m].tokenLen) return csInvalidStringID;
    const char* tabTok = tokenTab[m].token;
    int d = tabTok[pos] - *p;
    if (d == 0)
    {
      do
      {
        pos++; p++;
      } while (tabTok[pos] == *p);
      if (pos >= tokenLen)
        return tokenTab[m].id;
      continue;
    }
    else if (d < 0)
    {
      l = m+1;
    }
    else
    {
      h = m;
    }
  }
  return csInvalidStringID;
}

static const TokenTabEntry commonTokens[] = {
  {"arccos", 6, OP_FUNC_ARCCOS},
  {"arcsin", 6, OP_FUNC_ARCSIN},
  {"arctan", 6, OP_FUNC_ARCTAN},
  {"cos", 3, OP_FUNC_COS},
  {"cross", 5, OP_FUNC_CROSS},
  {"dot", 3, OP_FUNC_DOT},
  {"elt1", 4, OP_VEC_ELT1},
  {"elt2", 4, OP_VEC_ELT2},
  {"elt3", 4, OP_VEC_ELT3},
  {"elt4", 4, OP_VEC_ELT4},
  {"frame", 4, OP_FUNC_FRAME},
  {"make-vector", 11, OP_PS_MAKE_VECTOR},
  {"max", 3, OP_FUNC_MAX},
  {"min", 3, OP_FUNC_MIN},
  {"norm", 4, OP_FUNC_NORMAL},
  {"pow", 3, OP_FUNC_POW},
  {"sin", 3, OP_FUNC_SIN},
  {"tan", 3, OP_FUNC_TAN},
  {"time", 4, OP_FUNC_TIME},
  {"vec-len", 7, OP_FUNC_VEC_LEN}
};
const size_t commonTokenNum = sizeof(commonTokens)/sizeof(TokenTabEntry);

csStringID csShaderExpression::GetCommonTokenOp (const char* token)
{
  return GetTokenID (commonTokens, commonTokenNum, token);
}

static const TokenTabEntry xmlTokens[] = {
  {"add", 3, OP_ADD},
  {"atom", 4, OP_XML_ATOM},
  {"div", 3, OP_DIV},
  {"mul", 3, OP_MUL},
  {"sexp", 4, OP_XML_SEXP},
  {"sub", 4, OP_SUB}
};
const size_t xmlTokenNum = sizeof(xmlTokens)/sizeof(TokenTabEntry);

csStringID csShaderExpression::GetXmlTokenOp (const char* token)
{
  csStringID id = GetCommonTokenOp (token);
  if (id != csInvalidStringID) return id;
  return GetTokenID (xmlTokens, xmlTokenNum, token);
}

static const TokenTabEntry sexpTokens[] = {
  {"*", 1, OP_MUL},
  {"+", 1, OP_ADD},
  {"-", 1, OP_SUB},
  {"/", 1, OP_DIV}
};
const size_t sexpTokenNum = sizeof(sexpTokens)/sizeof(TokenTabEntry);

csStringID csShaderExpression::GetSexpTokenOp (const char* token)
{
  csStringID id = GetCommonTokenOp (token);
  if (id != csInvalidStringID) return id;
  return GetTokenID (sexpTokens, sexpTokenNum, token);
}

static const TokenTabEntry xmlTypeTokens[] = {
  {"num", 3, TYPE_NUMBER},
  {"var", 3, TYPE_VARIABLE},
  {"vec2", 4, TYPE_VECTOR2},
  {"vec3", 4, TYPE_VECTOR3},
  {"vec4", 4, TYPE_VECTOR4}
};
const size_t xmlTypeTokenNum = sizeof(xmlTypeTokens)/sizeof(TokenTabEntry);

csStringID csShaderExpression::GetXmlType (const char* token)
{
  return GetTokenID (xmlTypeTokens, xmlTypeTokenNum, token);
}
