#------------------------------------------------------------------------------
# A makefile stub which defines variables common to DOS-like platforms.
# Platform-specific makefiles include this stub and may override values which
# are specific to a given platform, if necessary.
#------------------------------------------------------------------------------

# The command for running shell scripts (usually the same as SHELL)

RUN_SCRIPT=$(SHELL)

# Typical extension for executables on this system (e.g. EXE=.exe)
EXE=.exe

# Typical extension for dynamic libraries on this system.
DLL=.dll

# Typical extension for object files
O=.obj

# Typical extension for static libraries
LIB=.lib

# Typical prefix for library filenames
LIB_PREFIX=

# Where to put the dynamic libraries on this system?
OUTDLL=

# We don't need separate directories for dynamic libraries
OUTSUFX.yes=

# Command sequence for creating a directory.
# Note that directories will have forward slashes. Please
# make sure that this command accepts that (or use 'subst' first).
MKDIR = mkdir $(subst /,\,$(@:/=))

# The command to remove all specified files.
RM=rm -f

# The command to remove a directory tree.
RMDIR=rm -rf

# The command to copy a file to another file or a list of files to a directory
CP=cp

# The command to rename a file or move a list of files to a directory
MV=mv

# The command to change the current working directory.
CD=cd

# The command to print out the current working directory.  The output from
# this command should be suitable for input back into the $(CD) command.
PWD=pwd

# The command to invoke a Perl interpreter.
PERL=perl

# Extra parameters for 'sed' which are used for doing 'make depend'.
SYS_SED_DEPEND=-e "s/\.o/$$O/g"

# Use makedep to build dependencies
DEPEND_TOOL=mkdep
