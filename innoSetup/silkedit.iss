; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "SilkEdit"
#define MyAppExeName "silkedit.exe"
#define ReleaseDir "..\build\Release"
#define ResourcesDir "..\resources"
#define SilkResourcesDir "silk_resources"
#define MyAppVersion GetStringFileInfo("..\build\Release\silkedit.exe", PRODUCT_VERSION)
#define MyAppPublisher "SilkEdit"
#define MyAppURL "http://silkedit.io"
#define Arch "x64"

[Setup]
AppId=silkedit.io.{#MyAppName}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
OutputBaseFilename={#MyAppName} Setup {#Arch}
OutputDir={#ReleaseDir}
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64
ArchitecturesAllowed=x64
ShowLanguageDialog=no
WizardSmallImageFile={#ResourcesDir}\app_icons\app_icon_39x41.bmp
UninstallDisplayIcon={app}\{#MyAppExeName}

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"
Name: "ja"; MessagesFile: "compiler:languages\Japanese.isl"

[CustomMessages]
en.OpenWithSilkEdit=Open with {#MyAppName}
en.AddContextMenu=Add "Open with {#MyAppName}" &context menu
ja.OpenWithSilkEdit={#MyAppName}�ŊJ��
ja.AddContextMenu="{#MyAppName}�ŊJ��"�R���e�L�X�g���j���[��ǉ�����(&C)

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"
Name: "contextmenu"; Description: "{cm:AddContextMenu}"

[Files]
Source: "{#ReleaseDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs

[Icons]
Name: "{userprograms}\{#MyAppName} {#MyAppVersion}"; Filename: "{app}\{#MyAppExeName}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Registry]
Root: HKCR; Subkey: "*\shell\{cm:OpenWithSilkEdit}"; Flags: uninsdeletekey; Tasks: contextmenu
Root: HKCR; Subkey: "*\shell\{cm:OpenWithSilkEdit}\command"; ValueType: string; ValueData: "{app}\{#MyAppExeName} ""%1"""; Tasks: contextmenu

[InstallDelete]
Type: filesandordirs; Name: "{app}\{#SilkResourcesDir}"