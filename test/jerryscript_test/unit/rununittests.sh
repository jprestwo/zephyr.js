#!/bin/bash

curdir=$(pwd)

test_dir=${curdir}/../../../deps/jerryscript/tests/unit

echo "Starting all unit tests"

for test in `ls ${test_dir}`
do
    # check if it is unit test
    if [[ "$test" =~ ^test-.*\.c$ ]]; then
        echo -e "RUNNING TEST: $test"
        read -p "Press any key to continue... (ctrl-c to exit)" -n1 -s

        # strip off the "test-"
        test=$(echo ${test:5})
        echo $test

        # strip off the .c
        test=`basename $test ".c"`

        # build and run
        make TEST=$test qemu
    fi
done

echo "Finished all unit tests"
