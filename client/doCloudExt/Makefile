
CFLAGS+=-DUNICODE -D_UNICODE
CXXFLAGS+=-DUNICODE -D_UNICODE
# -static-libgcc -static-libstdc++
LDFLAGS+=-shared
LIBS+=-lshlwapi -lshell32 -luuid -lole32 -lgdi32 -L. -lsqlite3
OBJS=dllmain.o reg.o shellext.o resource.o fileinfo.o docloudext.def

WINDRES:=$(CROSS_COMPILE)windres
WINDRES_CXX=$(CXX)
WINDRES_FLAGS=--preprocessor="$(WINDRES_CXX) -E -xc"

.PHONY: all clean

all: docloudext.dll sqlite3.dll

resource.o: doCloudExt.rc
	$(WINDRES) $(WINDRES_FLAGS) --input-format=rc --output-format=coff -i $? -o $@

fileinfo.o: fileinfo_sqlite.cpp
	$(CXX) -o $@ -c $^ -I./sqlite3

sqlite3.dll:
	-cd sqlite3 && make && mv sqlite3.dll ..

docloudext.dll: $(OBJS) sqlite3.dll
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) -shared -s -Wl,--subsystem,windows,--out-implib,$@.a $(LIBS)
clean:
	-rm *.o *.dll *.a
