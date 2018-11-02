	       -------------------------------------------
	       Kermit for Windows Version 0.85 README File
	       -------------------------------------------

This document provides important information about installing and using
the associated release of Kermit for Windows.  Please read all entries in
this file or risk losing data and/or improper operation.

******************
PRERELEASE WARNING
******************

This is a PRERELEASE of version 0.85.  Many internal aspects of the
program have been changed and things may not be completely stable
at this point.  Please review the remainder of this file notes about
incomplete features, etc.

*********
Licensing
*********

Kermit for Windows is Freeware.  That means you may use and redistribute
this software freely.  The software must be distributed in its complete
and unaltered form.  This work IS copyrighted and may NOT be
sold commercially.  I am usually willing to grant permission to shareware
vendors to redistribute the application without royalties when the
fees are minimal, but such distribution requires permission from the author.

************
Distribution
************

I currently distribute Kermit for Windows via the following:

CompuServe:
    WINSHARE Forum
    PCCOM Forum

Internet:
    http://www.wwarthen.com

Kermit for Windows is distributed in two separate formats.  As a
16 bit application for Standard Windows 3.1 and as a 32 bit application for
Windows 95/NT.  You must download the appropriate version and then
follow the instructions below.

KW16V085.EXE       Kermit for Windows 3.1 (16 bit application)
KW32V085.EXE       Kermit for Windows 95/NT (32 bit application)

************
Installation
************

Kermit for Windows now supports a small, simple, and highly effective
setup program similar to just about all other Windows applications.

Just download and execute the appropriate distribution file.  The
setup program is automatically invoked.

Please refer to the program documentation in KERMIT.WRI for detailed
installation instructions (or for manual installation instructions).

Note that to take advantage of the new TCP/IP (Telnet) support
you must acquire and install a WinSock library separately.  Windows 95/NT
comes with one and Microsoft is now provideing one for Windows for
Workgroups that may be downloaded for free (ftp.microsoft.com).

Also, note that to take advantage of the new scripting support under
the 32bit version requires that the Microsoft VBScript engine be
installed separately.  This is most easily accomplished by simply
installing MS Internet Explorer 3.0 or greater.

*************
Documentation
*************

The primary documentation for this program is contained in the file
KERMIT.WRI.  This is a Windows Write formatted file.  To view or
print the documentation, start Windows Write from inside of Windows
and open the Kermit documentation file.  You may then scroll through
the documentation or print it by choosing Print from the File Menu.

******************
Session Save Files
******************

The format of the data (session) files for this release are compatible
with all versions since (and including) version 0.76.  If you are 
upgrading from a release earlier than 0.76, you must manually recreate 
each of your session save files for this release (I considered writing 
a conversion program, but decided this release had been held up long enough).

WARNING: I suspect the session files will be completely redone in the
next significant release.  This will mean that the current files will
be unusable by the next release (sorry). 

************
Enhancements
************

In addition to the neat installation program, version 0.85 sports
significantly improved DEC VT-102 Terminal Emulation.  Specifically,
support for double high/wide characters, 132 column mode, and
reverse video.  The venerable "vttest" program is now handled without
a hitch.

An initial stab at keyboard remapping is also included.  For each emulation
library (such as kermdec.trm), there is a corresponding key map file
(such as kermdec.key).  See the kermit.wri documentation for details.

Also, support for MS ActiveX scripting has been added.  Unfortunately,
ActiveX scripting does not appear to be possible under 16-bit Windows
at this time, so scripting is only available in the 32-bit version of
this program.

At this time only VBScript is supported (Java Script will be supported
shortly).  You must acquire and install the VBScript Engine from
Microsoft separately.  This is most easily done by simply installing
MS Internet Explorer Version 3.0 -- VBScript is included in this
distribution and it's completely free.  MS Internet Explorer is
available at www.microsoft.com.

Although VBScript is a fairly full featured implementation of the
BASIC language, I have only provided a minimal set of Kermit
extensions so far.  My intent was to provide only a rudimentary
login automation capability initially.  Programmatic access to
all of Kermit's features will be available in the next release.
See the Kermit.wri documentation file for details on creating
scripts and a short reference on the Kermit extensions.

*********
Thank You
*********

I wish to thank all of the current users of this program who have
provided both suggestions and good wishes.  I regret that my limited
time prevents me from enhancing this software in a more timely manner.

Comments and suggestions are always appreciated.

Wayne Warthen

email:   wwarthen@wwarthen.com
website: http://www.wwarthen.com