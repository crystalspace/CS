/*
    Copyright (C) 2001 by Norman Krämer
  
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

#ifndef _I_SYNTAXSERVICE_H_
#define _I_SYNTAXSERVICE_H_

#include "csutil/scf.h"

SCF_VERSION (iSyntaxService, 0, 0, 1);

/**
 * This component provides services for other loaders to easily parse properties of 
 * standard CS world syntax.
 */

class csMatrix3;
class csVector3;

struct iSyntaxService : public iBase
{
  /**
   * Retrieve the last error occured.
   */
  virtual const char *GetLastError () = 0;

  /**
   * Parse a MATRIX description. Returns true if successful.
   */
  virtual bool ParseMatrix (char *buffer, csMatrix3 &m) = 0;

  /**
   * Parse a VECTOR description. Returns true if successful.
   */
  virtual bool ParseVector (char *buffer, csVector3 &v) = 0;

  /**
   * Parse a MIXMODE description. Returns true if successful.
   */
  virtual bool ParseMixmode (char *buffer, UInt &mixmode) = 0;

  /**
   * Parse a SHADING description. Returns true if successful.
   */
  virtual bool ParseShading (char *buf, int &shading) = 0;
  
};

#endif
