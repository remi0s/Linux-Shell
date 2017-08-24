GCC=gcc -Wall -std=gnu99

all:
	$(GCC) myshell.c -lm -o myshell

clean:
	rm -f *~ *.o *.bin myshell 

release:
	tar -cvf myshell.tar *.c *.h *m *sh Makefile
