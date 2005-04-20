/* Both VC8 and the latest Platform SDK define the same macros in two files
 * (sal.h/SpecStrings.h), unfortunately, there's no way (some #define etc.)
 * to disable the definitions in one of the file... so we trick VC into 
 * thinking it would actually have already included one of those files. */
#pragma once
