
CFLAGS+=-DUNICODE -D_UNICODE
CXXFLAGS+=-DUNICODE -D_UNICODE
LDFLAGS+=-shared
LIBS+=-lshlwapi -lshell32 -luuid -lole32 -lgdi32

WINDRES:=$(CROSS_COMPILE)windres
WINDRES_CXX=$(CXX)
WINDRES_FLAGS=--preprocessor="$(WINDRES_CXX) -E -xc"


.PHONY: all clean

all: CppShellExtContextMenuHandler.dll

CppShellExtContextMenuHandler.o: CppShellExtContextMenuHandler.rc
	$(WINDRES) $(WINDRES_FLAGS) --input-format=rc --output-format=coff -i $? -o $@

CppShellExtContextMenuHandler.dll: dllmain.o reg.o FileContextMenuExt.o CppShellExtContextMenuHandler.o CppShellExtContextMenuHandler.def
#	$(CXX) $(LDFLAGS) -o $@ $^ -shared -s -Wl,--subsystem,windows,--out-implib,$@.a
	$(CXX) $(LDFLAGS) $(CXXFLAGS) -s -o $@ \
	   -Wl,--enable-auto-image-base \
	   -Wl,--enable-auto-import \
	   -Wl,--whole-archive \
	   $^ \
	   -Wl,--no-whole-archive \
	   $(LIBS)
	#
clean:
	-rm *.o *.dll *.a
