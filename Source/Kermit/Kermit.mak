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

CompDir=Kermit
ModType=exe

!INCLUDE "$(ProjDir)\Source\ProjDefs.mak"

INCLUDE  = .;$(SrcDir);$(INCLUDE)

!IF "$(Platform)" == "WIN32"
AXSCRIPT=1
!ENDIF

DOCFILES = readme.txt kermit.wri keymap.txt
#DISTFILES = $(DistDir)\kermit.exe $(DistDir)\kermit.hlp $(DistDir)\kermit.ini \
DISTFILES = $(DistDir)\kermit.exe $(DistDir)\kermit.ini \
            $(DistDir)\readme.txt $(DistDir)\kermit.wri \
            $(DistDir)\keymap.txt $(DistDir)\install.scf \
            $(DistDir)\remove.scf $(DistDir)\cleanup.scf
MODLST1 = kermit.obj kerminit.obj kermmsg.obj kermdlg.obj kermdev.obj \
          kermmisc.obj kermcomm.obj kermterm.obj kermstat.obj kermc3d.obj
MODLST2 = kermwnd.obj kermprot.obj kermsys.obj kermsess.obj kermtext.obj \
          kermtdef.obj kermtxfc.obj kermtapi.obj kermasy.obj kermtcp.obj
CINC =    $(SrcDir)\kermit.h $(SrcDir)\kermres.h

# !IF "$(Debug)"=="yes"

MODLST1 = $(MODLST1) kermdeb.obj
CINC    = $(CINC)    $(SrcDir)\kermdeb.h

# !ENDIF

# IF WIN32 ADD ACTIVEX STUFF --------------------------------------------

!IFDEF AXSCRIPT

CFLAGS   = $(CFLAGS) /DAXSCRIPT
RFLAGS   = $(RFLAGS) /DAXSCRIPT
MODLST1  = $(MODLST1) kermax.obj

!ENDIF

TLB		 = ikermoa.tlb
MODLST1  = $(MODLST1) kermole.obj dkermoa.obj kermoa.obj ikermoa_i.obj


# EVERYTHING -----------------------------------------------------------

#all : kermit.hlp kermit.exe install.scf remove.scf cleanup.scf kermit.ini $(DOCFILES) $(DISTFILES)
all : kermit.exe install.scf remove.scf cleanup.scf kermit.ini $(DOCFILES) $(DISTFILES)

# LINK -----------------------------------------------------------------

!IF "$(Platform)" == "WIN32"

kermit.exe   : $*.res $(MODLST1) $(MODLST2)
    link @<<
-subsystem:windows
-machine:I386
-out:$*.exe
-version:0.85
-map:$*.map
$(LFLAGS)
$*.res
$(MODLST1)
$(MODLST2)
$(NTLIBS) version.lib ole32.lib oleaut32.lib uuid.lib 
<<
!IF "$(Browse)" == "yes"
	bscmake /o $*.bsc $(MODLST1:.obj=.sbr) $(MODLST2:.obj=.sbr)
!ENDIF

!ENDIF

!IF "$(Platform)" == "Win16"

kermit.exe   : $*.res $(SrcDir)\$*.def $(MODLST1) $(MODLST2)
    link $(LFLAGS) @<<
$(MODLST1) +
$(MODLST2)
$*
$*
libw mlibcew commdlg ver ole2.lib compobj.lib ole2disp.lib typelib.lib 
$(SrcDir)\$*
<<
    $(RC) $*.res
     
!ENDIF

# COMPILE HELP FILE ----------------------------------------------------

!IF "$(Platform)" == "WIN32"
kermit.hlp   : $(SrcDir)\kermit32.hpj $(SrcDir)\kermx32.rtf $(SrcDir)\kermit.rtf \
                           $(SrcDir)\kermit.ico $(SrcDir)\.\kermit.bmp
    $(HC) -o $(MAKEDIR)\kermit.hlp -xn $(SrcDir)\kermit32.hpj
!ENDIF

!IF "$(Platform)" == "Win16"
kermit.hlp   : $(SrcDir)\kermit16.hpj $(SrcDir)\kermx16.rtf $(SrcDir)\kermit.rtf \
                           $(SrcDir)\kermit.ico $(SrcDir)\kermit.bmp
    $(HC) $(SrcDir)\kermit16.hpj
        if exist $*.hlp del $*.hlp
        ren $*16.hlp $*.hlp
!ENDIF
     
# SETUP DATA FILE ------------------------------------------------------

install.scf    : $(SrcDir)\instal$(Bits).scf
    copy $(SrcDir)\instal$(Bits).scf $@

remove.scf    : $(SrcDir)\remove$(Bits).scf
    copy $(SrcDir)\remove$(Bits).scf $@

cleanup.scf    : $(SrcDir)\clenup$(Bits).scf
    copy $(SrcDir)\clenup$(Bits).scf $@

# Initialization File --------------------------------------------------

Kermit.ini     : $(SrcDir)\Kermit.ini
    copy $(SrcDir)\Kermit.ini $@

# COMPILE RESOURCE FILE ------------------------------------------------

kermit.res   : $(SrcDir)\$*.rc $(SrcDir)\kermres.h \
                $(SrcDir)\kermit.ico $(TLB)
        $(RC) $(RFLAGS) /r /fo $*.res $(SrcDir)\$*

# COMPILE C SOURCE -----------------------------------------------------

kermit.obj   : $(SrcDir)\kermit.cpp $(SrcDir)\kermprot.h \
               $(SrcDir)\kermtloc.h $(SrcDir)\KermOA.h $(CINC)

kermstat.obj : $(SrcDir)\$*.cpp $(CINC)

kermdev.obj : $(SrcDir)\$*.cpp $(CINC)

kerminit.obj : $(SrcDir)\$*.cpp $(SrcDir)\KermOA.h $(CINC)

kermmsg.obj  : $(SrcDir)\$*.cpp $(CINC)

kermsess.obj : $(SrcDir)\$*.cpp $(CINC)

kermdlg.obj  : $(SrcDir)\$*.cpp $(CINC)

kermmisc.obj : $(SrcDir)\$*.cpp $(CINC)

kermcomm.obj : $(SrcDir)\$*.cpp $(CINC) $(SrcDir)\kermasy.h $(SrcDir)\kermtcp.h

kermasy.obj   : $(SrcDir)\$*.cpp $(CINC) $(SrcDir)\$*.h

kermtcp.obj   : $(SrcDir)\$*.cpp $(CINC) $(SrcDir)\$*.h

kermterm.obj : $(SrcDir)\$*.cpp $(CINC) $(SrcDir)\kermtapi.h \
               $(SrcDir)\kermtdef.h $(SrcDir)\kermtloc.h \
               $(SrcDir)\kermtxfc.h

kermdeb.obj  : $(SrcDir)\$*.cpp $(CINC)

kermwnd.obj  : $(SrcDir)\$*.cpp $(CINC) $(SrcDir)\kermprot.h

kermprot.obj : $(SrcDir)\$*.cpp $(CINC) $(SrcDir)\kermprot.h

kermsys.obj  : $(SrcDir)\$*.cpp $(CINC) $(SrcDir)\kermprot.h

kermtext.obj  : $(SrcDir)\$*.cpp $(CINC)

kermtdef.obj  : $(SrcDir)\$*.cpp $(CINC) $(SrcDir)\kermtdef.h \
                $(SrcDir)\kermtloc.h

kermtapi.obj  : $(SrcDir)\$*.cpp $(CINC) $(SrcDir)\kermtapi.h \
                $(SrcDir)\kermtloc.h

kermtxfc.obj  : $(SrcDir)\$*.cpp $(CINC) $(SrcDir)\kermtxfc.h \
                $(SrcDir)\kermtdef.h $(SrcDir)\kermtloc.h

kermpy.obj    : $(SrcDir)\$*.cpp $(CINC)

kermc3d.obj   : $(SrcDir)\$*.cpp $(CINC)

oleauto.obj   : $(SrcDir)\$*.cpp $(CINC) $(SrcDir)\oleauto.h

kermax.obj    : $(SrcDir)\$*.cpp $(SrcDir)\KermOA.h $(CINC) $(SrcDir)\KermOLE.h

dkermoa.obj   : $(SrcDir)\$*.cpp $(SrcDir)\DKermOA.h $(SrcDir)\KermOLE.h $(SrcDir)\KermOA.h IKermOA.h

kermoa.obj    : $(SrcDir)\$*.cpp $(SrcDir)\KermOLE.h $(SrcDir)\KermOA.h IKermOA.h

ikermoa.h     : $(SrcDir)\$*.odl
!IF "$(Platform)" == "WIN32"
     midl /nologo /D$(Platform) /h $*.h /tlb $*.tlb $(SrcDir)\$*.odl
!ENDIF
!IF "$(Platform)" == "Win16"
     cl /nologo /c /D__MKTYPLIB__ /P $(SrcDir)\$*.odl
     start /w MkTypLib /nologo /nocpp /o mktyplib.log /D$(Platform) /h $*.h /tlb $*.tlb $*.i
     type mktyplib.log
!ENDIF

ikermoa.tlb : ikermoa.h
     echo $@ created implicitly...

# STATIC FILES ---------------------------------------------------------

$(DOCFILES)  : $(SrcDir)\$$@
    copy $(SrcDir)\$@

# COPY DISTRIBUTION FILES ----------------------------------------------

$(DISTFILES) : $(@F)
    copy $(@F) $@
