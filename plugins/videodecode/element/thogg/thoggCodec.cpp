#include <cssysdef.h>
#include "thoggCodec.h"
#include <iutil/objreg.h>
#include <iutil/plugin.h>

#include <theora/theoradec.h>
#include <vorbis/codec.h>

#pragma comment (lib,"../libs/libtheora_static.lib")
#pragma comment (lib,"ogg.lib")
#pragma comment (lib,"vorbis.lib")

SCF_IMPLEMENT_FACTORY (thoggCodec)


thoggCodec::thoggCodec (iBase* parent) :
scfImplementationType (this, parent),
object_reg(0)
{
}

thoggCodec::~thoggCodec ()
{
}

bool thoggCodec::Initialize (iObjectRegistry* r)
{
  object_reg = r;
  oggFile=0;
  return true;
}

bool thoggCodec::InitCodec (FILE *infile)
{
  oggFile = new thoggFile ((iBase*)this);

  return oggFile->InitFile(infile);
}

const csVPLvideoFormat *thoggCodec::GetFormat()
{
  return this->format;
}

void thoggCodec::SetFormat(csVPLvideoFormat *vplFormat)
{
  this->format = vplFormat;
}

size_t thoggCodec::GetFrameCount()
{
  return 0;
}

size_t thoggCodec::GetDataSize()
{
  return 0;
}

void thoggCodec::SetDescription(const char *pDescription)
{
  this->pDescription=pDescription;
}

const char *thoggCodec::GetDescription()
{
  return this->pDescription;
}

void thoggCodec::getNextFrame(vidFrameData &data)
{
}

void thoggCodec::update()
{
}
