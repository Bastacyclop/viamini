SRCDIR := src
BLDDIR := build
TSTDIR := tests
CFLAGS := -W -Wall -Werror -pedantic -pedantic-errors -std=c11

ifeq ($(RELEASE), yes)
	CC := gcc
	CFLAGS += -O3
else
	CC := clang
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

all: $(BLDDIR)/main $(BLDDIR)/simple

$(BLDDIR)/%.o: $(SRCDIR)/%.c $(SRCDIR)/%.h builddir
	@echo "Compiling $< into $@"
	@$(CC) -c $(CFLAGS) $< -o $@

.PHONY: test
test: $(BLDDIR)/tests/vec $(BLDDIR)/tests/binary_heap $(BLDDIR)/tests/avl_tree $(BLDDIR)/tests/list
	@echo "[33m--------------- running tests ---------------[0m"
	@for t in $(BLDDIR)/tests/*; do \
	  if "./$$t"; then \
	    echo "[32m$$t: ✓[0m"; \
	  else \
	    echo "[31m$$t: ✕[0m"; \
	  fi \
	done
	@echo "[33m---------------------------------------------[0m"

.PHONY: bench
bench: $(BLDDIR)/bench
	@echo "[33m---------------- benchmarking ----------------[0m"
	@if "./$(BLDDIR)/bench"; then \
	  echo "[32mbenchmarked[0m"; \
	else \
	  echo "[31mbenchmark failed[0m"; \
	fi && \
	gnuplot < plot_cmds
	@echo "[33m---------------------------------------------[0m"

CIRCUIT_DEP := $(BLDDIR)/circuit.o $(BLDDIR)/binary_heap.o $(BLDDIR)/avl_tree.o $(BLDDIR)/vec.o $(BLDDIR)/list.o $(BLDDIR)/util.o

$(BLDDIR)/simple: $(SRCDIR)/simple.c $(BLDDIR)/output.o $(CIRCUIT_DEP) builddir
	$(CC) $(CFLAGS) -lm $(SRCDIR)/simple.c $(BLDDIR)/output.o $(CIRCUIT_DEP) -o $(BLDDIR)/simple

$(BLDDIR)/bench: $(SRCDIR)/bench.c $(CIRCUIT_DEP) builddir
	$(CC) $(CFLAGS) -lm $(SRCDIR)/bench.c $(CIRCUIT_DEP) -o $(BLDDIR)/bench
	
$(BLDDIR)/main: $(SRCDIR)/main.c $(BLDDIR)/output.o $(CIRCUIT_DEP) builddir
	$(CC) $(CFLAGS) -pthread -lm $(SRCDIR)/main.c $(BLDDIR)/output.o $(CIRCUIT_DEP) -o $(BLDDIR)/main

$(BLDDIR)/tests/vec: $(TSTDIR)/vec.c $(BLDDIR)/vec.o $(BLDDIR)/util.o testdir
	$(CC) $(CFLAGS) $(TSTDIR)/vec.c $(BLDDIR)/vec.o $(BLDDIR)/util.o -o $(BLDDIR)/tests/vec

$(BLDDIR)/tests/binary_heap: $(TSTDIR)/binary_heap.c $(BLDDIR)/binary_heap.o $(BLDDIR)/vec.o testdir
	$(CC) $(CFLAGS) $(TSTDIR)/binary_heap.c $(BLDDIR)/binary_heap.o $(BLDDIR)/vec.o $(BLDDIR)/util.o -o $(BLDDIR)/tests/binary_heap

$(BLDDIR)/tests/avl_tree: $(TSTDIR)/avl_tree.c $(BLDDIR)/avl_tree.o testdir
	$(CC) $(CFLAGS) $(TSTDIR)/avl_tree.c $(BLDDIR)/avl_tree.o $(BLDDIR)/util.o -o $(BLDDIR)/tests/avl_tree

$(BLDDIR)/tests/list: $(TSTDIR)/list.c $(BLDDIR)/list.o testdir
	$(CC) $(CFLAGS) $(TSTDIR)/list.c $(BLDDIR)/list.o $(BLDDIR)/util.o -o $(BLDDIR)/tests/list

builddir:
	@mkdir -p $(BLDDIR)

testdir:
	@mkdir -p $(BLDDIR)/tests

.PHONY: clean
clean:
	rm -rf $(BLDDIR)
