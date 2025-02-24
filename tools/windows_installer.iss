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

[Types]
Name: "full"; Description: "Full installation (VST, VST3, CLAP)"
Name: "custom"; Description: "Custom installation"; Flags: iscustom

[Components]
Name: "vst"; Description: "Install VST Plugin"; Types: full custom; Flags: checkablealone
Name: "vst3"; Description: "Install VST3 Plugin"; Types: full custom; Flags: checkablealone
Name: "clap"; Description: "Install CLAP Plugin"; Types: full custom; Flags: checkablealone

[Files]
Source: "..\build\bin\WAIVE_Sampler-vst2.dll"; DestDir: "{commoncf}\VST2\"; Flags: recursesubdirs; Components: vst
Source: "..\build\bin\WAIVE_Sequencer-vst2.dll"; DestDir: "{commoncf}\VST2\"; Flags: recursesubdirs; Components: vst

Source: "..\build\bin\WAIVE_Sampler.vst3\*"; DestDir: "{commoncf}\VST3\WAIVE_Sampler.vst3"; Flags: recursesubdirs; Components: vst3
Source: "..\build\bin\WAIVE_Sequencer.vst3\*"; DestDir: "{commoncf}\VST3\WAIVE_Sequencer.vst3"; Flags: recursesubdirs; Components: vst3

Source: "..\build\bin\WAIVE_Sampler.clap"; DestDir: "{commoncf}\CLAP\"; Components: clap
Source: "..\build\bin\WAIVE_Sequencer.clap"; DestDir: "{commoncf}\CLAP\"; Components: clap

[Icons]
Name: "{group}\Uninstall WAIVE-Plugins"; Filename: "{uninstallexe}"

[UninstallDelete]
Type: filesandordirs; Name: "{commoncf}\VST3\WAIVE_Sampler.vst3"
Type: filesandordirs; Name: "{commoncf}\VST3\WAIVE_Sequencer.vst3"
Type: filesandordirs; Name: "{localappdata}\WAIVE"