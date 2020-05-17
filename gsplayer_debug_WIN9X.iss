[Setup]
AppName=GSPlayer
AppVersion=2.29
;AppVerName=GSPlayer 2.29
AppPublisher=GreenSoftware
DefaultDirName={pf}\GSPlayer
DefaultGroupName=GSPlayer
AllowNoIcons=yes
LicenseFile=gpl.txt
OutputBaseFilename=GSPlayerDebug9XSetup
Compression=lzma
SolidCompression=yes

[Languages]
Name: en; MessagesFile: "compiler:Default.isl"
Name: ja; MessagesFile: "compiler:Languages\Japanese.isl"

[Files]
Source: "bin\eng\Win32\Debug\GSPlayer2.exe"; DestDir: "{app}"; DestName: "GSPlayer2e.exe"
Source: "bin\jpn\Win32\Debug\GSPlayer2.exe"; DestDir: "{app}"; Components: "japanese"
Source: "document.txt"; DestDir: "{app}"
Source: "document_jp.txt"; DestDir: "{app}"; Components: "japanese"
Source: "gpl.txt"; DestDir: "{app}"
Source: "gspflac\gspflac\Debug\gspflac.dll"; DestDir: "{app}"; Components: "flac"
Source: "gspmidi\gspmidi\Debug\gspmidi.dll"; DestDir: "{app}"; Components: "midi"
Source: "gstta\Debug\gstta.dll"; DestDir: "{app}"; Components: "tta"
Source: "ModPlugin\WIN32\BASSMOD.dll"; DestDir: "{app}"; Components: "mod"
Source: "ModPlugin\WIN32\Debug\ModPlugin.dll"; DestDir: "{app}"; Components: "mod"
Source: "ResetConfig\Debug\ResetConfig.exe"; DestDir: "{app}"; Components: "reset"
Source: "gmbank\gm.cfg"; DestDir: "{app}"; Components: "inst"
Source: "gmbank\gm\*.pat"; DestDir: "{app}\gm"; Components: "inst"
Source: "skins\*"; DestDir: "{app}\Skins"; Flags: recursesubdirs; Components: "skins"

[Registry]
Root: HKCU; Subkey: "Software\GreenSoftware\GSPlayer\Plug-ins\gspmidi"; ValueName: "ConfigFile"; ValueType: String; ValueData: "{app}\gm.cfg"; Components: "inst"

[Run]
Filename: "{app}\GSPlayer2e.exe"; Description: "{cm:LaunchProgram,GSPlayer}"; Flags: nowait postinstall skipifsilent
Filename: "{app}\document.txt"; Description: "View Documentation"; Flags: shellexec postinstall skipifsilent unchecked

[Icons]
Name: "{group}\GSPlayer"; Filename: "{app}\GSPlayer2e.exe"
Name: "{group}\GSPlayer (Japanese)"; Filename: "{app}\GSPlayer2.exe"; Components: "japanese"
Name: "{group}\Documentation"; Filename: "{app}\document.txt"
Name: "{group}\Documentation (Japanese)"; Filename: "{app}\document_jp.txt"; Components: "japanese"
Name: "{group}\Reset GSPlayer Configuration"; Filename: "{app}\ResetConfig.exe"; Components: "reset"
Name: "{group}\{cm:UninstallProgram,GSPlayer}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\GSPlayer"; Filename: "{app}\GSPlayer2e.exe"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Components]
Name: "japanese"; Description: "GSPlayer (Japanese)"; Types: "full"
Name: "flac"; Description: "FLAC Plugin"; Types: "full"
Name: "midi"; Description: "MIDI Plugin"; Types: "full"
Name: "mod"; Description: "MOD Plugin"; Types: "full"
Name: "tta"; Description: "TTA Plugin"; Types: "full"
Name: "reset"; Description: "Configuration Reset Tool"; Types: "full"
Name: "skins"; Description: "Skins"; Types: "full"
Name: "inst"; Description: "MIDI Instruments"; Types: "full"

