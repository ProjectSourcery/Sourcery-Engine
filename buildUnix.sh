#!/bin/bash

if [ -z $1 ] 
then
	BUILD_TYPE=Debug
else
	BUILD_TYPE=$1
	shift
fi

if [ -z $1 ] 
then
	COMPILER=clang++
else
	COMPILER=$1
	shift
fi

BUILD_DIR=Linux_$BUILD_TYPE

echo Usage: ./cmake_linux_clang_gcc.sh [Configuration] [Compiler]
echo "Possible configurations: Debug (default), Release, Distribution"
echo "Possible compilers: clang++, clang++-XX, g++, g++-XX where XX is the version"

mkdir -p build
cd build

cmake -S . -B $BUILD_DIR -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_CXX_COMPILER=$COMPILER "${@}"

echo Generating Makefile for build type \"$BUILD_TYPE\" and compiler \"$COMPILER\" in folder \"$BUILD_DIR\"

make -j 8 && make Shaders && ./SourceryEngine

cd ..