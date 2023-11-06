# Microsoft Developer Studio Project File - Name="gdopt" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=gdopt - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "gdopt.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "gdopt.mak" CFG="gdopt - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "gdopt - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "gdopt - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "gdopt - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /G5 /MT /W3 /GX /Zd /Ox /Ot /Ow /Og /Oi /Op /Ob2 /Gf /I "./src/include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FAs /FD /GE -Qunroll4 -Qip /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib /nologo /subsystem:windows /profile /map /debug /debugtype:both /machine:I386 /OPT:ICF,4 /OPT:REF

!ELSEIF  "$(CFG)" == "gdopt - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "./src/include/" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ws2_32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "gdopt - Win32 Release"
# Name "gdopt - Win32 Debug"
# Begin Group "Base"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\SRC\base\bitarray.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\base\comm.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\base\command.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\base\gconsole.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\base\gdopt.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\base\init.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\base\kbd.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\base\mcore.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\base\messages.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\base\module.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\base\options.cpp
# End Source File
# Begin Source File

SOURCE=.\resource.rc
# End Source File
# Begin Source File

SOURCE=.\SRC\base\util.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\base\value.cpp
# End Source File
# End Group
# Begin Group "Header"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\SRC\INCLUDE\bitarray.h
# End Source File
# Begin Source File

SOURCE=.\SRC\INCLUDE\ckbd.h
# End Source File
# Begin Source File

SOURCE=.\SRC\INCLUDE\comm.h
# End Source File
# Begin Source File

SOURCE=.\SRC\INCLUDE\command.h
# End Source File
# Begin Source File

SOURCE=.\src\include\def.h
# End Source File
# Begin Source File

SOURCE=.\SRC\INCLUDE\dopt.h
# End Source File
# Begin Source File

SOURCE=.\src\include\gcon.h
# End Source File
# Begin Source File

SOURCE=.\SRC\INCLUDE\mcore.h
# End Source File
# Begin Source File

SOURCE=.\SRC\INCLUDE\message.h
# End Source File
# Begin Source File

SOURCE=.\SRC\INCLUDE\module.h
# End Source File
# Begin Source File

SOURCE=.\src\include\resstr.hpp
# End Source File
# Begin Source File

SOURCE=.\SRC\INCLUDE\util.h
# End Source File
# Begin Source File

SOURCE=.\SRC\INCLUDE\value.h
# End Source File
# End Group
# Begin Group "Dopt"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SRC\dopt\cpp_data.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\dopt\dop_data.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\dopt\p_func.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\dopt\pdc_func.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\dopt\print.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\dopt\rs232_dop.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\dopt\table.cpp
# End Source File
# End Group
# End Target
# End Project
