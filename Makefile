CXX := g++

SRCDIR := src
BINDIR := bin
OBJDIR := obj
# Too lazy adding another directory to .gitignore, lol
DEPDIR := $(OBJDIR)

SRC := $(wildcard $(SRCDIR)/*.cpp)
HEADERS := $(wildcard $(SRCDIR)/*.h)
DEF := exports_mingw.def

DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d
CXXFLAGS := -std=c++17 -Wall -pedantic-errors -m32 -Iexternal/include -D_WIN32_WINNT=0x600
LDFLAGS := -Lexternal/lib -luuid -lole32 -lwinmm -lshlwapi -lSDL2 -lfreetype -static-libgcc -static-libstdc++ -shared -Wl,--subsystem,windows

BIN := $(BINDIR)/dinput.dll
OBJ := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRC))
DEP := $(patsubst $(SRCDIR)/%.cpp,$(DEPDIR)/%.d,$(SRC))

OBJ_SUBDIRS := $(patsubst %/,%,$(sort $(dir $(OBJ))))

# Need to compile with mingw32-x64-i686-gcc to not have errors when linking
ifneq ($(MSYSTEM_CARCH), i686)
$(error Please compile in the MSYS2 MinGW 32-bit environment!)
endif

all: release
release: OFLAGS += -O2 -s
debug: OFLAGS += -g

release debug: $(BIN)

clean:
	rm -r $(BINDIR) $(OBJDIR)

.SECONDEXPANSION:
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(DEPDIR)/%.d | $$(@D)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) $(OFLAGS) -c $< -o $@

$(BIN): $(OBJ) $(DEF) | $(BINDIR)
	$(CXX) $(CXXFLAGS) $(OFLAGS) $^ $(LDFLAGS) -o $(BIN)

$(BINDIR) $(OBJ_SUBDIRS):
	@mkdir -p $@

$(DEP):
include $(wildcard $(DEP))

.PHONY: all release debug clean
