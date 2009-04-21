/*
 
 crystalspace.h *must* be the very *first* header we include from our
 module code, otherwise things (eg. int64 types from stdint.h) will
 break.

 Since it is impossible to tell SWIG to put any code above it's auto-
 generated header (which includes some system files), we have to do it this
 way (tell SWIG to write to cswigpl5.inc, and include that from here, after
 we've done what needs doing before any SWIG code.

*/

#include "crystalspace.h"
#undef MIN
#undef MAX
#include "cswigpl5.inc"
