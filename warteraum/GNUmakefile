CC ?= clang

PREFIX ?= ./out
BINDIR ?= $(PREFIX)/bin

# baseline settings
CFLAGS += -W -Wall -Wextra -pedantic -std=c99
CFLAGS += -D_DEFAULT_SOURCE # make httpserver.h work with NixOS glibc

# ignore some warnings
CFLAGS += -Wno-gnu-empty-initializer  # httpserver.h

LDFLAGS = -lscrypt-kdf

HTTPSERVER = ../third_party/httpserver.h/httpserver.h

TEST_BINS = test/emitjson.test test/queue.test test/form.test test/routing.test

.PHONY: install clean check

all: warteraum hashtoken

warteraum: emitjson.o queue.o routing.o form.o auth.o main.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

hashtoken: hashtoken.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

main.o: main.c queue.h routing.h form.h v1_static.h emitjson.h \
	auth.h http_string.h $(HTTPSERVER)

form.o: form.c http_string.h $(HTTPSERVER)

routing.o: routing.c $(HTTPSERVER)

hashtoken.o: hashtoken.c auth.h

auth.o: auth.h tokens.h http_string.h $(HTTPSERVER)

install: all
	install -Dm755 hashtoken -t $(BINDIR)
	install -Dm755 warteraum -t $(BINDIR)

clean:
	rm -f warteraum hashtoken
	rm -f *.o
	rm -f test/*.o
	rm -f $(TEST_BINS)

test/%.o: http_string.h

test/%.test: %.o test/test_%.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

check: warteraum $(TEST_BINS)
	@echo == Running unit tests
	for t in $(TEST_BINS); do ./$$t; done
	
	@echo == Checking emitjson with jq
	./test/emitjson.test -o | jq
	
	@echo == Running integration tests
	./test/integration
