/*
    Copyright (C) 2007 by Seth Yastrov

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

#ifndef __IEDITOR_ACTION_H__
#define __IEDITOR_ACTION_H__

#include <csutil/scf.h>
#include <csutil/scf_implementation.h>

#include <wx/string.h>

namespace CS {
namespace EditorApp {

/**
 * An undoable action. All reversible operations should implement this
 * interface so that they can be undone.
 */

struct iAction : public virtual iBase
{
  SCF_INTERFACE (iAction, 0, 0, 1);

  /**
   * Does the action.
   * \return An action representing the inverse operation that can be used
   *         to undo the action, or 0 if the action cannot be undone.
   */
  virtual csPtr<iAction> Do () = 0;

  /// Get a user-friendly description of the action.
  virtual const wxChar* GetDescription () const = 0;
};

} // namespace EditorApp
} // namespace CS

#endif
