#==============================================================================
# Copyright (C)2003-2006 by Eric Sunshine <sunshine@sunshineco.com>
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
# CS_CHECK_CHARP_COMPILERS
#	Checks for the differents C# compilers
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_CSHARP_COMPILERS],
    [CS_CHECK_PROGS([csharpcompiler_ms], [csc])
     CS_CHECK_PROGS([csharpcompiler_mono], [mcs])
     CS_CHECK_PROGS([csharpcompiler_pnet], [cscc])])

#------------------------------------------------------------------------------
# CS_CHECK_GACUTIL
#	Checks for the .Net gacutil tool
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_GACUTIL],
    [CS_CHECK_PROGS([CMD_GACUTIL], [gacutil])
     CS_CHECK_PROGS([pnet_gacutil], [ilgac])])

#------------------------------------------------------------------------------
# CS_CHECK_NANT
#	Checks for the NAnt build system
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_NANT],
    [CS_CHECK_PROGS([CMD_NANT], [nant NAnt])
     CS_EMIT_BUILD_PROPERTY([CMD.NANT], [$CMD_NANT])])

#------------------------------------------------------------------------------
# CS_CHECK_CSHARP_COMPILER
#	This is a placeholder for a repetitive code
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_CSHARP_COMPILER],
    [AS_IF([test -n $1],
	[csharpcompiler="$1"
	 csharp_available="yes"
	 dotnet_runtime="$2"
	 AS_IF([test "$2" = "PNET"],
	     [AS_IF([test -z "$pnet_gacutil"],
		[csharp_available="no"])
	      AS_IF([test -n "$pnet_gacutil"],
		[CMD_GACUTIL="$pnet_gacutil"])])
        ])
    ])

#------------------------------------------------------------------------------
# CS_CHECK_DOTNET_RUNTIME
#	Performs checks with the underlying .Net Runtime, and the compiler that
#	we use
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_DOTNET_RUNTIME],
    [
    AC_MSG_CHECKING([which .NET runtime are we using])
    case $1 in
        mono*)
	     CS_CHECK_CSHARP_COMPILER([$csharpcompiler_mono], [MONO])
	     ;;
	msnet|ms*)
	     CS_CHECK_CSHARP_COMPILER([$csharpcompiler_ms], [MS])
	     ;;
	pnet*)
	     CS_CHECK_CSHARP_COMPILER([$csharp], [PNET])
	     ;;
	yes*)
	     # We prefer free software CLIs against the propetaries
	     CS_CHECK_CSHARP_COMPILER([$csharpcompiler_ms], [MS])
	     CS_CHECK_CSHARP_COMPILER([$csharp], [PNET])
	     CS_CHECK_CSHARP_COMPILER([$csharpcompiler_mono], [MONO])
	     ;;
	no*)
	     dotnet_runtime="NONE"
	     csharp_available="no"
	     ;;
    esac
    AC_MSG_RESULT([$dotnet_runtime])
    ])

#------------------------------------------------------------------------------
# CS_CHECK_DOTNET
#	Performs all the checking needed for the .Net framework
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_DOTNET],
    [
    # C# binding is experimental/testing, by default no
    AC_ARG_WITH([dotnet], [AC_HELP_STRING([--with-dotnet],
	[use C# scripting interface, which requires a CLI(default NO)])])

    AS_IF([test -z "$with_dotnet"],
	[with_dotnet="no"])

    CS_CHECK_CSHARP_COMPILERS
    CS_CHECK_GACUTIL
    CS_CHECK_NANT

    CS_CHECK_DOTNET_RUNTIME([$with_dotnet])

    AS_IF([test -z "$CMD_GACUTIL"],
	[csharp_available="no"])

    AS_IF([test -z "$CMD_NANT"],
	[csharp_available="no"])

    AC_MSG_CHECKING([if the .NET tools are usable])
    AC_MSG_RESULT([$csharp_available])

    CS_EMIT_BUILD_PROPERTY([CMD.GACUTIL], [$CMD_GACUTIL])
    CS_EMIT_BUILD_PROPERTY([CMD.CSC], [$csharpcompiler])
    CS_EMIT_BUILD_PROPERTY([CSHARP.COMPILER], [$csharpcompiler])
    CS_EMIT_BUILD_PROPERTY([DOTNET.RUNTIME], [$dotnet_runtime])

    CS_EMIT_BUILD_PROPERTY([CSHARP.AVAILABLE], [$csharp_available])
    ])


