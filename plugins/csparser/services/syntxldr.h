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

#ifndef __CS_SYNTAX_SERVICE_H__
#define __CS_SYNTAX_SERVICE_H__


#include "imap/services.h"
#include "iutil/comp.h"
#include "iutil/dbghelp.h"
#include "ivideo/shader/shader.h"
#include "csutil/csstring.h"
#include "csutil/scf_implementation.h"
#include "csutil/strhash.h"

struct iObjectRegistry;
struct iReporter;
struct csGradientShade;

CS_PLUGIN_NAMESPACE_BEGIN(SyntaxService)
{

#include "csutil/deprecated_warn_off.h"

/**
 * This component provides services for other loaders to easily parse
 * properties of standard CS world syntax. This implementation will parse
 * the textual representation.
 */
class csTextSyntaxService : 
  public scfImplementation2<csTextSyntaxService,
                            iSyntaxService,
                            iComponent>
{
protected:
  iObjectRegistry* object_reg;
  csRef<iReporter> reporter;
  csStringHash xmltokens;
#define CS_TOKEN_ITEM_FILE "plugins/csparser/services/syntxldr.tok"
#include "cstool/tokenlist.h"
  csRef<iShaderVarStringSet> strings;

  void ReportV (const char* msgid, int severity, 
	iDocumentNode* errornode, const char* msg, va_list arg);
  /**
   * Returns whether information relevant to world saving should be kept
   * (default behaviour is to discard such info).
   * \sa iEngine::GetSaveableFlag()
   */
  bool KeepSaveInfo ();

  bool ParseGradientShade (iDocumentNode* node, csGradientShade& shade);
  bool WriteGradientShade (iDocumentNode* node, const csGradientShade& shade);

public:
  csTextSyntaxService (iBase *parent);
  virtual ~csTextSyntaxService ();

  bool Initialize (iObjectRegistry* object_reg);

  virtual bool ParseBool (iDocumentNode* node, bool& result, bool def_result);
  virtual bool ParseBoolAttribute (iDocumentNode* node, const char* attrname,
  	bool& result, bool def_result, bool required);
  virtual bool WriteBool (iDocumentNode* node, const char* name, bool value);
  virtual bool ParseMatrix (iDocumentNode* node, csMatrix3 &m);
  virtual bool WriteMatrix (iDocumentNode* node, const csMatrix3& m);
  virtual bool ParsePlane (iDocumentNode* node, csPlane3 &p);
  virtual bool WritePlane (iDocumentNode* node, const csPlane3 &p);
  virtual bool ParseVector (iDocumentNode* node, csVector3 &v);
  virtual bool WriteVector (iDocumentNode* node, const csVector3& v);
  virtual bool ParseVector (iDocumentNode* node, csVector2 &v);
  virtual bool WriteVector (iDocumentNode* node, const csVector2& v);
  virtual bool ParseBox (iDocumentNode* node, csBox3 &v);
  virtual bool WriteBox (iDocumentNode* node, const csBox3& v);
  virtual bool ParseBox (iDocumentNode* node, csOBB &v);
  virtual bool WriteBox (iDocumentNode* node, const csOBB& v);
  virtual bool ParseColor (iDocumentNode* node, csColor &c);
  virtual bool WriteColor (iDocumentNode* node, const csColor& c);
  virtual bool ParseColor (iDocumentNode* node, csColor4 &c);
  virtual bool WriteColor (iDocumentNode* node, const csColor4& c);
  virtual bool ParseMixmode (iDocumentNode* node, uint &mixmode,
    bool allowFxMesh = false);
  virtual bool WriteMixmode (iDocumentNode* node, uint mixmode,
  	bool allowFxMesh);
  virtual bool HandlePortalParameter (
    iDocumentNode* child, iLoaderContext* ldr_context,
    csRef<csRefCount>& parseState, CS::Utility::PortalParameters& params,
    bool& handled);
  virtual bool ParseGradient (iDocumentNode* node, iGradient* gradient);
  virtual bool WriteGradient (iDocumentNode* node, iGradient* gradient);
  virtual bool ParseShaderVar (iLoaderContext* ldr_context,
      iDocumentNode* node, csShaderVariable& var,
      iStringArray* failedTextures);
  virtual csRef<iShaderVariableAccessor> ParseShaderVarExpr (
    iDocumentNode* node);
  virtual bool WriteShaderVar (iDocumentNode* node, csShaderVariable& var);
  virtual bool ParseAlphaMode (iDocumentNode* node, iStringSet* strings,
    csAlphaMode& alphaMode, bool allowAutoMode);
  virtual bool WriteAlphaMode (iDocumentNode* node, iStringSet* strings,
    const csAlphaMode& alphaMode);
  virtual bool ParseZMode (iDocumentNode* node, csZBufMode& zmode,
    bool allowZmesh);
  virtual bool WriteZMode (iDocumentNode* node, csZBufMode zmode,
    bool allowZmesh);
  virtual csPtr<iKeyValuePair> ParseKey (iDocumentNode* node);
  virtual bool WriteKey (iDocumentNode* node, iKeyValuePair* keyvalue);

  virtual csRef<iRenderBuffer> ParseRenderBuffer (iDocumentNode* node);
  virtual bool ParseRenderBuffer (iDocumentNode* node, iRenderBuffer* buffer);
  virtual bool WriteRenderBuffer (iDocumentNode* node, iRenderBuffer* buffer);
  /**
   * Read a render buffer from a data buffer. If \a filename is not 0, the
   * returned buffer will also exhibit an iRenderBufferPersistence that returns
   * the given filename.
   */
  csRef<iRenderBuffer> ReadRenderBuffer (iDataBuffer* buf, const char* filename);
  virtual csRef<iRenderBuffer> ReadRenderBuffer (iDataBuffer* buf)
  { return ReadRenderBuffer (buf, 0); }
  virtual csRef<iDataBuffer> StoreRenderBuffer (iRenderBuffer* rbuf);

  virtual csRef<iShader> ParseShaderRef (iLoaderContext* ldr_context,
      iDocumentNode* node);
  virtual csRef<iShader> ParseShader (iLoaderContext* ldr_context,
      iDocumentNode* node);

  virtual void ReportError (const char* msgid, iDocumentNode* errornode,
	const char* msg, ...);
  virtual void ReportBadToken (iDocumentNode* badtokennode);
  virtual void Report (const char* msgid, int severity, 
	iDocumentNode* errornode, const char* msg, ...);
};

#include "csutil/deprecated_warn_on.h"

}
CS_PLUGIN_NAMESPACE_END(SyntaxService)

#endif // __CS_SYNTAX_SERVICE_H__
