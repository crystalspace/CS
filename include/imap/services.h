/*
    Copyright (C) 2001 by Norman Kraemer

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

#ifndef __CS_IMAP_SYNTAXSERVICE_H__
#define __CS_IMAP_SYNTAXSERVICE_H__

/**\file
 */
/**\addtogroup loadsave
 * @{ */
#include "csutil/scf.h"
#include "csutil/ref.h"
#include "ivideo/graph3d.h"

class csShaderVariable;
class csMatrix3;
class csVector3;
class csVector2;
class csColor;
class csBox3;
class csGradient;
struct csAlphaMode;
struct iEngine;
struct iSector;
struct iMaterialWrapper;
struct iThingFactoryState;
struct iLoaderContext;
struct iDocumentNode;
struct iString;
struct iStringSet;
struct iKeyValuePair;

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

SCF_VERSION (iSyntaxService, 1, 6, 0);

/**
 * This component provides services for other loaders to easily parse
 * properties of standard CS world syntax.
 */
struct iSyntaxService : public iBase
{
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
   * Write a node representing the value of the boolean.
   */
  virtual bool WriteBool (iDocumentNode* node, const char* name, 
    bool value) = 0;
  
  /**
   * Write a node representing the value of the boolean, if it differs from
   * a provided default value.
   */
  bool WriteBool (iDocumentNode* node, const char* name, bool value, 
    bool default_value) 
  { 
    if (value != default_value) 
      return WriteBool (node, name, value); 
    else
      return true;
  }
  
  /**
   * Parse a matrix description. Returns true if successful.
   */
  virtual bool ParseMatrix (iDocumentNode* node, csMatrix3 &m) = 0;

  /**
   * Write a matrix description. Returns true if successful.
   */
  virtual bool WriteMatrix (iDocumentNode* node, csMatrix3* m) = 0;

  /**
   * Parse a vector description. Returns true if successful.
   */
  virtual bool ParseVector (iDocumentNode* node, csVector3 &v) = 0;

  /**
   * Write a vector description. Returns true if successful.
   */
  virtual bool WriteVector (iDocumentNode* node, csVector3* v) = 0;

  /**
   * Parse a vector description. Returns true if successful.
   */
  virtual bool ParseVector (iDocumentNode* node, csVector2 &v) = 0;

  /**
   * Write a vector description. Returns true if successful.
   */
  virtual bool WriteVector (iDocumentNode* node, csVector2* v) = 0;

  /**
   * Parse a box description. Returns true if successful.
   */
  virtual bool ParseBox (iDocumentNode* node, csBox3 &v) = 0;

  /**
   * Write a box description. Returns true if successful.
   */
  virtual bool WriteBox (iDocumentNode* node, csBox3* v) = 0;

  /**
   * Parse a color description. Returns true if successful.
   */
  virtual bool ParseColor (iDocumentNode* node, csColor &c) = 0;

  /**
   * Write a color description. Returns true if successful.
   */
  virtual bool WriteColor (iDocumentNode* node, csColor* c) = 0;

  /**
   * Parse a color description. Returns true if successful.
   */
  virtual bool ParseColor (iDocumentNode* node, csColor4 &c) = 0;

  /**
   * Write a color description. Returns true if successful.
   */
  virtual bool WriteColor (iDocumentNode* node, csColor4* c) = 0;

  /**
   * Parse a mixmode description. Returns true if successful.
   */
  virtual bool ParseMixmode (iDocumentNode* node, uint &mixmode,
    bool allowFxMesh = false) = 0;

  /**
   * Write a mixmode description. Returns true if successful.
   */
  virtual bool WriteMixmode (iDocumentNode* node, uint mixmode,
    bool allowFxMesh) = 0;

  /**
   * Handles a common portal parameter.
   * flags: contains all flags found in the description.
   * Returns false on failure. Returns false in 'handled' if it couldn't
   * understand the token.
   */
  virtual bool HandlePortalParameter (
	iDocumentNode* child, iLoaderContext* ldr_context,
	uint32 &flags, bool &mirror, bool &warp, int& msv,
	csMatrix3 &m, csVector3 &before, csVector3 &after,
	iString* destSector, bool& handled, bool& autoresolve) = 0;

  /**
   * Parse a color gradient.
   */
  virtual bool ParseGradient (iDocumentNode* node,
			      csGradient& gradient) = 0;

  /**
   * Write a color gradient.
   */
  virtual bool WriteGradient (iDocumentNode* node,
			      csGradient* gradient) = 0;

  /**
   * Parse a shader variable declaration
   */
  virtual bool ParseShaderVar (iDocumentNode* node, 
    csShaderVariable& var) = 0;
			    
  /**
   * Write a shader variable declaration
   */
  virtual bool WriteShaderVar (iDocumentNode* node, 
    csShaderVariable* var) = 0;
			    
  /**
   * Report an error and also gives a path in the XML tree.
   */
  virtual void ReportError (const char* msgid, iDocumentNode* errornode,
	const char* msg, ...) CS_GNUC_PRINTF(4,5) = 0;

  /**
   * Report a bad token. This is a convenience function which will
   * eventually call ReportError().
   */
  virtual void ReportBadToken (iDocumentNode* badtokennode) = 0;

  /**
   * Report something, also gives a path in the XML tree.
   */
  virtual void Report (const char* msgid, int severity, 
    iDocumentNode* errornode, const char* msg, ...) CS_GNUC_PRINTF(5,6) = 0;
  
  /**
   * Parse an alphamode description. Returns true if successful.
   */
  virtual bool ParseAlphaMode (iDocumentNode* node, iStringSet* strings,
    csAlphaMode& alphaMode, bool allowAutoMode = true) = 0;
    
  /**
   * Write an alphamode description. Returns true if successful.
   */
  virtual bool WriteAlphaMode (iDocumentNode* node, iStringSet* strings,
    csAlphaMode* alphaMode) = 0;
    
  /**
   * Attempt to parse a zmode from \a node.
   * \a allowZmesh specifies whether ZMESH and ZMESH2 zmodes should be 
   * saved to \a zmode or rejected, causing the method to fail and return
   * 'false'.
   * \remark As z modes usually appear "in between" other document nodes,
   * this function does not report an error if the token isn't recognized,
   * but only returns 'false'.
   */
  virtual bool ParseZMode (iDocumentNode* node, csZBufMode& zmode,
    bool allowZmesh = false) = 0;

  /**
   * Write a ZMode description. Returns true if successful.
   */
  virtual bool WriteZMode (iDocumentNode* node, csZBufMode* zmode,
    bool allowZmesh) = 0;

  /**
   * Parse a key definition. A iKeyValuePair instance is
   * return in "keyvalue", with refcount 1
   * Returns true if successful.
   */
  virtual bool ParseKey (iDocumentNode* node, iKeyValuePair* &keyvalue) = 0;

  /**
   * Write a key definition and add the key to the given object, 
   * Returns true if successful.
   */
  virtual bool WriteKey (iDocumentNode* node, iKeyValuePair* keyvalue) = 0;

  /**
   * Parse a user render buffer.
   */
  virtual csRef<iRenderBuffer> ParseRenderBuffer (iDocumentNode* node) = 0;
};

/** @} */

#endif // __CS_IMAP_SYNTAXSERVICE_H__

