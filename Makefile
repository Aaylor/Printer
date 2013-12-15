CC=gcc
CFLAGS=-g -Wall -pedantic -std=c99 -D _POSIX_C_SOURCE=200112L
EXEC=
BIN=

all: message.o queue.o cancel_impress init_simulateurs mpr print_demon \
	 sim_impress wait_impress clean

mpr: message.h message.o
cancel_impress: message.h message.o
print_demon: queue.h queue.o

%: %.o
	$(CC) -o $(BIN)$@ $^

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -f *.o

cleanall:
	rm -f *.o $(BIN)cancel_impress $(BIN)init_simulateurs $(BIN)mpr \
		$(BIN)print_demon $(BIN)sim_impress $(BIN)wait_impress
