/*
    Copyright (C) 2001 by Jorrit Tyberghein

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __IUTIL_COMP_H__
#define __IUTIL_COMP_H__

#include "csutil/scf.h"
struct iObjectRegistry;

SCF_VERSION (iComponent, 0, 0, 1);

/**
 * This interface describes a generic component in Crystal Space.
 */
struct iComponent : public iBase
{
  /// Initialize the component, and return success status.
  virtual bool Initialize (iObjectRegistry*) = 0;
};

#endif // __IUTIL_COMP_H__
