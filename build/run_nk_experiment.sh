#!/bin/bash

DATA_DIR=./exp_data/
NUM_REPS=50
THIS_DIR=$(pwd)
SEED_BASE=10000000
MAX_INDEX=1023
INDEX_SEED_OFFSET=1000

mkdir -p ${DATA_DIR}

for INDEX in `seq 0 ${MAX_INDEX}`
do
  echo ${INDEX}

  echo "Starting index ${INDEX}"
  INDEX_DIR=${DATA_DIR}/index_${INDEX}
  mkdir -p ${INDEX_DIR}
  for SEED_OFFSET in $(seq 0 $(( ${NUM_REPS} - 1 )) )
  do
    SEED=$(( ${SEED_BASE} + ${SEED_OFFSET} ))
    echo "  Starting seed #${SEED_OFFSET} -> ${SEED}"
    REP_DIR=${INDEX_DIR}/${SEED_OFFSET}
    mkdir -p ${REP_DIR}
    ./MABE -f ../settings/NK.mabe \
      -s starting_index=${INDEX} \
      -s random_seed=${SEED} > mabe_trace.txt
    mv fitness.csv ${REP_DIR}
    mv max_org.csv ${REP_DIR}
    mv mabe_trace.txt ${REP_DIR}
  done 
  SEED_BASE=$(( ${SEED_BASE} + ${INDEX_SEED_OFFSET} ))
done
