#!/bin/bash

#grep * -r -e "#include" | grep -P "^(tools|core|orgs|select|evaluate|config|analyze|interface|placement|schema)" | grep -v "OLD"
CUR_DIR=$(pwd)
cd ../..
grep -r -e "#include" | grep -P "^(tools|core|orgs|select|evaluate|config|interface|placement|schema)" | grep -v "OLD" | grep -v -P "<.+>$" | grep -v "emp/" > ${CUR_DIR}/include_list.txt
cd ${CUR_DIR}

python3 process_includes.py
