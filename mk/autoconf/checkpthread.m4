# checkpthread.m4                                              -*- Autoconf -*-
#==============================================================================
# Copyright (C)2003,2004 by Eric Sunshine <sunshine@sunshineco.com>
#
#    This library is free software; you can redistribute it and/or modify it
#    under the terms of the GNU Library General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or (at your
#    option) any later version.
#
#    This library is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
#    License for more details.
#
#    You should have received a copy of the GNU Library General Public License
#    along with this library; if not, write to the Free Software Foundation,
#    Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#==============================================================================
AC_PREREQ([2.56])

#------------------------------------------------------------------------------
# CS_CHECK_PTHREAD([REJECT-MASK])
#	Check for pthread.  Also check if the pthread implementation supports
#	the recursive mutex extension. The shell variable cs_cv_sys_pthread is
#	set to "yes" if pthread is available, else "no". If available, then the
#	variables cs_cv_sys_pthread_cflags, cs_cv_sys_pthread_lflags, and
#	cs_cv_sys_pthread_libs are set. (As a convenience, these variables can
#	be emitted to an output file with CS_EMIT_BUILD_RESULT() by passing
#	"cs_cv_sys_pthread" as its CACHE-VAR argument.) If the recursive mutex
#	extension is supported, then cs_cv_sys_pthread_recursive will be set
#	with the literal name of the constant which must be passed to
#	pthread_mutexattr_settype() to enable this feature. The constant name
#	will be typically PTHREAD_MUTEX_RECURSIVE or
#	PTHREAD_MUTEX_RECURSIVE_NP. If the recursive mutex extension is not
#	available, then cs_cv_sys_pthread_recursive will be set to "no".
#	REJECT-MASK can be used to limit the platforms on which the pthread
#	test is performed. It is compared against $host_os; matches are
#	rejected. If omitted, then the test is performed on all
#	platforms. Examples: To avoid testing on Cygwin, use "cygwin*"; to
#	avoid testing on Cygwin and AIX, use "cygwin*|aix*".
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_PTHREAD],
    [AC_REQUIRE([AC_CANONICAL_HOST])
    case $host_os in
	m4_ifval([$1],
	[$1)
	    cs_cv_sys_pthread=no
	    ;;
	])
	*)
	    CS_CHECK_BUILD([for pthread], [cs_cv_sys_pthread],
		[AC_LANG_PROGRAM(
		    [[#include <pthread.h>
		    void* worker(void* p) { (void)p; return p; }]],
		    [pthread_t tid; pthread_create(&tid, 0, worker, 0);])],
		[cs_pthread_flags])
	    ;;
    esac

    AS_IF([test $cs_cv_sys_pthread = yes],
	[AC_CACHE_CHECK([for pthread recursive mutexes],
	    [cs_cv_sys_pthread_recursive],
	    [CS_BUILD_IFELSE(
		[AC_LANG_PROGRAM(
		    [[#include <pthread.h>]],
		    [pthread_mutexattr_t attr;
		    pthread_mutexattr_settype (&attr, CS_RECURSIVE);])],
		[CS_CREATE_TUPLE([-DCS_RECURSIVE=PTHREAD_MUTEX_RECURSIVE]) \
		CS_CREATE_TUPLE([-DCS_RECURSIVE=PTHREAD_MUTEX_RECURSIVE_NP])],
		[],
		[cs_cv_sys_pthread_recursive=`echo $cs_build_cflags | \
		    sed 's/.*\(PTHREAD_MUTEX_RECURSIVE_*N*P*\).*/\1/'`],
		[cs_cv_sys_pthread_recursive=no],
		[$cs_cv_sys_pthread_cflags -D_GNU_SOURCE],
		[$cs_cv_sys_pthread_lflags],
		[$cs_cv_sys_pthread_libs])])],
	[cs_cv_sys_pthread_recursive=no])])])

m4_define([cs_pthread_flags],
    [CS_CREATE_TUPLE() \
    CS_CREATE_TUPLE([], [], [-lpthread]) \
    CS_CREATE_TUPLE([-pthread], [-pthread], []) \
    CS_CREATE_TUPLE([-pthread], [-pthread], [-lpthread]) \
    CS_CREATE_TUPLE([-pthread], [-pthread], [-lc_r])])
