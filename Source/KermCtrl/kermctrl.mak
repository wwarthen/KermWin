########################################################################
##                                                                    ##
##                    Kermit for Microsoft Windows                    ##
##                    ----------------------------                    ##
##                               KERMIT                               ##
##                                                                    ##
##  Make file for building Kermit for MS Windows.  Requires the       ##
##  MS C Compiler and Windows Development Kit.                        ##
##                                                                    ##
########################################################################

# MACRO DEFINITIONS ----------------------------------------------------

CompDir=KermCtrl
ModType=exe

!INCLUDE "$(ProjDir)\Source\ProjDefs.mak"

INCLUDE  = .;$(SrcDir);$(INCLUDE)

DOCFILES = 
DISTFILES = $(DistDir)\kermctrl.exe
MODLST1 = kermctrl.obj ikermoa_i.obj
CINC =    $(SrcDir)\kermctrl.h ikermoa.h

!IF !EXIST(ikermoa.h)
!ERROR Required file ikermoa.h is missing from build of componenet Kermit!
!ENDIF

# EVERYTHING -----------------------------------------------------------

all : kermctrl.exe $(DOCFILES) $(DISTFILES)

# LINK -----------------------------------------------------------------

!IF "$(Platform)" == "WIN32"

kermctrl.exe   : $*.res $(MODLST1)
    link @<<
-subsystem:windows
-machine:I386
-out:$*.exe
-version:0.85
-map:$*.map
$(LFLAGS)
$*.res
$(MODLST1)
$(NTLIBS) version.lib ole32.lib oleaut32.lib uuid.lib
<<
!IF "$(Browse)" == "yes"
	bscmake /o $*.bsc $(MODLST1:.obj=.sbr)
!ENDIF

!ENDIF

!IF "$(Platform)" == "Win16"

kermctrl.exe   : $*.res $(SrcDir)\$*.def $(MODLST1)
    link $(LFLAGS) @<<
$(MODLST1)
$*
$*
libw mlibcew commdlg ver ole2.lib compobj.lib ole2disp.lib typelib.lib
$(SrcDir)\$*
<<
    $(RC) $*.res
     
!ENDIF

# COMPILE RESOURCE FILE ------------------------------------------------

kermctrl.res : $(SrcDir)\$*.rc $(SrcDir)\kermctrl.h \
                $(SrcDir)\kermctrl.ico
        $(RC) $(RFLAGS) /r /fo $*.res $(SrcDir)\$*

# COMPILE C SOURCE -----------------------------------------------------

kermctrl.obj : $(SrcDir)\kermctrl.cpp $(CINC)

# STATIC FILES ---------------------------------------------------------

# $(DOCFILES) : $(SrcDir)\$$@
#     copy $(SrcDir)\$@

# COPY DISTRIBUTION FILES ----------------------------------------------

$(DISTFILES) : $(@F)
    copy $(@F) $@
