CFLAGS+=-DUNICODE -D_UNICODE
CXXFLAGS+=-DUNICODE -D_UNICODE -Isqlite3

OBJS=reg.o sqlite.o

.PHONY: all clean

all: $(OBJS)
	@make $(MFLAGS) -C sqlite3

clean:
	@make $(MFLAGS) -C sqlite3 clean
	-rm -f $(OBJS)

-include ../config.mk
