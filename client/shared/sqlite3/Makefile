CFLAGS=-L%rootdir%\lib\win64\debug -I %rootdir%\include\windows\crtl -I %rootdir%\include\windows\sdk -D "SQLITE_API=__declspec(dllexport)" -DWIN64 -DNDEBUG -D_WINDOWS -D_USRDLL -DNO_TCL -D_CRT_SECURE_NO_DEPRECATE -DTHREADSAFE=1 -DTEMP_STORE=1 \
       -DSQLITE_MAX_EXPR_DEPTH=0 \
       -DSQLITE_ENABLE_COLUMN_METADATA \
       -DSQLITE_4_BYTE_ALIGNED_MALLOC \
       -DSQLITE_ENABLE_FTS3 \
       -DSQLITE_ENABLE_FTS4

LDFLAGS= -shared -s -Wl,--subsystem,windows,--out-implib,$@.a
CC=gcc

sqlite3.dll: sqlite3.o shell.o
	$(CC) -o $@ $^ $(LDFLAGS)
