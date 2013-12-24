-include ../config.mk
CXXFLAGS+=-DUNICODE -D_UNICODE -I../shared -I../shared/sqlite3
LDFLAGS+=-shared -L../shared/sqlite3 -static-libgcc -static-libstdc++
LIBS+=-lshlwapi -lshell32 -luuid -lole32 -lgdi32 -lsqlite3
SRCS=dllmain.cpp shellext.cpp fileinfo_sqlite.cpp ../shared/reg.cpp ../shared/sqlite.cpp

RESCOMP?=$(CROSS_COMPILE)windres
WINDRES_CXX=$(CXX)
WINDRES_FLAGS=--preprocessor="$(WINDRES_CXX) -E -xc"

.PHONY: all clean

all: docloudext.dll

OBJS=$(patsubst %.cpp, %.o, $(SRCS))
OBJS+=resource.o
DEPS=$(patsubst %.cpp, %.d, $(SRCS))

resource.o: doCloudExt.rc
	@echo "$(RESCOMP) $<"
	@$(RESCOMP) $(WINDRES_FLAGS) --input-format=rc --output-format=coff -i $? -o $@

fileinfo.o: fileinfo_sqlite.cpp
	@echo "$(CXX) $<"
	@$(CXX) -o $@ -c $^ $(CXXFLAGS)

docloudext.dll: $(OBJS) ../shared/reg.o ../shared/sqlite.o ../shared/docloudfile.o
	@echo "$(CXX) $^ -o $@ $(LIBS) $(LDFLAGS)"
	@$(CXX) $(LDFLAGS) -o $@ $^ -shared -s -Wl,--subsystem,windows,--out-implib,$@.a $(LIBS)

dependencies: $(DEPS)

deps: dependencies

clean:
	-rm -f $(OBJS) $(DEPS) *.dll *.a

-include $(OBJS:%.o=%.d)
