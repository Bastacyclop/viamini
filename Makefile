vpath %.c src
vpath %.h src
vpath %.o build

FLAGS = -g -Wall -std=c11 -lm

all: build/main

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

.PHONY: dirs
dirs:
	mkdir -p build/tests

.PHONY: clean
clean:
	rm -rf build/*

build/main: main.c output.o circuit.o vec.o util.o dirs
	gcc $(FLAGS) src/main.c build/output.o build/circuit.o build/binary_heap.o build/vec.o build/util.o -o build/main

output.o: output.h output.c circuit.o dirs
	gcc -c $(FLAGS) src/output.c -o build/output.o

circuit.o: circuit.h circuit.c vec.o binary_heap.o dirs
	gcc -c $(FLAGS) src/circuit.c -o build/circuit.o

binary_heap.o: binary_heap.h binary_heap.c vec.o dirs
	gcc -c $(FLAGS) src/binary_heap.c -o build/binary_heap.o

vec.o: vec.h vec.c util.o dirs
	gcc -c $(FLAGS) src/vec.c -o build/vec.o

util.o: util.h util.c dirs
	gcc -c $(FLAGS) src/util.c -o build/util.o

build/tests/vec: vec.c vec.o util.o dirs
	gcc $(FLAGS) tests/vec.c build/vec.o build/util.o -o build/tests/vec

build/tests/binary_heap: tests/binary_heap.c binary_heap.o vec.o dirs
	gcc $(FLAGS) tests/binary_heap.c build/binary_heap.o build/vec.o build/util.o -o build/tests/binary_heap
