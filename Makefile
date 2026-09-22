CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -IHeaders
SRC = $(wildcard src/*.c)

vmx: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o vmx

clean:
	rm -f vmx

.PHONY: clean
