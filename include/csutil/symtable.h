/*
    Copyright (C) 2003 by Mat Sutcliffe <oktal@gmx.co.uk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
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

#ifndef __CS_CSUTIL_SYMTABLE_H__
#define __CS_CSUTIL_SYMTABLE_H__

#include "csutil/scf.h"
#include "csutil/ref.h"
#include "csutil/strhash.h"
#include "csutil/array.h"

class csShaderVariable;

/**
 * This class provides a system for storing inheritable properties, by allowing
 * instances to be stacked in a tree formation with parent/child relationships.
 * <p>
 * Used by the Graphics3D shader system.
 * <p>
 * @@@ Only stores csShaderVariable's, maybe it should be template-ized?
 */
class csSymbolTable
{
private:
  struct Stack
  {
    struct Symbol
    {
      csSymbolTable *Owner;
      csRef<csShaderVariable> Val;

      Symbol (csShaderVariable *value, csSymbolTable *owner);
      Symbol (const Symbol &other);
    };
    csArray<Symbol> Vals;
    csStringID Name;

    Stack (csStringID id);
    Stack (csStringID id, const csArray<Symbol> &vals);
    Stack (csStringID id, csShaderVariable *value, csSymbolTable *owner);
  };

protected:
  csHashMap Hash;

  csArray<csSymbolTable*> Children;
  csSymbolTable *Parent;

  void Propagate (const Stack *stack);

public:
  /**
   * Construct the table with a hash of the given initial size,
   * which should be a prime number, for optimisation reasons.
   * See the csHashMap docs for more info.
   */
  csSymbolTable (int size = 53) : Hash (size), Parent (0) {}

  /**
  * Construct the table with a hash of the given initial size,
  * which should be a prime number, for optimisation reasons.
  * See the csHashMap docs for more info.
  * Also copies auth variables from another symbol table.
  */
  csSymbolTable (const csSymbolTable &other, int size = 53);

  /**
  * Destruct the table and delete all variables.
  */
  ~csSymbolTable ();

  /// Add a child table which will inherit the symbols of this one.
  void AddChild (csSymbolTable *);

  /// Add child tables which will inherit the symbols of this one.
  void AddChildren (csArray<csSymbolTable*> &);

  /// Get the whole array of chlidren.
  csArray<csSymbolTable *> GetChildren ();

  /// Set the value of a symbol, or create a new one if it doesn't exist.
  void SetSymbol (csStringID name, csShaderVariable *value);

  /// SetSymbol for multiple symbols.
  void SetSymbols (const csArray<csStringID>&names,csArray<csShaderVariable*>&);

  /// Delete a symbol.
  bool DeleteSymbol (csStringID name);

  /// Delete multiple symbols.
  bool DeleteSymbols (const csArray<csStringID> &names);

  /// Get the value of a symbol.
  csShaderVariable* GetSymbol (csStringID name);

  /// Get the values of multiple symbols.
  csArray<csShaderVariable *> GetSymbols (const csArray<csStringID> &names);

  /// Get all the symbols.
  csArray<csShaderVariable *> GetSymbols ();

  /// Check if a symbol exists.
  bool SymbolExists (csStringID name) const;

  /// Check if all of a set of symbols exist.
  bool SymbolsExist (const csArray<csStringID> &names) const;
};

#endif
