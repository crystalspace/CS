#ifndef __AWSPARSER_H__
#define __AWSPARSER_H__

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
  awsParser (iObjectRegistry* objreg, iAws* prefmgr, awsPrefManager* prefmgr);
  ~awsParser ();
  bool Initialize (const char* filename);

  int Read (char* buf, size_t maxsize)
  { return input->Read (buf, maxsize); }

  awsKey* MapSourceToSink (unsigned long signal, const char* sinkname, const char* triggername);

  bool GetConstantValue (const char* name, int &v);

  /** Report parse errors */
  void ReportError (const char* msg, ...);

  void AddGlobalWindowDef (awsComponentNode* win)
  { prefmgr->AddWindowDef(win); }
  void AddGlobalSkinDef (awsSkinNode* skin)
  { prefmgr->AddSkinDef(skin); }

protected:
  iFile* input;
  iObjectRegistry* objreg;
  iAws* aws;
  awsPrefManager* prefmgr;
};

#endif

