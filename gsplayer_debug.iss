[Setup]
AppName=GSPlayer
AppVersion=2.29
;AppVerName=GSPlayer 2.29
AppPublisher=GreenSoftware
DefaultDirName={pf}\GSPlayer
DefaultGroupName=GSPlayer
AllowNoIcons=yes
LicenseFile=gpl.txt
OutputBaseFilename=GSPlayerSetup
Compression=lzma
SolidCompression=yes

[Languages]
Name: en; MessagesFile: "compiler:Default.isl"
Name: ja; MessagesFile: "compiler:Languages\Japanese.isl"

[Files]
Source: "bin\eng\Win32\DebugUnicode\GSPlayer2.exe"; DestDir: "{app}"; DestName: "GSPlayer2e.exe"
Source: "bin\jpn\Win32\DebugUnicode\GSPlayer2.exe"; DestDir: "{app}"
Source: "gspflac\gspflac\DebugUnicode\gspflac.dll"; DestDir: "{app}"
Source: "gspmidi\gspmidi\DebugUnicode\gspmidi.dll"; DestDir: "{app}"
Source: "gspmp4\gspmp4\gspmp4\Debug\gspmp4.dll"; DestDir: "{app}"
Source: "gspwma\gspwma\Debug\gspwma.dll"; DestDir: "{app}"
Source: "gstta\DebugUnicode\gstta.dll"; DestDir: "{app}"
Source: "ModPlugin\WIN32\BASSMOD.dll"; DestDir: "{app}"
Source: "ModPlugin\WIN32\DebugUnicode\ModPlugin.dll"; DestDir: "{app}"
Source: "gmbank\gm.cfg"; DestDir: "{app}"
Source: "gmbank\gm\*.pat"; DestDir: "{app}\gm"
Source: "skins\*"; DestDir: "{app}\Skins"; Flags: recursesubdirs

[Registry]
Root: HKCU; Subkey: "Software\GreenSoftware\GSPlayer\Plug-ins\gspmidi"; ValueName: "ConfigFile"; ValueType: String; ValueData: "{app}\gm.cfg"

[Run]
Filename: "{app}\GSPlayer2e.exe"; Description: "{cm:LaunchProgram,GSPlayer}"; Flags: nowait postinstall skipifsilent

[Icons]
Name: "{group}\GSPlayer"; Filename: "{app}\GSPlayer2e.exe"
Name: "{group}\GSPlayer (Japanese)"; Filename: "{app}\GSPlayer2.exe"
Name: "{group}\{cm:UninstallProgram,GSPlayer}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\GSPlayer"; Filename: "{app}\GSPlayer2e.exe"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

