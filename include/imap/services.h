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
class csVector2;

struct iSyntaxService : public iBase
{
  /********************* loading services *********************/  
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

  /********************* writing services *********************/

  /**
   * Transform the matrix into its textual representation.
   * <indent> is the number of leading spaces every line is prefixed with.
   * <newline> signals whether to append a newline to the result.
   */
  virtual const char* MatrixToText (const csMatrix3 &m, int indent, bool newline=true) = 0;

  /**
   * Transform the vector into its textual representation.
   * <indent> is the number of leading spaces every line is prefixed with.
   * <vname> is the vector specifier like VECTOR, V, W, etc.
   * <newline> signals whether to append a newline to the result.
   */
  virtual const char* VectorToText (const char *vname, const csVector3 &v, int indent, 
				    bool newline=true) = 0;
  virtual const char* VectorToText (const char *vname, float x, float y, float z, int indent,
				    bool newline=true) = 0;
  virtual const char* VectorToText (const char *vname, const csVector2 &v, int indent,
				    bool newline=true) = 0;
  virtual const char* VectorToText (const char *vname, float x, float y, int indent,
				    bool newline=true) = 0;

  /**
   * Transform the boolean into its textual representation.
   * <indent> is the number of leading spaces every line is prefixed with.
   * <vname> is the vector specifier like LIGHTING, MIPMAP, etc.
   * <newline> signals whether to append a newline to the result.
   */
  virtual const char* BoolToText (const char *vname, bool b, int indent, bool newline=true) = 0;

  /**
   * Transform the mixmode into its textual representation.
   * <indent> is the number of leading spaces every line is prefixed with.
   * <newline> signals whether to append a newline to the result.
   */
  virtual const char* MixmodeToText (UInt mixmode, int indent, bool newline=true) = 0;
};

#endif
