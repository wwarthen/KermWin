!IFNDEF ProjDir
!ERROR Required ProjDir parameter omitted!!!
!ENDIF

!INCLUDE "$(ProjDir)\Source\ProjDefs.mak"

components=kermit.cmp kermtty.cmp kermansi.cmp \
           kermdec.cmp setup.cmp kermctrl.cmp
optional=web

all: nonpub

components : $(components)

$(components) $(optional) :
	@echo *** Building component $*...
	cd Build\$(PlatformDir)\$(BuildTypeDir)
        $(MAKE) /nologo /$(MAKEFLAGS) -f $(ProjDir)\Source\$*\$*.mak \
        ProjDir=$(ProjDir) $(Parms)
	cd ..\..\..

nonpub : $(components)

pub : $(components)
        cd Publish\Files\$(VerDir)
        if exist $(ArcFile).zip del $(ArcFile).zip
        $(WZ) -a $(ArcFile).zip $(DistDir)
        if exist $(ArcFile).exe del $(ArcFile).exe
        $(WZSX) $(ArcFile).zip @<<
-setup -3 $(WZFLAGS) -le 
-t $(ProjDir)\Source\WinZipSX\Dialog.txt
-i $(ProjDir)\Source\WinZipSX\Kermit.ico
-a $(ProjDir)\Source\WinZipSX\Dialog.txt
-st "Kermit for Windows Setup"
-c Setup.exe
<<
        cd ..\..\..
