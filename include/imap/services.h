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

/**\file
 */
/**\addtogroup loadsave
 * @{ */
#include "csutil/scf.h"
#include "csutil/ref.h"

class csMatrix3;
class csVector3;
class csVector2;
class csVector;
class csColor;
class csBox3;
struct iPolygon3D;
struct iEngine;
struct iSector;
struct iMaterialWrapper;
struct iThingState;
struct iLoaderContext;
struct iDocumentNode;

/**\name Texture transformation description
 * @{ */
/// UV is given
#define CSTEX_UV 1  
/// vector1 is given
#define CSTEX_V1 2  
/// vector2 is given
#define CSTEX_V2 4  
/// explicit (u,v) <-> vertex mapping is given
#define CSTEX_UV_SHIFT 8 
/** @} */

SCF_VERSION (iSyntaxService, 1, 1, 4);

/**
 * This component provides services for other loaders to easily parse
 * properties of standard CS world syntax.
 */
struct iSyntaxService : public iBase
{
  /**
   * Transform the matrix into its textual representation.
   * <ul>
   * <li>indent: is the number of leading spaces every line is prefixed with.
   * <li>newline: signals whether to append a newline to the result.
   * </ul>
   */
  virtual const char* MatrixToText (const csMatrix3 &m, int indent,
  	bool newline=true) = 0;

  /**
   * Transform the vector into its textual representation.
   * <ul>
   * <li>indent: is the number of leading spaces every line is prefixed with.
   * <li>vname: is the vector specifier like VECTOR, V, W, etc.
   * <li>newline: signals whether to append a newline to the result.
   * </ul>
   */
  virtual const char* VectorToText (const char *vname, const csVector3 &v,
  	int indent, bool newline=true) = 0;
  virtual const char* VectorToText (const char *vname,
  	float x, float y, float z, int indent, bool newline=true) = 0;
  virtual const char* VectorToText (const char *vname, const csVector2 &v,
  	int indent, bool newline=true) = 0;
  virtual const char* VectorToText (const char *vname, float x, float y,
  	int indent, bool newline=true) = 0;

  /**
   * Transform the boolean into its textual representation.
   * <ul>
   * <li>indent: is the number of leading spaces every line is prefixed with.
   * <li>vname: is the vector specifier like LIGHTING, MIPMAP, etc.
   * <li>newline: signals whether to append a newline to the result.
   * </ul>
   */
  virtual const char* BoolToText (const char *vname, bool b, int indent,
    bool newline=true) = 0;

  /**
   * Transform the mixmode into its textual representation.
   * <ul>
   * <li>indent: is the number of leading spaces every line is prefixed with.
   * <li>newline: signals whether to append a newline to the result.
   * </ul>
   */
  virtual const char* MixmodeToText (uint mixmode, int indent,
  	bool newline=true) = 0;

  /**
   * Parse the value of this node and return a boolean depending
   * on this value. The following mapping happens (case insensitive):
   * <ul>
   * <li>yes -> true
   * <li>no -> false
   * <li>true -> true
   * <li>false -> false
   * <li>on -> true
   * <li>off -> false
   * <li>(empty value) -> (def_result)
   * <li>(everyting else) -> error
   * </ul>
   */
  virtual bool ParseBool (iDocumentNode* node, bool& result,
  	bool def_result) = 0;
  
  /**
   * Parse a matrix description. Returns true if successful.
   */
  virtual bool ParseMatrix (iDocumentNode* node, csMatrix3 &m) = 0;

  /**
   * Parse a vector description. Returns true if successful.
   */
  virtual bool ParseVector (iDocumentNode* node, csVector3 &v) = 0;

  /**
   * Parse a box description. Returns true if successful.
   */
  virtual bool ParseBox (iDocumentNode* node, csBox3 &v) = 0;

  /**
   * Parse a color description. Returns true if successful.
   */
  virtual bool ParseColor (iDocumentNode* node, csColor &c) = 0;

  /**
   * Parse a mixmode description. Returns true if successful.
   */
  virtual bool ParseMixmode (iDocumentNode* node, uint &mixmode) = 0;

  /**
   * Parse a texture mapping specification.
   * <ul>
   * <li>vref: is the array containing vertices which can be referenced
   *     by indices in the description.
   * <li>texspec: describes the data found for the texture transformation.
   *     It consists of or'ed CSTEX_.
   * <li>tx_orig, tx1, tx2, len: texture transformation is given by 3
   *     points describing a 3d space (third vector is implicitly given to
   *     be perpendicular on the 2 vectors described by the 3 points),
   * <li>width and height of the texture.
   * <li>tx_m and tx_v: if texture transformation is given explicitly by
   *     matrix/vector.
   * <li>uv_shift: contains UV_SHIFT value.
   * <li>idx? and uv?: if texture mapping is given explicitly by defining
   *     the u,v coordinate that belongs to vertex idx? of the polygon.
   * <li>plane: is the name of a plane defining the texture transformation.
   * <li>polyname: name of polygon to which this texture description belongs.
   *     This is used to make errormessages more verbose.
   * </ul>
   * \sa #CSTEX_UV
   */
  virtual bool ParseTextureMapping (iDocumentNode* node,
  			     const csVector3* vref, uint &texspec,
			     csVector3 &tx_orig, csVector3 &tx1,
			     csVector3 &tx2, csVector3 &len,
			     csMatrix3 &tx_m, csVector3 &tx_v,
			     csVector2 &uv_shift,
			     int &idx1, csVector2 &uv1,
			     int &idx2, csVector2 &uv2,
			     int &idx3, csVector2 &uv3,
			     char *plane, const char *polyname) = 0;

  /**
   * Parses a portal definition specification.
   * flags: contains all flags found in the description.
   */
  virtual  bool ParsePortal (iDocumentNode* node, iLoaderContext* ldr_context,
		  	   iPolygon3D* poly3d,
		  	   csVector &flags, bool &mirror,
  			   bool& warp, int& msv,
			   csMatrix3 &m, csVector3 &before,
			   csVector3 &after) = 0;


  /**
   * Parses a POLYGON.
   */
  virtual bool ParsePoly3d (iDocumentNode* node,
   			    iLoaderContext* ldr_context,
  			    iEngine* engine, iPolygon3D* poly3d,
			    float default_texlen,
			    iThingState* thing_state, int vt_offset) = 0;

  /**
   * Report an error and also gives a path in the XML tree.
   */
  virtual void ReportError (const char* msgid, iDocumentNode* errornode,
	const char* msg, ...) = 0;

  /**
   * Report a bad token. This is a conveniance function which will
   * eventually call ReportError().
   */
  virtual void ReportBadToken (iDocumentNode* badtokennode) = 0;

  /**
   * Report something, also gives a path in the XML tree.
   */
  virtual void Report (const char* msgid, int severity, 
    iDocumentNode* errornode, const char* msg, ...) = 0;
};

/** @} */

#endif

