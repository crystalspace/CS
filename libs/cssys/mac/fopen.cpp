#include "sysdef.h"
#include "cssys/common/system.h"

FILE* csSystemDriver::fopen (char* filename, char* mode)
{
	return ::fopen( filename, mode );
}
