// Implementation specific to the pure Python "cspace" module (pythmod). Not
// used by the cspython Crystal Space plugin.

#include "cssysdef.h"

CS_IMPLEMENT_APPLICATION

extern "C" void SWIG_init();

extern "C" CS_EXPORT_SYM_DLL void init_cspace ()
{
  SWIG_init();
}
