#!/bin/bash

ROOT_PATH=`pwd`


#echo ROOT_PATH:${ROOT_PATH}

if [ $# == 1 ]
then
	CompBuilding=$1
else
	CompBuilding=all
fi

if [ ${CompBuilding} == toolchain ]
then
	echo building toolchain now!!!
	pushd ./toolchain
		./build_toolchain_3.4.x
	popd
	exit
fi



