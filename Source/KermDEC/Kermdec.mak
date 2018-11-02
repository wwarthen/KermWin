########################################################################
##                                                                    ##
##                    Kermit for Microsoft Windows                    ##
##                    ----------------------------                    ##
##                              KERMDEC                               ##
##                                                                    ##
##  Make file for building Kermit DEC  Terminal Emulation Module.     ##
##                                                                    ##
########################################################################

# MACRO DEFINITIONS ----------------------------------------------------

CompDir=KermDEC
ModType=dll

!INCLUDE "$(ProjDir)\Source\ProjDefs.mak"

INCLUDE  = $(SrcDir);$(INCLUDE)

MODLST = kermdec.obj
CINC   = $(SrcDir)\kermdec.h $(SrcDir)\kermdec.rch $(SrcDir)\kermemul.h \
         $(SrcDir)\decver.h
DISTFILES = $(DistDir)\kermdec.trm $(DistDir)\kermdec.key

all : kermdec.trm kermdec.key $(DISTFILES)

# LINK -----------------------------------------------------------------

!IF "$(Platform)" == "WIN32"

kermdec.trm   : $*.res $(SrcDir)\kermem32.def $(MODLST)
    link @<<
-dll
-map:$*.map
-out:$*.trm
-version:0.85
-subsystem:windows
-machine:I386
-def:$(SrcDir)\kermem32.def
$(LFLAGS)
$*.res
$(MODLST)
$(NTLIBS)
<<
!IF "$(Browse)" == "yes"
	bscmake /o $*.bsc $(MODLST:.obj=.sbr)
!ENDIF

!ENDIF

!IF "$(Platform)" == "Win16"

kermdec.trm : $*.res $(SrcDir)\$*.def $(MODLST)
    link $(LFLAGS) @<<
$(MODLST)
$*.trm
$*
libw sdllcew
$(SrcDir)\$*
<<
    $(RC) $*.res $*.trm

!ENDIF

# COMPILE RESOURCE FILE ------------------------------------------------

kermdec.res : $(SrcDir)\$*.rc $(SrcDir)\$*.rch $(SrcDir)\winstyle.h
        $(RC) $(RFLAGS) /r /fo $*.res $(SrcDir)\$*

# COMPILE C SOURCE -----------------------------------------------------
     
.obj : $(SrcDir)\$*.cpp $(CINC)

# STATIC FILES ---------------------------------------------------------

kermdec.key : $(SrcDir)\$$@
    copy $(SrcDir)\$@

# COPY DISTRIBUTION FILES ----------------------------------------------

$(DISTFILES) : $(@F)
    copy $(@F) $@
