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
struct iPolygon3D;
struct iThingState;
struct iEngine;
struct iMaterialWrapper;
struct iReporter;
struct iLoader;
struct iMeshObjectType;

class csTextSyntaxService : public iSyntaxService
{
protected:
  float list[30];
  int num;
  csString text;
  iObjectRegistry* object_reg;
  iMeshObjectType* thing_type;
  iReporter* reporter;

  void OptimizePolygon (iPolygon3D *p);

  iMaterialWrapper* FindMaterial (iEngine* engine, const char* name);

public:

  SCF_DECLARE_IBASE;
  csTextSyntaxService (iBase *parent);
  virtual ~csTextSyntaxService ();

  bool Initialize (iObjectRegistry* object_reg);

  virtual bool ParseMatrix (char *buffer, csMatrix3 &m);
  virtual bool ParseVector (char *buffer, csVector3 &v);
  virtual bool ParseMixmode (char *buffer, UInt &mixmode);
  virtual bool ParseShading (char *buf, int &shading);
  virtual bool ParseTexture (
  	char *buf, const csVector3* vref, UInt &texspec, 
	csVector3 &tx_orig, csVector3 &tx1, csVector3 &tx2, csVector3 &len,
	csMatrix3 &tx_m, csVector3 &tx_v,
	csVector2 &uv_shift,
	int &idx1, csVector2 &uv1,
	int &idx2, csVector2 &uv2,
	int &idx3, csVector2 &uv3,
	char *plane, const char *polyname);

  virtual  bool ParseWarp (char *buf, csVector &flags, bool &mirror, 
			   csMatrix3 &m, csVector3 &before, csVector3 &after);

  virtual bool ParsePoly3d (iEngine* engine, iPolygon3D* poly3d, char* buf,
			    float default_texlen,
			    iThingState* thing_state, int vt_offset);


  virtual const char* MatrixToText (
  	const csMatrix3 &m, int indent, bool newline=true);
  virtual const char* VectorToText (
  	const char *vname, const csVector3 &v, int indent, 
				    bool newline=true);
  virtual const char* VectorToText (
  	const char *vname, float x, float y, float z, int indent,
	bool newline=true);
  virtual const char* VectorToText (
  	const char *vname, const csVector2 &v, int indent,
	bool newline=true);
  virtual const char* VectorToText (
  	const char *vname, float x, float y, int indent,
	bool newline=true);

  virtual const char* BoolToText (
  	const char *vname, bool b, int indent, bool newline=true);

  virtual const char* MixmodeToText (
  	UInt mixmode, int indent, bool newline = true);

 private:
  /// make it plugable
  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csTextSyntaxService);
    virtual bool Initialize (iObjectRegistry* object_reg)
    { return scfParent->Initialize (object_reg); }
  }scfiComponent;
  friend struct eiComponent;
};

#endif
