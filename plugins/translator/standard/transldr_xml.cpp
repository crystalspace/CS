/*
    Copyright (C) 2006, 2007 by Dariusz Dawidowski
    Copyright (C) 2007 by Amir Taaki

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
#include "csutil/cfgacc.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/document.h"
#include "iutil/object.h"
#include "iutil/cfgmgr.h"
#include "iutil/stringarray.h"
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

bool csTranslatorLoaderXml::Process (iDocumentNode* node, const char* lang)
{
  bool found_language = false;
  csRef<iDocumentNodeIterator> it1 = node->GetNodes ();
  while (lang && it1->HasNext ())
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
          	node, "Translator: Missing %s attribute!",
		CS::Quote::Single ("name"));
          return false;
        }
        if (!strcmp (lang, language))
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
                      return false;
                  }
                }
                if ((src && dst) && (strcmp (src, dst)))
                	trans->SetMsg (src, dst);
              }
              break;
              default:
                if (synldr) synldr->ReportBadToken (ch2);
                return false;
            }
          }
        }
      }
      break;
      default:
        if (synldr) synldr->ReportBadToken (ch1);
        return false;
    }
  }
  return found_language;
}

csPtr<iBase> csTranslatorLoaderXml::Parse (iDocumentNode* node,
	iStreamSource*, iLoaderContext* ldr_context, iBase* context)
{
  size_t start = 0;
  size_t pos = 0;
  if (!node)
    return 0;
  trans.AttachNew (new csTranslator (context));
  if (!trans)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Couldn't initialize csTranslator!");
    return 0;
  }
  // local and global configuration
  csConfigAccess cfg;
  cfg.AddConfig (object_reg, "/config/translator.cfg");
  const char* lang = 0;
  lang = cfg->GetStr ("Translator.Language", 0);
/*  if (!lang)
  {
    const char* syslang = 0;
    // Unix
    syslang = getenv ("LANG");
    // Windows
    //WCHAR wcBuffer [32];
    //if (GetSystemDefaultLocaleName (wcBuffer, 32) > 0)
    //{
    //  syslang = wcBuffer;
    //}
    csRef<iConfigIterator> it_alias (cfg->Enumerate ("Translator.Alias."));
    if (it_alias)
      while (it_alias->Next ())
      {
        csString keystr = it_alias->GetStr ();
        start = 0;
        pos = 0;
        do
        {
          pos = keystr.Find (" ", pos + 1);
          csString keyslice = keystr.Slice (start, pos - start);
          if (!strcmp (syslang, keyslice.GetData ()))
          {
            lang = it_alias->GetKey (true);
            break;
          }
          start = pos + 1;
        }
        while (pos != (size_t)-1);
        if (pos != (size_t)-1)
          break;
      }
  }*/
  if (lang)
  {
    if (!Process (node, lang))
    {
      char fallbackname [24];
      strcpy (fallbackname, "Translator.Fallback.");
      strcat (fallbackname, lang);
      csString fallback = cfg->GetStr (fallbackname);
      start = 0;
      pos = 0;
      do
      {
        pos = fallback.Find (" ", pos + 1);
        csString keyslice = fallback.Slice (start, pos - start);
        if (Process (node, keyslice.GetData ()))
          break;
        start = pos + 1;
      }
      while (pos != (size_t)-1);
    }
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
