#!/bin/bash

REBUILD=''
NVIDIA=''
NATIVE=''

while test $# -gt 0
do
    case "$1" in
        --rebuild) REBUILD='1'
            ;;
        --nvidia) NVIDIA="CONFIG += nvidia"
            ;;
        --native) NATIVE="CONFIG += native"
            ;;
        --*) echo "bad option $1"; exit 1
            ;;
        *) echo "unrecognised argument $1"; exit 1
            ;;
    esac
    shift
done

DIR=$( cd "$(dirname "${BASH_SOURCE[0]}" )" && pwd )

for PROJ in ${DIR}/{Ember,EmberCL,EmberGenome,EmberRender,EmberAnimate,Fractorium}
do
  pushd $PROJ
  if [ "x1" = "x$REBUILD" ]; then
    make clean
  fi
  qmake "$NVIDIA" "$NATIVE"
  make -j9
  if [ "x$?" != "x0" ]; then
    echo "Build failed! Check output for errors."
    exit 1
  fi
  popd
done

