#include <cssysdef.h>
#include "thoggData.h"
#include <iutil/objreg.h>
#include <iutil/plugin.h>

#include <theora/theoradec.h>
#include <vorbis/codec.h>

#pragma comment (lib,"../libs/libtheora_static.lib")
#pragma comment (lib,"ogg.lib")
#pragma comment (lib,"vorbis.lib")

SCF_IMPLEMENT_FACTORY (thoggData)


thoggData::thoggData (iBase* parent) :
scfImplementationType (this, parent),
object_reg(0)
{
}

thoggData::~thoggData ()
{
}

bool thoggData::Initialize (iObjectRegistry* r)
{
  object_reg = r;
  return true;
}
const csVPLvideoFormat *thoggData::GetFormat()
{
  return this->format;
}

void thoggData::SetFormat(csVPLvideoFormat *vplFormat)
{
  this->format = vplFormat;
}

/// Get size of this sound in frames.
size_t thoggData::GetFrameCount()
{
  return 0;
}

/**
* Return the size of the data stored in bytes.  This is informational only
* and is not guaranteed to be a number usable for sound calculations.
* For example, an audio file compressed with variable rate compression may
* result in a situation where FILE_SIZE is not equal to
* FRAME_COUNT * FRAME_SIZE since FRAME_SIZE may vary throughout the
* audio data.
*/
size_t thoggData::GetDataSize()
{
  return 0;
}

/// Set an optional description to be associated with this sound data
//   A filename isn't a bad idea!
void thoggData::SetDescription(const char *pDescription)
{
  this->pDescription=pDescription;
}

/// Retrieve the description associated with this sound data
//   This may return 0 if no description is set.
const char *thoggData::GetDescription()
{
  return this->pDescription;
}

void thoggData::getNextFrame(vidFrameData &data)
{
}
