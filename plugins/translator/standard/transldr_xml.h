/*
    Copyright (C) 2006-2007 by Dariusz Dawidowski

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

#ifndef __CS_TRANSLDR_XML_H__
#define __CS_TRANSLDR_XML_H__

#include "iutil/comp.h"
#include "csutil/scf.h"
#include "csutil/scf_implementation.h"
#include "csutil/csstring.h"
#include "imap/reader.h"
#include "imap/services.h"
#include "trans.h"

struct iObjectRegistry;

CS_PLUGIN_NAMESPACE_BEGIN(TransStd)
{

/**
 * Implementation of the iTranslator API.
 */
class csTranslatorLoaderXml :public scfImplementation2<csTranslatorLoaderXml,
	iComponent, iLoaderPlugin>
{
private:
  iObjectRegistry* object_reg;
  csRef<csTranslator> trans;
  csRef<iSyntaxService> synldr;
  csStringHash tokens;
#define CS_TOKEN_ITEM_FILE "plugins/translator/standard/transldr_xml.tok"
#include "cstool/tokenlist.h"
#undef CS_TOKEN_ITEM_FILE
  void Report (int severity, const char* msg, ...);
  bool Process (iDocumentNode* node, const char* lang);

public:
  csTranslatorLoaderXml (iBase* parent);
  virtual ~csTranslatorLoaderXml ();
  virtual bool Initialize (iObjectRegistry *object_reg);
  virtual csPtr<iBase> Parse (iDocumentNode* node,
  	iStreamSource*, iLoaderContext* ldr_context, iBase* context);

  virtual bool IsThreadSafe() { return true; }
};

}
CS_PLUGIN_NAMESPACE_END(TransStd)

#endif // __CS_TRANSLDR_XML_H__
