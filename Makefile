CC=gcc
CFLAGS=-Wall -Wextra -g

LOAD=load_balancer
SERVER=server
CACHE=lru_cache
UTILS=utils
HASH=hash
QUEUE=queue

# Add new source file names here:
# EXTRA=<extra source file name>

.PHONY: build clean

build: tema2

tema2: main.o $(LOAD).o $(SERVER).o $(CACHE).o $(UTILS).o $(HASH).o $(QUEUE).o # $(EXTRA).o
	$(CC) -g $^ -o $@

main.o: main.c
	$(CC) $(CFLAGS) $^ -c

$(LOAD).o: $(LOAD).c $(LOAD).h
	$(CC) $(CFLAGS) $^ -c

$(SERVER).o: $(SERVER).c $(SERVER).h
	$(CC) $(CFLAGS) $^ -c

$(CACHE).o: $(CACHE).c $(CACHE).h
	$(CC) $(CFLAGS) $^ -c

$(UTILS).o: $(UTILS).c $(UTILS).h
	$(CC) $(CFLAGS) $^ -c
$(HASH).o: $(HASH).c $(HASH).h
	$(CC) $(CFLAGS) $^ -c
$(QUEUE).o: $(QUEUE).c $(QUEUE).h
	$(CC) $(CFLAGS) $^ -c

# $(EXTRA).o: $(EXTRA).c $(EXTRA).h
# 	$(CC) $(CFLAGS) $^ -c

clean:
	rm -f *.o tema2 *.h.gch
