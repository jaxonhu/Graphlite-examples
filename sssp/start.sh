#!/bin/bash
DIR_PATH=$(cd "$(dirname "$0")";pwd)
path="/home/jaxonhu/code/GraphLite-0.20/GraphLite/sssp"
M_PATH=${DIR_PATH%GraphLite/sssp}
echo ${M_PATH}"GraphLite/output/"
rm -rf ${M_PATH}"GraphLite/output/"

cd $M_PATH;start-graphlite ${M_PATH}"GraphLite/sssp/Sssp.so" ${M_PATH}"GraphLite/part2-input/SSSP-graph0_4w" ${M_PATH}"GraphLite/output/sssp" 0
