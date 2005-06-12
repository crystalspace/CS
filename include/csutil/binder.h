/*
    Copyright (C) 2003, 04 by Mathew Sutcliffe <oktal@gmx.co.uk>

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
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "iutil/evdefs.h"
#include "csutil/inputdef.h"
#include "csutil/array.h"
#include "csutil/hash.h"

/**
 * Use this class to bind input events (keypress, button press, mouse move,
 * etc.) to commands which are represented by an unsigned integer. It is
 * up to the application to specify the meaning of a command value.
 * <p>
 * Example:
 * \code
 * enum MyCommand = { Walk, Shoot, Jump, LookX, LookY };
 * ...
 * csRef<iInputBinder> binder = ...;
 * binder->BindButton (csInputDefinition ("ctrl"), Shoot);
 * binder->BindAxis (csInputDefinition ("mousex"), LookX);
 * ...
 * if (binder->Button (Shoot))
 *   ...
 * else
 * {
 *   DoSomething (binder->Axis (LookX), binder->Axis (LookY));
 * }
 * \endcode
 */
class CS_CRYSTALSPACE_EXPORT csInputBinder : public iInputBinder
{
  struct AxisCmd
  {
    unsigned cmd;
    int val, sens;
    bool wrap;
    AxisCmd (unsigned cmd0, int sens0)
    : cmd (cmd0), val (0), sens (sens0) {}
  };
  typedef csHash<AxisCmd *, csInputDefinition> AxisHash;
  AxisHash axisHash;
  csArray<AxisCmd *> axisArray;

  struct BtnCmd
  {
    unsigned cmd;
    bool down, toggle;
    BtnCmd (unsigned cmd0, bool toggle0)
    : cmd (cmd0), down (false), toggle (toggle0) {}
  };
  typedef csHash<BtnCmd *, csInputDefinition> BtnHash;
  BtnHash btnHash;
  csArray<BtnCmd *> btnArray;

protected:
  bool HandleEvent (iEvent&);

public:
  SCF_DECLARE_IBASE;

  /**
   * Create a new binder with an initial bindings hash size.
   * For optimum hash storage, size should be a prime number.
   */
  csInputBinder (iBase *parent = 0, int btnSize = 127, int axisSize = 13);
  virtual ~csInputBinder ();

  struct CS_CRYSTALSPACE_EXPORT eiEventHandler : public iEventHandler
  {
    SCF_DECLARE_EMBEDDED_IBASE (csInputBinder);
    bool HandleEvent (iEvent &ev) { return scfParent->HandleEvent (ev); }
  } scfiEventHandler;
  friend struct eiEventHandler;

  virtual iEventHandler* QueryHandler () { return &scfiEventHandler; }

  virtual int Axis (unsigned cmd);
  virtual bool Button (unsigned cmd);

  virtual void BindAxis (const csInputDefinition &, unsigned cmd, int sens);
  virtual void BindButton (const csInputDefinition &, unsigned cmd, bool tgl);

  virtual bool UnbindAxis (unsigned cmd);
  virtual bool UnbindButton (unsigned cmd);
  virtual void UnbindAll ();

  virtual void LoadConfig (iConfigFile *, const char *subsection);
  virtual void SaveConfig (iConfigFile *, const char *subsection);
};

#endif // __CS_UTIL_BINDER_H__
