#!/bin/sh

mkdir -p build
cd build && cmake .. && cmake --build . && cd ..

DLLS=`ldd ./build/psvita-opengl.exe | grep ucrt64 | cut -d " " -f 3`

if [ -z "$DLLS" ]
then
      echo "DLLs already copied"
else
      echo "Copying DLLs..."
      cp $DLLS build/
fi
