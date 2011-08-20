
#include "cssysdef.h"

#include <ctype.h>

#include "imap/ldrctxt.h"
#include "imap/services.h"
#include "imesh/genmesh.h"
#include "imesh/object.h"
#include "iutil/comp.h"
#include "iutil/document.h"
#include "iutil/eventh.h"
#include "iutil/object.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/stringarray.h"
#include "ivaria/reporter.h"

#include <cssysdef.h>
#include "vplParser.h"
#include <iutil/objreg.h>
#include <iutil/plugin.h>
#include <iostream>

using namespace std;

SCF_IMPLEMENT_FACTORY (vplParser)

vplParser::vplParser (iBase* parent) :
scfImplementationType (this, parent),
object_reg(0)
{
}

vplParser::~vplParser ()
{
}

bool vplParser::Initialize (iObjectRegistry* r)
{
  vplParser::object_reg = r;
  reporter = csQueryRegistry<iReporter> (object_reg);
  synldr = csQueryRegistry<iSyntaxService> (object_reg);

  mediaPath = mediaType = NULL;
  languages.DeleteAll ();

  InitTokenTable (xmltokens);
  return true;
}
bool vplParser::Parse (iDocumentNode* doc) 
{
  csRef<iDocumentNodeIterator> it = doc->GetNodes ();

  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_MEDIA:
        {
          // Get the type of the media
          mediaType = csString(child->GetAttributeValue ("type"));

          // Iterate through the rest of the nodes in the media file
          csRef<iDocumentNodeIterator> it2 = child->GetNodes ();
          
          while (it2->HasNext ())
          {
            csRef<iDocumentNode> child2 = it2->Next ();
            if (child2->GetType () != CS_NODE_ELEMENT) continue;
            const char* value2 = child2->GetValue ();
            csStringID id2 = xmltokens.Request (value2);
            switch (id2)
            {
              case XMLTOKEN_VIDEOSTREAM:
                {
                  // Get the path for the media file
                  mediaPath = csString(child2->GetAttributeValue ("path"));
                }
              case XMLTOKEN_AUDIOSTREAM:
                {
                  // Get the info about the different audio streams available
                  csRef<iDocumentNodeIterator> it3 = child2->GetNodes ();

                  while (it3->HasNext ())
                  {
                    csRef<iDocumentNode> child3 = it3->Next ();
                    if (child3->GetType () != CS_NODE_ELEMENT) continue;
                    const char* value3 = child3->GetValue ();
                    csStringID id3 = xmltokens.Request (value3);
                    switch (id3)
                    {
                      case XMLTOKEN_LANGUAGE:
                        {
                          // Store the info about the language
                          //we want to store all the language streams
                          Language buff;

                          //store the name
                          buff.name = new char[strlen (child3->GetAttributeValue ("name"))];
                          strcpy (buff.name,child3->GetAttributeValue ("name"));

                          //and the path
                          buff.path = new char[strlen (child3->GetAttributeValue ("path"))];
                          strcpy (buff.path,child3->GetAttributeValue ("path"));

                          languages.Push (buff);
                        }
                    }
                  }
                }
            }
          }
        }
    }
  }
  return true;
}

csString vplParser::GetMediaPath ()
{
  return mediaPath;
}

csString vplParser::GetMediaType ()
{
  return mediaType;
}

csArray<Language> vplParser::GetLanguages ()
{
  return languages;
}