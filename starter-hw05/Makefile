
# tokens:
# 	echo "#!/bin/sh" > tokens
# 	echo "echo fix the makefile" >> tokens
# 	chmod a+x tokens

# clean:
# 	rm -rf tmp tokens *.o

# .PHONY: all clean

CFLAGS := -g
LDLIBS := -lbsd

all: tokens

tokens: tokens.o svec.o
	gcc $(CFLAGS) -o $@ $^ $(LDLIBS)

tokens.o: tokens.c svec.h
	gcc $(CFLAGS) -c -o $@ $<

svec.o: svec.c svec.h
	gcc $(CFLAGS) -c -o $@ $<

clean:
	rm -rf tmp *.plist valgrind.out *.o tokens

.PHONY: all clean