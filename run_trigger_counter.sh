#!/usr/bin/env bash
set -x

DATA=/raid/data/l1

# Nhit 100 Med with low channel thresholds
RUNS="17450 17451 17452 17453 17454 17455 17456 17457 17459 17460 17461 17462 17463 "
# Nhit 40 Lo with low channel thresholds
RUNS+="17466 17467 17468 17469 17470 17471 17472 17473 17474 17475 17476 17477 "
# Nhit 100 Lo with low channel thresholds
RUNS+="17479 17480 17481 17482 17483 17484 17485 "
# Nhit 100 Med with high channel thresholds
RUNS+="17929 17930 17931 17932 17933 17934 17935 17936 17937 17938 17939 17940 17941 17942"
# Nhit 40 Lo with high channel thresholds
RUNS="17945 17946 17947 17948 17949 17950 17951 17952 17953 17954 17955 17956 17957 17958"
# Nhit 100 Lo with high channel thresholds
RUNS="17960 17961 17962 17963 17964 17965 17966 17967 17968 17969 17970 17971 17972"

for run in $RUNS; do
    filename=$(printf "SNOP_%010d" $run)
    ./triggercounter $DATA/${filename}_000.zdab > ${filename}.txt
    # remove the last line
    sed -i '$ d' $filename.txt
    #./caencounter $DATA/${filename}_000.zdab > ${filename}_caen.txt
    #sed -i '$ d' ${filename}_caen.txt
done;
