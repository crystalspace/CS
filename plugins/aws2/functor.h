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

#ifndef __KEILA_FUNCTION_OBJECT_H__
#define __KEILA_FUNCTION_OBJECT_H__

#include "object.h"
#include <map>

namespace aws
{

namespace autom
{
  /**
   * This is an executable function for Keila.  All actual execution is done
   * in native code since Keila doesn't really have a sort of mainline
   * bytecode context at the moment.  It may eventually if that
   * proves useful.
   */
  class function : public object
  {
  public:
    /** A ref counted parameter type, so we avoid memory leaks. */
    typedef keeper rc_parm;

    /** The map, uses the safe rc_parm. */
    typedef std::map<csString, rc_parm > parm_map_type;

    /** Functor slot. */
    struct slot
    {
    public:
      slot() {}
      virtual ~slot() {}
    };

    /** Functor signals for actual call-out. */
    class signal
    {
    private:
      slot *slot_object;
      rc_parm (slot::*memfunc)(function& fn);

    public:
      signal():slot_object(0), memfunc(0) {}
      signal(const signal& sig):slot_object(sig.slot_object),
      	memfunc(sig.memfunc) {}

      virtual ~signal() {}

      /** Returns true if the signal is valid. */
      bool Valid() { return slot_object!=0; }

      /** Connect this signal to a given slot. This is a 1:many relationship! */
      void bind(slot *_object, rc_parm (slot::*_memfunc)(function& fn))
      {
        slot_object=_object;
        memfunc=_memfunc;
      }

      /** Disconnect the signal. */
      void drop()
      {
        slot_object=0;
        memfunc=0;
      }

      /** Fire the signal. */
      rc_parm operator()(function& fn)
      {
        if (slot_object)
        {
          return (slot_object->*memfunc)(fn);
        }
        else
        {
          return rc_parm((object *)new string(
	  	"Keila: error: unbound function!"));
        }
      }
    };

    typedef slot * slot_ptr;
    typedef rc_parm (slot::*slot_mem_ptr)(function&);

  private:
    /** Parameters for the function. */
    parm_map_type parms;

    /** Return value. */
    rc_parm rv;

    /** Parent function. */
    function *parent;

    /** Set to true if this object should execute on representation. */
    bool repr_exec;

  protected:
    /** Binds this function object to some native code. */
    bool bind();

  public:
    function():object(T_FUNCTION), rv(0), parent(0), repr_exec(false) {}

    function(const std::string &_name, bool _exec_on_represent=false)
    	: object(T_FUNCTION), rv(0), parent(0), repr_exec(_exec_on_represent)
    {
      scfString s (_name.c_str());
      SetName (&s);
    }

    /** Copy constructor: does NOT copy the return value or the parameters! */
    function(const function& func):iObject(), object(T_FUNCTION), rv(0),
    	parent(0), repr_exec(func.repr_exec), Called(func.Called) {}

    /** Signal fired when function is called. */
    signal Called;

    /** Sets a return value parameter. */
    void setReturnValue(rc_parm value) { rv=value; }

    /** Returns the return value for this function. */
    rc_parm ReturnValue() { return rv; }

    /** Sets the parent of the function. */
    void setParent(function *_p) { parent = _p; }

    /** Adds the named parameter, compiling the value into a keila object. */
    bool addParm(const scfString &parm_name, std::string &_value);

    /** Adds the named parameter, assume that it's a string. */
    bool addStringParm(const scfString &parm_name, const csString &_value)
    {
      parms.insert(std::make_pair(parm_name, new string(_value)));
      return true;
    }

    /** Adds the named parameter, assume that it's an integer. */
    bool addIntParm(const scfString &parm_name, longlong _value)
    {
      parms.insert(std::make_pair(parm_name, new integer(_value)));
      return true;
    }

    /** Adds the named parameter, assume that it's a float. */
    bool addFloatParm(const scfString &parm_name, double _value)
    {
      parms.insert(std::make_pair(parm_name, new floating(_value)));
      return true;
    }

    /**
     * Copies all of the parameters from the given function into this
     * function.
     */
    void copyParms(const function &func)
    {
#if defined(CS_COMPILER_MSVC) && defined(_MSC_VER) && (_MSC_VER < 1300)
      // The old MSVC6 STL map<> lacks the ability to insert()
      // ranges from any type of iterator (it can only insert
      // a range of raw map<>::value_type pointers). Further,
      // the one-at-a-time insert() wants non-const iterator.
      function& f = const_cast<function&>(func);
      parm_map_type::iterator b = f.parms.begin();
      parm_map_type::iterator e = f.parms.end();
      for ( ; b != e; ++b)
        parms.insert(b, *b);
#else
      parms.insert(func.parms.begin(), func.parms.end());
#endif
    }

    /** Clones this function and sets the parent. */
    function *clone(function *parent)
    {
      function *cl = new function(*this);
      cl->setParent(parent);
      cl->copyParms(*this);
      return cl;
    }

    /** Converts the object into a string object, if possible. */
    virtual string ToString();

    /** Converts the object into an integer object, if possible. */
    virtual integer ToInt();

    /** Converts the object into a float object, if possible. */
    virtual floating ToFloat();

    /**
     * Converts the object into the text representation of it. This is the
     * inverse of parsing.
     */
    virtual csRef<iString> ReprObject();

    /**
     * Executes the function and returns the value the function returns
     * (if any.)
     */
    virtual rc_parm Execute()
    {
      if (Called.Valid() || bind()) rv=Called(*this);

      return rv;
    }

    /**
     * Parses an object out of a string.  The string is known to hold the
     * whole representation of some object.
     */
    virtual bool parseObject(std::string::iterator &pos,
    	const std::string::iterator &end);

    /** Returns the value of a parameter, uses scoped lookup. */
    rc_parm operator[](const csString& name);

  };

  typedef function::rc_parm func_parm;
  typedef function::slot    func_slot;
  typedef function::parm_map_type func_parm_map;

} // namespace autom

} // namespace aws

#endif
