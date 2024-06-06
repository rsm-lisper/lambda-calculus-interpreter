
TESTDIR=../tests.lc/
TESTS=$(wildcard $(TESTDIR)*.lc)
TEST_RESULTS=$(patsubst %.lc,%.res,$(TESTS))

ANSIGREEN="\033[32m"
ANSIYELLOW="\033[33m"
ANSIBOLD="\033[1m"
ANSIRST="\033[0m"
ANSIGOK=" ["$(ANSIGREEN)"OK"$(ANSIRST)"] "
ANSIYBIN=$(ANSIYELLOW)$(SCRIPT)$(ANSIRST)

MAKEFLAGS+=--silent

all:

check: $(TEST_RESULTS)
	echo "** "$(ANSIBOLD)$(ANSIYBIN)" test results:"$(ANSIBOLD)$(ANSIGOK)

%.res: %.lc
	echo -n " - testing "$(ANSIBOLD)$(SCRIPT)$(ANSIRST)":" \
		$(ANSIYELLOW)$(patsubst $(TESTDIR)%.lc,%,$?)$(ANSIRST)
	$(INTERP) $(SCRIPT) <$? >$@ult
	diff --color=always --text $@ult $(patsubst %.lc,%.expect,$?)
	echo $(ANSIGOK)

clean:
	echo -n "** Cleaning "$(ANSIYBIN)":"
	rm -f $(TESTDIR)*.result
	echo $(ANSIGOK)
