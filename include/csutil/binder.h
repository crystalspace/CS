/*
    Copyright (C) 2002 by Mathew Sutcliffe <oktal@gmx.co.uk>

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

#ifndef __CSUTIL_BINDER_H__
#define __CSUTIL_BINDER_H__

#include "iutil/event.h"
#include "csutil/csevent.h"
#include "iutil/eventh.h"
#include "csutil/hashmap.h"

/**
 * Create a csHashMap key from an iEvent.
 * Used internally by csInputBinder.
 */
extern csHashKey csHashComputeEvent (iEvent *ev);

struct csEvBind
{
  void *x, *y;
};

class csInputBinder : public iEventHandler
{
  private:
    csHashMap *Hash;

  public:
    SCF_DECLARE_IBASE;

    /**
     * Create a new binder with initial bindings hash size size.
     * size should be a prime number.
     */
    csInputBinder (int size = 127);

    /**
     * Destructor does UnbindAll automatically.
     */
    virtual ~csInputBinder ();

    /**
     * Handle an event, a method of iEventHandler
     * This class can be registered with the event queue:
     * iEventQueue::RegisterListener(this, CSMASK_Input);
     */
    bool HandleEvent (iEvent &ev);

    /**
     * Bind one or two variables to an event.
     *  Bind one or two 'int's to a csevXXXMove type event.
     *   You can bind the two axes simultaneously or separately.
     *   If yvar is NULL and the event is a y-axis type, xvar is used as yvar.
     *  Or bind a button status '(int)bool' to a csevXXXUp/Down type event.
     * Will modify the existing binding if any.
     * It is recommended that you use this in conjuction with csParseKeyDef
     */
    void Bind (iEvent *ev, int *xvar = NULL, int *yvar = NULL);

    void Bind (csEvent &ev, int *xvar = NULL, int *yvar = NULL);

    /**
     * Remove a binding.
     */
    bool Unbind (iEvent *ev);

    bool Unbind (csEvent &ev);

    /**
     * Remove all bindings.
     */
    bool UnbindAll();
};

#endif // __CSUTIL_BINDER_H__
