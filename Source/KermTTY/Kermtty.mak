########################################################################
##                                                                    ##
##                    Kermit for Microsoft Windows                    ##
##                    ----------------------------                    ##
##                              KERMTTY                               ##
##                                                                    ##
##  Make file for building Kermit TTY  Terminal Emulation Module.     ##
##                                                                    ##
########################################################################

# MACRO DEFINITIONS ----------------------------------------------------

CompDir=KermTTY
ModType=dll

!INCLUDE "$(ProjDir)\Source\ProjDefs.mak"

INCLUDE  = $(SrcDir);$(INCLUDE)

MODLST = kermtty.obj
CINC   = $(SrcDir)\kermtty.h $(SrcDir)\kermtty.rch $(SrcDir)\kermemul.h \
         $(SrcDir)\ttyver.h
DISTFILES = $(DistDir)\kermtty.trm $(DistDir)\kermtty.key

all : kermtty.trm kermtty.key $(DISTFILES)

# LINK -----------------------------------------------------------------

!IF "$(Platform)" == "WIN32"

kermtty.trm   : $*.res $(SrcDir)\kermem32.def $(MODLST)
    link @<<
-dll
-map:$*.map
-out:$*.trm
-version:0.85
-subsystem:windows
-def:$(SrcDir)\kermem32.def
-machine:I386
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

kermtty.trm : $*.res $(SrcDir)\$*.def $(MODLST)
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

kermtty.res : $(SrcDir)\$*.rc $(SrcDir)\$*.rch $(SrcDir)\winstyle.h
        $(RC) $(RFLAGS) /r /fo $*.res $(SrcDir)\$*

# COMPILE C SOURCE -----------------------------------------------------
     
kermtty.obj : $(SrcDir)\$*.cpp $(CINC)

# STATIC FILES ---------------------------------------------------------

kermtty.key : $(SrcDir)\$$@
    copy $(SrcDir)\$@

# COPY DISTRIBUTION FILES ----------------------------------------------

$(DISTFILES) : $(@F)
    copy $(@F) $@
