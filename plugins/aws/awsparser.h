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

#ifndef __CS_AWS_PARSER_H__
#define __CS_AWS_PARSER_H__

#include "iutil/vfs.h"
#include "awsprefs.h"

struct iAws;
struct iObjectRegistry;

class awsParser;
extern awsParser* static_awsparser;

/**
 * Helper class for the bison parser.
 */
class awsParser
{
protected:
  csRef<iFile> input;
  iObjectRegistry* objreg;
  iAws* aws;
  awsPrefManager* prefmgr;
public:
  awsParser (iObjectRegistry* objreg, iAws* aws, awsPrefManager* prefmgr);
  ~awsParser ();

  bool Initialize (const char* filename);

  size_t Read (char* buf, size_t maxsize)
  {
    return input->Read (buf, maxsize);
  }

  awsKey* MapSourceToSink (
    unsigned long signal,
    const char* sinkname,
    const char* triggername);

  bool GetConstantValue (const char* name, int &v);

  /// Report parse errors.
  void ReportError (const char* msg, ...);

  void AddGlobalWindowDef (awsComponentNode* win);

  void AddGlobalSkinDef (awsSkinNode* skin);
};

#endif // __CS_AWS_PARSER_H__
