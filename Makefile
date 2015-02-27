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

all: $(BLDDIR)/main

$(BLDDIR)/%.o: $(SRCDIR)/%.c $(SRCDIR)/%.h builddir
	@echo "Compiling $< into $@"
	@$(CC) -c $(CFLAGS) $< -o $@

.PHONY: test
test: $(BLDDIR)/tests/vec $(BLDDIR)/tests/binary_heap $(BLDDIR)/tests/bit_set
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

$(BLDDIR)/bench: $(SRCDIR)/main.c $(BLDDIR)/output.o $(BLDDIR)/circuit.o $(BLDDIR)/binary_heap.o $(BLDDIR)/vec.o $(BLDDIR)/util.o builddir
	$(CC) $(CFLAGS) -lm $(SRCDIR)/bench.c $(BLDDIR)/output.o $(BLDDIR)/circuit.o $(BLDDIR)/binary_heap.o $(BLDDIR)/vec.o $(BLDDIR)/util.o -o $(BLDDIR)/bench
	
$(BLDDIR)/main: $(SRCDIR)/main.c $(BLDDIR)/output.o $(BLDDIR)/circuit.o $(BLDDIR)/binary_heap.o $(BLDDIR)/vec.o $(BLDDIR)/util.o builddir
	$(CC) $(CFLAGS) -lm $(SRCDIR)/main.c $(BLDDIR)/output.o $(BLDDIR)/circuit.o $(BLDDIR)/binary_heap.o $(BLDDIR)/vec.o $(BLDDIR)/util.o -o $(BLDDIR)/main

$(BLDDIR)/tests/vec: $(TSTDIR)/vec.c $(BLDDIR)/vec.o $(BLDDIR)/util.o testdir
	$(CC) $(CFLAGS) $(TSTDIR)/vec.c $(BLDDIR)/vec.o $(BLDDIR)/util.o -o $(BLDDIR)/tests/vec

$(BLDDIR)/tests/binary_heap: $(TSTDIR)/binary_heap.c $(BLDDIR)/binary_heap.o $(BLDDIR)/vec.o testdir
	$(CC) $(CFLAGS) $(TSTDIR)/binary_heap.c $(BLDDIR)/binary_heap.o $(BLDDIR)/vec.o $(BLDDIR)/util.o -o $(BLDDIR)/tests/binary_heap

#$(BLDDIR)/tests/avl_tree: $(TSTDIR)/avl_tree.c $(BLDDIR)/avl_tree.o $(BLDDIR)/vec.o testdir
#	$(CC) $(CFLAGS) $(TSTDIR)/avl_tree.c $(BLDDIR)/avl_tree.o $(BLDDIR)/vec.o $(BLDDIR)/util.o -o $(BLDDIR)/tests/avl_tree

$(BLDDIR)/tests/bit_set: $(TSTDIR)/bit_set.c $(BLDDIR)/bit_set.o $(BLDDIR)/vec.o testdir
	$(CC) $(CFLAGS) $(TSTDIR)/bit_set.c $(BLDDIR)/bit_set.o $(BLDDIR)/vec.o $(BLDDIR)/util.o -o $(BLDDIR)/tests/bit_set

builddir:
	@mkdir -p $(BLDDIR)

testdir:
	@mkdir -p $(BLDDIR)/tests

.PHONY: clean
clean:
	rm -rf $(BLDDIR)
