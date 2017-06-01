#!/bin/bash
DIR_PATH=$(cd "$(dirname "$0")";pwd)
path="/home/jaxonhu/code/GraphLite-0.20/GraphLite/color"
M_PATH=${DIR_PATH%GraphLite/color}
echo ${M_PATH}"GraphLite/output/"
rm -rf ${M_PATH}"GraphLite/output/"

cd $M_PATH;start-graphlite ${M_PATH}"GraphLite/color/Color.so" ${M_PATH}"GraphLite/part2-input/Color-graph0_4w" ${M_PATH}"GraphLite/output/color" 7
