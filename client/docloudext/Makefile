-include ../config.mk
CXXFLAGS+=-DUNICODE -D_UNICODE -I../shared -I../shared/sqlite3
LDFLAGS+=-shared -L../shared/sqlite3 -static-libgcc -static-libstdc++
LIBS+=-lshlwapi -lshell32 -luuid -lole32 -lgdi32 -lsqlite3
SRCS=dllmain.cpp shellext.cpp

RESCOMP?=$(CROSS_COMPILE)windres
WINDRES_CXX=$(CXX)
WINDRES_FLAGS=--preprocessor="$(WINDRES_CXX) -E -xc"

.PHONY: all clean

all: docloudext.dll

OBJS=$(patsubst %.cpp, %.o, $(SRCS))
OBJS+=resource.o ../shared/reg.o ../shared/sqlite.o ../shared/common.o ../shared/config.o
DEPS=$(patsubst %.cpp, %.d, $(SRCS))

resource.o: doCloudExt.rc
	@echo "$(RESCOMP) $<"
	@$(RESCOMP) $(WINDRES_FLAGS) --input-format=rc --output-format=coff -i $? -o $@

fileinfo.o: fileinfo_sqlite.cpp
	@echo "$(CXX) $<"
	@$(CXX) -o $@ -c $^ $(CXXFLAGS)

docloudext.dll: $(OBJS) ../shared/reg.o ../shared/sqlite.o ../shared/docloudfile.o
	@echo "$(CXX) $^ -o $@ $(LIBS) $(LDFLAGS)"
	@$(CXX) $(LDFLAGS) -shared -o $@ $^ -Wl,--kill-at,--out-implib,$@.a $(LIBS)

dependencies: $(DEPS)

deps: dependencies

clean:
	-rm -f $(patsubst %.cpp, %.o, $(SRCS)) $(DEPS) resource.o *.dll *.a

-include $(OBJS:%.o=%.d)
