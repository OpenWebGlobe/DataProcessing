!include "EnvVarUpdate.nsh"

;defines
!define PRODUCT_NAME "OWG-Processing"
!define PRODUCT_VERSION "0.1.0"
!define PRODUCT_PUBLISHER "Martin Christen"
!define PRODUCT_WEB_SITE "http://www.openwebglobe.org"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

; MUI 1.67 compatible ------
!include "MUI.nsh"

; MUI Settings
!define MUI_ABORTWARNING
;!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install-blue-full.ico"
!define MUI_ICON "artwork\owg.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall-blue-full.ico"

!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\win.bmp" 

!define MUI_WELCOMEFINISHPAGE_BITMAP "artwork\installer.bmp"

; Welcome page
!insertmacro MUI_PAGE_WELCOME
; License page
!insertmacro MUI_PAGE_LICENSE "..\LICENSE"
; Directory page
!insertmacro MUI_PAGE_DIRECTORY
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
; Finish page
!insertmacro MUI_PAGE_FINISH

ShowUnInstDetails show

; Uninstaller pages
;!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "English"

; MUI end ------

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "Setup_DataProcessing.exe"
InstallDir "$PROGRAMFILES\OpenWebGlobeProcessing"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show

Section "Main" SEC01
  IfFileExists "$INSTDIR\uninst.exe" uninst continue
    uninst:
     ;MessageBox MB_YESNO|MB_ICONQUESTION "An older version was detected. Do you want to uninstall it first ?" IDNO continue
     ExecWait '"$INSTDIR\uninst.exe" _?=$INSTDIR'
     BringToFront
    continue:
  ${EnvVarUpdate} $0 "PATH" "A" "HKLM" "$INSTDIR" 
  ${EnvVarUpdate} $0 "PATH" "A" "HKCU" "$INSTDIR"
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  SetOverwrite try
  File "..\bin\resample.exe"
  File "..\bin\adddata.exe"
  File "..\bin\calcextent.exe"
  File "..\bin\createlayer.exe"
  File "..\bin\filelocktest.exe"
  SetOutPath "$INSTDIR\gdal-data"
  File "..\bin\gdal-data\compdcs.csv"
  File "..\bin\gdal-data\coordinate_axis.csv"
  File "..\bin\gdal-data\cubewerx_extra.wkt"
  File "..\bin\gdal-data\datum_shift.csv"
  File "..\bin\gdal-data\ecw_cs.wkt"
  File "..\bin\gdal-data\ellipsoid.csv"
  File "..\bin\gdal-data\epsg.wkt"
  File "..\bin\gdal-data\esri_extra.wkt"
  File "..\bin\gdal-data\gcs.csv"
  File "..\bin\gdal-data\gcs.override.csv"
  File "..\bin\gdal-data\gdalicon.png"
  File "..\bin\gdal-data\GDALLogoBW.svg"
  File "..\bin\gdal-data\GDALLogoColor.svg"
  File "..\bin\gdal-data\GDALLogoGS.svg"
  File "..\bin\gdal-data\gdal_datum.csv"
  File "..\bin\gdal-data\gt_datum.csv"
  File "..\bin\gdal-data\gt_ellips.csv"
  File "..\bin\gdal-data\header.dxf"
  File "..\bin\gdal-data\pci_datum.txt"
  File "..\bin\gdal-data\pci_ellips.txt"
  File "..\bin\gdal-data\pcs.csv"
  File "..\bin\gdal-data\pcs.override.csv"
  File "..\bin\gdal-data\prime_meridian.csv"
  File "..\bin\gdal-data\projop_wparm.csv"
  File "..\bin\gdal-data\s57agencies.csv"
  File "..\bin\gdal-data\s57attributes.csv"
  File "..\bin\gdal-data\s57attributes_aml.csv"
  File "..\bin\gdal-data\s57attributes_iw.csv"
  File "..\bin\gdal-data\s57expectedinput.csv"
  File "..\bin\gdal-data\s57objectclasses.csv"
  File "..\bin\gdal-data\s57objectclasses_aml.csv"
  File "..\bin\gdal-data\s57objectclasses_iw.csv"
  File "..\bin\gdal-data\seed_2d.dgn"
  File "..\bin\gdal-data\seed_3d.dgn"
  File "..\bin\gdal-data\stateplane.csv"
  File "..\bin\gdal-data\trailer.dxf"
  File "..\bin\gdal-data\unit_of_measure.csv"
  File "..\bin\gdal-data\vertcs.csv"
  File "..\bin\gdal-data\vertcs.override.csv"
  SetOutPath "$INSTDIR"
  File "..\bin\gdal18.dll"
  File "..\bin\OpenWebGlobeProcessing.dll"
  File "..\bin\proj.dll"
  File "..\bin\msvcp100.dll"
  File "..\bin\msvcr100.dll"
  File "..\bin\vcomp100.dll"
  File "..\bin\setup.xml"
  File "..\bin\test_adddata.bat"
  File "..\bin\test_calcextent_TestMosaik.bat"
  File "..\bin\test_createlayer_TestMosaik.bat"
  File "..\bin\test_filelock.bat"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd


;;;;;;;;;;UNINSTALL SECTION;;;;;;;;;;;;;;;;


Function un.onInit
  ;MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Do you really want to uninstall $(^Name) and all of its components ?" IDYES +2
  ;Abort
FunctionEnd

Function un.onUninstSuccess
  ;HideWindow
  ;MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) has been uninstalled."
FunctionEnd

Section "un.DataProcessing"  
  
  ${un.EnvVarUpdate} $0 "PATH" "R" "HKLM" "$INSTDIR" 
  ${un.EnvVarUpdate} $0 "PATH" "R" "HKCU" "$INSTDIR"

  Delete "$INSTDIR\uninst.exe"
  Delete "$INSTDIR\test_filelock.bat"
  Delete "$INSTDIR\test_createlayer_TestMosaik.bat"
  Delete "$INSTDIR\test_calcextent_TestMosaik.bat"
  Delete "$INSTDIR\test_adddata.bat"
  Delete "$INSTDIR\setup.xml"
  Delete "$INSTDIR\proj.dll"
  Delete "$INSTDIR\OpenWebGlobeProcessing.dll"
  Delete "$INSTDIR\msvcp100.dll"
  Delete "$INSTDIR\msvcr100.dll"
  Delete "$INSTDIR\vcomp100.dll"
  Delete "$INSTDIR\gdal18.dll"
  Delete "$INSTDIR\gdal-data\vertcs.override.csv"
  Delete "$INSTDIR\gdal-data\vertcs.csv"
  Delete "$INSTDIR\gdal-data\unit_of_measure.csv"
  Delete "$INSTDIR\gdal-data\trailer.dxf"
  Delete "$INSTDIR\gdal-data\stateplane.csv"
  Delete "$INSTDIR\gdal-data\seed_3d.dgn"
  Delete "$INSTDIR\gdal-data\seed_2d.dgn"
  Delete "$INSTDIR\gdal-data\s57objectclasses_iw.csv"
  Delete "$INSTDIR\gdal-data\s57objectclasses_aml.csv"
  Delete "$INSTDIR\gdal-data\s57objectclasses.csv"
  Delete "$INSTDIR\gdal-data\s57expectedinput.csv"
  Delete "$INSTDIR\gdal-data\s57attributes_iw.csv"
  Delete "$INSTDIR\gdal-data\s57attributes_aml.csv"
  Delete "$INSTDIR\gdal-data\s57attributes.csv"
  Delete "$INSTDIR\gdal-data\s57agencies.csv"
  Delete "$INSTDIR\gdal-data\projop_wparm.csv"
  Delete "$INSTDIR\gdal-data\prime_meridian.csv"
  Delete "$INSTDIR\gdal-data\pcs.override.csv"
  Delete "$INSTDIR\gdal-data\pcs.csv"
  Delete "$INSTDIR\gdal-data\pci_ellips.txt"
  Delete "$INSTDIR\gdal-data\pci_datum.txt"
  Delete "$INSTDIR\gdal-data\header.dxf"
  Delete "$INSTDIR\gdal-data\gt_ellips.csv"
  Delete "$INSTDIR\gdal-data\gt_datum.csv"
  Delete "$INSTDIR\gdal-data\gdal_datum.csv"
  Delete "$INSTDIR\gdal-data\GDALLogoGS.svg"
  Delete "$INSTDIR\gdal-data\GDALLogoColor.svg"
  Delete "$INSTDIR\gdal-data\GDALLogoBW.svg"
  Delete "$INSTDIR\gdal-data\gdalicon.png"
  Delete "$INSTDIR\gdal-data\gcs.override.csv"
  Delete "$INSTDIR\gdal-data\gcs.csv"
  Delete "$INSTDIR\gdal-data\esri_extra.wkt"
  Delete "$INSTDIR\gdal-data\epsg.wkt"
  Delete "$INSTDIR\gdal-data\ellipsoid.csv"
  Delete "$INSTDIR\gdal-data\ecw_cs.wkt"
  Delete "$INSTDIR\gdal-data\datum_shift.csv"
  Delete "$INSTDIR\gdal-data\cubewerx_extra.wkt"
  Delete "$INSTDIR\gdal-data\coordinate_axis.csv"
  Delete "$INSTDIR\gdal-data\compdcs.csv"
  Delete "$INSTDIR\filelocktest.exe"
  Delete "$INSTDIR\createlayer.exe"
  Delete "$INSTDIR\calcextent.exe"
  Delete "$INSTDIR\adddata.exe"
  Delete "$INSTDIR\resample.exe"

  RMDir "$INSTDIR\gdal-data"
  RMDir "$INSTDIR"

  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  SetAutoClose true
SectionEnd

