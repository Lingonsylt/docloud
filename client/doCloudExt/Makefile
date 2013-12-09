
CFLAGS+=-DUNICODE -D_UNICODE
CXXFLAGS+=-DUNICODE -D_UNICODE -I../shared -I../shared/sqlite3
# -static-libgcc -static-libstdc++
LDFLAGS+=-shared -L../shared/sqlite3
LIBS+=-lshlwapi -lshell32 -luuid -lole32 -lgdi32 -lsqlite3
OBJS=dllmain.o shellext.o resource.o fileinfo.o docloudext.def ../shared/reg.o

WINDRES:=$(CROSS_COMPILE)windres
WINDRES_CXX=$(CXX)
WINDRES_FLAGS=--preprocessor="$(WINDRES_CXX) -E -xc"

.PHONY: all clean

all: docloudext.dll

resource.o: doCloudExt.rc
	$(WINDRES) $(WINDRES_FLAGS) --input-format=rc --output-format=coff -i $? -o $@

fileinfo.o: fileinfo_sqlite.cpp
	$(CXX) -o $@ -c $^ $(CXXFLAGS)

docloudext.dll: $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) -shared -s -Wl,--subsystem,windows,--out-implib,$@.a $(LIBS)
clean:
	-rm *.o *.dll *.a
