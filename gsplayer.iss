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
MinVersion=4.1,5.0

[Languages]
Name: en; MessagesFile: "compiler:Default.isl"
Name: ba; MessagesFile: "compiler:Languages\Basque.isl"
Name: br; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"
Name: ca; MessagesFile: "compiler:Languages\Catalan.isl"
Name: cz; MessagesFile: "compiler:Languages\Czech.isl"
Name: da; MessagesFile: "compiler:Languages\Danish.isl"
Name: nl; MessagesFile: "compiler:Languages\Dutch.isl"
Name: fi; MessagesFile: "compiler:Languages\Finnish.isl"
Name: fr; MessagesFile: "compiler:Languages\French.isl"
Name: de; MessagesFile: "compiler:Languages\German.isl"
Name: he; MessagesFile: "compiler:Languages\Hebrew.isl"
Name: hu; MessagesFile: "compiler:Languages\Hungarian.isl"
Name: it; MessagesFile: "compiler:Languages\Italian.isl"
Name: ja; MessagesFile: "compiler:Languages\Japanese.isl"
Name: no; MessagesFile: "compiler:Languages\Norwegian.isl"
Name: pl; MessagesFile: "compiler:Languages\Polish.isl"
Name: pt; MessagesFile: "compiler:Languages\Portuguese.isl"
Name: ru; MessagesFile: "compiler:Languages\Russian.isl"
Name: se; MessagesFile: "compiler:Languages\SerbianLatin.isl"
Name: se2; MessagesFile: "compiler:Languages\SerbianCyrillic.isl"
Name: sl; MessagesFile: "compiler:Languages\Slovak.isl"
Name: sl2; MessagesFile: "compiler:Languages\Slovenian.isl"
Name: sp; MessagesFile: "compiler:Languages\Spanish.isl"
Name: uk; MessagesFile: "compiler:Languages\Ukrainian.isl"

[Files]
Source: "bin\eng\Win32\ReleaseUnicode\GSPlayer2.exe"; DestDir: "{app}"; DestName: "GSPlayer2e.exe"
Source: "bin\jpn\Win32\ReleaseUnicode\GSPlayer2.exe"; DestDir: "{app}"; Components: "japanese"
Source: "document.txt"; DestDir: "{app}"
Source: "document_jp.txt"; DestDir: "{app}"; Components: "japanese"
Source: "gpl.txt"; DestDir: "{app}"
Source: "gspflac\gspflac\ReleaseUnicode\gspflac.dll"; DestDir: "{app}"; Components: "flac"
Source: "gspmidi\gspmidi\ReleaseUnicode\gspmidi.dll"; DestDir: "{app}"; Components: "midi"
Source: "gspmp4\gspmp4\gspmp4\Release\gspmp4.dll"; DestDir: "{app}"; Components: "mp4"
Source: "gspwma\gspwma\Release\gspwma.dll"; DestDir: "{app}"; Components: "wma"
Source: "gstta\ReleaseUnicode\gstta.dll"; DestDir: "{app}"; Components: "tta"
Source: "ModPlugin\WIN32\BASSMOD.dll"; DestDir: "{app}"; Components: "mod"
Source: "ModPlugin\WIN32\ReleaseUnicode\ModPlugin.dll"; DestDir: "{app}"; Components: "mod"
Source: "ResetConfig\ReleaseUnicode\ResetConfig.exe"; DestDir: "{app}"; Components: "reset"
Source: "gmbank\gm.cfg"; DestDir: "{app}"; Components: "inst"
Source: "gmbank\gm\*.pat"; DestDir: "{app}\gm"; Components: "inst"
Source: "skins\*"; DestDir: "{app}\Skins"; Flags: recursesubdirs; Components: "skins"

[Registry]
Root: HKCU; Subkey: "Software\GreenSoftware\GSPlayer\Plug-ins\gspmidi"; ValueName: "ConfigFile"; ValueType: String; ValueData: "{app}\gm.cfg"; Components: "inst"

[Run]
Filename: "{app}\GSPlayer2e.exe"; Description: "{cm:LaunchProgram,GSPlayer}"; Flags: nowait postinstall skipifsilent
Filename: "{app}\document.txt"; Description: "View Documentation"; Flags: shellexec postinstall skipifsilent unchecked

[UninstallRun]
Filename: "{app}\ResetConfig.exe"; Parameters: "/u"; Components: "reset"

[UninstallDelete]
Type: files; Name: "{app}\GSPlayer2.m3u"
Type: files; Name: "{app}\GSPlayer2e.m3u"

[Icons]
Name: "{group}\GSPlayer"; Filename: "{app}\GSPlayer2e.exe"
Name: "{group}\GSPlayer (Japanese)"; Filename: "{app}\GSPlayer2.exe"; Components: "japanese"
Name: "{group}\Documentation"; Filename: "{app}\document.txt"
Name: "{group}\Documentation (Japanese)"; Filename: "{app}\document_jp.txt"; Components: "japanese"
Name: "{group}\License"; Filename: "{app}\gpl.txt"
Name: "{group}\Open Skins Directory"; Filename: "{app}\Skins"; Components: "skins"
Name: "{group}\Reset GSPlayer Configuration"; Filename: "{app}\ResetConfig.exe"; Components: "reset"
Name: "{group}\{cm:UninstallProgram,GSPlayer}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\GSPlayer"; Filename: "{app}\GSPlayer2e.exe"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\GSPlayer"; Filename: "{app}\GSPlayer2e.exe"; Tasks: quicklaunchicon

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 0,6.1

[Components]
Name: "japanese"; Description: "GSPlayer (Japanese)"; Types: "full"
Name: "flac"; Description: "FLAC Plugin"; Types: "full"
Name: "midi"; Description: "MIDI Plugin"; Types: "full"
Name: "mod"; Description: "MOD Plugin"; Types: "full"
Name: "mp4"; Description: "MP4 Plugin"; Types: "full"
Name: "tta"; Description: "TTA Plugin"; Types: "full"
Name: "wma"; Description: "WMA Plugin"; Types: "full"
Name: "reset"; Description: "Configuration Reset Tool"; Types: "full"
Name: "skins"; Description: "Skins"; Types: "full"
Name: "inst"; Description: "MIDI Instruments"; Types: "full"

