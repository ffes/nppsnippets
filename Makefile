#############################################################################
#                                                                           #
#             Makefile for building a MinGW NppSnippets.dll                 #
#                                                                           #
#  NOTE: remember to keep tabs in the file. Don't convert them to spaces!   #
#                                                                           #
#############################################################################

.SUFFIXES: .exe .res .a .o .gch .c .cpp .cc .cxx .m .rc .p .f .F .r .y .l .s .S .def .h

CC = gcc
CXX = g++
AR = ar
RANLIB = ranlib

#
CFLAGS = -c -O2 -DUNICODE
CXXFLAGS = $(CFLAGS) -Wall -Wunused -Wunused-parameter
LDFLAGS = -lstdc++ -lgdi32 -lkernel32 -luuid -lshlwapi -lwinmm -lcomdlg32 -lole32 -loleaut32 -lcomctl32 -luuid -lwininet -lwsock32

.c.o:
	$(CC) $(CFLAGS) -o $@ $<

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ $<

.rc.o:
	windres -o $@ -i $<

PROGRAM=NppSnippets

now: $(PROGRAM).dll

all: clean depend now

PROGRAM_SRCS_CPP = \
	Database.cpp \
	DlgAbout.cpp \
	DlgConsole.cpp \
	DlgEditSnippet.cpp \
	DlgEditLanguages.cpp \
	DlgEditLibrary.cpp \
	DlgImportLibrary.cpp \
	NppSnippets.cpp \
	Library.cpp \
	Snippets.cpp \
	Options.cpp \
	Version.cpp \
	WaitCursor.cpp

PROGRAM_SRCS_C = \
	sqlite3.c

PROGRAM_OBJS_CPP=$(PROGRAM_SRCS_CPP:.cpp=.o)
PROGRAM_OBJS_C=$(PROGRAM_SRCS_C:.c=.o)

PROGRAM_RC=$(PROGRAM)_res.rc
PROGRAM_OBJS_RC=$(PROGRAM_RC:.rc=.o)

$(PROGRAM).dll: $(PROGRAM_OBJS_CPP) $(PROGRAM_OBJS_C) $(PROGRAM_OBJS_RC)
	$(CXX) -shared -o $@ $(PROGRAM_OBJS_CPP) $(PROGRAM_OBJS_C) $(PROGRAM_OBJS_RC) $(LDFLAGS) -mthreads -Wl,--out-implib,$(PROGRAM).a

depend: $(PROGRAM_SRCS_CPP)
	$(CXX) $(WX_CFLAGS) -MM $^ > Makefile.deps

#$(PROGRAM).h.gch: $(PROGRAM).h Version.h
#	$(CXX) -c $(CFLAGS) $(WX_CFLAGS) $(PROGRAM).h -o $(PROGRAM).h.gch

clean:
	del $(PROGRAM_OBJS_CPP) $(PROGRAM_OBJS_C) $(PROGRAM_OBJS_RC) $(PROGRAM).h.gch $(PROGRAM).dll $(PROGRAM).a tags Makefile.deps
	type nul > Makefile.deps

### code dependencies ###

$(PROGRAM)_res.o: $(PROGRAM)_res.rc Version.h

include Makefile.deps
