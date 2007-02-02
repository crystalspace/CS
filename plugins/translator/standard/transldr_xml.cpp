/*
    Copyright (C) 2006 by Dariusz Dawidowski

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

#include "cssysdef.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/document.h"
#include "iutil/object.h"
#include "iutil/cfgmgr.h"
#include "ivaria/translator.h"
#include "transldr_xml.h"
#include "trans.h"

CS_PLUGIN_NAMESPACE_BEGIN(TransStd)
{

SCF_IMPLEMENT_FACTORY (csTranslatorLoaderXml)

csTranslatorLoaderXml::csTranslatorLoaderXml (iBase* parent) :
	scfImplementationType (this, parent), object_reg (0)
{
  InitTokenTable (tokens);
}

csTranslatorLoaderXml::~csTranslatorLoaderXml ()
{
}

bool csTranslatorLoaderXml::Initialize (iObjectRegistry *object_reg)
{
  csTranslatorLoaderXml::object_reg = object_reg;
  synldr = csQueryRegistry<iSyntaxService> (object_reg);
  return true;
}

void csTranslatorLoaderXml::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csRef<iReporter> rep = csQueryRegistry<iReporter> (object_reg);
  if (rep)
    rep->ReportV (severity, "crystalspace.translation.loader.xml", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

csPtr<iBase> csTranslatorLoaderXml::Parse (iDocumentNode* node,
	iStreamSource*, iLoaderContext* ldr_context, iBase* context)
{
  if (!node)
    return 0;
  bool found_language = false;
  csRef<csTranslator> trans;
  trans.AttachNew (new csTranslator (context));
  if (!trans)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Couldn't initialize csTranslator!");
    return 0;
  }
  csRef<iConfigManager> cfg = csQueryRegistry<iConfigManager> (object_reg);
  if (!cfg)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Couldn't load iConfigManager!");
    return 0;
  }
  const char* cfglang = 0;
  cfglang = cfg->GetStr ("Translator.Language", 0);
  if (!cfglang)
  {
    Report (CS_REPORTER_SEVERITY_ERROR,
    	"Couldn't found 'Translator.Language' in the cfg!");
    return 0;
  }
  csRef<iDocumentNodeIterator> it1 = node->GetNodes ();
  while (it1->HasNext ())
  {
    csRef<iDocumentNode> ch1 = it1->Next ();
    if (ch1->GetType () != CS_NODE_ELEMENT) continue;
    csStringID id1 = tokens.Request (ch1->GetValue ());
    switch (id1)
    {
      case XMLTOKEN_LANGUAGE:
      {
        const char* language = ch1->GetAttributeValue ("name");
        if (!language)
        {
          synldr->ReportError (
          	"crystalspace.translation.loader.xml",
          	node, "Missing 'name' attribute!");
          return 0;
        }
        if (!strcmp (cfglang, language))
        {
          found_language = true;
          csRef<iDocumentNodeIterator> it2 = ch1->GetNodes ();
          while (it2->HasNext ())
          {
            csRef<iDocumentNode> ch2 = it2->Next ();
            if (ch2->GetType () != CS_NODE_ELEMENT) continue;
            csStringID id2 = tokens.Request (ch2->GetValue ());
            switch (id2)
            {
              case XMLTOKEN_MSG:
              {
                csRef<iDocumentNodeIterator> it3 = ch2->GetNodes ();
                const char* src = 0;
                const char* dst = 0;
                while (it3->HasNext ())
                {
                  csRef<iDocumentNode> ch3 = it3->Next ();
                  if (ch3->GetType () != CS_NODE_ELEMENT) continue;
                  csStringID id3 = tokens.Request (ch3->GetValue ());
                  switch (id3)
                  {
                    case XMLTOKEN_SRC:
                      src = ch3->GetContentsValue ();
                    break;
                    case XMLTOKEN_DST:
                      dst = ch3->GetContentsValue ();
                    break;
                    default:
                      if (synldr) synldr->ReportBadToken (ch3);
                      return 0;
                  }
                }
                if (src && dst) trans->SetMsg (src, dst);
              }
              break;
              default:
                if (synldr) synldr->ReportBadToken (ch2);
                return 0;
            }
          }
        }
      }
      break;
      default:
        if (synldr) synldr->ReportBadToken (ch1);
        return 0;
    }
  }
  if (!found_language)
  {
    Report (CS_REPORTER_SEVERITY_ERROR,
    	"Couldn't found '%s' language!", cfglang);
    return 0;
  }
  csRef<iTranslator> old = csQueryRegistry<iTranslator> (object_reg);
  object_reg->Unregister (old, "iTranslator");
  if (!object_reg->Register (trans, "iTranslator"))
    Report (CS_REPORTER_SEVERITY_ERROR,
    	"Couldn't register iTranslator!");
  return csPtr<iBase> (trans);
}

}
CS_PLUGIN_NAMESPACE_END(TransStd)
