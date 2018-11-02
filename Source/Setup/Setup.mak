########################################################################
##                                                                    ##
##                    Setup  for Microsoft Windows                    ##
##                    ----------------------------                    ##
##                               SETUP                                ##
##                                                                    ##
##  Make file for building Setup  for MS Windows.  Requires the       ##
##  MS C Compiler and Windows Development Kit.                        ##
##                                                                    ##
########################################################################

# MACRO DEFINITIONS ----------------------------------------------------

CompDir=Setup
ModType=exe
LibType=static

!INCLUDE "$(ProjDir)\Source\ProjDefs.mak"

INCLUDE  = $(SrcDir);$(INCLUDE)

MODLST1 = setup.obj kermc3d.obj
CINC =    $(SrcDir)\setup.h $(SrcDir)\setupres.h $(SrcDir)\setupver.h $(SrcDir)\..\Kermit\KermC3D.h
DISTFILES = $(DistDir)\setup.exe

# INFERENCE RULES ------------------------------------------------------

{$(SrcDir)}.cpp{}.obj:
    $(CPP) $(CFLAGS) /c $(SrcDir)\$*.cpp

{$(SrcDir)}.rc{}.res:
    $(RC) $(RFLAGS) /r -fo $*.res $(SrcDir)\$*

# EVERYTHING -----------------------------------------------------------

all : setup.exe $(DISTFILES)

# LINK -----------------------------------------------------------------

!IF "$(Platform)" == "WIN32"

setup.exe   : $*.res $(MODLST1)
    link @<<
-subsystem:windows
-machine:I386
-out:$*.exe
-version:0.85
-map:$*.map
$(LFLAGS)
$*.res
$(MODLST1)
$(NTLIBS) version.lib
<<
!IF "$(Browse)" == "yes"
	bscmake /o $*.bsc $(MODLST1:.obj=.sbr)
!ENDIF

!ENDIF

!IF "$(Platform)" == "Win16"

setup.exe   : $*.res $(SrcDir)\$*.def $(MODLST1)
    link $(LFLAGS) @<<
$(MODLST1)
$*
$*
libw mlibcew ver ddeml shell
$(SrcDir)\$*
<<
    $(RC) $*.res
     
!ENDIF
     
# COMPILE RESOURCE FILE ------------------------------------------------

setup.res   : $(SrcDir)\$*.rc $(SrcDir)\setupres.h $(SrcDir)\setup.ico

# COMPILE C SOURCE -----------------------------------------------------

setup.obj   : $(SrcDir)\setup.cpp $(CINC)

# COPY DISTRIBUTION FILES ----------------------------------------------

$(DISTFILES) : $(@F)
    copy $(@F) $@
