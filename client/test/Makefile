-include ../config.mk

# Note - our SRW-locks in dirwatcher.cpp requires Vista or later!,
# thus, we have our _WIN32_WINNT=0x0600
CFLAGS+=-DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0600
CXXFLAGS+=-DUNICODE -D_UNICODE -g -D_WIN32_WINNT=0x0600 -I../shared -I../shared/sqlite3
LDFLAGS+=-L../shared/sqlite3 -static-libgcc -static-libstdc++
LIBS+=-lshlwapi -L../shared/sqlite3 -lsqlite3 -lole32
OBJS=main.o dirwatcher.o service.o sqlitewatcher.o ../shared/reg.o ../shared/sqlite.o ../shared/docloudfile.o ../shared/common.o ../shared/config.o


.PHONY: clean all

all: test
	
test: test-config test-docloudfile
	@test-config
	@test-docloudfile

DEPS=$(patsubst %.o, %.d, $(OBJS))

test-config: test-config.o ../shared/config.o ../shared/common.o ../shared/reg.o
	@echo "$(CXX) $^ -o $@"
	@$(CXX) $(LDFLAGS) -o $@ $^ -lshlwapi -lole32

test-docloudfile: test-docloudfile.o ../shared/docloudfile.o ../shared/config.o ../shared/common.o ../shared/reg.o ../shared/sqlite.o
		@echo "$(CXX) $^ -o $@"
		@$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

dependencies: $(DEPS)

deps: dependencies

clean:
	-rm -f docloud-svc.exe $(OBJS) $(DEPS)

-include $(DEPS)
