# textcache.m4                                                 -*- Autoconf -*-
#==============================================================================
# Copyright (C)2003 by Eric Sunshine <sunshine@sunshineco.com>
#==============================================================================
AC_PREREQ([2.56])

#------------------------------------------------------------------------------
# Text cache facility.  These macros provide a way to incrementally store
# arbitrary text in a shell variable, and to write the saved text to a file.
#
# CS_TEXT_CACHE_APPEND(VARIABLE, TEXT)
#	Append text to the contents of the named shell variable.  If the text
#	contains references to shell variables (such as $foo), then those
#	references will be expanded.  If expansion is not desired, then protect
#	the text with AS_ESCAPE().
#
# CS_TEXT_CACHE_PREPEND(VARIABLE, TEXT)
#	Prepend text to the contents of the named shell variable.  If the text
#	contains references to shell variables (such as $foo), then those
#	references will be expanded.  If expansion is not desired, then protect
#	the text with AS_ESCAPE().
#
# CS_TEXT_CACHE_OUTPUT(VARIABLE, FILENAME, [FILTER])
#	Instruct config.status to write the contents of the named shell
#	variable to the given filename.  If the file resides in a directory,
#	the directory will be created, if necessary.  If the output file
#	already exists, and if the cached text is identical to the contents of
#	the existing file, then the existing file is left alone, thus its time
#	stamp remains unmolested.  This heuristic may help to minimize rebuilds
#	when the file is listed as a dependency in a makefile.  If FILTER is
#	supplied, it should be a shell command which accepts text (the content
#	of VARIABLE) on its standard input and emits a filtered or transformed
#	version of that text to its standard output.  The filtered text will
#	instead be written to FILENAME.  Be aware that the text sent through
#	the filter is exactly the text stored in the cache variable, and that
#	this text might not end with a line terminator, thus the filter command
#	should be capable of editing non-terminated text, if you expect such
#	text to exist in the cache variable.  This issue might affect some
#	older versions of sed, for instance, which discard the last line of
#	text if the line terminator is missing.  NOTE: There is a bug in
#	Autoconf, at least as recently as 2.57, which forces us to invoke
#	AS_MKDIR_P in the third argument to AC_CONFIG_COMMANDS (the
#	`init-cmds') rather than the second (the `cmds') This is undesirable
#	because it means that the directory will be created anytime
#	config.status is invoked (even for a simple --help), rather than being
#	created only when requested to output the text cache.  This bug was
#	submitted to the Autoconf GNATS database by Eric Sunshine as #228.
#------------------------------------------------------------------------------
AC_DEFUN([CS_TEXT_CACHE_APPEND], [$1="${$1}$2"])
AC_DEFUN([CS_TEXT_CACHE_PREPEND], [$1="$2${$1}"])
AC_DEFUN([CS_TEXT_CACHE_OUTPUT],
    [AC_CONFIG_COMMANDS([$2],
	[echo $ECHO_N "$$1$ECHO_C"m4_ifval([$3],[|$3]) > $tmp/tcache
	    if diff $2 $tmp/tcache >/dev/null 2>&1; then
		AC_MSG_NOTICE([$2 is unchanged])
	    else
		rm -f $2
		mv $tmp/tcache $2
	    fi],
	[$1='$$1' cs_dir=`AS_DIRNAME([$2])`
	    AS_MKDIR_P([AS_ESCAPE([$cs_dir])])])])
