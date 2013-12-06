
# Note - our SRW-locks in dirwatcher.cpp requires Vista or later!,
# thus, we have our _WIN32_WINNT=0x0600
CFLAGS+=-DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0600
CXXFLAGS+=-DUNICODE -D_UNICODE -std=c++11 -g -D_WIN32_WINNT=0x0600
LDFLAGS+=-g
OBJS=main.o dirwatcher.o service.o

.PHONY: clean

all: docloud-svc.exe

docloud-svc.exe: $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
clean:
	-rm -f docloud-svc.exe $(OBJS)
