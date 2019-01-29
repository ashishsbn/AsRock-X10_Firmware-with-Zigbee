#!/bin/sh

mkdir -p bin
cd ipq
python pack.py -t nor -B -F boardconfig_x10 -o ../bin/x10-nor-ipq40xx-single.img . 
python pack.py -t nor -B -F boardconfig_x10_boot -o ../bin/x10-nor_boot-ipq40xx-single.img . 
python pack.py -t nor -B -F boardconfig_x10_all -o ../bin/x10-nor_all-ipq40xx-single.img . 
./imgtool  -s ../bin/x10-nor-ipq40xx-single.img  -o ../bin/pega_x10-nor-ipq40xx-single.img -h 1 -v 1.00 -t 2 -m ASRockX10
./imgtool  -s ../bin/x10-nor_boot-ipq40xx-single.img  -o ../bin/pega_x10-nor_boot-ipq40xx-single.img -h 1 -v 1.00 -t 1 -m ASRockX10
./imgtool  -s ../bin/x10-nor_all-ipq40xx-single.img  -o ../bin/pega_x10-nor_all-ipq40xx-single.img -h 1 -v 1.00 -t 2 -m ASRockX10
cd ..
