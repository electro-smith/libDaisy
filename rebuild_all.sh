#!/bin/bash
start_dir=$PWD

echo "rebuilding everything. . . "
echo "only errors, and warnings will output. . . "
echo "-------------------"
sleep 1

echo "rebuilding libdaisy"
cd libdaisy
make clean | grep "warningr\|error"
make | grep "warningr\|error"
echo "done"

echo "rebuilding daisysp"
cd $start_dir
cd daisysp
make clean | grep "warningr\|error"
make | grep "warningr\|error"
cd $start_dir
echo "done"

echo "rebuilding examples:"
example_dirs=(example_projects daisysp/examples libdaisy/examples)
for e in ${example_dirs[@]}; do
    for d in $e/*/; do
        echo "rebuilding $d"
        cd $d
        make clean | grep "warning\|error" # grep for silencing make outputs when regenerating everything.
        make | grep "warning\|error"
        cd $start_dir
        echo "done"
    done
done

echo "finished"

