#include "sysdef.h"
#include "cssys/common/system.h"

FILE* csSystemDriver::fopen (const char* filename, const char* mode)
{
	return ::fopen( filename, mode );
}
