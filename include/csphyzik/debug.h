#ifndef DEBUG_H
#define DEBUG_H

//!me swig doesn't compile with this stuff working.

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define __CYSTALSPACE__
#ifdef __CYSTALSPACE__
#include "sysdef.h"
#include "cssys/system.h"
//class csSystemDriver;
#endif
//#include <string>
//#include <iostream>

//using namespace std;


//#define ferr cout
#define McAssert( A, B ) { if( !A ){ Debug::log( B ); } }
#define McAssertFail( A, B ) { if( !A ){ Debug::log( B ); abort(); } }
#define McAssertGoTo( A, B, C ) { if( !A ){ Debug::log( B ); goto C; } }
#ifdef __CRYSTALSPACE__
#define assert McAssert
#endif
#define assert_fail McAssertFail 
#define assert_goto McAssertGoTo

#ifdef __CYSTALSPACE__
#define CT_DEBUG_LEVEL MSG_DEBUG_1
#define Debug csSystemDriver
#define logf Printf
#define log Printf
#define DEBUGLOGF( A, B )  csSystemDriver::Printf( MSG_DEBUG_1, A, B )
#define DEBUGLOG( A )  csSystemDriver::Printf( MSG_DEBUG_1, A )
#else
#define CT_DEBUG_LEVEL 1
//!me broken
#define DEBUGLOGF( A,B )  Debug::log( A )  
#define DEBUGLOG( A )  Debug::log( A )  


class Debug
{
public:

	static void log( char *err_mess/*string err_mess*/ ){  /*ferr << err_mess;*/ }
	static void logf( char *err_mess, ... ){}
};
#endif

#endif
