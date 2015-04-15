SRCDIR := src
BLDDIR := build
TSTDIR := tests
TSTBLDDIR := $(BLDDIR)/tests
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

all: $(BLDDIR)/intersect $(BLDDIR)/intersect_all $(BLDDIR)/solve

$(BLDDIR)/%.o: $(SRCDIR)/%.c $(SRCDIR)/%.h $(BLDDIR)
	@echo "Compiling $< into $@"
	@$(CC) -c $(CFLAGS) $< -o $@

.PHONY: test
test: $(BLDDIR)/tests/vec $(BLDDIR)/tests/bit_set $(BLDDIR)/tests/binary_heap $(BLDDIR)/tests/avl_tree $(BLDDIR)/tests/list
	@echo "[33m--------------- running tests ---------------[0m"
	@for t in $(BLDDIR)/tests/*; do \
	  if "./$$t"; then \
	    echo "[32m$$t: ✓[0m"; \
	  else \
	    echo "[31m$$t: ✕[0m"; \
	  fi \
	done
	@echo "[33m---------------------------------------------[0m"

.PHONY: intersect_bench
intersect_bench: $(BLDDIR)/intersect_bench
	@echo "[33m---------------- benchmarking ----------------[0m"
	@if "./$(BLDDIR)/intersect_bench"; then \
	  echo "[32mbenchmarked[0m"; \
	else \
	  echo "[31mbenchmark failed[0m"; \
	fi && \
	cd intersect_bench && \
	gnuplot < plot_cmds
	@echo "[33m----------------------------------------------[0m"

.PHONY: solve_bench
solve_bench: $(BLDDIR)/solve_bench
	@echo "[33m---------------- benchmarking ----------------[0m"
	@if "./$(BLDDIR)/solve_bench"; then \
	  echo "[32mbenchmarked[0m"; \
	else \
	  echo "[31mbenchmark failed[0m"; \
	fi && \
	cd solve_bench && \
	gnuplot < plot_cmds
	@echo "[33m----------------------------------------------[0m"

NETLIST_DEP := $(BLDDIR)/netlist.o $(BLDDIR)/binary_heap.o $(BLDDIR)/avl_tree.o $(BLDDIR)/vec.o $(BLDDIR)/list.o $(BLDDIR)/bit_set.o $(BLDDIR)/util.o $(BLDDIR)/core.o

$(BLDDIR)/intersect: $(SRCDIR)/intersect.c $(BLDDIR)/display.o $(NETLIST_DEP)
	$(CC) $(CFLAGS) -lm $(SRCDIR)/intersect.c $(BLDDIR)/display.o $(NETLIST_DEP) -o $(BLDDIR)/intersect

$(BLDDIR)/intersect_all: $(SRCDIR)/intersect_all.c $(BLDDIR)/display.o $(NETLIST_DEP)
	$(CC) $(CFLAGS) -pthread -lm $(SRCDIR)/intersect_all.c $(BLDDIR)/display.o $(NETLIST_DEP) -o $(BLDDIR)/intersect_all

$(BLDDIR)/intersect_bench: $(SRCDIR)/intersect_bench.c $(NETLIST_DEP)
	$(CC) $(CFLAGS) -lm $(SRCDIR)/intersect_bench.c $(NETLIST_DEP) -o $(BLDDIR)/intersect_bench

$(BLDDIR)/solve: $(SRCDIR)/solve.c $(BLDDIR)/display.o $(NETLIST_DEP)
	$(CC) $(CFLAGS) -lm $(SRCDIR)/solve.c $(BLDDIR)/display.o $(NETLIST_DEP) -o $(BLDDIR)/solve

$(BLDDIR)/solve_bench: $(SRCDIR)/solve_bench.c $(NETLIST_DEP)
	$(CC) $(CFLAGS) -lm $(SRCDIR)/solve_bench.c $(NETLIST_DEP) -o $(BLDDIR)/solve_bench

$(TSTBLDDIR)/vec: $(TSTDIR)/vec.c $(BLDDIR)/vec.o $(BLDDIR)/core.o $(TSTBLDDIR)
	$(CC) $(CFLAGS) $(TSTDIR)/vec.c $(BLDDIR)/vec.o $(BLDDIR)/core.o -o $(TSTBLDDIR)/vec

$(TSTBLDDIR)/bit_set: $(TSTDIR)/bit_set.c $(BLDDIR)/bit_set.o $(BLDDIR)/vec.o $(BLDDIR)/core.o $(TSTBLDDIR)
	$(CC) $(CFLAGS) $(TSTDIR)/bit_set.c $(BLDDIR)/bit_set.o $(BLDDIR)/vec.o $(BLDDIR)/core.o -o $(TSTBLDDIR)/bit_set

$(TSTBLDDIR)/binary_heap: $(TSTDIR)/binary_heap.c $(BLDDIR)/binary_heap.o $(BLDDIR)/vec.o $(BLDDIR)/core.o $(TSTBLDDIR)
	$(CC) $(CFLAGS) $(TSTDIR)/binary_heap.c $(BLDDIR)/binary_heap.o $(BLDDIR)/vec.o $(BLDDIR)/core.o -o $(TSTBLDDIR)/binary_heap

$(TSTBLDDIR)/avl_tree: $(TSTDIR)/avl_tree.c $(BLDDIR)/avl_tree.o $(BLDDIR)/core.o $(TSTBLDDIR)
	$(CC) $(CFLAGS) $(TSTDIR)/avl_tree.c $(BLDDIR)/avl_tree.o $(BLDDIR)/core.o -o $(TSTBLDDIR)/avl_tree

$(TSTBLDDIR)/list: $(TSTDIR)/list.c $(BLDDIR)/list.o $(BLDDIR)/core.o $(TSTBLDDIR)
	$(CC) $(CFLAGS) $(TSTDIR)/list.c $(BLDDIR)/list.o $(BLDDIR)/core.o -o $(TSTBLDDIR)/list

$(BLDDIR):
	@mkdir -p $(BLDDIR)

$(TSTBLDDIR):
	@mkdir -p $(TSTBLDDIR)

.PHONY: clean
clean:
	rm -rf $(BLDDIR)
