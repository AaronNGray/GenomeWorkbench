; $Id: gbench-sdk.nsi 14565 2007-05-18 12:32:01Z dicuccio $
;--------------------------------
;Include Modern UI
!include "MUI.nsh"

;--------------------------------
;Definitions
!define GBENCH_NAME	"Genome Workbench SDK"
!define GBENCH_PATH "\\snowman\win-coremake\App\Ncbi\gbench\current\"

!define VER_MAJOR		1
!define VER_MINOR		0
!define VER_REVISION	1
!define VER_BUILD		0

!define GBENCH_SDK  		"SDK"
!define GBENCH_BIN  		"bin"
!define GBENCH_COMPILERS  	"compilers"
!define GBENCH_DOC			"doc"
!define GBENCH_INCLUDE		"include"
!define GBENCH_SRC			"src"
!define GBENCH_SCRIPTS		"scripts"

!define GBENCH_DEBUG		"DebugDLL"
!define GBENCH_RELEASE		"ReleaseDLL"


!define CACHE_FOLDER	"$APPDATA\GenomeWorkbench\cache"

!define VER_FILE		"${VER_MAJOR}${VER_MINOR}${VER_REVISION}"
!define VER_DISPLAY		"${VER_MAJOR}.${VER_MINOR}.${VER_REVISION}"

;SetCompressor zlib

;--------------------------------
;General
  ;Name and file
  Name "${GBENCH_NAME}"
  OutFile "${GBENCH_NAME} Setup.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\${GBENCH_NAME}"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\${GBENCH_NAME}" ""

;--------------------------------
;Interface Settings
  !define MUI_ABORTWARNING
  !define MUI_COMPONENTSPAGE_SMALLDESC
  
;--------------------------------
;Pages
  !define MUI_WELCOMEPAGE_TITLE "Welcome to the ${GBENCH_NAME} Setup Wizard"
  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "license.txt"
  Page custom PageReinstall PageLeaveReinstall
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  
  ;!define MUI_FINISHPAGE_RUN "$INSTDIR\${GBENCH_BIN}\gbench.exe"
  !define MUI_FINISHPAGE_NOREBOOTSUPPORT
  !insertmacro MUI_PAGE_FINISH
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  
;--------------------------------
;Languages 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Reserve Files
  ;These files should be inserted before other files in the data block
  ReserveFile "gbench_install.ini"
  !insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

;--------------------------------
;Installer Sections
;
Section "${GBENCH_NAME} core files (required)" SecGbench
  SetDetailsPrint textonly
  DetailPrint "Installing SDK Core Files..."
  SetDetailsPrint listonly
  SectionIn 1 2 3 RO
  SetOutPath "$INSTDIR"  
  SetOverwrite on
  
  ; bin
  SetDetailsPrint textonly
  DetailPrint "Installing binary utilities..."
  SetDetailsPrint listonly
  SetOutPath $INSTDIR\${GBENCH_BIN}
  File /nonfatal /r ${GBENCH_PATH}\${GBENCH_SDK}\${GBENCH_BIN}\*.exe  

  ; compilers
  SetDetailsPrint textonly
  DetailPrint "Installing project files..."
  SetDetailsPrint listonly
  SetOutPath $INSTDIR\${GBENCH_COMPILERS}
  File /nonfatal /r /x *.pdb ${GBENCH_PATH}\${GBENCH_SDK}\${GBENCH_COMPILERS}\*.*

  ; scripts
  SetDetailsPrint textonly
  DetailPrint "Installing scripts..."
  SetDetailsPrint listonly  
  SetOutPath $INSTDIR\${GBENCH_SCRIPTS}  
  File /nonfatal /r ${GBENCH_PATH}\${GBENCH_SDK}\${GBENCH_SCRIPTS}\*.*

  ; source code
  SetDetailsPrint textonly
  DetailPrint "Installing source code..."
  SetDetailsPrint listonly  
  SetOutPath $INSTDIR\${GBENCH_INCLUDE}
  File /nonfatal /r ${GBENCH_PATH}\${GBENCH_SDK}\${GBENCH_INCLUDE}\*.*    
  SetOutPath $INSTDIR\${GBENCH_SRC}
  File /nonfatal /r ${GBENCH_PATH}\${GBENCH_SDK}\${GBENCH_SRC}\*.*  
  
  SetDetailsPrint both
SectionEnd

Section "SDK Documentation" SecDocumentation	
	SetDetailsPrint textonly
	DetailPrint "Installing SDK documentation..."
	SetDetailsPrint listonly	
	SetOutPath "$INSTDIR"  
	SetOverwrite on
	SetOutPath $INSTDIR\${GBENCH_DOC}
    File /nonfatal /r ${GBENCH_PATH}\${GBENCH_SDK}\${GBENCH_DOC}\*.*    
    SetDetailsPrint both
SectionEnd

SectionGroup /e "Pre-builded configurations" SecPlugins

Section "Debug" SecDebug	
	SetDetailsPrint textonly
	DetailPrint "Installing Debug build..."
	SetDetailsPrint listonly	
	SetOutPath "$INSTDIR"  
	SetOverwrite on
	SetOutPath $INSTDIR\${GBENCH_DEBUG}
    File /nonfatal /r /x *.exp /x *.pdb ${GBENCH_PATH}\${GBENCH_SDK}\${GBENCH_DEBUG}\*.*    
    SetDetailsPrint both
SectionEnd

Section "Release" SecRelease
	SetDetailsPrint textonly
	DetailPrint "Installing Release build..."
	SetDetailsPrint listonly	
	SetOutPath "$INSTDIR"  
	SetOverwrite on
	SetOutPath $INSTDIR\${GBENCH_RELEASE}
    File /nonfatal /r /x *.exp /x *.pdb ${GBENCH_PATH}\${GBENCH_SDK}\${GBENCH_RELEASE}\*.*    
    SetDetailsPrint both
SectionEnd
SectionGroupEnd

Section -post
	;SetDetailsPrint textonly
	;DetailPrint "Registering plugins..."
	;SetDetailsPrint listonly
	;Call DoCleanCache
	;Call DoPluginScan

	SetDetailsPrint textonly
	DetailPrint "Creating Registry Keys..."
	SetDetailsPrint listonly	
	SetOutPath $INSTDIR
	WriteRegStr HKCU "Software\${GBENCH_NAME}" "" $INSTDIR
	WriteRegDword HKCU "Software\${GBENCH_NAME}" "VersionMajor" "${VER_MAJOR}"
	WriteRegDword HKCU "Software\${GBENCH_NAME}" "VersionMinor" "${VER_MINOR}"
	WriteRegDword HKCU "Software\${GBENCH_NAME}" "VersionRevision" "${VER_REVISION}"
	WriteRegDword HKCU "Software\${GBENCH_NAME}" "VersionBuild" "${VER_BUILD}"
	WriteRegExpandStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${GBENCH_NAME}" "UninstallString" '"$INSTDIR\uninst-gbench-sdk.exe"'
	WriteRegExpandStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${GBENCH_NAME}" "InstallLocation" "$INSTDIR"
	WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${GBENCH_NAME}" "DisplayName" "${GBENCH_NAME}"
	WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${GBENCH_NAME}" "DisplayIcon" "$INSTDIR\${GBENCH_BIN}\gbench.exe,0"
	WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${GBENCH_NAME}" "DisplayVersion" "${VER_DISPLAY}"
	WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${GBENCH_NAME}" "VersionMajor" "${VER_MAJOR}"
	WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${GBENCH_NAME}" "VersionMinor" "${VER_MINOR}.${VER_REVISION}"
	WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${GBENCH_NAME}" "URLInfoAbout" "http://www.ncbi.nlm.nih.gov/"
	WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${GBENCH_NAME}" "NoModify" "1"
	WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${GBENCH_NAME}" "NoRepair" "1"

	;Create uninstaller
	SetDetailsPrint textonly
	DetailPrint "Creating Uninstaller..."
	SetDetailsPrint listonly
	WriteUninstaller "$INSTDIR\uninst-gbench-sdk.exe"    
	SetDetailsPrint both  
SectionEnd

;-------------------------------------------------------
; Shortcuts
!ifndef NO_STARTMENUSHORTCUTS
Section "Start Menu and Desktop Shortcuts" SecShortcuts
  SetDetailsPrint textonly
  DetailPrint "Installing Start Menu and Desktop Shortcuts..."
  SetDetailsPrint listonly
!else
Section "Desktop Shortcut" SecShortcuts
  SetDetailsPrint textonly
  DetailPrint "Installing Desktop Shortcut..."
  SetDetailsPrint listonly
!endif
  SectionIn 1 2 3
  SetOutPath $INSTDIR
!ifndef NO_STARTMENUSHORTCUTS
  CreateDirectory "$SMPROGRAMS\${GBENCH_NAME}"
  IfFileExists $INSTDIR\${GBENCH_DOC}\*.* "" +2
  CreateShortCut "$SMPROGRAMS\${GBENCH_NAME}\${GBENCH_NAME} Documentation.lnk" "$INSTDIR\${GBENCH_DOC}\index.html" ""   
	
  CreateShortCut "$SMPROGRAMS\${GBENCH_NAME}\Uninstall ${GBENCH_NAME}.lnk" "$INSTDIR\uninst-gbench-sdk.exe"
!endif
;  CreateShortCut "$DESKTOP\${GBENCH_NAME}.lnk" "$INSTDIR\${GBENCH_BIN}\gbench.exe"
  SetDetailsPrint both
SectionEnd



;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecGbench  ${LANG_ENGLISH} "${GBENCH_NAME} main application. This component is requered."
  LangString DESC_SecPlugins ${LANG_ENGLISH} "Optional plugins."
  LangString DESC_SecProteus ${LANG_ENGLISH} "Proteus."
  LangString DESC_SecRadar   ${LANG_ENGLISH} "Radar."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecGbench}  $(DESC_SecGbench)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecPlugins} $(DESC_SecPlugins)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecProteus} $(DESC_SecProteus)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecRadar}   $(DESC_SecRadar)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Functions
;
Function .onInit
	!insertmacro MUI_INSTALLOPTIONS_EXTRACT "gbench_install.ini"
FunctionEnd


Function DoPluginScan
	ExecWait '"$INSTDIR\${GBENCH_BIN}\gbench_plugin_scan.exe" -strict  "$INSTDIR\${GBENCH_PLUGINS}"'	
FunctionEnd

Function DoCleanCache
	IfFileExists ${CACHE_FOLDER}\*.* "" +2
	RMDir /r ${CACHE_FOLDER}  
FunctionEnd

Function PageReinstall
	ReadRegStr $R0 HKCU "Software\${GBENCH_NAME}" ""

	StrCmp $R0 "" 0 +2
	Abort

	;Detect version
	ReadRegDWORD $R0 HKCU "Software\${GBENCH_NAME}" "VersionMajor"
	IntCmp $R0 ${VER_MAJOR} minor_check new_version older_version
	minor_check:
	ReadRegDWORD $R0 HKCU "Software\${GBENCH_NAME}" "VersionMinor"
	IntCmp $R0 ${VER_MINOR} revision_check new_version older_version
	revision_check:
	ReadRegDWORD $R0 HKCU "Software\${GBENCH_NAME}" "VersionRevision"
	IntCmp $R0 ${VER_REVISION} build_check new_version older_version
	build_check:
	ReadRegDWORD $R0 HKCU "Software\${GBENCH_NAME}" "VersionBuild"
	IntCmp $R0 ${VER_BUILD} same_version new_version older_version

	new_version:
		!insertmacro MUI_INSTALLOPTIONS_WRITE "gbench_install.ini" "Field 1" "Text" "An older version of ${GBENCH_NAME} is installed on your system. It's recommended that you uninstall the current version before installing. Select the operation you want to perform and click Next to continue."
		!insertmacro MUI_INSTALLOPTIONS_WRITE "gbench_install.ini" "Field 2" "Text" "Uninstall before installing"
		!insertmacro MUI_INSTALLOPTIONS_WRITE "gbench_install.ini" "Field 3" "Text" "Do not uninstall ${GBENCH_NAME}"
		!insertmacro MUI_HEADER_TEXT "Already Installed" "Choose how you want to install ${GBENCH_NAME}."
		StrCpy $R0 "1"
		Goto reinst_start
		
	older_version:
		!insertmacro MUI_INSTALLOPTIONS_WRITE "gbench_install.ini" "Field 1" "Text" "A newer version of ${GBENCH_NAME} is already installed! It is not recommended that you install an older version. If you really want to install this older version, it's better to uninstall the current version first. Select the operation you want to perform and click Next to continue."
		!insertmacro MUI_INSTALLOPTIONS_WRITE "gbench_install.ini" "Field 2" "Text" "Uninstall before installing"
		!insertmacro MUI_INSTALLOPTIONS_WRITE "gbench_install.ini" "Field 3" "Text" "Do not uninstall ${GBENCH_NAME}"
		!insertmacro MUI_HEADER_TEXT "Already Installed" "Choose how you want to install ${GBENCH_NAME}."
		StrCpy $R0 "1"
		Goto reinst_start

	same_version:
		!insertmacro MUI_INSTALLOPTIONS_WRITE "gbench_install.ini" "Field 1" "Text" "${GBENCH_NAME} ${VER_DISPLAY} is already installed. Select the operation you want to perform and click Next to continue."
		!insertmacro MUI_INSTALLOPTIONS_WRITE "gbench_install.ini" "Field 2" "Text" "Add/Reinstall components"
		!insertmacro MUI_INSTALLOPTIONS_WRITE "gbench_install.ini" "Field 3" "Text" "Uninstall ${GBENCH_NAME}"
		!insertmacro MUI_HEADER_TEXT "Already Installed" "Choose the maintenance option to perform."
		StrCpy $R0 "2"
	
	reinst_start:
		!insertmacro MUI_INSTALLOPTIONS_DISPLAY "gbench_install.ini"	
FunctionEnd

Function PageLeaveReinstall
  !insertmacro MUI_INSTALLOPTIONS_READ $R1 "gbench_install.ini" "Field 2" "State"

  StrCmp $R0 "1" 0 +2
    StrCmp $R1 "1" reinst_uninstall reinst_done

  StrCmp $R0 "2" 0 +3
    StrCmp $R1 "1" reinst_done reinst_uninstall

  reinst_uninstall:
  ReadRegStr $R1 HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${GBENCH_NAME}" "UninstallString"

  ;Run uninstaller
  HideWindow
    ClearErrors
    ExecWait '$R1 _?=$INSTDIR'

    IfErrors no_remove_uninstaller
    IfFileExists "$INSTDIR\${GBENCH_BIN}\gbench.exe" no_remove_uninstaller

      Delete $R1
      RMDir $INSTDIR

    no_remove_uninstaller:

  StrCmp $R0 "2" 0 +2
    Quit
  
  BringToFront
  
  reinst_done:
FunctionEnd


;--------------------------------
;Uninstaller Section
;
Section "Uninstall"  
  RMDir /r "$SMPROGRAMS\${GBENCH_NAME}"
  ;Delete "$DESKTOP\${GBENCH_NAME}.lnk"
  
  SetDetailsPrint textonly
  DetailPrint "Uninstalling core files..."
  SetDetailsPrint listonly
  RMDir /r $INSTDIR\${GBENCH_BIN}
  
  SetDetailsPrint textonly
  DetailPrint "Uninstalling project files..."
  SetDetailsPrint listonly
  RMDir /r $INSTDIR\${GBENCH_COMPILERS}
  
  SetDetailsPrint textonly
  DetailPrint "Uninstalling scripts..."
  SetDetailsPrint listonly
  RMDir /r $INSTDIR\${GBENCH_SCRIPTS}
  
  SetDetailsPrint textonly
  DetailPrint "Uninstalling source code..."
  SetDetailsPrint listonly
  RMDir /r $INSTDIR\${GBENCH_INCLUDE}
  RMDir /r $INSTDIR\${GBENCH_SRC}
  
  SetDetailsPrint textonly
  DetailPrint "Uninstalling documentation..."
  SetDetailsPrint listonly
  IfFileExists $INSTDIR\${GBENCH_DOC}\*.* "" +2
  RMDir /r $INSTDIR\${GBENCH_DOC}    
  
  SetDetailsPrint textonly
  DetailPrint "Uninstalling configurations..."
  SetDetailsPrint listonly
  IfFileExists $INSTDIR\${GBENCH_DEBUG}\*.* "" +2
  RMDir /r $INSTDIR\${GBENCH_DEBUG}  
  
  IfFileExists $INSTDIR\${GBENCH_RELEASE}\*.* "" +2
  RMDir /r $INSTDIR\${GBENCH_RELEASE}  
      
  Delete "$INSTDIR\uninst-gbench-sdk.exe"
  RMDir "$INSTDIR" 
  
  SetDetailsPrint textonly
  DetailPrint "Removing registry keys..."
  SetDetailsPrint listonly  
  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${GBENCH_NAME}"
  DeleteRegKey HKCU "Software\${GBENCH_NAME}"

  SetDetailsPrint both
SectionEnd
