#ifndef __CS_AWS_PARSER_H__
#define __CS_AWS_PARSER_H__

#include "iutil/vfs.h"

#include "awsprefs.h"

struct iAws;
struct iObjectRegistry;

class awsParser;
extern awsParser* static_awsparser;

/** Helper class for the bison parser */
class awsParser
{
public:
  awsParser (iObjectRegistry* objreg, iAws* aws, awsPrefManager* prefmgr);
  ~awsParser ();
  bool Initialize (const char* filename);

  int Read (char* buf, size_t maxsize)
  { return input->Read (buf, maxsize); }

  awsKey* MapSourceToSink (unsigned long signal, const char* sinkname, const char* triggername);

  bool GetConstantValue (const char* name, int &v);

  /** Report parse errors */
  void ReportError (const char* msg, ...);

  void AddGlobalWindowDef (awsComponentNode* win);

  void AddGlobalSkinDef (awsSkinNode* skin);

protected:
  csRef<iFile> input;
  iObjectRegistry* objreg;
  iAws* aws;
  awsPrefManager* prefmgr;
};

#endif // __CS_AWS_PARSER_H__

