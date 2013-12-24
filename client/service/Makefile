
# Note - our SRW-locks in dirwatcher.cpp requires Vista or later!,
# thus, we have our _WIN32_WINNT=0x0600
CFLAGS+=-DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0600
CXXFLAGS+=-DUNICODE -D_UNICODE -g -D_WIN32_WINNT=0x0600 -I../shared -I../shared/sqlite3
LDFLAGS+=-g -static-libgcc -static-libstdc++
LIBS+=-lshlwapi -L../shared/sqlite3 -lsqlite3 -lole32
OBJS=main.o dirwatcher.o service.o sqlitewatcher.o docloudfile.o ../shared/reg.o ../shared/sqlite.o


.PHONY: clean all

all: docloud-svc.exe

DEPS=$(patsubst %.o, %.d, $(OBJS))

docloud-svc.exe: $(OBJS)
	@echo "$(CXX) $^ -o $@"
	@$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

dependencies: $(DEPS)

deps: dependencies

clean:
	-rm -f docloud-svc.exe $(OBJS) $(OBJS:%.o=%.d)

-include $(OBJS:%.o=%.d)
-include ../config.mk
