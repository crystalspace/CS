#include "cssysdef.h"
#include "csutil/cscolor.h"
#include "csgeom/vector2.h"
#include "csgeom/vector3.h"
#include "ivideo/rndbuf.h"

#include "gl_softrbufmgr.h"

SCF_IMPLEMENT_IBASE (csSoftRenderBuffer)
  SCF_IMPLEMENTS_INTERFACE (iRenderBuffer)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (csSoftRenderBufferManager)
  SCF_IMPLEMENTS_INTERFACE (iRenderBufferManager)
SCF_IMPLEMENT_IBASE_END


csPtr<iRenderBuffer> csSoftRenderBufferManager::GetBuffer(int buffersize, CS_RENDERBUFFER_TYPE location)
{
  csSoftRenderBuffer *buffer = new csSoftRenderBuffer (
    new char[buffersize], buffersize, location);
  return csPtr<iRenderBuffer> (buffer);
}