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
