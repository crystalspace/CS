#include "cssysdef.h"
#include "csutil/cscolor.h"
#include "csgeom/vector2.h"
#include "csgeom/vector3.h"
#include "ivideo/rndbuf.h"

#include "gl_sysbufmgr.h"

SCF_IMPLEMENT_IBASE (csSysRenderBuffer)
  SCF_IMPLEMENTS_INTERFACE (iRenderBuffer)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (csSysRenderBufferManager)
  SCF_IMPLEMENTS_INTERFACE (iRenderBufferManager)
SCF_IMPLEMENT_IBASE_END


csPtr<iRenderBuffer> csSysRenderBufferManager::GetBuffer(int buffersize, CS_RENDERBUFFER_TYPE location)
{
  csSysRenderBuffer *buffer = new csSysRenderBuffer (
    new char[buffersize], buffersize, location);
  return csPtr<iRenderBuffer> (buffer);
}