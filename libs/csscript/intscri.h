/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

#ifndef INTSCRI_H
#define INTSCRI_H

#include "csscript/csscript.h"

//---------------------------------------------------------------------
// This class represents an implementation of csScript and csRunScript.
// This particular implementation supports the calling of normally
// compiled C functions linked with the executable of Crystal Space.
// For a C function to be callable from within a script it needs to be
// registered using the IntScriptRegister class.
//---------------------------------------------------------------------

class IntRunScript;

/**
 * The type that a function needs to have so that it can be
 * registered as a internal script.
 */
typedef bool (IntScriptFunc)(IntRunScript*, char*);

/**
 * This class holds all registered internal scripts.
 */
class IntScriptRegister
{
private:
  ///
  struct Reg
  {
    Reg* next;
    char* name;
    IntScriptFunc* func;
  };
  ///
  Reg* first;

public:
  ///
  IntScriptRegister ();
  ///
  ~IntScriptRegister ();

  /// Register a function as an internal script.
  void reg (char* name, IntScriptFunc* func);
  /// Find a named function that was previously registered.
  IntScriptFunc* find_function (char* name);
};

extern IntScriptRegister int_script_reg;

/**
 * Internal script.
 */
class IntScript : public csScript
{
  ///
  friend class IntRunScript;

private:
  /// Corresponding function to be called.
  IntScriptFunc* func;
  /// Userdata to give to the function.
  char* data;

public:
  ///
  IntScript (LanguageLayer* layer);
  ///
  virtual ~IntScript ();

  /// Load a script from the world file.
  void load (char* buf);

  ///
  virtual void prepare ();
  ///
  virtual csRunScript* run_script (csObject* attached);

  CSOBJTYPE;
};

/**
 * For every running script there is an instance of this class.
 */
class IntRunScript : public csRunScript
{
public:
  ///
  IntRunScript (IntScript* script, csObject* attached);
  ///
  virtual ~IntRunScript ();

  ///
  virtual void init ();
  ///
  virtual bool step ();
  ///
  virtual void deliver_event (csScriptEvent event);
};

#endif

