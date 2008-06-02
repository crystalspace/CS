#include <EXTERN.h>
#include <perl.h>
extern void boot_DynaLoader (pTHXo_ CV* cv);
extern void xs_init (pTHXo)
{
  char *file = __FILE__;
  dXSUB_SYS;
  newXS("DynaLoader::boot_DynaLoader", boot_DynaLoader, file);
}
