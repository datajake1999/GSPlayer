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
Source: "bin\jpn\Win32\DebugUnicode\GSPlayer2.exe"; DestDir: "{app}"; Components: "japanese"
Source: "gspflac\gspflac\DebugUnicode\gspflac.dll"; DestDir: "{app}"; Components: "flac"
Source: "gspmidi\gspmidi\DebugUnicode\gspmidi.dll"; DestDir: "{app}"; Components: "midi"
Source: "gspmp4\gspmp4\gspmp4\Debug\gspmp4.dll"; DestDir: "{app}"; Components: "mp4"
Source: "gspwma\gspwma\Debug\gspwma.dll"; DestDir: "{app}"; Components: "wma"
Source: "gstta\DebugUnicode\gstta.dll"; DestDir: "{app}"; Components: "tta"
Source: "ModPlugin\WIN32\BASSMOD.dll"; DestDir: "{app}"; Components: "mod"
Source: "ModPlugin\WIN32\DebugUnicode\ModPlugin.dll"; DestDir: "{app}"; Components: "mod"
Source: "gmbank\gm.cfg"; DestDir: "{app}"; Components: "inst"
Source: "gmbank\gm\*.pat"; DestDir: "{app}\gm"; Components: "inst"
Source: "skins\*"; DestDir: "{app}\Skins"; Flags: recursesubdirs; Components: "skins"

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

[Components]
Name: "japanese"; Description: "GSPlayer (Japanese)"; Types: "full"
Name: "flac"; Description: "FLAC Plugin"; Types: "full"
Name: "midi"; Description: "MIDI Plugin"; Types: "full"
Name: "mod"; Description: "MOD Plugin"; Types: "full"
Name: "mp4"; Description: "MP4 Plugin"; Types: "full"
Name: "tta"; Description: "TTA Plugin"; Types: "full"
Name: "wma"; Description: "WMA Plugin"; Types: "full"
Name: "skins"; Description: "Skins"; Types: "full"
Name: "inst"; Description: "MIDI Instruments"; Types: "full"
