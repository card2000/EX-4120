#-include ../toolchain.conf
#-include ../install.conf
CC=gcc
OUTDIR?=${PWD}

TARGETS=kenwood_tool
#TARGETS+=lib_test.so
TARGETS:=$(TARGETS:%=${OUTDIR}/%)

CFLAGS += -O2 -std=gnu99 -Os

all:${TARGETS}

${OUTDIR}/kenwood_tool: main.c $(wildcard *.h) $(wildcard ../include/*.h)
	$(CC) $(filter %.c %.cpp,$^) -w ${CFLAGS} -o $@

${OUTDIR}:
	@mkdir -p ${OUTDIR}
	
clean:
	rm -f ${TARGETS}

.PHONY: clean
