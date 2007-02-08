#define SDKEdition "MSVC " + MSVCVersion
#define SDKShort   "msvc" + MSVCVersion
#include "sdk-common.inc"

[Files]
#define LIBRARY(str name)                    \
'Source: "..\\..\\out\\release' + MSVCVersion + '\\libs\\lib'+name+'.lib"; DestDir:  "{app}\\out\\release' + MSVCVersion + '\\libs"; Flags: Touch' + '\r' + \
'Source: "..\\..\\out\\release' + MSVCVersion + '\\build\\lib'+name+'\\'+name'.pdb"; DestDir:  "{app}\\out\\release' + MSVCVersion + '\\libs"' +  '\r' + \
'Source: "..\\..\\out\\debug' + MSVCVersion + '\\libs\\lib'+name+'_d.lib"; DestDir:  "{app}\\out\\debug' + MSVCVersion + '\\libs"; Flags: Touch' + '\r' + \
'Source: "..\\..\\out\\debug' + MSVCVersion + '\\build\\lib'+name+'\\'+name'.pdb"; DestDir:  "{app}\\out\\debug' + MSVCVersion + '\\libs"'
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

#include "[Files]-common.inc"

#define IncludeDir    "{app}\\include\\crystalspace"
#include "[Files]-include.inc"

;#expr SaveToFile(AddBackslash(SourcePath) + "Preprocessed.iss")
