#ifndef __NeXT_malloc_h
#define __NeXT_malloc_h
//=============================================================================
//
//	Copyright (C)1999 by Eric Sunshine <sunshine@sunshineco.com>
//
// The contents of this file are copyrighted by Eric Sunshine.  This work is
// distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE.  You may distribute this file provided that this
// copyright notice is retained.  Send comments to <sunshine@sunshineco.com>.
//
//=============================================================================
//-----------------------------------------------------------------------------
// malloc.h
//
//	Some modules still include <malloc.h> even though it has been 
//	deprecated in favor of <stdlib.h>.  On NeXT, the prototype for 
//	malloc() in <malloc.h> is incorrect and conflicts with <stdlib.h>.  
//	Therefore provide this fake "malloc.h" which includes the real 
//	prototype from <stdlib.h>.  
//
//-----------------------------------------------------------------------------
#include <stdlib.h>

#endif // __NeXT_malloc_h
