CC=gcc
CFLAGS=-std=c99 -Wall -Wextra
LOAD=load_balancer
SERVER=server
DATASTRUCT_FUNCS=datastruct_funcs

.PHONY: build clean

build: tema2

tema2: main.o $(LOAD).o $(SERVER).o $(DATASTRUCT_FUNCS).o
	$(CC) $^ -o $@

main.o: main.c
	$(CC) $(CFLAGS) $^ -c

$(SERVER).o: $(SERVER).c $(SERVER).h
	$(CC) $(CFLAGS) $^ -c

$(LOAD).o: $(LOAD).c $(LOAD).h
	$(CC) $(CFLAGS) $^ -c

$(DATASTRUCT_FUNCS).o : $(DATASTRUCT_FUNCS).c $(DATASTRUCT_FUNCS).h
	$(CC) $(CFLAGS) $^ -c
clean:
	rm -f *.o tema2 *.h.gch
