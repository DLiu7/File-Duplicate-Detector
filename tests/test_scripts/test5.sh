#!/bin/bash
BASE_DIR=${PWD}

TEST_DIR=$BASE_DIR/test5

rm -rf $TEST_DIR

mkdir $TEST_DIR
echo "Ut enim ad minim veniam quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat" > $TEST_DIR/file1.txt
echo "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur" > $TEST_DIR/file2.txt

# hards
ln $TEST_DIR/file1.txt $TEST_DIR/hard1.txt
ln $TEST_DIR/hard1.txt $TEST_DIR/hardh1.txt

# softs
ln -s $TEST_DIR/file1.txt $TEST_DIR/soft11.txt
ln -s $TEST_DIR/file1.txt $TEST_DIR/soft12.txt

${PWD}/test_scripts/test1.sh test5

mkdir $TEST_DIR/test2
echo "Ut enim ad minim veniam quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat" > $TEST_DIR/test2/file3.txt
echo "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur" >> $TEST_DIR/test2/file3.txt

ln -s $TEST_DIR/test1 $TEST_DIR/test1/tests1
ln -s $TEST_DIR/test1/tests1 $TEST_DIR/test2/tests1
ln -s $TEST_DIR/test2 $TEST_DIR/test2/tests2
ln -s $TEST_DIR $TEST_DIR/tests5

# hards - test2
ln $TEST_DIR/test2/file3.txt $TEST_DIR/tests5/file3.txt

# softs - test2
ln -s $TEST_DIR/test2/file3.txt $TEST_DIR/tests5/file31s.txt
ln -s $TEST_DIR/test2/file3.txt $TEST_DIR/tests5/file32s.txt
ln $TEST_DIR/test2/file31s.txt $TEST_DIR/tests5/file31sh.txt

ls -laiR $TEST_DIR
