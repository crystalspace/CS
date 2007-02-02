#include "version.inc"

#pragma parseroption -p-

#define MyAppName "Crystal Space SDK (" + SDKEdition + ")"
#define MyAppVerName "Crystal Space SDK (" + SDKEdition + ") "+ CSVersion
#define MyAppURL "http://crystalspace3d.org"
#define MyAppUrlName "Crystal Space home page.url"

[Setup]
AppName={#MyAppName}
AppVerName={#MyAppVerName}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
LicenseFile=lgpl.txt
OutputBaseFilename=crystalspace-sdk-{#SDKshort}-{#CSVersion}
OutputDir=..\..\out\installer
Compression=none
;Compression=lzma
SolidCompression=yes
TimeStampsInUTC=yes

[Languages]
Name: "eng"; MessagesFile: "compiler:Default.isl"

[Icons]
Name: "{group}\{cm:ProgramOnTheWeb,{#MyAppName}}"; Filename: "{#MyAppURL}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{group}\Users Manual"; Filename: "{app}\docs\html\manual\index.html"
Name: "{group}\Public API Documentation"; Filename: "{app}\docs\html\api\index.html"
Name: "{group}\StartMe Demo Launcher"; Filename: "{app}\startme.exe"
