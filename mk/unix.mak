#------------------------------------------------------------------------------
# A makefile stub which defines variables common to Unix-like platforms.
# Platform-specific makefiles include this stub and may override values which
# are specific to a given platform, if necessary.
#------------------------------------------------------------------------------

# The command for running shell scripts (usually the same as SHELL)

RUN_SCRIPT=$(SHELL)

# Typical extension for GUI executables on this system (e.g. EXE=.exe)
EXE=

# Typical extension for console executables on this system (e.g. EXE=.exe)
EXE.CONSOLE=

# Typical extension for dynamic libraries on this system.
DLL=.so

# Typical extension for object files
O=.o

# Typical extension for static libraries
LIB=.a

# Typical prefix for library filenames
LIB_PREFIX=lib

# The library (archive) manager
AR=ar
ARFLAGS=cr

# Where to put the dynamic libraries on this system?
OUTDLL=.

# If we don't use -fpic we don't need separate output directories
ifeq ($(CFLAGS.DLL),)
OUTSUFX.yes=
endif

# Command sequence for creating a directory.
# Note that directories will have forward slashes. Please
# make sure that this command accepts that (or use 'subst' first).
MKDIR=$(CMD.MKDIR) $(patsubst %/,%,$@)

# Command for creating a directory including missing parents.
MKDIRS=$(CMD.MKDIRS) $(patsubst %/,%,$@)

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
SYS_SED_DEPEND=

# Include support for the XSHM extension in Crystal Space.
# Note that you don't need to disable this if you don't have XSHM
# support in your X server because Crystal Space will autodetect
# the availability of XSHM.
DO_SHM=yes

# Put this in front of the application name to run the test
RUN_TEST=./
