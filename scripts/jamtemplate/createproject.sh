#!/bin/sh

# Helper function to read variables
function ReadValue()
{
    TEXT="$2"
    VARNAME="$1"
    echo -n "$TEXT"
    read READVAL

    export $VARNAME=$READVAL
}

function ReadValueStrict()
{
    TEXT="$2"
    VARNAME="$1"
    READVAL=""
    
    while test -z "$READVAL"; do
	echo -n "$TEXT"
	read READVAL
	if test -z "$READVAL"; then
	    echo "Please enter a value!"
	fi
    done

    export $VARNAME="$READVAL"
}

function CheckYesNo()
{
    VARNAME="$1"
    TEXT="$2"
    DEFAULT="$3"
    READOK=no
    
    while test "$READOK" = "no"; do
        echo -n "$TEXT"
	read READVAL       
	
	if test -z "$READVAL"; then
	    export $VARNAME=$DEFAULT
	    READOK=yes
	fi
	if test "$READVAL" = "yes"; then
	    export $VARNAME=yes
	    READOK=yes
	fi
	if test "$READVAL" = "no"; then
	    export $VARNAME=no
	    READOK=yes
	fi
	
	if test "$READOK" = "no"; then
	    echo 'Please enter "yes" or "no"!'
	fi
    done
}

# Part 1: Interaktive - Gather informations
cat << __EOF__
Crystal Space external Project Template

This script will generate a basic Crystal Space project for you, complete
with configuration(autoconf) and build system(jam). The project will be created
in the current directory. If you don't want that press CTRL+C now!

__EOF__

cat << "__EOF__"
**** Project
You should choose a projectname now. We need a shortform of the name which
will be used in directory and filenames. The shortname should be lowercase not
must not contain any special chars or spaces. The long name will be present in
helptexts and README files. So If you project is called "Space Fighters -
Revenge for $$$" then you should probably choose shortname spacefighters and
"Space Fighters - Revenge for $$$" as longname.
You may provide a Homepage address for the project. (Please specify a complete
URI. Example: "http://www.spacefighter.org")

__EOF__
ReadValueStrict PROJECTNAME "Shortname: "
ReadValueStrict LONGNAME "Longname: "
ReadValueStrict VERSION "Version: "
ReadValue HOMEPAGE "Homepage: "

cat << __EOF__

**** Contact
The informations about the author are mentioned in the README file and in the
configuration scripts so that people see a support address when typing
"configure --help".

__EOF__
ReadValueStrict AUTHOR "Author: "
ReadValueStrict EMAIL "E-Mail: "
ReadValue COPYRIGHT "Copyright: "

cat << __EOF__

**** Fine Tuning
ok.
__EOF__
#CheckYesNo KDEVELOP "Do you want me to generate a kdevelop projectfile?[no] " "no"

echo ""
echo "*Creating $PROJECTNAME Project."

TEMPLATEDIR=`dirname $0`
CSDIR="$TEMPLATEDIR/../.."

function SetupFromTemplate
{
    SOURCE="$1"
    TARGET="$2"

# XXX We should somehow quote here...
    cat "$1" | sed -e "s^#PROJECTNAME#^$PROJECTNAME^g" \
	     | sed -e "s^#EMAIL#^$EMAIL^g" \
	     | sed -e "s^#HOMEPAGE#^$HOMEPAGE^g" \
	     | sed -e "s^#LONGNAME#^$LONGNAME^g" \
	     | sed -e "s^#AUTHOR#^$AUTHOR^g" \
	     | sed -e "s^#VERSION#^$VERSION^g" \
	     | sed -e "s^#COPYRIGHT#^$VERSION^g" \
	       > "$2"
}

# create directories
mkdir "$PROJECTNAME" || exit 2
mkdir "$PROJECTNAME/mk" || exit 3
mkdir "$PROJECTNAME/src" || exit 4

# copy jamrules
cp -pr "$CSDIR/mk/jam" "$PROJECTNAME/mk"
# copy autoconf macros
cp -pr "$CSDIR/mk/autoconf" "$PROJECTNAME/mk"

# copy templatefiles
cp -p "$TEMPLATEDIR/autogen.template" "$PROJECTNAME/autogen.sh"
chmod +x "$PROJECTNAME/autogen.sh"
SetupFromTemplate "$TEMPLATEDIR/configure.template" "$PROJECTNAME/configure.ac"
SetupFromTemplate "$TEMPLATEDIR/README.template" "$PROJECTNAME/README"
cp -p "$TEMPLATEDIR/Jamfile.template" "$PROJECTNAME/Jamfile"
cp -p "$TEMPLATEDIR/Jamrules.template" "$PROJECTNAME/Jamrules"
cp -p "$TEMPLATEDIR/initjamfile.m4" "$PROJECTNAME/mk/autoconf"
cp -p "$TEMPLATEDIR/cs_check_host.m4" "$PROJECTNAME/mk/autoconf"
cp -p "$CSDIR/mk/autoconf/crystal.m4" "$PROJECTNAME/mk/autoconf"

# create some simple Jamfile and dummy file in src
cat > "$PROJECTNAME/src/Jamfile" << __END__
SubDir TOP src ;

Application $PROJECTNAME
	: [ Wildcard *.cpp *.h ]
;
ExternalLibs $PROJECTNAME : CRYSTAL ;
__END__

cat > "$PROJECTNAME/src/main.cpp" << __END__
#include <config.h>

CS_IMPLEMENT_APPLICATION

int main(int argc, char** argv)
{
  (void) argc;
  (void) argv;
  printf ("This is the insanely great $PROJECTNAME project.\n");
  return 0;
}
__END__

echo Running autoconf
cd "$PROJECTNAME"
./autogen.sh

# Display notice
cat << "__EOF__"

*Generation successfull.

You should now look at your project and try to customize it at your needs. Esp.
the README file only contains some generic blabla now.

Have fun with Crystal Space.
__EOF__
