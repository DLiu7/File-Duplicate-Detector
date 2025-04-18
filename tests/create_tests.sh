#!/bin/bash
for i in 1 5 ; do
    if [ ! -f test_scripts/test$i.sh ]; then
        exit 1
    fi

    chmod +x test_scripts/test$i.sh
    rm -rf test$i
    ./test_scripts/test$i.sh
done
