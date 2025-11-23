SHELL:=/bin/bash
HOSTNAME:=$(shell hostname)
BIN:=instructions.out
SRC:=instructions.cpp

LIKWID_BASE?=$(shell dirname $(shell bash -lc 'command -v likwid-perfctr' 2>/dev/null))

LIKWID_LIB:=$(LIKWID_BASE)/../lib/
LIKWID_INCLUDE:=$(LIKWID_BASE)/../include/

CXX:=g++ -std=c++17
CXXFLAGS:=-O3 -g3 -fno-tree-vectorize -DLIKWID_PERFMON -I$(LIKWID_INCLUDE) 
LDFLAGS += -L$(LIKWID_LIB) -Wl,-rpath,$(LIKWID_LIB)
LDLIBS += -llikwid

# setting of arithmetic operator
OP?=+
# setting of maximum number of dependency chains
CHAINS_MAX?=1U
# setting of CPU to pin the thread
CPU?=2

CSV_FILE:=$(HOSTNAME)_$(CHAINS_MAX).csv
LIKWID_FILE:=$(HOSTNAME)_$(CHAINS_MAX).txt
LIKWID_GROUP?=CLOCK

all: $(BIN)

$(BIN): $(SRC) Makefile
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -DOP=$(OP) -DCHAINS_MAX=$(CHAINS_MAX) $< -o $@ $(LDLIBS)

.PHONY: clean bench-instructions

bench-instructions:
	mkdir -p "./data"
	@echo dtype,op,chains,time,rep > "./data/$(CSV_FILE)"
	@for op in "+" "*" "/"; do \
		make $(BIN) -B OP="$$op"; \
		likwid-perfctr -C 2 -g $(LIKWID_GROUP) -o "./data/$(LIKWID_FILE)" -m ./$(BIN) 27 >> "./data/$(CSV_FILE)"; \
	done;

clean:
	$(RM) $(BIN)
