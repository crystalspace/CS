#!/bin/sh

TEMPLATEDIR=`dirname $0`
CSDIR="$TEMPLATEDIR/../.."
PROJECTNAME="$1"
EMAIL="$2"
if [ -z "$EMAIL" ]; then
  echo "Please invoke this script with a name for your project and an email"
  echo "address of the author."
  exit 1
fi

echo Creating package

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
cat "$TEMPLATEDIR/configure.template" | sed -e "s/#PROJECTNAME#/$PROJECTNAME/g" -e "s/#EMAIL#/$EMAIL/g" > "$PROJECTNAME/configure.ac"
cp -p "$TEMPLATEDIR/Jamfile.template" "$PROJECTNAME/Jamfile"
cp -p "$TEMPLATEDIR/Jamrules.template" "$PROJECTNAME/Jamrules"
cp -p "$TEMPLATEDIR/initjamfile.m4" "$PROJECTNAME/mk/autoconf"
cp -p "$TEMPLATEDIR/cs_check_host.m4" "$PROJECTNAME/mk/autoconf"
cp -p "$TEMPLATEDIR/crystal_jam.m4" "$PROJECTNAME/mk/autoconf"

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

