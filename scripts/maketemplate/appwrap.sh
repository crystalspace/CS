#! /bin/sh
#==============================================================================
# appwrap.sh
#
#	Create a proper Cocoa application wrapper for the specified program.
#
# Copyright (C)2002,2004 by Eric Sunshine <sunshine@sunshineco.com>
#==============================================================================

NAME=$1
DIR=$2
VERSION=$3
RELEASE_DATE="$4"

APP="${DIR}/${NAME}.app"

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
    <key>CFBundleInfoDictionaryVersion</key>
    <string>6.0</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleSignature</key>
    <string>????</string>
    <key>CFBundleVersion</key>
    <string>0</string>
    <key>CFBundleShortVersionString</key>
    <string>${VERSION}</string>
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
CFBundleShortVersionString = "${VERSION}";
CFBundleGetInfoString = "${NAME}, ${VERSION}, ${RELEASE_DATE}";
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
    <string>${VERSION}</string>
    <key>CFBundleVersion</key>
    <string>${VERSION}, ${RELEASE_DATE}</string>
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
  wrap_macosx_info_plist
  wrap_macosx_info_plist_strings
  wrap_macosx_pkg_info
  wrap_macosx_version_plist
}


#==============================================================================
# Dispatcher
#==============================================================================
wrap_macosx
exit 0
