##################################################
# This file is for inclusion in all the          #
# individual makefiles that make up the project. #
# It relies upon the variable ProjDir to be set  #
# beforehand (either via the including makefile  #
# or by the nmake command line).                 #
# We abort with an error if this is not true.    #
##################################################

!IFNDEF ProjDir
!ERROR Required parameter ProjDir omitted!!!
!ENDIF

##################################################
# The following attempts to grab known parms     #
# from the command line and save them in a PARMS #
# macro.  This just makes it easier for a        #
# makefile to call another makefile (since nmake #
# doesn't provide a nice built-in macro that     #
# does this).                                    #
##################################################
##################################################
# It looks like this may only apply to 16 nmake. #
# The current 4.0 nmake 32-bit seems to now      #
# inherit a parents command line options!        #
##################################################

!IFDEF ProjDir
Parms=$(Parms) ProjDir=$(ProjDir)
!ENDIF

!IFDEF Debug
Parms=$(Parms) Debug=$(Debug)
!ENDIF

!IFDEF Platform
Parms=$(Parms) Platform=$(Platform)
!ENDIF

!IFDEF Browse
Parms=$(Parms) Browse=$(Browse)
!ENDIF

##################################################
# Apply default build settings if none have been #
# established by the parent makefile.  It is     #
# intended that these will be specified when the #
# parent makefile is invoked.                    #
##################################################

!IFNDEF Platform
Platform = WIN32
!ENDIF

!IFNDEF Debug
Debug = yes
!ENDIF

!IFNDEF ModType
ModType = exe
!ENDIF

!IFNDEF LibType
LibType = static
!ENDIF

!IFNDEF AppVer
AppVer = 085
!ENDIF

##################################################
# Establish build environment (directories)      #
# All these MUST be relative to the directory    #
# that nmake is INVOKED from!!!                  #
##################################################

!IF "$(Platform)" == "WIN32"
PlatformDir=WIN32
Bits=32
!ELSEIF "$(Platform)" == "Win16"
PlatformDir=Win16
Bits=16
!ELSEIF "$(Platform)" == "Web"
PlatformDir=Web
!ELSE
!ERROR Unknown Build Platform specified: $(Platform)
!ENDIF

VerDir=v$(AppVer)
!IF "$(Debug)" == "yes"
VerDir=$(VerDir)Dbg
!ENDIF
           
!IF "$(Platform)" == "Web"
BuildTypeDir=.
!ELSEIF "$(Debug)" == "yes"
BuildTypeDir=Debug
!ELSE
BuildTypeDir=Release
!ENDIF

!IFNDEF CompDir
CompDir=.
!ENDIF

SrcDir   = $(ProjDir)\Source\$(CompDir)
BuildDir = $(ProjDir)\Build\$(PlatformDir)\$(BuildTypeDir)
DistDir  = $(ProjDir)\Dist\$(PlatformDir)\$(BuildTypeDir)
PubDir   = $(ProjDir)\Publish

##################################################
# Control macro defaults                         #   
##################################################

!IFDEF Platform
!MESSAGE Platform: $(Platform), Debug: $(Debug), ModType: $(ModType), LibType: $(LibType).
!ENDIF

##################################################
# WinZip build definitions                       #
##################################################

ArcFile=kw$(Bits)v$(AppVer)

WZ = "c:\Program Files\WinZip\WinZip32"
WZSX = "c:\Program Files\WinZip Self-Extractor\WinZipSE"

!IF "$(Platform)" == "WIN32"
WZFLAGS = -$(Platform)
!ELSE
WZ = start /w $(WZ)
WZSX = start /w $(WZSX)
WZFLAGS =
!ENDIF

##################################################
# WIN32 build definitions                        #
##################################################

!IF "$(Platform)" == "WIN32"
HC = hcrtf

CFLAGS = /W4 /WX /nologo
AFLAGS = /W2 /WX /nologo
RFLAGS = /DWIN32
LFLAGS = /nologo /INCREMENTAL:NO

!IF "$(LibType)" == "dynamic"
ModFlags = /MD
!ELSE
ModFlags = /MT
!ENDIF

!  IF "$(Debug)" == "yes"
ModFlags=$(ModFlags)d
!  ENDIF

CFLAGS = $(CFLAGS) $(ModFlags)

! IF "$(Debug)" == "yes"
CFLAGS = $(CFLAGS) /DDEBUG /Od /Zi /Fd$(CompDir)
AFLAGS = $(AFLAGS) /Zi /DDEBUG
RFLAGS = $(RFLAGS) /DDEBUG
LFLAGS = $(LFLAGS) -debug
!  ELSE
CFLAGS = $(CFLAGS) /O1
!  ENDIF

! IF "$(Browse)" == "yes"
CFLAGS = $(CFLAGS) /FR
!  ENDIF

NTLIBS = kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib

!ENDIF

##################################################
# Win16 build definitions                        #
##################################################

!IF "$(Platform)" == "Win16"
HC = hc31

CFLAGS = /W4 /Zp /nologo
AFLAGS = /W2 /WX /nologo
RFLAGS =
LFLAGS = /AL:16 /noD /noFARCALL /ONERROR:noEXE /nologo

!  IF "$(ModType)" == "dll"
ModFlags = /ASw /GD2 
!  ELSE
ModFlags = /AM /GA2
!  ENDIF

CFLAGS = $(CFLAGS) $(ModFlags)

! IF "$(Debug)" == "yes"
CFLAGS = $(CFLAGS) /DDEBUG /Od /Zi
AFLAGS = $(AFLAGS) /Zi /DDEBUG
RFLAGS = $(RFLAGS) /DDEBUG
LFLAGS = $(LFLAGS) /CO
!  ELSE
CFLAGS = $(CFLAGS) /O1
!  ENDIF

!ENDIF

##################################################
# Standard Inference Rules                       #
##################################################

{$(SrcDir)}.c{}.obj:
    $(CC) $(CFLAGS) /c $(SrcDir)\$*.c

{$(SrcDir)}.cpp{}.obj:
    $(CPP) $(CFLAGS) /c $(SrcDir)\$*.cpp

{$(SrcDir)}.rc{}.res:
    $(RC) $(RFLAGS) /r $(SrcDir)\$*
