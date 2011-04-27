# checkjava.m4                                                 -*- Autoconf -*-
#==============================================================================
# Copyright (C)2004-2009 by Eric Sunshine <sunshine@sunshineco.com>
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
# CS_CHECK_JAVA([EMITTER])
#       Check for Java, a working Java SDK (JDK), and Ant build tool. Sets the
#       shell variables JAVA, JAVAC, and ANT and invokes AC_SUBST() upon each.
#       The shell variable cs_cv_java is set to "yes' if a working JDK is
#       discovered, else "no". If available, then the variables cs_cv_java,
#       cs_cv_java_lflags, and cs_cv_java_libs are set. (As a convenience,
#       these variables can be emitted to an output file with
#       CS_EMIT_BUILD_RESULT() by passing "cs_cv_java" as its CACHE-VAR
#       argument.)  The JDK check can be enabled or disabled with
#       --with[out]-java.  If EMITTER is provided, then
#       CS_EMIT_BUILD_PROPERTY() and CS_EMIT_BUILD_RESULT() are invoked with
#       EMITTER in order to record the results in an output file. As a
#       convenience, if EMITTER is the literal value "emit" or "yes", then the
#       default emitter of CS_EMIT_BUILD_PROPERTY() and CS_EMIT_BUILD_RESULT()
#       will be used.  When EMITTER is provided, the following properties are
#       emitted to the output file: JAVA, JAVA.AVAILABLE ("yes" or "no"),
#       JAVA.CFLAGS, JAVA.LFLAGS, JAVAC, and ANT.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_JAVA],
    [JAVA_HOME=CS_PATH_NORMALIZE([$JAVA_HOME])

    AS_IF([test -z "$JAVA" && test -n "$JAVACMD"],
	[JAVA=CS_PATH_NORMALIZE([$JAVACMD])])
    CS_PATH_PROGS([JAVA], [java], [],
	[$JAVA_HOME/bin$PATH_SEPARATOR$JAVA_HOME/jre/bin$PATH_SEPARATOR$PATH])
    CS_EMIT_BUILD_PROPERTY([JAVA], [$JAVA], [], [], CS_EMITTER_OPTIONAL([$1]))
    AC_SUBST([JAVA])

    CS_PATH_PROGS([JAVAC], [javac], [],
	[$JAVA_HOME/bin$PATH_SEPARATOR$JAVA_HOME/jre/bin$PATH_SEPARATOR$PATH])
    CS_EMIT_BUILD_PROPERTY([JAVAC], [$JAVAC], [], [], CS_EMITTER_OPTIONAL([$1]))
    AC_SUBST([JAVAC])

    AC_ARG_WITH([java], [AC_HELP_STRING([--with-java],
	[use Java (default YES)])])
    AS_IF([test -z "$with_java"], [with_java=yes])

    AS_IF([test "$with_java" != no && test -n "$JAVAC" && test -n "$JAVA"],
	[_CS_JAVA_SDK_FLAGS([cs_java], [$JAVA], [$JAVAC])
	CS_CHECK_BUILD([if Java SDK is usable], [cs_cv_java],
	    [AC_LANG_PROGRAM(
		[[#if defined(__GNUC__)
		typedef long long __int64; /* For GCC on Windows */
		#endif
		/* Fix for 'jlong' on x86_64. */
		#if defined(__x86_64)
		#ifdef _LP64 /* 64-bit Solaris */
		#undef _LP64
		#endif
		#endif
		#include <jni.h>
		JNIEXPORT void JNICALL f(JNIEnv* env, jobject obj);]],
		[JNI_GetCreatedJavaVMs(0,0,0);])],
	    [CS_CREATE_TUPLE() \
	    CS_CREATE_TUPLE(
	        [$cs_java_cflags],
		[$cs_java_lflags],
		[$cs_java_libs])],
	    [], [CS_EMIT_BUILD_RESULT([cs_cv_java], [JAVA],
	        CS_EMITTER_OPTIONAL([$1]))])])

    CS_PATH_PROGS([ANT], [ant], [], [$PATH$PATH_SEPARATOR$ANT_HOME/bin])
    CS_EMIT_BUILD_PROPERTY([ANT], [$ANT], [], [], CS_EMITTER_OPTIONAL([$1]))
    AC_SUBST([ANT])
    ])


#------------------------------------------------------------------------------
# Private utility macros.
#------------------------------------------------------------------------------
AC_DEFUN([_CS_JAVA_PROPERTY_BUILD],
    [cat <<EOF > conftest.java
public class conftest {
    public static void main(String[[]] args) {
        System.out.println(System.getProperty(args[[0]]));
    }
}
EOF
    AC_RUN_LOG([$1 conftest.java])
    ])


AC_DEFUN([_CS_JAVA_PROPERTY_CLEAN],
    [rm -f conftest.java conftest.class
    ])


AC_DEFUN([_CS_JAVA_PROPERTY],
    [AC_RUN_LOG([CLASSPATH=. $2 conftest $1])])


AC_DEFUN([_CS_JAVA_PLATFORM],
    [AC_REQUIRE([AC_CANONICAL_HOST])
    case $host_os in
	mingw*|cygwin*) cs_java_platform=win32 ;;
	*) cs_java_platform=`echo $host_os |
	    sed 's/^\([[^-]]*\).*$/\1/'` ;;
    esac
    ])


AC_DEFUN([_CS_JAVA_CFLAGS],
    [AC_REQUIRE([_CS_JAVA_PLATFORM])
    $1_cflags="$$1_cflags -I$2/include"
    AS_IF([test -n "$cs_java_platform"],
	[$1_cflags="$$1_cflags -I$2/include/$cs_java_platform"])])


AC_DEFUN([_CS_JAVA_LFLAGS],
    [cs_save_ifs=$IFS; IFS=$PATH_SEPARATOR
    for cs_dir in $2
    do
        IFS=$cs_save_ifs
        test -z "$cs_dir" && cs_dir=.
        $1_lflags="$$1_lflags -L$cs_dir"
    done
    IFS=$cs_save_ifs
    ])


AC_DEFUN([_CS_JAVA_FLAGS],
    [AC_REQUIRE([AC_CANONICAL_HOST])
    AC_REQUIRE([CS_CHECK_HOST])
    AS_IF([test x$cs_host_macosx = xyes && # Not cross-building for Darwin.
	test -r /System/Library/Frameworks/JavaVM.framework/Headers],
	[$1_cflags="-I/System/Library/Frameworks/JavaVM.framework/Headers"
	$1_libs="-framework JavaVM"],
	[AS_IF([test -n "$JAVA_HOME"],
	    [_CS_JAVA_CFLAGS([$1], [$JAVA_HOME])
	    _CS_JAVA_LFLAGS([$1], [$JAVA_HOME])])
	cs_java_home=`_CS_JAVA_PROPERTY([java.home], [$2])`
	AS_IF([test -n "$cs_java_home"],
	    [_CS_JAVA_CFLAGS([$1], [$cs_java_home])
	    _CS_JAVA_CFLAGS([$1], [$cs_java_home/..])
	    _CS_JAVA_LFLAGS([$1],
	        [$cs_java_home$PATH_SEPARATOR$cs_java_home/..])])
	cs_java_lib_path=`_CS_JAVA_PROPERTY([java.library.path], [$2])`
	AS_IF([test -n "$cs_java_lib_path"],
	    [_CS_JAVA_LFLAGS([$1], [$cs_java_lib_path])])
	$1_cflags=CS_PATH_NORMALIZE([$$1_cflags])
	$1_lflags=CS_PATH_NORMALIZE([$$1_lflags])
	$1_libs="-ljvm"])])


AC_DEFUN([_CS_JAVA_SDK_FLAGS],
    [AC_CACHE_CHECK([for Java SDK], [cs_cv_java_sdk],
        [AS_IF([test -n "$2" && test -n "$3"],
	    [_CS_JAVA_PROPERTY_BUILD([$3])
	    _CS_JAVA_FLAGS([cs_cv_java_sdk], [$2])
	    _CS_JAVA_PROPERTY_CLEAN()
	    $1_cflags="$cs_cv_java_sdk_cflags"
	    $1_lflags="$cs_cv_java_sdk_lflags"
	    $1_libs="$cs_cv_java_sdk_libs"
	    cs_cv_java_sdk=yes],
	    [cs_cv_java_sdk=no])])])
