/*
    Copyright (C) 2003 by Mathew Sutcliffe <oktal@gmx.co.uk>

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

#ifndef __CS_UTIL_BINDER_H__
#define __CS_UTIL_BINDER_H__

#include "csextern.h"
#include "iutil/binder.h"
#include "iutil/eventh.h"
#include "hashmap.h"

/**
 * Create a csHashMap key from an iEvent.
 * Used internally by csInputBinder.
 */
csHashKey csHashComputeEvent (iEvent* const);

/**
 * Bind an input event to a pointer to a variable so that that variable will
 * reflect the state of a given key, button or axis.
 */
class CS_CSUTIL_EXPORT csInputBinder : public iInputBinder
{
private:
  csHashMap Hash;

protected:
  bool HandleEvent (iEvent&);

public:
  SCF_DECLARE_IBASE;

  /**
   * Create a new binder with an initial bindings hash size.
   * For optimum hash storage, size should be a prime number.
   */
  csInputBinder (iBase *parent = 0, int size = 127);

  /**
   * Destructor invokes UnbindAll() automatically.
   */
  virtual ~csInputBinder ();

  struct CS_CSUTIL_EXPORT eiEventHandler : public iEventHandler
  {
    SCF_DECLARE_EMBEDDED_IBASE (csInputBinder);
    bool HandleEvent (iEvent &ev) { return scfParent->HandleEvent (ev); }
  } scfiEventHandler;
  friend struct eiEventHandler;

  /**
   * Get a pointer to the embedded event handler.
   * This class can be registered with the event queue:
   * iEventQueue::RegisterListener(this, CSMASK_Input);
   */
  virtual iEventHandler* QueryHandler () { return &scfiEventHandler; }

  /**
   * Bind a bool to a keyboard key or mouse or joystick button status.
   * If toggle is true, one press activates and the second deactivates.
   * Otherwise, keydown activates and keyup deactivates.
   */
  virtual void Bind (iEvent&, iInputBinderBoolean*, bool toggle = false);

  /**
   * Bind two integers to the x and y axes of a mouse or joystick.
   */
  virtual void Bind (iEvent&, iInputBinderPosition*);

  /**
   * Remove a binding.
   */
  virtual bool Unbind (iEvent&);

  /**
   * Remove all bindings.
   */
  virtual bool UnbindAll();
};

/**
 * Represents the position of a mouse or joystick axis, shared between modules.
 */
class CS_CSUTIL_EXPORT csInputBinderPosition : public iInputBinderPosition
{
private:
  /// The internally held value of the position.
  int p;

public:
  SCF_DECLARE_IBASE;

  /// Initialize constructor.
  csInputBinderPosition (int pp) : p (pp)
  { SCF_CONSTRUCT_IBASE (0); }
  /// Empty constructor.
  csInputBinderPosition () : p (0)
  { SCF_CONSTRUCT_IBASE (0); }
  /// Copy constructor.
  csInputBinderPosition (iInputBinderPosition *pp) : p (pp->Get ())
  { SCF_CONSTRUCT_IBASE (0); }
  /// Destructor.
  virtual ~csInputBinderPosition ()
  { SCF_DESTRUCT_IBASE(); }

  /// Set the position; called by csInputBinder.
  virtual void Set (int pp) { p = pp; }
  /// Get the position; called by the application.
  virtual int Get () const { return p; }
};

/**
 * Represents the up or down state of a keyboard key or a mouse or joystick
 * button, shared between modules.
 */
class CS_CSUTIL_EXPORT csInputBinderBoolean : public iInputBinderBoolean
{
private:
  /// The internally held state of the button.
  bool s;

public:
  SCF_DECLARE_IBASE;

  /// Initialize constructor.
  csInputBinderBoolean (bool ss) : s (ss)
  { SCF_CONSTRUCT_IBASE (0); }
  /// Empty constructor.
  csInputBinderBoolean () : s (0)
  { SCF_CONSTRUCT_IBASE (0); }
  /// Copy constructor.
  csInputBinderBoolean (iInputBinderBoolean *ss) : s (ss->Get ())
  { SCF_CONSTRUCT_IBASE (0); }
  /// Destructor.
  virtual ~csInputBinderBoolean ()
  { SCF_DESTRUCT_IBASE(); }

  /// Set the state; called by csInputBinder.
  virtual void Set (bool ss) { s = ss; }
  /// Get the state; called by the application.
  virtual bool Get () const { return s; }
};

#endif // __CS_UTIL_BINDER_H__
