#!/bin/bash
if [ -n "$1" ]; then
    BASE_DIR=$1
    if [[ "$BASE_DIR" != /* ]]; then
        BASE_DIR=${PWD}/$BASE_DIR
    fi
else
    BASE_DIR=${PWD}
fi

TEST_DIR=$BASE_DIR/test1

rm -rf $TEST_DIR

mkdir $TEST_DIR
echo "Lorem ipsum dolor sit amet consectetur adipiscing elit" > $TEST_DIR/file1.txt
echo "Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua" > $TEST_DIR/file2.txt

# hards
ln $TEST_DIR/file1.txt $TEST_DIR/hard1.txt
ln $TEST_DIR/file2.txt $TEST_DIR/hard2.txt
ln $TEST_DIR/hard1.txt $TEST_DIR/hardh1.txt
ln $TEST_DIR/hard2.txt $TEST_DIR/hardh2.txt

# softs
ln -s $TEST_DIR/file1.txt $TEST_DIR/soft1.txt
ln -s $TEST_DIR/file2.txt $TEST_DIR/soft2.txt
ln $TEST_DIR/soft1.txt $TEST_DIR/hards1.txt
ln $TEST_DIR/soft2.txt $TEST_DIR/hards2.txt

# dups
cp $TEST_DIR/file1.txt $TEST_DIR/file3.txt
ln $TEST_DIR/file3.txt $TEST_DIR/hards3.txt

ls -laiR $TEST_DIR
