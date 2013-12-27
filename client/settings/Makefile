
# Setup paths to all needed libraries for cross-compiling
xc-win32: export LDFLAGS+=-L../shared/libwin32/gtk3/lib
xc-win32: export CXXFLAGS+=-I../shared/libwin32/gtk3/include/gtk-3.0
xc-win32: export CXXFLAGS+=-I../shared/libwin32/gtk3/include/glib-2.0
xc-win32: export CXXFLAGS+=-I../shared/libwin32/gtk3/include/pango-1.0
xc-win32: export CXXFLAGS+=-I../shared/libwin32/gtk3/include/cairo
xc-win32: export CXXFLAGS+=-I../shared/libwin32/gtk3/include/gdk-pixbuf-2.0
xc-win32: export CXXFLAGS+=-I../shared/libwin32/gtk3/include/atk-1.0
xc-win32: export CXXFLAGS+=-I../shared/libwin32/gtk3/lib/glib-2.0/include
xc-win32: export LIBS+=-lgtk-3 -lgdk-3 -lglib-2.0 -lgobject-2.0

xc-win64: export LDFLAGS+=-L../shared/libwin64/gtk3/lib
xc-win64: export CXXFLAGS+=-I../shared/libwin64/gtk3/include/gtk-3.0
xc-win64: export CXXFLAGS+=-I../shared/libwin64/gtk3/include/glib-2.0
xc-win64: export CXXFLAGS+=-I../shared/libwin64/gtk3/include/pango-1.0
xc-win64: export CXXFLAGS+=-I../shared/libwin64/gtk3/include/cairo
xc-win64: export CXXFLAGS+=-I../shared/libwin64/gtk3/include/gdk-pixbuf-2.0
xc-win64: export CXXFLAGS+=-I../shared/libwin64/gtk3/include/atk-1.0
xc-win64: export CXXFLAGS+=-I../shared/libwin64/gtk3/lib/glib-2.0/include
xc-win64: export LIBS+=-lgtk-3 -lgdk-3 -lglib-2.0 -lgobject-2.0

-include ../config.mk

# Note - our SRW-locks in dirwatcher.cpp requires Vista or later!,
# thus, we have our _WIN32_WINNT=0x0600
CFLAGS+=-DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0600
CXXFLAGS+=-DUNICODE -D_UNICODE -g -D_WIN32_WINNT=0x0600 -I../shared -I../shared/sqlite3
LDFLAGS+=-static-libgcc -static-libstdc++
LIBS+=-lshlwapi -lshell32 -luuid -lole32 -lgdi32
SRCS=settings.cpp

.PHONY: clean all

all: docloud-settings.exe

OBJS=$(patsubst %.cpp, %.o, $(SRCS))
OBJS+=../shared/reg.o ../shared/common.o ../shared/config.o
DEPS=$(patsubst %.o, %.d, $(OBJS))

docloud-settings.exe: $(OBJS)
	@echo "$(CXX) $^ -o $@"
	@$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

dependencies: $(DEPS)

deps: dependencies

clean:
	-rm -f settings.exe $(patsubst %.cpp, %.o, $(SRCS)) $(DEPS)

-include $(DEPS)
