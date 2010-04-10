Name QSynth

# Defines
!define BINARY_FILES "C:\Projects\build\all"
!define REGKEY "SOFTWARE\$(^Name)"
!define VERSION 0.3.5
!define COMPANY QSynth
!define URL http://qsynth.sourceforge.net/

# MUI defines
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_FINISHPAGE_NOAUTOCLOSE
!define MUI_STARTMENUPAGE_REGISTRY_ROOT HKLM
!define MUI_STARTMENUPAGE_NODISABLE
!define MUI_STARTMENUPAGE_REGISTRY_KEY ${REGKEY}
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME StartMenuGroup
!define MUI_STARTMENUPAGE_DEFAULTFOLDER QSynth
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"
!define MUI_UNFINISHPAGE_NOAUTOCLOSE

# Included files
!include Sections.nsh
!include MUI.nsh
!include Library.nsh

# Variables
Var StartMenuGroup
Var LibInstall

# Installer pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE gpl.rtf
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_STARTMENU Application $StartMenuGroup
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

# Installer languages
!insertmacro MUI_LANGUAGE "Czech"
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "German"
!insertmacro MUI_LANGUAGE "Russian"
!insertmacro MUI_LANGUAGE "Spanish"

# Installer attributes
OutFile qsynth-${VERSION}-setup.exe
InstallDir $PROGRAMFILES\QSynth
CRCCheck on
XPStyle on
ShowInstDetails show
VIProductVersion 0.3.5.0
VIAddVersionKey ProductName QSynth
VIAddVersionKey ProductVersion "${VERSION}"
VIAddVersionKey CompanyName "${COMPANY}"
VIAddVersionKey CompanyWebsite "${URL}"
VIAddVersionKey FileVersion "${VERSION}"
VIAddVersionKey FileDescription ""
VIAddVersionKey LegalCopyright ""
InstallDirRegKey HKLM "${REGKEY}" Path
ShowUninstDetails show

# Installer sections
Section -Main SEC0000
    SetOutPath $INSTDIR
    SetOverwrite on
    File ${BINARY_FILES}\qsynth.exe
    File ${BINARY_FILES}\fluidsynth.exe
    SetOutPath $INSTDIR\share\locale
    File ${BINARY_FILES}\qsynth_cs.qm
    File ${BINARY_FILES}\qsynth_de.qm
    File ${BINARY_FILES}\qsynth_es.qm
    File ${BINARY_FILES}\qsynth_ru.qm

    # Installing library libfluidsynth.dll
    !insertmacro InstallLib DLL NOTSHARED REBOOT_PROTECTED ${BINARY_FILES}\libfluidsynth.dll $INSTDIR\libfluidsynth.dll $INSTDIR

    # Installing library mingwm10.dll
    !insertmacro InstallLib DLL NOTSHARED REBOOT_PROTECTED ${BINARY_FILES}\mingwm10.dll $INSTDIR\mingwm10.dll $INSTDIR

    # Installing library libgcc_s_dw2-1.dll
    !insertmacro InstallLib DLL NOTSHARED REBOOT_PROTECTED ${BINARY_FILES}\libgcc_s_dw2-1.dll $INSTDIR\libgcc_s_dw2-1.dll $INSTDIR

    # Installing library QtCore4.dll
    !insertmacro InstallLib DLL NOTSHARED REBOOT_PROTECTED ${BINARY_FILES}\QtCore4.dll $INSTDIR\QtCore4.dll $INSTDIR

    # Installing library QtGui4.dll
    !insertmacro InstallLib DLL NOTSHARED REBOOT_PROTECTED ${BINARY_FILES}\QtGui4.dll $INSTDIR\QtGui4.dll $INSTDIR

    # Installing library libglib.dll
    !insertmacro InstallLib DLL NOTSHARED REBOOT_PROTECTED ${BINARY_FILES}\libglib-2.0-0.dll $INSTDIR\libglib-2.0-0.dll $INSTDIR

    # Installing library libgthread.dll
    !insertmacro InstallLib DLL NOTSHARED REBOOT_PROTECTED ${BINARY_FILES}\libgthread-2.0-0.dll $INSTDIR\libgthread-2.0-0.dll $INSTDIR
    
    # Installing library intl.dll
    !insertmacro InstallLib DLL NOTSHARED REBOOT_PROTECTED ${BINARY_FILES}\intl.dll $INSTDIR\intl.dll $INSTDIR

    WriteRegStr HKLM "${REGKEY}\Components" Main 1
SectionEnd

Section -post SEC0001
    WriteRegStr HKLM "${REGKEY}" Path $INSTDIR
    SetOutPath $INSTDIR
    WriteUninstaller $INSTDIR\uninstall.exe
    !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    CreateDirectory $SMPROGRAMS\$StartMenuGroup
    CreateShortcut "$SMPROGRAMS\$StartMenuGroup\Uninstall $(^Name).lnk" $INSTDIR\uninstall.exe
    CreateShortcut "$SMPROGRAMS\$StartMenuGroup\fluidsynth.lnk" $INSTDIR\fluidsynth.exe
    CreateShortcut "$SMPROGRAMS\$StartMenuGroup\$(^Name).lnk" $INSTDIR\qsynth.exe
    !insertmacro MUI_STARTMENU_WRITE_END
    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" DisplayName "$(^Name)"
    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" DisplayVersion "${VERSION}"
    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" Publisher "${COMPANY}"
    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" URLInfoAbout "${URL}"
    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" DisplayIcon $INSTDIR\uninstall.exe
    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" UninstallString $INSTDIR\uninstall.exe
    WriteRegDWORD HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" NoModify 1
    WriteRegDWORD HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" NoRepair 1
SectionEnd

# Macro for selecting uninstaller sections
!macro SELECT_UNSECTION SECTION_NAME UNSECTION_ID
    Push $R0
    ReadRegStr $R0 HKLM "${REGKEY}\Components" "${SECTION_NAME}"
    StrCmp $R0 1 0 next${UNSECTION_ID}
    !insertmacro SelectSection "${UNSECTION_ID}"
    GoTo done${UNSECTION_ID}
next${UNSECTION_ID}:
    !insertmacro UnselectSection "${UNSECTION_ID}"
done${UNSECTION_ID}:
    Pop $R0
!macroend

# Uninstaller sections
Section /o -un.Main UNSEC0000
    Delete /REBOOTOK $INSTDIR\share\locale\qsynth_ru.qm
    Delete /REBOOTOK $INSTDIR\share\locale\qsynth_es.qm
    Delete /REBOOTOK $INSTDIR\share\locale\qsynth_de.qm
    Delete /REBOOTOK $INSTDIR\share\locale\qsynth_cs.qm
    Delete /REBOOTOK $INSTDIR\fluidsynth.exe
    Delete /REBOOTOK $INSTDIR\qsynth.exe

    # Uninstalling library $INSTDIR\libfluidsynth-1.dll
    !insertmacro UnInstallLib DLL NOTSHARED REBOOT_PROTECTED $INSTDIR\libfluidsynth.dll

    # Uninstalling library $INSTDIR\mingwm10.dll
    !insertmacro UnInstallLib DLL NOTSHARED REBOOT_PROTECTED $INSTDIR\mingwm10.dll

    # Uninstalling library $INSTDIR\libgcc_s_dw2-1.dll
    !insertmacro UnInstallLib DLL NOTSHARED REBOOT_PROTECTED $INSTDIR\libgcc_s_dw2-1.dll

    # Uninstalling library $INSTDIR\QtCore4.dll
    !insertmacro UnInstallLib DLL NOTSHARED REBOOT_PROTECTED $INSTDIR\QtCore4.dll

    # Uninstalling library $INSTDIR\QtGui4.dll
    !insertmacro UnInstallLib DLL NOTSHARED REBOOT_PROTECTED $INSTDIR\QtGui4.dll
    
    # Uninstalling library $INSTDIR\libglib.dll
    !insertmacro UnInstallLib DLL NOTSHARED REBOOT_PROTECTED $INSTDIR\libglib-2.0-0.dll 

    # Uninstalling library $INSTDIR\libgthread.dll
    !insertmacro UnInstallLib DLL NOTSHARED REBOOT_PROTECTED $INSTDIR\libgthread-2.0-0.dll 
    
    # Uninstalling library $INSTDIR\intl.dll
    !insertmacro UnInstallLib DLL NOTSHARED REBOOT_PROTECTED $INSTDIR\intl.dll 

    DeleteRegValue HKLM "${REGKEY}\Components" Main
SectionEnd

Section -un.post UNSEC0001
    DeleteRegKey HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)"
    Delete /REBOOTOK "$SMPROGRAMS\$StartMenuGroup\Uninstall $(^Name).lnk"
    Delete /REBOOTOK "$SMPROGRAMS\$StartMenuGroup\fluidsynth.lnk"
    Delete /REBOOTOK "$SMPROGRAMS\$StartMenuGroup\$(^Name).lnk"
    Delete /REBOOTOK $INSTDIR\uninstall.exe
    DeleteRegValue HKLM "${REGKEY}" StartMenuGroup
    DeleteRegValue HKLM "${REGKEY}" Path
    DeleteRegKey /IfEmpty HKLM "${REGKEY}\Components"
    DeleteRegKey /IfEmpty HKLM "${REGKEY}"
    RmDir /REBOOTOK $SMPROGRAMS\$StartMenuGroup
    RmDir /REBOOTOK $INSTDIR
SectionEnd

# Installer functions
Function .onInit
    Call GetDXVersion
    Pop $R3
    IntCmp $R3 900 +3 0 +3
        MessageBox "MB_OK" "Requires DirectX 9.0 or later."
        Abort
    InitPluginsDir
    Push $0
    ReadRegStr $0 HKLM "${REGKEY}" Path
    ClearErrors
    StrCmp $0 "" +2
    StrCpy $LibInstall 1
    Pop $0
FunctionEnd

# Uninstaller functions
Function un.onInit
    ReadRegStr $INSTDIR HKLM "${REGKEY}" Path
    !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuGroup
    !insertmacro SELECT_UNSECTION Main ${UNSEC0000}
FunctionEnd

Function GetDXVersion
    Push $0
    Push $1
 
    ReadRegStr $0 HKLM "Software\Microsoft\DirectX" "Version"
    IfErrors noDirectX
 
    StrCpy $1 $0 2 5    ; get the minor version
    StrCpy $0 $0 2 2    ; get the major version
    IntOp $0 $0 * 100   ; $0 = major * 100 + minor
    IntOp $0 $0 + $1
    Goto done
 
    noDirectX:
      StrCpy $0 0
 
    done:
      Pop $1
      Exch $0
FunctionEnd
