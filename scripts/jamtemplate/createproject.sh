#!/bin/sh

# Helper to read variables
ReadValue()
{
    TEXT="$2"
    VARNAME="$1"
    echo -n "$TEXT "
    read READVAL
    eval "$VARNAME=\"$READVAL\" ; export $VARNAME"
}

ReadValueStrict()
{
    TEXT="$2"
    VARNAME="$1"
    READVAL=""
    
    while test -z "$READVAL"; do
	echo -n "$TEXT "
	read READVAL
	if test -z "$READVAL"; then
	    echo "Please enter a value."
	fi
    done

    eval "$VARNAME=\"$READVAL\" ; export $VARNAME"
}

CheckYesNo()
{
    VARNAME="$1"
    TEXT="$2"
    DEFAULT="$3"
    
    while true; do
        echo -n "$TEXT? [$DEFAULT] "
	read READVAL       
	case $READVAL in
	    "")
		eval "$VARNAME=\"$DEFAULT\" ; export $VARNAME"
		break
		;;
	    [Yy]|[Yy][Ee]|[Yy][Ee][Ss])
		eval "$VARNAME=yes ; export $VARNAME"
		break
		;;
	    [Nn]|[Nn][Oo])
		eval "$VARNAME=no ; export $VARNAME"
		break
		;;
	    *)
		echo 'Please enter "yes" or "no".'
		;;
	esac
    done
}

# Poor man's (portable) 'dirname' command.
dirpart()
{
    expr "$1" : '\(..*\)[/\\].*'
}

# Poor man's (portable) capitalization command.
capitalize()
{
    echo "$1" | sed '
	h;
	s/\(.\).*/\1/;
	y/abcdefghijklmnopqrstuvwxyz/ABCDEFGHIJKLMNOPQRSTUVWXYZ/;
	G;
	s/\(.\)\n./\1/'
}

# Part 1: Interactive - Gather information
cat << __EOF__
Crystal Space external Project Template

This script will generate a basic Crystal Space project for you, complete with
configuration (Autoconf) and build system (Jam).  The project will be created
in a subdirectory of the current directory.  You can abort the script at any
time by typing Ctrl-C.

__EOF__

cat << __EOF__
**** Project
Choose a project name.  We need a short form of the name for use in directory
and file names.  The short name should be lowercase (though not strictly
required) and must not contain any special characters or spaces.  We also need
a long name for use in help messages and the README file.  For example, if you
project is called "Space Fighters - Revenge for Ducky", then you may want to
choose a short name "spacefighters", and a long name "Space Fighters - Revenge
for Ducky".  You may also provide an address for the project's home page.
Please specify a complete URI.  For example: http://www.spacefighter.org/

__EOF__
ReadValueStrict PROJECTNAME "Short name:"
ReadValueStrict LONGNAME "Long name:"
ReadValueStrict VERSION "Version:"
ReadValue HOMEPAGE "Home page URI:"

cat << __EOF__

**** Contact
The information about the author is mentioned in the README file and in the
configuration script so that people see a support address when they invoke
"./configure --help".  When asked for the copyright information, type the full
copyright notice as you wish it to appear in the generated files. For instance:
Copyright (C)2004 by Duffer McFluffer

__EOF__
ReadValueStrict AUTHOR "Author:"
ReadValueStrict EMAIL "Email:"
ReadValue COPYRIGHT "Copyright:"

cat << __EOF__

**** Dependencies
The Crystal Entity Layer (CEL) is a set of classes and modules which layer
game-oriented facilities atop Crystal Space. (http://cel.sourceforge.net)
The Autoconf configuration script and Jam build system can be set up to work
with CEL if your project will utilize this SDK.

__EOF__

CheckYesNo USECEL "Utilize CEL" no

echo ""
echo "*Creating project: $PROJECTNAME"

TEMPLATEDIR=`dirpart $0`
CSDIR="$TEMPLATEDIR/../.."
PROJECTNAMECAP=`capitalize $PROJECTNAME`

Instantiate()
{
    SOURCE="$1"
    TARGET="$2"

    # XXX We should somehow quote here...
    cat "$SOURCE" | sed " \
	s^#PROJECTNAME#^$PROJECTNAME^g;
	s^#PROJECTNAMECAP#^$PROJECTNAMECAP^g;
	s^#EMAIL#^$EMAIL^g; \
	s^#HOMEPAGE#^$HOMEPAGE^g; \
	s^#LONGNAME#^$LONGNAME^g; \
	s^#AUTHOR#^$AUTHOR^g; \
	s^#VERSION#^$VERSION^g; \
	s^#COPYRIGHT#^$COPYRIGHT^g; \
	s^#USECEL#^$USECEL^g; \
	" > "$TARGET"
}

# create directories
mkdir "$PROJECTNAME" || exit 2
mkdir "$PROJECTNAME/mk" || exit 2
mkdir "$PROJECTNAME/mk/msvcgen" || exit 2
mkdir "$PROJECTNAME/msvc" || exit 2
mkdir "$PROJECTNAME/src" || exit 2

# copy Autoconf, Jam, and msvcgen support files.
cp -pr "$CSDIR/mk/autoconf" "$PROJECTNAME/mk"
cp -pr "$CSDIR/mk/jam" "$PROJECTNAME/mk"
cp -p  "$CSDIR/mk/msvcgen/"*.tlib "$PROJECTNAME/mk/msvcgen"
cp -pr "$CSDIR/mk/perl5" "$PROJECTNAME/mk"

cp -p "$TEMPLATEDIR/cel.m4" "$PROJECTNAME/mk/autoconf"
cp -p "$TEMPLATEDIR/cs_check_host.m4" "$PROJECTNAME/mk/autoconf"

# instantiate template files
Instantiate "$TEMPLATEDIR/autogen.template" "$PROJECTNAME/autogen.sh"
Instantiate "$TEMPLATEDIR/config-msvc.template" "$PROJECTNAME/config-msvc.h"
Instantiate "$TEMPLATEDIR/configure.template" "$PROJECTNAME/configure.ac"
Instantiate "$TEMPLATEDIR/Jamfile.template" "$PROJECTNAME/Jamfile.in"
Instantiate "$TEMPLATEDIR/Jamfile-src.template" "$PROJECTNAME/src/Jamfile"
Instantiate "$TEMPLATEDIR/Jamrules.template" "$PROJECTNAME/Jamrules"
Instantiate "$TEMPLATEDIR/main.template" "$PROJECTNAME/src/main.cpp"
Instantiate "$TEMPLATEDIR/app.cpp.template" \
	    "$PROJECTNAME/src/app$PROJECTNAME.cpp"
Instantiate "$TEMPLATEDIR/app.h.template" "$PROJECTNAME/src/app$PROJECTNAME.h"
Instantiate "$TEMPLATEDIR/projheader.template" \
	    "$PROJECTNAME/src/$PROJECTNAME.h"
Instantiate "$TEMPLATEDIR/README.template" "$PROJECTNAME/README"
Instantiate "$TEMPLATEDIR/README-msvc.template" "$PROJECTNAME/msvc/README"

chmod +x "$PROJECTNAME/autogen.sh"
chmod +x "$PROJECTNAME/mk/perl5/bin/ttree"

# remove CVS directories which may have been copied from the source location
find "$PROJECTNAME" -type d -name CVS -exec rm -rf {} \; -prune

echo "*Running: autoheader autoconf"
cd "$PROJECTNAME"
./autogen.sh
rm -rf autom4te.cache

# Display notice
cat << __EOF__

*Generation successful.

You should now examine the generated project and customize it as needed.  In
particular, the files README, app$PROJECTNAME.cpp, and app$PROJECTNAME.h will
require modification.

Have fun with Crystal Space.
__EOF__
