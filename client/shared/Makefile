CFLAGS+=-DUNICODE -D_UNICODE
CXXFLAGS+=-DUNICODE -D_UNICODE -Isqlite3

OBJECTS=reg.o sqlite.o docloudfile.o

all: $(OBJECTS)
	-cd sqlite3 && make all

clean:
	-cd sqlite3 && make clean
	-rm $(OBJECTS)
