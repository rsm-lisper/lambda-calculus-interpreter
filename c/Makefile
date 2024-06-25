BINARY=lci

CFILES=$(wildcard *.c)
TESTDIR=../tests.lc/
TESTS=$(wildcard $(TESTDIR)*.lc)
TEST_RESULTS=$(patsubst %.lc,%.res,$(TESTS))

CC=gcc
CFLAGS=-Wall -Wextra -Werror -std=c89

ANSIGREEN="\033[32m"
ANSIYELLOW="\033[33m"
ANSIBOLD="\033[1m"
ANSIRST="\033[0m"
ANSIGOK=" ["$(ANSIGREEN)"OK"$(ANSIRST)"] "
ANSIYBIN=$(ANSIYELLOW)$(BINARY)$(ANSIRST)

MAKEFLAGS+=--silent

all: $(BINARY)

$(BINARY): $(CFILES)
	echo -n "** Building "$(ANSIYBIN)":"
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)
	echo $(ANSIGOK)

check: $(TEST_RESULTS)
	echo "** "$(ANSIBOLD)$(ANSIYBIN)" test results:"$(ANSIBOLD)$(ANSIGOK)

%.res: %.lc
	echo -n " - testing "$(ANSIBOLD)$(BINARY)$(ANSIRST)":" \
		$(ANSIYELLOW)$(patsubst $(TESTDIR)%.lc,%,$?)$(ANSIRST)
	./$(BINARY) <$? >$@ult
	diff --color=always --text $@ult $(patsubst %.lc,%.expect,$?)
	echo $(ANSIGOK)

clean:
	echo -n "** Cleaning "$(ANSIYBIN)":"
	rm -f $(BINARY)
	rm -f $(TESTDIR)*.result
	echo $(ANSIGOK)
