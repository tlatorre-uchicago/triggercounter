#!/usr/bin/env bash
set -x

DATA=/raid/data/l1

./triggercounter $DATA/SNOP_0000015071_000.zdab > SNOP_0000015071.txt
./triggercounter $DATA/SNOP_0000015072_000.zdab > SNOP_0000015072.txt
./triggercounter $DATA/SNOP_0000015073_000.zdab > SNOP_0000015073.txt
./triggercounter $DATA/SNOP_0000015074_000.zdab > SNOP_0000015074.txt
./triggercounter $DATA/SNOP_0000015075_000.zdab > SNOP_0000015075.txt
./triggercounter $DATA/SNOP_0000015076_000.zdab > SNOP_0000015076.txt
./triggercounter $DATA/SNOP_0000015077_000.zdab > SNOP_0000015077.txt
./triggercounter $DATA/SNOP_0000015078_000.zdab > SNOP_0000015078.txt
./triggercounter $DATA/SNOP_0000015079_000.zdab > SNOP_0000015079.txt
./triggercounter $DATA/SNOP_0000015080_000.zdab > SNOP_0000015080.txt
./triggercounter $DATA/SNOP_0000015081_000.zdab > SNOP_0000015081.txt
./triggercounter $DATA/SNOP_0000015082_000.zdab > SNOP_0000015082.txt
./triggercounter $DATA/SNOP_0000015083_000.zdab > SNOP_0000015083.txt
./triggercounter $DATA/SNOP_0000015084_000.zdab > SNOP_0000015084.txt
./triggercounter $DATA/SNOP_0000015091_000.zdab > SNOP_0000015091.txt
./triggercounter $DATA/SNOP_0000015092_000.zdab > SNOP_0000015092.txt

# remove the last line
sed -i '$ d' SNOP_0000015071.txt
sed -i '$ d' SNOP_0000015072.txt
sed -i '$ d' SNOP_0000015073.txt
sed -i '$ d' SNOP_0000015074.txt
sed -i '$ d' SNOP_0000015075.txt
sed -i '$ d' SNOP_0000015076.txt
sed -i '$ d' SNOP_0000015077.txt
sed -i '$ d' SNOP_0000015078.txt
sed -i '$ d' SNOP_0000015079.txt
sed -i '$ d' SNOP_0000015080.txt
sed -i '$ d' SNOP_0000015081.txt
sed -i '$ d' SNOP_0000015082.txt
sed -i '$ d' SNOP_0000015083.txt
sed -i '$ d' SNOP_0000015084.txt
sed -i '$ d' SNOP_0000015091.txt
sed -i '$ d' SNOP_0000015092.txt

./caencounter $DATA/SNOP_0000015071_000.zdab > SNOP_0000015071_caen.txt
./caencounter $DATA/SNOP_0000015072_000.zdab > SNOP_0000015072_caen.txt
./caencounter $DATA/SNOP_0000015073_000.zdab > SNOP_0000015073_caen.txt
./caencounter $DATA/SNOP_0000015074_000.zdab > SNOP_0000015074_caen.txt
./caencounter $DATA/SNOP_0000015075_000.zdab > SNOP_0000015075_caen.txt
./caencounter $DATA/SNOP_0000015076_000.zdab > SNOP_0000015076_caen.txt
./caencounter $DATA/SNOP_0000015077_000.zdab > SNOP_0000015077_caen.txt
./caencounter $DATA/SNOP_0000015078_000.zdab > SNOP_0000015078_caen.txt
./caencounter $DATA/SNOP_0000015079_000.zdab > SNOP_0000015079_caen.txt
./caencounter $DATA/SNOP_0000015080_000.zdab > SNOP_0000015080_caen.txt
./caencounter $DATA/SNOP_0000015081_000.zdab > SNOP_0000015081_caen.txt
./caencounter $DATA/SNOP_0000015082_000.zdab > SNOP_0000015082_caen.txt
./caencounter $DATA/SNOP_0000015083_000.zdab > SNOP_0000015083_caen.txt
./caencounter $DATA/SNOP_0000015084_000.zdab > SNOP_0000015084_caen.txt
./caencounter $DATA/SNOP_0000015091_000.zdab > SNOP_0000015091_caen.txt
./caencounter $DATA/SNOP_0000015092_000.zdab > SNOP_0000015092_caen.txt

sed -i '$ d' SNOP_0000015071_caen.txt
sed -i '$ d' SNOP_0000015072_caen.txt
sed -i '$ d' SNOP_0000015073_caen.txt
sed -i '$ d' SNOP_0000015074_caen.txt
sed -i '$ d' SNOP_0000015075_caen.txt
sed -i '$ d' SNOP_0000015076_caen.txt
sed -i '$ d' SNOP_0000015077_caen.txt
sed -i '$ d' SNOP_0000015078_caen.txt
sed -i '$ d' SNOP_0000015079_caen.txt
sed -i '$ d' SNOP_0000015080_caen.txt
sed -i '$ d' SNOP_0000015081_caen.txt
sed -i '$ d' SNOP_0000015082_caen.txt
sed -i '$ d' SNOP_0000015083_caen.txt
sed -i '$ d' SNOP_0000015084_caen.txt
sed -i '$ d' SNOP_0000015091_caen.txt
sed -i '$ d' SNOP_0000015092_caen.txt
