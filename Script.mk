
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
	./$(SCRIPT) <$? >$@ult 2>$@ult_err
	diff --color=always --text $@ult $(patsubst %.lc,%.expect,$?)
	diff --color=always --text $@ult_err $(patsubst %.lc,%.expect_err,$?)
	echo $(ANSIGOK)

clean:
	echo -n "** Cleaning "$(ANSIYBIN)":"
	rm -f $(TESTDIR)*.result
	rm -f $(TESTDIR)*.result_err
	echo $(ANSIGOK)
