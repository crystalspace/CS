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

#ifndef _I_CS_SYNTAX_SERVICE_H_
#define _I_CS_SYNTAX_SERVICE_H_


/**
 * This component provides services for other loaders to easily parse properties of 
 * standard CS world syntax. This implementation will parse the textual representation.
 */

#include "imap/services.h"
#include "iutil/comp.h"
#include "csutil/csstring.h"

struct iObjectRegistry;

class csTextSyntaxService : public iSyntaxService
{
 protected:
  float list[30];
  int num;
  bool success;
  char *last_error;
  csString text;

  void SetError (const char *msg, ...);

 public:

  SCF_DECLARE_IBASE;
  csTextSyntaxService (iBase *parent);
  virtual ~csTextSyntaxService ();

  /// return the last error occured
  virtual const char *GetLastError ();

  virtual bool ParseMatrix (char *buffer, csMatrix3 &m);
  virtual bool ParseVector (char *buffer, csVector3 &v);
  virtual bool ParseMixmode (char *buffer, UInt &mixmode);
  virtual bool ParseShading (char *buf, int &shading);


  virtual const char* MatrixToText (const csMatrix3 &m, int indent, bool newline=true);
  virtual const char* VectorToText (const char *vname, const csVector3 &v, int indent, 
				    bool newline=true);
  virtual const char* VectorToText (const char *vname, float x, float y, float z, int indent,
				    bool newline=true);
  virtual const char* VectorToText (const char *vname, const csVector2 &v, int indent,
				    bool newline=true);
  virtual const char* VectorToText (const char *vname, float x, float y, int indent,
				    bool newline=true);

  virtual const char* BoolToText (const char *vname, bool b, int indent, bool newline=true);

  virtual const char* MixmodeToText (UInt mixmode, int indent, bool newline = true);


 private:
  /// make it plugable
  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csTextSyntaxService);
    virtual bool Initialize (iObjectRegistry *)
    {return true;}
  }scfiComponent;
  friend struct eiComponent;
};

#endif
