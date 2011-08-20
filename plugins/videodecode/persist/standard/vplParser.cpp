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
  object_reg = r;
  return true;
}
bool vplParser::Parse (iDocumentNode* doc) 
{
  return true;
}

const char* vplParser::GetMediaPath ()
{
  return 0;
}

const char* vplParser::GetMediaType ()
{
  return 0;
}

csArray<Language> vplParser::GetLanguages ()
{
  return 0;
}