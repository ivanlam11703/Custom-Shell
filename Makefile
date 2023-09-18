sleep_seconds=3
ecode=0

CC = gcc
CFLAGS = -m32 -Os

all: myshell sne
	
myshell: main.o parser.o
	$(CC) $(CFLAGS) -static -ggdb parser.o main.o -o $@

main.o: main.c parser.h
	$(CC) $(CFLAGS) -ggdb -c main.c

parser.o: parser.c parser.h
	$(CC) $(CFLAGS) -ggdb -c parser.c

sne: sleep_and_echo.c Makefile
	$(CC) $(CFLAGS) -static -ggdb -DSECS=$(sleep_seconds) -DECODE=$(ecode) sleep_and_echo.c -o $@

clean:
	rm -f myshell sne *.o
