#                                               -*- Autoconf -*-
#==============================================================================
# Copyright (C)2003 by Eric Sunshine <sunshine@sunshineco.com>
# edited by Matze Braun
#
# Autoconf macros for text cache facility.
#==============================================================================

#------------------------------------------------------------------------------
# Text cache facility.  These macros provide a way to incrementally store
# arbitrary text in a shell variable, and to write the saved text to a file.
#
# CS_TEXT_CACHE_APPEND(VARIABLE, TEXT)
#       Append text to the contents of the named shell variable.  If the text
#       contains references to shell variables (such as $foo), then those
#       references will be expanded.  If expansion is not desired, then protect
#       the text with AS_ESCAPE().
#
# CS_TEXT_CACHE_PREPEND(VARIABLE, TEXT)
#       Prepend text verbatim to the contents of the named shell variable.  If
#       the text contains references to shell variables (such as $foo), then
#       those references will be expanded.  If expansion is not desired, then
#       protect the text with AS_ESCAPE().
#
# CS_TEXT_CACHE_OUTPUT(VARIABLE, FILENAME, [FILTERCOMMAND])
#       Instruct config.status to write the contents of the named shell
#       variable to the given filename.  If the file resides in a directory,
#       the directory will be created, if necessary.  If the output file
#       already exists, and if the cached text is identical to the contents of
#       the existing file, then the existing file is left alone, thus its time
#       stamp remains unmolested.  This heuristic may help to minimize rebuilds
#       when the file is listed as a dependency in a makefile.
#	If provided runs the filtercommand on the file before writing it to
#	disk.
#------------------------------------------------------------------------------
AC_DEFUN([CS_TEXT_CACHE_APPEND], [$1="${$1}$2"])
AC_DEFUN([CS_TEXT_CACHE_PREPEND], [$1="$2${$1}"])
AC_DEFUN([CS_TEXT_CACHE_OUTPUT],
    [AC_CONFIG_COMMANDS([$2],
        [echo $ECHO_N "$$1$ECHO_C" m4_ifval([$3], | [$3]) > $tmp/tcache
            if diff $2 $tmp/tcache >/dev/null 2>&1; then
                AC_MSG_NOTICE([$2 is unchanged])
            else
                rm -f $2
                mv $tmp/tcache $2
            fi],
        [$1='$$1' cs_dir=`AS_DIRNAME([$2])`
            AS_MKDIR_P([AS_ESCAPE([$cs_dir])])])])



