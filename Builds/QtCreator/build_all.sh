#!/bin/bash

DIR=$( cd "$(dirname "${BASH_SOURCE[0]}" )" && pwd )

for PROJ in ${DIR}/{Ember,EmberCL,EmberGenome,EmberRender,EmberAnimate,Fractorium}
do
  pushd $PROJ
  if [ "x--rebuild" = "x$1" ]; then
    make clean
  fi
  qmake
  make -j9
  if [ "x$?" != "x0" ]; then
    echo "Build failed! Check output for errors."
    exit 1
  fi
  popd
done

