#define SDKEdition "MSVC " + MSVCVersion
#define SDKShort   "msvc" + MSVCVersion
#include "sdk-common.inc"

[Files]
#define LIBRARY(str name)                    \
'Source: "..\\..\\out\\release' + MSVCVersion + '\\libs\\lib'+name+'.lib"; DestDir:  "{app}\\out\\release8\\libs"; Flags: Touch' + '\r' + \
'Source: "..\\..\\out\\debug' + MSVCVersion + '\\libs\\lib'+name+'_d.lib"; DestDir:  "{app}\\out\\debug8\\libs"; Flags: Touch'
#define DEPENDENCY(str name)                 \
'Source: "..\\..\\lib'+name+'-cs.dll"; DestDir:  "{app}"'
#define DEPENDENCY_LINK_ONLY(str name)       \
'Source: "..\\..\\libs\\csutil\\win32\\libs\\'+name+'.lib"; DestDir:  "{app}\\libs\\csutil\\win32\\libs"; Flags: Touch'
#define DEPENDENCY_LINK(str name)            \
DEPENDENCY_LINK_ONLY(name) + '\r' + DEPENDENCY(name)
#define DEPENDENCY_LINK2(str name, str base) \
DEPENDENCY_LINK_ONLY(base) + '\r' + DEPENDENCY(name)
#define DEPENDENCY_LIT(str name)             \
'Source: "..\\..\\'+name+'.dll"; DestDir:  "{app}"'
#define DEPENDENCY_SPECIFIC(str name)              \
'Source: "..\\..\\lib'+name+'-csvc' + MSVCVersion + '.dll"; DestDir:  "{app}\\bin-release"' + '\r' + \
'Source: "..\\..\\lib'+name+'-csvc' + MSVCVersion + '_d.dll"; DestDir:  "{app}\\bin-debug"'
#define DEPENDENCY_SPECIFIC_LINK_ONLY(str name)    \
'Source: "..\\..\\libs\\csutil\\win32\\libs\\'+name+'-vc' + MSVCVersion + '.lib"; DestDir:  "{app}\\libs\\csutil\\win32\\libs"; Flags: Touch' + '\r' + \
'Source: "..\\..\\libs\\csutil\\win32\\libs\\'+name+'-vc' + MSVCVersion + '_d.lib"; DestDir:  "{app}\\libs\\csutil\\win32\\libs"; Flags: Touch'
#define DEPENDENCY_SPECIFIC_LINK(str name)         \
DEPENDENCY_SPECIFIC(name) + '\r' + DEPENDENCY_SPECIFIC_LINK_ONLY(name)
#define APPLICATION(str name)                \
'Source: "..\\..\\out\\release' + MSVCVersion + '\\bin\\'+name+'.exe"; DestDir:  "{app}"'
#define APPLICATION_DEMO(str name)           APPLICATION(name)
#define APPLICATION_IMPORT(str name)         APPLICATION(name)
#define APPLICATION_TEST(str name)           ''
#define APPLICATION_TOOL(str name)           APPLICATION(name)
#define APPLICATION_TUTORIAL(str name)       APPLICATION(name)
#define PLUGIN(str name)                     \
'Source: "..\\..\\out\\release' + MSVCVersion + '\\bin\\'+name+'.dll"; DestDir:  "{app}"'
#define PLUGIN_DEBUG_RELEASE(str name)       \
'Source: "..\\..\\out\\release' + MSVCVersion + '\\bin\\'+name+'.dll"; DestDir:  "{app}\\bin-release"' + '\r' + \
'Source: "..\\..\\out\\debug' + MSVCVersion + '\\bin\\'+name+'.dll"; DestDir:  "{app}\\bin-debug"'
#include "files.inc"

Source: "{app}\bin-release\*.dll"; DestDir: "{app}"; Flags: External

Source: "..\..\docs\html\*.*"; DestDir: "{app}\docs\html"; Excludes: ".svn,Jamfile"; Flags: RecurseSubDirs

Source: "..\..\mk\autoconf\*"; DestDir: "{app}\mk\autoconf"; Excludes: ".svn,Jamfile";
Source: "..\..\mk\jam\*"; DestDir: "{app}\mk\jam"; Excludes: ".svn,Jamfile";
Source: "..\..\mk\msvcgen\*.tlib"; DestDir: "{app}\mk\msvcgen";
Source: "..\..\mk\msvcgen\custom.cslib"; DestDir: "{app}\mk\msvcgen";

Source: "..\..\scripts\hammer\*"; DestDir: "{app}\scripts\hammer\"; Excludes: ".svn,Jamfile";
Source: "..\..\scripts\jamtemplate\*"; DestDir: "{app}\scripts\jamtemplate\"; Excludes: ".svn,Jamfile";
Source: "..\..\scripts\max\*"; DestDir: "{app}\scripts\max\"; Excludes: ".svn,Jamfile";
Source: "..\..\scripts\maya\*"; DestDir: "{app}\scripts\maya\"; Excludes: ".svn,Jamfile";

Source: "..\..\data\*.zip"; DestDir: "{app}\data\"
Source: "..\..\data\parallaxtest\*"; DestDir: "{app}\data\parallaxtest\"; Excludes: ".svn"; Flags: RecurseSubDirs
Source: "..\..\data\partsys\*"; DestDir: "{app}\data\partsys\"; Excludes: ".svn"; Flags: RecurseSubDirs
Source: "..\..\data\r3dtest\*"; DestDir: "{app}\data\r3dtest\"; Excludes: ".svn"; Flags: RecurseSubDirs
Source: "..\..\data\stenciltest\*"; DestDir: "{app}\data\stenciltest\"; Excludes: ".svn"; Flags: RecurseSubDirs
Source: "..\..\data\terrain\*"; DestDir: "{app}\data\terrain\"; Excludes: ".svn"; Flags: RecurseSubDirs
Source: "..\..\data\terrainf\*"; DestDir: "{app}\data\terrainf\"; Excludes: ".svn"; Flags: RecurseSubDirs

Source: "..\..\data\castle\*"; DestDir: "{app}\data\castle\"; Excludes: ".svn,Jamfile"; Flags: RecurseSubDirs
Source: "..\..\data\ceguitest\0.5\*"; DestDir: "{app}\data\ceguitest\0.5\"; Excludes: ".svn,Jamfile"; Flags: RecurseSubDirs
Source: "..\..\data\cube\*"; DestDir: "{app}\data\cube\"; Excludes: ".svn,Jamfile"; Flags: RecurseSubDirs
Source: "..\..\data\flarge\*"; DestDir: "{app}\data\flarge\"; Excludes: ".svn,Jamfile"; Flags: RecurseSubDirs

Source: "..\..\vfs.cfg"; DestDir: "{app}";
Source: "..\..\data\config\*"; DestDir: "{app}\data\config\"; Excludes: ".svn,Jamfile"; Flags: RecurseSubDirs
Source: "..\..\data\shader\*"; DestDir: "{app}\data\shader\"; Excludes: ".svn,Jamfile"; Flags: RecurseSubDirs
Source: "..\..\data\varia\*"; DestDir: "{app}\data\varia\"; Excludes: ".svn,Jamfile"; Flags: RecurseSubDirs


;#expr SaveToFile(AddBackslash(SourcePath) + "Preprocessed.iss")
