#############################################################################
#                                                                           #
#           Makefile for building a MinGW-w64 NppSnippets.dll               #
#                                                                           #
#############################################################################

.SUFFIXES: .dll .o .c .cpp .rc .h

ARCH = i686-w64-mingw32
CC = $(ARCH)-gcc
CXX = $(ARCH)-g++
WINDRES = $(ARCH)-windres

# The main targets
PROGRAM = NppSnippets
TARGET = $(PROGRAM).dll

now: $(TARGET)
all: clean now

# The general compiler flags
CFLAGS = -DUNICODE -mtune=i686
CXXFLAGS = $(CFLAGS) -Wno-write-strings --std=c++11
LIBS = -static -lshlwapi -lgdi32 -lcomdlg32 -lcomctl32
LDFLAGS = -Wl,--out-implib,$(TARGET) -shared

# Default target is RELEASE
DEBUG ?= 0
ifeq ($(DEBUG), 1)
	# Add DEBUG define, debug info and specific optimizations
	CFLAGS += -D_DEBUG -g -O0
	CXXFLAGS += -D_DEBUG -g -O0
	# Add dependencies flags
	CFLAGS += -MMD -MP
	CXXFLAGS += -MMD -MP
	# Enable almost all warnings
	CXXFLAGS += -W -Wall
else
	# Set the optimizations
	OPT = -fexpensive-optimizations -Os -O2
	# Strip the dll
	LDOPT = -s
endif

# Silent/verbose commands. For verbose output of commands set V=1
V ?= 0
ifeq ($(V), 0)
	SILENT = @
	V_CC = @echo [CC] $@;
	V_CXX = @echo [CXX] $@;
	V_RES = @echo [WINDRES] $@;
endif

# All the build targets
.c.o:
	$(V_CC) $(CC) -c $(CFLAGS) $(OPT) -o $@ $<

.cpp.o:
	$(V_CXX) $(CXX) -c $(CXXFLAGS) $(OPT) -o $@ $<

.rc.o:
	$(V_RES) $(WINDRES) -o $@ -i $<

PROGRAM_SRCS_CPP = \
	DlgAbout.cpp \
	DlgConsole.cpp \
	DlgEditSnippet.cpp \
	DlgEditLanguages.cpp \
	DlgEditLibrary.cpp \
	DlgImportLibrary.cpp \
	NppOptions.cpp \
	NppSnippets.cpp \
	Language.cpp \
	Library.cpp \
	Snippets.cpp \
	SnippetsDB.cpp \
	SqliteDB.cpp \
	Options.cpp \
	Version.cpp \
	WaitCursor.cpp

PROGRAM_SRCS_C = \
	sqlite3.c

PROGRAM_OBJS_CPP=$(PROGRAM_SRCS_CPP:.cpp=.o)
PROGRAM_OBJS_C=$(PROGRAM_SRCS_C:.c=.o)

PROGRAM_RC=$(PROGRAM)_res.rc
PROGRAM_OBJS_RC=$(PROGRAM_RC:.rc=.o)

PROGRAM_DEP_CPP=$(PROGRAM_SRCS_CPP:.cpp=.d)
PROGRAM_DEP_C=$(PROGRAM_SRCS_C:.c=.d)

$(PROGRAM).dll: $(PROGRAM_OBJS_CPP) $(PROGRAM_OBJS_C) $(PROGRAM_OBJS_RC)
	$(V_CXX) $(CXX) -o $@ $(PROGRAM_OBJS_CPP) $(PROGRAM_OBJS_C) $(PROGRAM_OBJS_RC) $(LDFLAGS) $(LDOPT) $(LIBS)

clean:
	@echo Cleaning
	$(SILENT) rm -f $(PROGRAM_OBJS_CPP) $(PROGRAM_OBJS_C) $(PROGRAM_OBJS_RC)
	$(SILENT) rm -f $(PROGRAM_DEP_CPP) $(PROGRAM_DEP_C) $(PROGRAM).dll
	$(SILENT) rm -f tags tags.out tags.sqlite

# The dependencies
$(PROGRAM)_res.o: $(PROGRAM)_res.rc Version.h

-include $(PROGRAM_DEP_CPP)
-include $(PROGRAM_DEP_C)
