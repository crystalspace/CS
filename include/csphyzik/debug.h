#ifndef DEBUG_H
#define DEBUG_H

//!me swig doesn't compile with this stuff working.

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "csphyzik/phyztype.h"

//#include <string>
//#include <iostream>

//using namespace std;


//#define ferr cout
#define McAssert( A, B ) { if( !A ){ Debug::log( B ); } }
#define McAssertFail( A, B ) { if( !A ){ Debug::log( B ); abort(); } }
#define McAssertGoTo( A, B, C ) { if( !A ){ Debug::log( B ); goto C; } }
//#define assert McAssert
#define assert_fail McAssertFail
#define assert_goto McAssertGoTo

#define logf printf
#define log printf
// only takes two args....
#define DEBUGLOGF2( A, B, C )  printf ( A, B, C )
#define DEBUGLOGF( A, B )  printf ( A, B )
#define DEBUGLOG( A )  printf ( A )

#endif
