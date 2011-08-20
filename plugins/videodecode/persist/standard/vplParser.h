/*
Copyright (C) 2010 by Alin Baciu

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __VPLPARSER_H__
#define __VPLPARSER_H__

#include "imap/reader.h"
#include "imap/writer.h"
#include "iutil/eventh.h"
#include "csutil/strhash.h"

#include <iutil/comp.h>
#include <csutil/scf_implementation.h>
#include <ivideodecode/mediastructs.h>
#include <ivideodecode/medialoader.h>

struct iReporter;
struct iPluginManager;
struct iSyntaxService;
struct iObjectRegistry;

/**
  * This is the implementation for our API and
  * also the implementation of the plugin.
  */
class vplParser : public scfImplementation2<vplParser,iLoaderPlugin, iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iReporter> reporter;
  csRef<iSyntaxService> synldr;

  csString          mediaPath;
  csString          mediaType;
  csArray<Language> languages;

  csStringHash xmltokens;
#define CS_TOKEN_ITEM_FILE "plugins/videodecode/persist/standard/vplParser.tok"
#include "cstool/tokenlist.h"
#undef CS_TOKEN_ITEM_FILE 

public:
  vplParser (iBase* parent);
  virtual ~vplParser ();

  // From iComponent.
  virtual bool Initialize (iObjectRegistry*);

  virtual bool IsThreadSafe() { return true; }

  /// Parse a given node and return a new object for it.
  virtual csPtr<iBase> Parse (iDocumentNode* node,
    iStreamSource*, iLoaderContext* ldr_context, iBase* context);
};

#endif // __VPLPARSER_H__
