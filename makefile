CC=c++
CFLAGS=-std=c++17 -Iinclude

test:
	$(CC) -o test example.cpp $< $(CFLAGS)
