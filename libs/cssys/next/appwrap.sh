#! /bin/sh
#==============================================================================
# wrapapp.sh
#
#	Create an application wrapper (.app) of the specified type (MacOS/X,
#	MacOS/X Server [Rhapsody], OpenStep, or NextStep) for the given
#	program.
#
# Copyright (C)2002 by Eric Sunshine <sunshine@sunshineco.com>
#==============================================================================

TYPE=$1
NAME=$2
DIR=$3
ICON=$4

APP="${DIR}/${NAME}.app"
ICON_ROOT=`basename ${ICON}`

VERSION_FILE="include/csver.h"
VERSION_MAJOR=`sed -e '/#define[ 	][ 	]*CS_VERSION_MAJOR/!d' -e 's/#define[ 	][ 	]*CS_VERSION_MAJOR[ 	][ 	]*CS_VER_QUOTE(\(..*\)).*/\1/' < ${VERSION_FILE}`
VERSION_MINOR=`sed -e '/#define[ 	][ 	]*CS_VERSION_MINOR/!d' -e 's/#define[ 	][ 	]*CS_VERSION_MINOR[ 	][ 	]*CS_VER_QUOTE(\(..*\)).*/\1/' < ${VERSION_FILE}`
VERSION_DATE=`sed -e '/#define[ 	][ 	]*CS_RELEASE_DATE/!d' -e 's/#define[ 	][ 	]*CS_RELEASE_DATE[ 	][ 	]*CS_VER_QUOTE(\(..*\)).*/\1/' < ${VERSION_FILE}`


#==============================================================================
# NextStep
#==============================================================================
#------------------------------------------------------------------------------
# Create NextStep wrapper
#------------------------------------------------------------------------------
wrap_nextstep()
{
  mkdirs ${APP}
}


#==============================================================================
# OpenStep
#==============================================================================
#------------------------------------------------------------------------------
# Create OpenStep Info.plist
#------------------------------------------------------------------------------
wrap_openstep_info_plist()
{
  mkdir -p ${APP}/Resources
  cat << EOT > ${APP}/Resources/Info-nextstep.plist
{
  NSExecutable = ${NAME};
  NSIcon = ${ICON_ROOT}; 
  NSPrincipalClass = NSApplication;
}
EOT
}


#------------------------------------------------------------------------------
# Create OpenStep wrapper
#------------------------------------------------------------------------------
wrap_openstep()
{
  mkdir -p ${APP}/Resources
  rm -f ${APP}/Resources/${ICON_ROOT}
  cp ${ICON} ${APP}/Resources
  wrap_openstep_info_plist
}


#==============================================================================
# MacOS/X Server (Rhapsody)
#==============================================================================
#------------------------------------------------------------------------------
# Create MacOS/X Server wrapper
#------------------------------------------------------------------------------
wrap_macosxs()
{
  wrap_openstep
}


#==============================================================================
# MacOS/X
#==============================================================================
#------------------------------------------------------------------------------
# Create MacOS/X Info.plist
#------------------------------------------------------------------------------
wrap_macosx_info_plist()
{
  mkdir -p ${APP}/Contents
  cat << EOT > ${APP}/Contents/Info.plist
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist SYSTEM "file://localhost/System/Library/DTDs/PropertyList.dtd">
<plist version="0.9">
  <dict>
    <key>CFBundleDevelopmentRegion</key>
    <string>English</string>
    <key>CFBundleName</key>
    <string>${NAME}</string>
    <key>CFBundleExecutable</key>
    <string>${NAME}</string>
    <key>CFBundleIconFile</key>
    <string>${ICON_ROOT}</string>
    <key>CFBundleInfoDictionaryVersion</key>
    <string>6.0</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleSignature</key>
    <string>????</string>
    <key>CFBundleVersion</key>
    <string>0</string>
    <key>CFBundleShortVersionString</key>
    <string>${VERSION_MAJOR}.${VERSION_MINOR}</string>
    <key>NSPrincipalClass</key>
    <string>NSApplication</string>
  </dict>
</plist>
EOT
}


#------------------------------------------------------------------------------
# Create MacOS/X InfoPlist.strings
#------------------------------------------------------------------------------
wrap_macosx_info_plist_strings()
{
  mkdir -p ${APP}/Contents/Resources/English.lproj
  cat << EOT > ${APP}/Contents/Resources/English.lproj/InfoPlist.strings
CFBundleName = "${NAME}";
CFBundleShortVersionString = "${VERSION_MAJOR}.${VERSION_MINOR}";
CFBundleGetInfoString = "${NAME}, ${VERSION_MAJOR}.${VERSION_MINOR}, ${VERSION_DATE}";
EOT
}


#------------------------------------------------------------------------------
# Create MacOS/X PkgInfo
#------------------------------------------------------------------------------
wrap_macosx_pkg_info()
{
  mkdir -p ${APP}/Contents
  cat << EOT > ${APP}/Contents/PkgInfo
APPL????
EOT
}


#------------------------------------------------------------------------------
# Create MacOS/X version.plist
#------------------------------------------------------------------------------
wrap_macosx_version_plist()
{
  mkdir -p ${APP}/Contents
  cat << EOT > ${APP}/Contents/version.plist
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist SYSTEM "file://localhost/System/Library/DTDs/PropertyList.dtd">
<plist version="0.9">
  <dict>
    <key>CFBundleShortVersionString</key>
    <string>${VERSION_MAJOR}.${VERSION_MINOR}</string>
    <key>CFBundleVersion</key>
    <string>${VERSION_MAJOR}.${VERSION_MINOR}, ${VERSION_DATE}</string>
    <key>ProjectName</key>
    <string>${NAME}</string>
  </dict>
</plist>
EOT
}


#------------------------------------------------------------------------------
# Create MacOS/X wrapper
#------------------------------------------------------------------------------
wrap_macosx()
{
  mkdir -p ${APP}/Contents/MacOS
  mkdir -p ${APP}/Contents/Resources
  rm -f ${APP}/Contents/Resources/${ICON_ROOT}
  cp ${ICON} ${APP}/Contents/Resources
  wrap_macosx_info_plist
  wrap_macosx_info_plist_strings
  wrap_macosx_pkg_info
  wrap_macosx_version_plist
}


#==============================================================================
# Dispatcher
#==============================================================================
case ${TYPE} in
  nextstep) wrap_nextstep ;;
  openstep) wrap_openstep ;;
  macosxs)  wrap_macosxs  ;;
  macosx)   wrap_macosx   ;;
  *) echo "$0: Unrecognized wrapper type: ${TYPE}"; exit 1
esac

exit 0
