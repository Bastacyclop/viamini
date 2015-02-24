vpath %.c src
vpath %.h src
vpath %.o build

SRCDIR = src
BUILDIR = build
CFLAGS = -W -Wall -Werror -pedantic -pedantic-errors -std=c11

ifeq ($(RELEASE), yes)
	CC = gcc
	CFLAGS += -O3
else
	CC = clang
	CFLAGS += -g3 -fstack-protector-all -Wshadow -Wunreachable-code \
		  -Wstack-protector -pedantic-errors -O0 -W -Wundef \
		  -Wfatal-errors -Wstrict-prototypes -Wmissing-prototypes \
		  -Wwrite-strings -Wunknown-pragmas -Wstrict-aliasing \
		  -Wold-style-definition -Wmissing-field-initializers \
		  -Wfloat-equal -Wpointer-arith -Wnested-externs \
		  -Wstrict-overflow=5 -Wswitch-default -Wswitch-enum \
		  -Wbad-function-cast -Wredundant-decls \
		  -fno-omit-frame-pointer -Winline -fstrict-aliasing
endif

all: build/main

builddir:
	@mkdir -p build

testdir:
	@mkdir -p build/tests

.PHONY: test
test: build/tests/vec build/tests/binary_heap
	@echo "[33m--------------- running tests ---------------[0m"
	@for t in build/tests/*; do \
	  if "./$$t"; then \
	    echo "[32m$$t: ✓[0m"; \
	  else \
	    echo "[31m$$t: ✕[0m"; \
	  fi \
	done
	@echo "[33m---------------------------------------------[0m"

.PHONY: clean
clean:
	rm -rf build/*

build/main: main.c output.o circuit.o vec.o util.o builddir
	$(CC) $(CFLAGS) -lm src/main.c build/output.o build/circuit.o build/binary_heap.o build/vec.o build/util.o -o build/main

output.o: output.h output.c circuit.o builddir
	$(CC) -c $(CFLAGS) src/output.c -o build/output.o

circuit.o: circuit.h circuit.c vec.o binary_heap.o builddir
	$(CC) -c $(CFLAGS) src/circuit.c -o build/circuit.o

binary_heap.o: binary_heap.h binary_heap.c vec.o builddir
	$(CC) -c $(CFLAGS) src/binary_heap.c -o build/binary_heap.o

vec.o: vec.h vec.c util.o builddir
	$(CC) -c $(CFLAGS) src/vec.c -o build/vec.o

util.o: util.h util.c builddir
	$(CC) -c $(CFLAGS) src/util.c -o build/util.o

build/tests/vec: vec.c vec.o util.o testdir
	$(CC) $(CFLAGS) tests/vec.c build/vec.o build/util.o -o build/tests/vec

build/tests/binary_heap: tests/binary_heap.c binary_heap.o vec.o testdir
	$(CC) $(CFLAGS) tests/binary_heap.c build/binary_heap.o build/vec.o build/util.o -o build/tests/binary_heap
