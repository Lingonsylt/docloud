#!/bin/sh

#Cross-compiler script for building dimp on linux for windows
export CFLAGS="-DWINVER=0x0400 -D__MINGW32__ -D__WIN95__ -D__GNUWIN32__ -DSTRICT -DHAVE_W32API_H -D__WXMSW__ -D__WINDOWS__"
export LDFLAGS="-DWINVER=0x0400 -D__MINGW32__ -D__WIN95__ -D__GNUWIN32__ -DSTRICT -DHAVE_W32API_H -D__WXMSW__ -D__WINDOWS__ -lgcc -ladvapi32"
export CC=x86_64-w64-mingw32-gcc
export CXX=x86_64-w64-mingw32-c++
export LD=x86_64-w64-mingw32-ld
export AR=x86_64-w64-mingw32-ar
export AS=x86_64-w64-mingw32-as
export NM=x86_64-w64-mingw32-nm
export STRIP=x86_64-w64-mingw32-strip
export RANLIB=x86_64-w64-mingw32-ranlib
export DLLTOOL=x86_64-w64-mingw32-dlltool
export OBJDUMP=x86_64-w64-mingw32-objdump
export RESCOMP=x86_64-w64-mingw32-windres

make $*
