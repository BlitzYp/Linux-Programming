#!/bin/bash

RUNS=10

run_test () {
    METHOD=$1

    REAL_SUM=0
    USER_SUM=0
    SYS_SUM=0

    PROGRAM_OUTPUT=""

    for i in $(seq 1 $RUNS); do
        OUT=$( (time ./md_mem_100 $METHOD -t) 2>&1 )

        if [ $i -eq 1 ]; then
            PROGRAM_OUTPUT=$(echo "$OUT" | grep -v -E "real|user|sys")
        fi

        REAL=$(echo "$OUT" | grep real | awk '{print $2}' | sed 's/0m//;s/s//')
        USER=$(echo "$OUT" | grep user | awk '{print $2}' | sed 's/0m//;s/s//')
        SYS=$(echo "$OUT" | grep sys  | awk '{print $2}' | sed 's/0m//;s/s//')

        REAL_SUM=$(echo "$REAL_SUM + $REAL" | bc)
        USER_SUM=$(echo "$USER_SUM + $USER" | bc)
        SYS_SUM=$(echo "$SYS_SUM + $SYS" | bc)
    done

    REAL_AVG=$(echo "scale=6; $REAL_SUM / $RUNS" | bc)
    USER_AVG=$(echo "scale=6; $USER_SUM / $RUNS" | bc)
    SYS_AVG=$(echo "scale=6; $SYS_SUM / $RUNS" | bc)

    echo "$PROGRAM_OUTPUT"
    echo
    printf "real\t0m%.3fs\n" $REAL_AVG
    printf "user\t0m%.3fs\n" $USER_AVG
    printf "sys\t0m%.3fs\n" $SYS_AVG
    echo
}

run_test malloc
run_test sbrk
run_test mmap