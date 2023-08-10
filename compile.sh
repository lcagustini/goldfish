#!/bin/bash

mkdir -p build
cd build && cmake .. && cmake --build . && cd ..

DLLS=`ldd ./build/psvita-opengl.exe | grep ucrt64 | cut -d " " -f 3`

if [[ $* == *--dll* ]]
then
    if [ -z "$DLLS" ]
    then
        echo "No DLL to copy"
    else
        echo "Copying DLLs..."
        cp $DLLS build/
    fi
fi

if [[ $* == *--zip* ]]
then
    zip -j ./build/standalone.zip $DLLS ./build/psvita-opengl.exe
    zip -r ./build/standalone.zip assets/
fi

if [[ $* == *--run* ]]
then
    ./build/psvita-opengl.exe
fi

if [[ $* == *--debug* ]]
then
    gdb ./build/psvita-opengl.exe
fi
