CC=c++
CFLAGS=-std=c++17

test:
	$(CC) -o test test.cpp $< $(CFLAGS)
