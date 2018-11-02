########################################################################
##                                                                    ##
##                    Kermit for Microsoft Windows                    ##
##                    ----------------------------                    ##
##                              KERMANSI                              ##
##                                                                    ##
##  Make file for building Kermit ANSI Terminal Emulation Module.     ##
##                                                                    ##
########################################################################

# MACRO DEFINITIONS ----------------------------------------------------

CompDir=KermANSI
ModType=dll

!INCLUDE "$(ProjDir)\Source\ProjDefs.mak"

INCLUDE  = $(SrcDir);$(INCLUDE)

MODLST = kermansi.obj
CINC   = $(SrcDir)\kermansi.h $(SrcDir)\kermansi.rch $(SrcDir)\kermemul.h \
         $(SrcDir)\ansiver.h
DISTFILES = $(DistDir)\kermansi.trm $(DistDir)\kermansi.key

all : kermansi.trm kermansi.key $(DISTFILES)

# LINK -----------------------------------------------------------------

!IF "$(Platform)" == "WIN32"

kermansi.trm   : $*.res $(SrcDir)\kermem32.def $(MODLST)
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

kermansi.trm : $*.res $(SrcDir)\$*.def $(MODLST)
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

kermansi.res : $(SrcDir)\$*.rc $(SrcDir)\$*.rch $(SrcDir)\winstyle.h
        $(RC) $(RFLAGS) /r /fo $*.res $(SrcDir)\$*

# COMPILE C SOURCE -----------------------------------------------------
     
kermansi.obj : $(SrcDir)\$*.cpp $(CINC)

# STATIC FILES ---------------------------------------------------------

kermansi.key : $(SrcDir)\$$@
    copy $(SrcDir)\$@

# COPY DISTRIBUTION FILES ----------------------------------------------

$(DISTFILES) : $(@F)
    copy $(@F) $@
