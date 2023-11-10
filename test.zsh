#!/bin/zsh

TEST_OUTPUT_FILE=/tmp/lc-test-output
TEST_ERROR_FILE=/tmp/lc-test-error

for INTERP_NAME in lci.*; do
    for LC_FILE in tests.lc/*.lc; do
        echo "* $INTERP_NAME $LC_FILE:"
        TEST_EXPECT_OUTPUT_FILE=$LC_FILE-expect
        ./$INTERP_NAME < $LC_FILE > $TEST_OUTPUT_FILE 2> $TEST_ERROR_FILE
        if [[ `cat $TEST_ERROR_FILE` != "" ]]; then
            echo "ERROR:\n"
            cat $TEST_ERROR_FILE
        fi
        if [[ `diff $TEST_EXPECT_OUTPUT_FILE $TEST_OUTPUT_FILE` != "" ]]; then
            echo "OUTPUT DIFFERENT THEN EXPECTED:\n"
            diff --color=always $TEST_EXPECT_OUTPUT_FILE $TEST_OUTPUT_FILE
        fi
        echo "----------"
    done
done

rm $TEST_OUTPUT_FILE $TEST_ERROR_FILE
