CC=c++
CFLAGS=-std=c++17 -Wall -Iinclude

test:
	$(CC) -o test example.cpp $< $(CFLAGS)

.PHONY: clean
clean:
	rm test
