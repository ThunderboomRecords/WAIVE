#include "..\build\inno_build_info.txt"
[Setup]
AppName=WAIVE-Plugins
AppPublisher=Thunderboom Records
AppPublisherURL=https://www.thunderboomrecords.com/
DefaultDirName={commoncf}\VST3
OutputDir=..\release\v{#Version}\win64\
OutputBaseFilename=WAIVE-Plugins_windows_x64_v{#Version}
Compression=lzma
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
UninstallDisplayName=WAIVE-Plugins
UninstallFilesDir={localappdata}\WAIVE

[Files]
Source: "..\build\bin\WAIVE_Sampler.vst3\*"; DestDir: "{commoncf}\VST3\WAIVE_Sampler.vst3"; Flags: recursesubdirs
Source: "..\build\bin\WAIVE_Sequencer.vst3\*"; DestDir: "{commoncf}\VST3\WAIVE_Sequencer.vst3"; Flags: recursesubdirs

[Icons]
Name: "{group}\Uninstall WAIVE-Plugins"; Filename: "{uninstallexe}"

[UninstallDelete]
Type: filesandordirs; Name: "{commoncf}\VST3\WAIVE_Sampler.vst3"
Type: filesandordirs; Name: "{commoncf}\VST3\WAIVE_Sequencer.vst3"
Type: filesandordirs; Name: "{localappdata}\WAIVE"