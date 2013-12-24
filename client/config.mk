.PHONY: settings

select-build:
	@echo "Usage:"
	@echo "make <target>, where <target> is one of:"
	@echo " 	win32 - compile for windows 32 on windows using mingw32"
	@echo " 	win64 - compile for windows 64 on windows using x86_x64-w64-mingw32"
	@echo " 	xc-win32 - cross-compile for win32 using mingw32"
	@echo " 	xc-win64 - cross-compile for win64 using x86_x64-w64-mingw32"
	@echo " 	xc-osx - cross-compile for osx using ..."
	@echo " 	linux - compile for linux on linux"
	@echo ""
	@echo " 	clean - clean tree"

win32: export TARGET=WIN32
win64: export TARGET=WIN64

settings:
	@echo ---------------------------------------------------
	@echo building for target $(TARGET) using the following:
	@echo CC = $(CC)
	@echo CXX = $(CXX)
	@echo ---------------------------------------------------

win32 win64 xc-win32 xc-win64: settings all

MFLAGS= --no-print-directory
%.o: %.cpp
	@echo "$(CXX) $<"
	@$(CXX) -c -o $@ $< $(CXXFLAGS)

%.d: %.cpp
	@echo DEPS $<
	@$(SHELL) -ec '$(CXX) $(CXXFLAGS) -MM $< | \
		sed -e '"'"'s|$*\.o: |\$*\.o $@: |g'"'"' >$@'

xc-win32: win32
xc-win32: export CFLAGS+="-DWINVER=0x0400 -D__MINGW32__ -D__WIN95__ -D__GNUWIN32__ -DSTRICT -DHAVE_W32API_H -D__WXMSW__ -D__WINDOWS__"
xc-win32: export LDFLAGS+="-DWINVER=0x0400 -D__MINGW32__ -D__WIN95__ -D__GNUWIN32__ -DSTRICT -DHAVE_W32API_H -D__WXMSW__ -D__WINDOWS__ -lgcc -ladvapi32"
xc-win32: export CC=i586-mingw32msvc-gcc
xc-win32: export CXX=i586-mingw32msvc-c++
xc-win32: export LD=i586-mingw32msvc-ld
xc-win32: export AR=i586-mingw32msvc-ar
xc-win32: export AS=i586-mingw32msvc-as
xc-win32: export NM=i586-mingw32msvc-nm
xc-win32: export STRIP=i586-mingw32msvc-strip
xc-win32: export RANLIB=i586-mingw32msvc-ranlib
xc-win32: export DLLTOOL=i586-mingw32msvc-dlltool
xc-win32: export OBJDUMP=i586-mingw32msvc-objdump
xc-win32: export RESCOMP=i586-mingw32msvc-windres

xc-win64: win64
xc-win64: export CFLAGS+="-DWINVER=0x0400 -D__MINGW32__ -D__WIN95__ -D__GNUWIN32__ -DSTRICT -DHAVE_W32API_H -D__WXMSW__ -D__WINDOWS__"
xc-win64: export LDFLAGS+="-DWINVER=0x0400 -D__MINGW32__ -D__WIN95__ -D__GNUWIN32__ -DSTRICT -DHAVE_W32API_H -D__WXMSW__ -D__WINDOWS__ -lgcc -ladvapi32"
xc-win64: export CC=x86_64-w64-mingw32-gcc
xc-win64: export CXX=x86_64-w64-mingw32-c++
xc-win64: export LD=x86_64-w64-mingw32-ld
xc-win64: export AR=x86_64-w64-mingw32-ar
xc-win64: export AS=x86_64-w64-mingw32-as
xc-win64: export NM=x86_64-w64-mingw32-nm
xc-win64: export STRIP=x86_64-w64-mingw32-strip
xc-win64: export RANLIB=x86_64-w64-mingw32-ranlib
xc-win64: export DLLTOOL=x86_64-w64-mingw32-dlltool
xc-win64: export OBJDUMP=x86_64-w64-mingw32-objdump
xc-win64: export RESCOMP=x86_64-w64-mingw32-windres
