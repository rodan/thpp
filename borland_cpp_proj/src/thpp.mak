# ---------------------------------------------------------------------------
VERSION = BCB.03
# ---------------------------------------------------------------------------
!ifndef BCB
BCB = $(MAKEDIR)\..
!endif
# ---------------------------------------------------------------------------
PROJECT = thpp.exe
OBJFILES = thpp.obj form.obj Unit2.obj Unit3.obj Unit4.obj
RESFILES = thpp.res
RESDEPEN = $(RESFILES) form.dfm Unit2.dfm Unit3.dfm Unit4.dfm
LIBFILES =
LIBRARIES = vcldbx35.lib vcldb35.lib vclx35.lib vcl35.lib
SPARELIBS = vcl35.lib vclx35.lib vcldb35.lib vcldbx35.lib
PACKAGES = VCLX35.bpi VCL35.bpi VCLDB35.bpi VCLDBX35.bpi bcbsmp35.bpi dclocx35.bpi \
  QRPT35.bpi TEEUI35.bpi TEEDB35.bpi TEE35.bpi ibsmp35.bpi NMFAST35.bpi \
  INETDB35.bpi INET35.bpi VCLMID35.bpi
PATHASM = .;
PATHCPP = .;
PATHPAS = .;
PATHRC = .;
DEBUGLIBPATH = $(BCB)\lib\debug
RELEASELIBPATH = $(BCB)\lib\release
DEFFILE =
# ---------------------------------------------------------------------------
CFLAG1 = -O2 -Hc -w -Ve -r- -k -y -v -vi- -c -b- -w-par -w-inl -Vx
CFLAG2 = -I$(BCB)\include;$(BCB)\include\vcl;fftw\rfftw;fftw\fftw -H=$(BCB)\lib\vcld.csm
CFLAG3 = -5
PFLAGS = -AWinTypes=Windows;WinProcs=Windows;DbiTypes=BDE;DbiProcs=BDE;DbiErrs=BDE \
  -U$(BCB)\lib\obj;$(BCB)\lib;$(DEBUGLIBPATH) \
  -I$(BCB)\include;$(BCB)\include\vcl;fftw\rfftw;fftw\fftw -$Y -$W -$O- -v -JPHNV \
  -M
RFLAGS = -i$(BCB)\include;$(BCB)\include\vcl;fftw\rfftw;fftw\fftw
AFLAGS = /i$(BCB)\include /i$(BCB)\include\vcl /ifftw\rfftw /ifftw\fftw /mx /w2 /zi
LFLAGS = -L$(BCB)\lib\obj;$(BCB)\lib;$(DEBUGLIBPATH) -aa -Tpe -x -v
IFLAGS =
LINKER = ilink32
# ---------------------------------------------------------------------------
ALLOBJ = c0w32.obj sysinit.obj $(OBJFILES)
ALLRES = $(RESFILES)
ALLLIB = $(LIBFILES) $(LIBRARIES) import32.lib cp32mt.lib
# ---------------------------------------------------------------------------
.autodepend

!ifdef IDEOPTIONS

[Version Info]
IncludeVerInfo=1
AutoIncBuild=1
MajorVer=0
MinorVer=7
Release=5
Build=469
Debug=0
PreRelease=1
Special=0
Private=0
DLL=0
Locale=1033
CodePage=1252

[Version Info Keys]
CompanyName=Tehnosistem SA
FileDescription=editor of infrared images
FileVersion=0.7.5.469
InternalName=ThPP
LegalCopyright=GNU GENERAL PUBLIC LICENSE
LegalTrademarks=
OriginalFilename=thpp.exe
ProductName=Thermal Processing Panel
ProductVersion=1.0.0.0
Comments=Petre Rodan <rodan@subdimension.com>

[Excluded Packages]
C:\Borland\CBuilder3\Bin\dcldss35.bpl=Borland Decision Cube Components

[HistoryLists\hlIncludePath]
Count=2
Item0=$(BCB)\include;$(BCB)\include\vcl;fftw\rfftw;fftw\fftw
Item1=$(BCB)\include;$(BCB)\include\vcl

[HistoryLists\hlLibraryPath]
Count=1
Item0=$(BCB)\lib\obj;$(BCB)\lib

[HistoryLists\hlUnitAliases]
Count=1
Item0=WinTypes=Windows;WinProcs=Windows;DbiTypes=BDE;DbiProcs=BDE;DbiErrs=BDE

[Debugging]
DebugSourceDirs=

[Parameters]
RunParams=
HostApplication=

!endif

$(PROJECT): $(OBJFILES) $(RESDEPEN) $(DEFFILE)
    $(BCB)\BIN\$(LINKER) @&&!
    $(LFLAGS) +
    $(ALLOBJ), +
    $(PROJECT),, +
    $(ALLLIB), +
    $(DEFFILE), +
    $(ALLRES) 
!

.pas.hpp:
    $(BCB)\BIN\dcc32 $(PFLAGS) { $** }

.pas.obj:
    $(BCB)\BIN\dcc32 $(PFLAGS) { $** }

.cpp.obj:
    $(BCB)\BIN\bcc32 $(CFLAG1) $(CFLAG2) -o$* $* 

.c.obj:
    $(BCB)\BIN\bcc32 $(CFLAG1) $(CFLAG2) -o$* $**

.rc.res:
    $(BCB)\BIN\brcc32 $(RFLAGS) $<
#-----------------------------------------------------------------------------
