#!/bin/sh

# Helper to read variables
ReadValue()
{
    TEXT="$2"
    VARNAME="$1"
    echo -n "$TEXT"
    read READVAL
    eval "$VARNAME=\"$READVAL\" ; export $VARNAME"
}

ReadValueStrict()
{
    TEXT="$2"
    VARNAME="$1"
    READVAL=""
    
    while test -z "$READVAL"; do
	echo -n "$TEXT"
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
    READOK=no
    
    while test "$READOK" = "no"; do
        echo -n "$TEXT"
	read READVAL       
	
	if test -z "$READVAL"; then
	    eval "$VARNAME=\"$DEFAULT\" ; export $VARNAME"
	    READOK=yes
	elif test "$READVAL" = "yes"; then
	    eval "$VARNAME=yes ; export $VARNAME"
	    READOK=yes
	elif test "$READVAL" = "no"; then
	    eval "$VARNAME=no ; export $VARNAME"
	    READOK=yes
	fi
	
	if test "$READOK" = "no"; then
	    echo 'Please enter "yes" or "no".'
	fi
    done
}

# Poor man's (portable) 'dirname' command.
dirpart() {
    expr "$1" : '\(..*\)[/\\].*'
}

# Part 1: Interactive - Gather information
cat << __EOF__
Crystal Space external Project Template

This script will generate a basic Crystal Space project for you, complete with
configuration (Autoconf) and build system (Jam).  The project will be created
in a subdirectory of the current directory.  You can abort the script at any
time by typing Ctrl-C.

__EOF__

cat << "__EOF__"
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
ReadValueStrict PROJECTNAME "Short name: "
ReadValueStrict LONGNAME "Long name: "
ReadValueStrict VERSION "Version: "
ReadValue HOMEPAGE "Home page URI: "

cat << __EOF__

**** Contact
The information about the author is mentioned in the README file and in the
configuration script so that people see a support address when they invoke
"./configure --help".

__EOF__
ReadValueStrict AUTHOR "Author: "
ReadValueStrict EMAIL "Email: "
ReadValue COPYRIGHT "Copyright: "

echo ""
echo "*Creating project: $PROJECTNAME"

TEMPLATEDIR=`dirpart $0`
CSDIR="$TEMPLATEDIR/../.."

Instantiate()
{
    SOURCE="$1"
    TARGET="$2"

    # XXX We should somehow quote here...
    cat "$SOURCE" | sed -e " \
	s^#PROJECTNAME#^$PROJECTNAME^g;
	s^#EMAIL#^$EMAIL^g; \
	s^#HOMEPAGE#^$HOMEPAGE^g; \
	s^#LONGNAME#^$LONGNAME^g; \
	s^#AUTHOR#^$AUTHOR^g; \
	s^#VERSION#^$VERSION^g; \
	s^#COPYRIGHT#^$COPYRIGHT^g; \
	" > "$TARGET"
}

# create directories
mkdir "$PROJECTNAME" || exit 2
mkdir "$PROJECTNAME/mk" || exit 2
mkdir "$PROJECTNAME/mk/msvcgen" || exit 2
mkdir "$PROJECTNAME/mk/msvcgen/template6" || exit 2
mkdir "$PROJECTNAME/mk/msvcgen/template7" || exit 2
mkdir "$PROJECTNAME/msvc" || exit 2
mkdir "$PROJECTNAME/src" || exit 2

# copy Autoconf, Jam, and msvcgen support files.
cp -pr "$CSDIR/mk/autoconf" "$PROJECTNAME/mk"
cp -pr "$CSDIR/mk/jam" "$PROJECTNAME/mk"
cp -pr "$CSDIR/mk/msvcgen/msvcgen.pl" "$PROJECTNAME/mk/msvcgen"
cp -pr "$TEMPLATEDIR/template6" "$PROJECTNAME/mk/msvcgen"
cp -pr "$TEMPLATEDIR/template7" "$PROJECTNAME/mk/msvcgen"

# instantiate template files
Instantiate "$TEMPLATEDIR/autogen.template" "$PROJECTNAME/autogen.sh"
Instantiate "$TEMPLATEDIR/configure.template" "$PROJECTNAME/configure.ac"
Instantiate "$TEMPLATEDIR/Jamfile.template" "$PROJECTNAME/Jamfile"
Instantiate "$TEMPLATEDIR/Jamfile-src.template" "$PROJECTNAME/src/Jamfile"
Instantiate "$TEMPLATEDIR/Jamrules.template" "$PROJECTNAME/Jamrules"
Instantiate "$TEMPLATEDIR/main.template" "$PROJECTNAME/src/main.cpp"
Instantiate "$TEMPLATEDIR/README.template" "$PROJECTNAME/README"
Instantiate "$TEMPLATEDIR/README-msvc.template" "$PROJECTNAME/msvc/README"

cp -p "$TEMPLATEDIR/initjamfile.m4" "$PROJECTNAME/mk/autoconf"
cp -p "$TEMPLATEDIR/cs_check_host.m4" "$PROJECTNAME/mk/autoconf"

chmod +x "$PROJECTNAME/autogen.sh"
chmod +x "$PROJECTNAME/mk/msvcgen/msvcgen.pl"

# remove CVS directories which may have been copied from the source location
find "$PROJECTNAME" -type d -name CVS -exec rm -rf {} \; -prune

echo "*Running: autoheader autoconf"
cd "$PROJECTNAME"
./autogen.sh
rm -rf autom4te.cache

# Display notice
cat << "__EOF__"

*Generation successful.

You should now examine the generated project and customize it as needed.  In
particular, the files README and main.cpp will require modification.

Have fun with Crystal Space.
__EOF__
