/*
    Copyright (C) 2000 by Jorrit Tyberghein
    Written by Martin Geisse
  
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

#ifndef __IVARIA_STRSERV_H__
#define __IVARIA_STRSERV_H__

#include "isys/plugin.h"

SCF_VERSION (iStringServer, 0, 0, 1);

/**
 * The string server plug-in is mainly a list of strings. All strings in the
 * list are different. Every string has an ID number assigned to it. <p>
 *
 * You may request strings from the server. This will return the ID number
 * if the string is already on the list. Otherwise it adds a copy of the
 * string to the list and assigns a new ID number. You may also get the
 * string for a given ID number.
 */
struct iStringServer : public iPlugIn
{
  /// Request a string
  virtual int Request (const char *str) = 0;

  /// Get the string for the given ID
  virtual const char *GetString (int ID) const = 0;
};

#endif // __IVARIA_STRSERV_H__
