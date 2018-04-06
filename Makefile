#
#	Makefile				Boston Micromachines Corp. executables makefile
#
#	Mar 15,     2015        CIW/SJT original entry
#
#	© Copyright 2015, Boston Micromachines, Inc. All rights reserved.
#
###
#
BMC_LIBDIR	:= ../../lib
BMC_TMPDIR	:= ../../tmp
BMC_INCDIR	:= ../inc
BMC_BINDIR	:= ../../bin

CFLAGS	+= -I$(BMC_INCDIR)
LFLAGS	+= -L $(BMC_LIBDIR)
LFLAGS	+= -lrt -lbmcmd -lm -lpthread
#
###
#
$(BMC_TMPDIR)/%.o: %.c
	gcc -c $(CFLAGS) -o $@ $<

$(BMC_BINDIR)/%: %.o
	gcc -o $@ $< $(LFLAGS)
#
###
#
all:	$(BMC_BINDIR)/bmc_ltest
#
###
#
$(BMC_BINDIR)/bmc_ltest: $(BMC_TMPDIR)/bmc_ltest.o $(BMC_TMPDIR)/ImageCreate.o

$(BMC_TMPDIR)/bmc_ltest.o: bmc_ltest.c

$(BMC_TMPDIR)/ImageCreate.o: ImageCreate.c
