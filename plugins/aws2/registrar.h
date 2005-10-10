/*
    Copyright (C) 2005 by Christopher Nelson

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

#ifndef _KEILA_FUNCTION_REGISTRY_H__
#define _KEILA_FUNCTION_REGISTRY_H__

#include "functor.h"
#include "split.h"

namespace aws
{

namespace autom
{
  /**
   * A scope object holds variables.  They are represented by a map from an
   * string to an object pointer.
   */
  class scope : public csRefCount
  {
    /** The type of map between a variable name and the object it maps. */
    typedef std::map<uint, keeper> variable_map_type;

    /** The the type of map between a scope name and the child scope it maps. */
    typedef std::map<uint, scope *> child_scope_map_type;

    /** The map of variable names. */
    variable_map_type vars;

    /** The map of child scopes. */
    child_scope_map_type children;

    /** The parent scope. */
    scope *parent;

  public:
    scope():parent(0) {}
    scope(scope *_parent):parent(_parent) {}
    ~scope() {}

    /** Return a keeper to the value. 
     * Will be nil if the value does not exist. 
     * Recursively checks parents in order to find
     * the value. */
    keeper get(const csString &name);  

    /** Return a keeper to the value. 
     * Will be nil if the value does not exist. 
     * Recursively checks parents in order to find
     * the value. */
    keeper get(uint id);      

    /** Set the value of a variable. */
    void set(const csString &name, keeper &val);   

    /** Set the value of a variable. */
    void set(uint id, keeper &val);   

    /** Adds a child scope to this scope. */
    void addChild(const csString &name, scope *child);

    /** Adds a child scope to this scope. */
    void addChild(uint id, scope *child);

    /** Finds the given child scope, returns null if it doesn't exist. */
    scope *findChild(const csString &name);

    /** Finds the given child scope, returns null if it doesn't exist. */
    scope *findChild(uint id);
  };


  /**
   * The registrar is a manager object, one per process, which keeps track
   * of all the registered functions.  It also supports containers that
   * essentially serve as namespaces.
   */
  class registrar
  {  
  private:
    struct container;
    
  public:        
      
    /** The type for function pointers. */
    typedef std::pair<function::slot_ptr, function::slot_mem_ptr> func_ptr;

    /** The type for mapping functions to names. */
    typedef std::map <uint, func_ptr> func_map_type;

    /** The type for mapping strings to numbers. */
    typedef std::map <csString, uint> string_num_map_type;      
    
  private:    
    /** The container is called the lobby. */
    func_map_type lobby;    

    /** The string to integer mapping facility. */
    string_num_map_type str_map;

    /** Used for incrementing the strings ids we have mapped. */
    uint str_id;
    
  public:
  
    registrar():lobby(), str_id(0) 
    {
    }
    
    ~registrar() 
    {    
    }
        
    /**
     * Performs lookup on the function.  All lookups start from the lobby and
     * work deeper. The function returns a pair: result, pointer.  If result
     * is true, the pointer is valid, otherwise the pointer is invalid.
     */
    std::pair<bool, func_ptr> lookup(const csString& name);
        
    /**
     * Register a function name with a function object.  It will create all
     * the needed containers in line to the function.  For example:
     *
     *  assign("size@user.profile.usage", fn_ptr)
     *
     * Would create containers user, profile, and usage; and then create size
     * inside the usage container. 
     */
    void assign(const csString &name, func_ptr func);

    /** 
     * Get's the id of the given string. 
     */
    uint getId(const csString &s)
    {
      string_num_map_type::iterator pos = str_map.find(s);

      if (pos == str_map.end())
      {
	str_map[s] = str_id;
	return str_id++;
      }
      else
      {
        return pos->second;
      }
    }

  };
  
  
  /** Returns the global registrar instance. */
  registrar *Registrar();

  /** Returns the global scope instance. */
  scope *GlobalScope();
  
  /** Returns the global nil object. */
  nil *Nil();

  /** Returns true if the object is Nil. */
  inline bool IsNil(object *o)
  { return o==Nil(); }

  /** Changes the scope of the object to the new scope 'sc'.  Only
   * affects objects which have a scope. (Variables, functions). */
  void Rescope(object *o, scope *sc);
  
  /**
   * Returns a fully populated object parsed from the given string, or zero
   * on failure.
   */
  object *Parse(std::string::iterator &pos, const std::string::iterator &end,
  	scope *sc=0);
  
  /**
   * Returns a fully populated object parsed from the given string, or zero
   * on failure.  Also performs the additional work of setting parent
   * functions when it finds a function.
   */
  object *ParseParameter(std::string::iterator &pos,
  	const std::string::iterator &end, function *parent, scope *sc=0);
  
  /** Returns a garbage-collected pointer that is safe for application usage. */
  keeper Compile(std::string &str);
  
  /**
   * Makes it easier to register a callback function for the automation
   * handler.
   */
  #define AUTOM_REGISTER(funcname, object_pointer, member_function)   \
    aws::autom::Registrar()->assign(funcname,          \
      std::make_pair(object_pointer,            \
      (aws::autom::function::slot_mem_ptr)member_function));

} // namespace autom

} // namespace aws

#endif
