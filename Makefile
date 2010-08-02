
UTIL := ../util

CFLAGS := -Wall -O3 -fomit-frame-pointer -I$(UTIL)

all: patcher.lib

patcher.lib: patch.o patch_handler.o
	ar cru $@ $^
	ranlib $@

clean:
	rm -f patcher.lib *.o

-include .patch.d
.%.d: %.c
	$(CC) $(CFLAGS) -o $@ -MM $<
