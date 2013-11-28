
CFLAGS+=-DUNICODE -D_UNICODE
CXXFLAGS+=-DUNICODE -D_UNICODE
# -static-libgcc -static-libstdc++
LDFLAGS+=-shared
LIBS+=-lshlwapi -lshell32 -luuid -lole32 -lgdi32

WINDRES:=$(CROSS_COMPILE)windres
WINDRES_CXX=$(CXX)
WINDRES_FLAGS=--preprocessor="$(WINDRES_CXX) -E -xc"

.PHONY: all clean

all: docloudext.dll

resource.o: doCloudExt.rc
	$(WINDRES) $(WINDRES_FLAGS) --input-format=rc --output-format=coff -i $? -o $@

docloudext.dll: dllmain.o reg.o shellext.o resource.o docloudext.def
	$(CXX) $(LDFLAGS) -o $@ $^ -shared -s -Wl,--subsystem,windows,--out-implib,$@.a $(LIBS)
clean:
	-rm *.o *.dll *.a
