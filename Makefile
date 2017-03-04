CFLAGS=-O2 -Wall -g
BIN=descramble scramble bfin_boot

all: $(BIN)

scramble: scramble.c
	$(CC) $(CFLAGS) -o $@ $^

descramble: descramble.c
	$(CC) $(CFLAGS) -o $@ $^

bfin_boot: bfin_boot.c elf.c misc.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf $(BIN)
