CC=gcc
CFLAGS=-Wall -pedantic -std=c99 -D _POSIX_C_SOURCE=200112L
EXEC=
BIN=

all: cancel_impress init_simulateurs mpr print_demon \
	 sim_impress wait_impress clean

%: %.o
	$(CC) -o $(BIN)$@ $^

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -f *.o

cleanall:
	rm -f *.o $(BIN)cancel_impress $(BIN)init_simulateurs $(BIN)mpr \
		$(BIN)print_demon $(BIN)sim_impress $(BIN)wait_impress
