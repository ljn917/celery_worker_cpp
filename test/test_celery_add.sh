#/bin/bash

EXECUTABLE_OUTPUT_PATH=$1

BIN=${EXECUTABLE_OUTPUT_PATH}/celery_add

$BIN &
worker_pid=$!

function cleanup {
    kill $worker_pid
}
trap cleanup EXIT

timeout 10s python ../python/run_add.py
