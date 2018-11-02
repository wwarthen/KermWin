This is the source tree for Kermit for Windows.

To build the application, start a Visual Studio 2017
command prompt by selecting "x86 Native Tools Command Prompt for VS 2017"
from the start menu after installing VS 2017.

Navigate to this directory and use the following
commands to build the application:

makekerm win32dbg
makekerm win32rel

The build process will use the Build directory as a work
area during the build and will place the final distributable
file in the Disk directory.

Note that the original Kermit for Windows help file is
not usable because Microsoft no longer supports the format.
This will be converted to a PDF file in the future.

Also note that the make files have a concept of building
a self extracting zip file for distribution.  This is
no longer working.  The distribution files can be put in a
zip file manually.  To install the application, jsut
run the Setup.exe application that is created in the
appropriate distribution directory.

You will notice that the source code has support for WIN16.
However, this is deprecated.