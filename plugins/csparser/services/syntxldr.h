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


#include "imap/services.h"
#include "iutil/comp.h"
#include "iutil/dbghelp.h"
#include "csutil/csstring.h"
#include "csutil/strhash.h"

struct iObjectRegistry;
struct iPolygon3D;
struct iThingState;
struct iEngine;
struct iSector;
struct iMaterialWrapper;
struct iReporter;
struct iLoader;
struct iMeshObjectType;

/**
 * This component provides services for other loaders to easily parse
 * properties of standard CS world syntax. This implementation will parse
 * the textual representation.
 */
class csTextSyntaxService : public iSyntaxService
{
protected:
  float list[30];
  int num;
  csString text;
  iObjectRegistry* object_reg;
  iMeshObjectType* thing_type;
  iReporter* reporter;
  csStringHash xmltokens;

  void OptimizePolygon (iPolygon3D *p);

public:

  SCF_DECLARE_IBASE;
  csTextSyntaxService (iBase *parent);
  virtual ~csTextSyntaxService ();

  bool Initialize (iObjectRegistry* object_reg);

  virtual bool ParseMatrix (csParser* parser, char *buffer, csMatrix3 &m);
  virtual bool ParseVector (csParser* parser, char *buffer, csVector3 &v);
  virtual bool ParseMixmode (csParser* parser, char *buffer, uint &mixmode);
  virtual bool ParseShading (csParser* parser, char *buf, int &shading);
  virtual bool ParseTexture (
  	csParser* parser, 
	char *buf, const csVector3* vref, uint &texspec,
	csVector3 &tx_orig, csVector3 &tx1, csVector3 &tx2, csVector3 &len,
	csMatrix3 &tx_m, csVector3 &tx_v,
	csVector2 &uv_shift,
	int &idx1, csVector2 &uv1,
	int &idx2, csVector2 &uv2,
	int &idx3, csVector2 &uv3,
	char *plane, const char *polyname);

  virtual  bool ParseWarp (csParser* parser, 
			   char *buf, csVector &flags, bool &mirror,
  			   bool& warp, int& msv,
			   csMatrix3 &m, csVector3 &before, csVector3 &after);

  virtual bool ParsePoly3d (csParser* parser, 
			    iLoaderContext* ldr_context,
  			    iEngine* engine, iPolygon3D* poly3d, char* buf,
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
  	uint mixmode, int indent, bool newline = true);

  virtual bool ParseBool (iDocumentNode* node, bool& result, bool def_result);
  virtual bool ParseMatrix (iDocumentNode* node, csMatrix3 &m);
  virtual bool ParseVector (iDocumentNode* node, csVector3 &v);
  virtual bool ParseBox (iDocumentNode* node, csBox3 &v);
  virtual bool ParseColor (iDocumentNode* node, csColor &c);
  virtual bool ParseMixmode (iDocumentNode* node, uint &mixmode);
  virtual bool ParseTextureMapping (iDocumentNode* node,
  			     const csVector3* vref, uint &texspec,
			     csVector3 &tx_orig, csVector3 &tx1,
			     csVector3 &tx2, csVector3 &len,
			     csMatrix3 &tx_m, csVector3 &tx_v,
			     csVector2 &uv_shift,
			     int &idx1, csVector2 &uv1,
			     int &idx2, csVector2 &uv2,
			     int &idx3, csVector2 &uv3,
			     char *plane, const char *polyname);
  virtual  bool ParsePortal (iDocumentNode* node, iLoaderContext* ldr_context,
		  	   iPolygon3D* poly_3d,
		  	   csVector &flags, bool &mirror,
  			   bool& warp, int& msv,
			   csMatrix3 &m, csVector3 &before,
			   csVector3 &after);
  virtual bool ParsePoly3d (iDocumentNode* node,
   			    iLoaderContext* ldr_context,
  			    iEngine* engine, iPolygon3D* poly3d,
			    float default_texlen,
			    iThingState* thing_state, int vt_offset);

private:
  /// make it plugable
  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csTextSyntaxService);
    virtual bool Initialize (iObjectRegistry* object_reg)
    { return scfParent->Initialize (object_reg); }
  }scfiComponent;
  friend struct eiComponent;

public:
  // Debugging functions.
  iString* Debug_UnitTest ();

  struct DebugHelper : public iDebugHelper
  {
    SCF_DECLARE_EMBEDDED_IBASE (csTextSyntaxService);
    virtual int GetSupportedTests () const
    {
      return CS_DBGHELP_UNITTEST;
    }
    virtual iString* UnitTest ()
    {
      return scfParent->Debug_UnitTest ();
    }
    virtual iString* StateTest ()
    {
      return NULL;
    }
    virtual csTicks Benchmark (int /*num_iterations*/)
    {
      return 0;
    }
    virtual iString* Dump ()
    {
      return NULL;
    }
    virtual void Dump (iGraphics3D* /*g3d*/)
    {
    }
    virtual bool DebugCommand (const char* /*cmd*/)
    {
      return false;
    }
  } scfiDebugHelper;
};

#endif
