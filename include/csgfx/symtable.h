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

#ifndef __CS_CSGFX_SYMTABLE_H__
#define __CS_CSGFX_SYMTABLE_H__

#include "csutil/strset.h"
#include "csutil/hashmap.h"
#include "csutil/array.h"

class csSymbolTable
{
private:
  struct Symbol
  {
    csStringID Name;
    void *Val;
    bool Auth;
    inline Symbol (csStringID id, void *value, bool authoritative)
    : Name (id), Val (value), Auth (authoritative) {}
  };

protected:
  csHashMap Hash;

  csArray<csSymbolTable *> Children;
  csSymbolTable *Parent;

  /// Inherit all symbols from the parent. @@@ Calling this twice will mess up.
  inline void SetParent (csSymbolTable *);

  /// Set the symbol in all children.
  inline void PropogateSymbol (csStringID name, void *value);

  /// Delete the symbol in all children.
  inline void PropogateDelete (csStringID name);

  /// Same as SetSymbol only if there is not already a symbol with that name.
  inline void SetSymbolSafe (csStringID name, void *value);

  /// Same as DeleteSymbol only if the symbol is inherited from the parent.
  inline void DeleteSymbolSafe (csStringID);

public:
  /// Construct the table with a hash of the given initial size,
  /// which should be a prime number, for optimisation reasons.
  csSymbolTable (int size) : Hash (size), Parent (0) {}

  /// Destruct the table.
  virtual ~csSymbolTable ();

  /// Add a child table which will inherit the symbols of this one.
  void AddChild (csSymbolTable *);

  /// Add child tables which will inherit the symbols of this one.
  void AddChildren (const csArray<csSymbolTable *> &);

  /// Add child tables which will inherit the symbols of this one.
  void AddChildren (csSymbolTable **, int len);

  /// Set the value of a symbol, or create a new one if it doesn't exist.
  void SetSymbol (csStringID name, void *value);

  /// SetSymbol for multiple symbols.
  void SetSymbols (const csArray<csStringID> &names, csArray<void *> &);

  /// SetSymbol for multiple symbols.
  void SetSymbols (csStringID *names, void **values, int len);

  /// Delete a symbol.
  bool DeleteSymbol (csStringID name);

  /// Delete multiple symbols.
  bool DeleteSymbols (const csArray<csStringID> &names);

  /// Delete multiple symbols.
  bool DeleteSymbols (csStringID *names, int len);

  /// Get the value of a symbol.
  bool GetSymbol (csStringID name, void *&value);

  /// Get the values of multiple symbols.
  bool GetSymbols (const csArray<csStringID> &names, csArray<void *> &values);

  /// Get the values of multiple symbols.
  bool GetSymbols (csStringID *names, void **values, int len);

  /// Check if a symbol exists.
  bool SymbolExists (csStringID name);

  /// Check if all of a set of symbols exist.
  bool SymbolsExist (const csArray<csStringID> &names);

  /// Check if all of a set of symbols exist.
  bool SymbolsExist (csStringID *names, int len);
};

#endif
