#! /bin/bash

BIN_DIR="$(pwd)/install/bin"
LIB_DIR="$(pwd)/install/lib"
INCLUDE_DIR="$(pwd)/install/include"

# check dir
if [ -d "$BIN_DIR" ]; then
    # add path
    export PATH="$BIN_DIR:$PATH"
    #echo "Added $BIN_DIR to PATH"
else
    echo "Directory $BIN_DIR does not exist."
fi

if [ -d "$LIB_DIR" ]; then
    # add ld
    export LD_LIBRARY_PATH="$LIB_DIR:$LD_LIBRARY_PATH"
    #echo "Added $LIB_DIR to LD_LIBRARY_PATH"
else
    echo "Directory $LIB_DIR does not exist."
fi

if [ -d "$INCLUDE_DIR" ]; then
    export CPLUS_INCLUDE_PATH="$INCLUDE_DIR:$CPLUS_INCLUDE_PATH"
else
    echo "Directory $INCLUDE_DIR does not exist."
fi
