/*
    Copyright (C) 2001 by Christopher Nelson

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
#include "ivaria/reporter.h"

#include "awsparser.h"

awsParser* static_awsparser = 0;

awsParser::awsParser (iObjectRegistry* newobjreg, iAws* newaws,
  awsPrefManager* newprefmgr)
    : objreg(newobjreg), aws(newaws), prefmgr(newprefmgr)
{ 
}

awsParser::~awsParser ()
{ 
}

bool awsParser::Initialize (const char* filename)
{
  csRef<iVFS> vfs (CS_QUERY_REGISTRY (objreg, iVFS));
  if (!vfs)
    return false;
  
  input = vfs->Open (filename, VFS_FILE_READ);
  if (!input)
    return false;

  return true;
}  

awsKey* awsParser::MapSourceToSink (unsigned long signal, const char* sinkname, const char* triggername)
{
  iAwsSink* sink = aws->GetSinkMgr()->FindSink(sinkname);
  
  if (sink == 0)
  {
    ReportError ("Couldn't find sink '%s'.", sinkname);
    return 0;
  }
  
  unsigned long trigger = sink->GetTriggerID(triggername);
  if (sink->GetError () != 0)
  {
    ReportError ("Couldn't find Trigger '%s' in Sink '%s'.", triggername, sinkname);
    return 0;
  }
  
  return new awsConnectionKey (aws, "connection", sink, trigger, signal);
}

bool awsParser::GetConstantValue (const char* name, int &v)
{
  if (!prefmgr->ConstantExists(name))
    return false;

  v = prefmgr->GetConstantValue(name);
  return true;
}

void awsParser::ReportError (const char* msg, ...)
{
  va_list arg;
  va_start(arg, msg);

  csReportV (objreg, CS_REPORTER_SEVERITY_ERROR, "aws",
      msg, arg);
  
  va_end (arg);
}

void awsParser::AddGlobalWindowDef(awsComponentNode* win)
{ 
  iAwsComponentNode* cn = (iAwsComponentNode*) win;
  prefmgr->AddWindowDef(cn);
  cn->DecRef();
}

void awsParser::AddGlobalSkinDef(awsSkinNode *skin)
{ 
  iAwsKeyContainer* kc = (iAwsKeyContainer*) skin;
  prefmgr->AddSkinDef(kc);
  kc->DecRef();
}

