#define CSWin32LibsDir 	"D:\Programme\CrystalSpaceLibs"

#define SDKEdition "MinGW"
#define SDKShort   "mingw"
#include "sdk-common.inc"

[Files]
#define LIBRARY(str name)                    \
'Source: "..\\..\\..\\CS-1.0-mingw\\out\\msysx86\\optimize\\libs\\lib'+name+'.a"; DestDir:  "{app}\\lib"; Flags: Touch'
#define DEPENDENCY(str name)                 \
'Source: "..\\..\\lib'+name+'-cs.dll"; DestDir:  "{app}"'
#define DEPENDENCY_LINK_ONLY(str name)       \
'Source: "'+CSWin32LibsDir+'\\common\\lib\\'+name+'.lib"; DestDir:  "{app}\\lib"; Flags: Touch'
#define DEPENDENCY_LINK(str name)            \
DEPENDENCY_LINK_ONLY(name) + '\r' + DEPENDENCY(name)
#define DEPENDENCY_LINK2(str name, str base) \
DEPENDENCY_LINK_ONLY(base) + '\r' + DEPENDENCY(name)
#define DEPENDENCY_LIT(str name)             \
'Source: "..\\..\\'+name+'.dll"; DestDir:  "{app}"'
#define DEPENDENCY_SPECIFIC(str name)              \
'Source: "..\\..\\lib'+name+'-csmingw-gcc-3.4-1.dll"; DestDir:  "{app}"; Flags: skipifsourcedoesntexist'
#define DEPENDENCY_SPECIFIC_LINK_ONLY(str name)    \
'Source: "'+CSWin32LibsDir+'\\mingw\\lib\\lib'+name+'.a"; DestDir:  "{app}\\lib"; Flags: Touch'
#define DEPENDENCY_SPECIFIC_LINK(str name)         \
DEPENDENCY_SPECIFIC(name) + '\r' + DEPENDENCY_SPECIFIC_LINK_ONLY(name)
#define APPLICATION(str name)                \
'Source: "..\\..\\..\\CS-1.0-mingw\\'+name+'.exe"; DestDir:  "{app}"'
#define APPLICATION_DEMO(str name)           APPLICATION(name)
#define APPLICATION_IMPORT(str name)         APPLICATION(name)
#define APPLICATION_TEST(str name)           ''
#define APPLICATION_TOOL(str name)           APPLICATION(name)
#define APPLICATION_TUTORIAL(str name)       APPLICATION(name)
#define PLUGIN(str name)                     \
'Source: "..\\..\\..\\CS-1.0-mingw\\'+name+'.dll"; DestDir:  "{app}"'
#define PLUGIN_DEBUG_RELEASE(str name)       \
'Source: "..\\..\\..\\CS-1.0-mingw\\'+name+'.dll"; DestDir:  "{app}"'
#include "files.inc"

#include "[Files]-common.inc"

#define IncludeDir    "{app}\\include\\crystalspace"
#include "[Files]-include.inc"
Source: "..\..\..\CS-1.0-mingw\include\csconfig.h"; DestDir: "{#IncludeDir}"

Source: "cs-config"; DestDir:  "{app}"

;#expr SaveToFile(AddBackslash(SourcePath) + "Preprocessed.iss")
