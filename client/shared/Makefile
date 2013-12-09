CFLAGS+=-DUNICODE -D_UNICODE
CXXFLAGS+=-DUNICODE -D_UNICODE

OBJECTS=reg.o

all: $(OBJECTS)
	-cd sqlite3 && make all

clean:
	-cd sqlite3 && make clean
	-rm $(OBJECTS)
