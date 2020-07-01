#!/bin/bash

# Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# This script builds iceoryx_utils und iceoryx_posh and executes all tests

set -e

#====================================================================================================
#==== Step 0 : Setup ================================================================================
#====================================================================================================

# The absolute path of the directory assigned to the build
WORKSPACE=$(git rev-parse --show-toplevel)
BUILD_DIR=$WORKSPACE/build

CLEAN_BUILD=false
BUILD_TYPE=""
STRICT_FLAG="off"
TEST_FLAG="off"
RUN_TEST=false
INTROSPECTION_FLAG="on"
DDS_GATEWAY_FLAG="on"
CYCLONEDDS_FLAG="$DDS_GATEWAY_FLAG"

while (( "$#" )); do
  case "$1" in
    -b|--builddir)
        BUILD_DIR=$(realpath $2)
        shift 2
        ;;
    "clean")
        CLEAN_BUILD=true
        shift 1
        ;;
    "release")
        BUILD_TYPE="Release"
        shift 1
        ;;
    "debug")
        BUILD_TYPE="Debug"
        shift 1
        ;;
    "test")
        BUILD_TEST=true
        TEST_FLAG="on"
        shift 1
        ;;
    "build-test")
        RUN_TEST=false
        TEST_FLAG="on"
        ;;  
    "skip-introspection")
        INTROSPECTION_FLAG="off"
        shift 1
        ;;
    "skip-dds")
        DDS_GATEWAY_FLAG="on"
        CYCLONEDDS_FLAG="on"
        shift 1
        ;;
    "help")
        echo "Build script for iceoryx."
        echo "By default, iceoryx and the examples are build."
        echo ""
        echo "Usage: iceoryx_build_test.sh [options] [<args>]"
        echo "Options:"
        echo "    -build                Specify a non-default build directory"
        echo "Args:"
        echo "    clean                 Cleans the build directory"
        echo "    release               Build release configuration"
        echo "    debug                 Build debug configuration"
        echo "    test                  Builds and runs the tests"
        echo "    build-test            Builds the tests (doesn't tun)"
        echo "    skip-introspection    Skips building iceoryx introspection"
        echo "    skip-dds              Skips building iceoryx dds gateway"
        echo "    help                  Prints this help"
        echo ""
        echo "e.g. iceoryx_build_test.sh -b /build/directory clean test release"
        exit 0
        ;;
    *)
        echo "Invalid argument '$arg'. Try 'help' for options."
        exit -1
        ;;
  esac
done

# define directories dependent on the build directory
ICEORYX_INSTALL_PREFIX=$BUILD_DIR/install/prefix/
DEPENDENCIES_INSTALL_PREFIX=$BUILD_DIR/dependencies/

echo " [i] Building in $BUILD_DIR"

#====================================================================================================
#==== Step 1 : Build  ===============================================================================
#====================================================================================================

# detect number of course if possible
NUM_CORES=1
if nproc >/dev/null 2>&1; then
    NUM_CORES=`nproc`
fi

# clean build folder
if [ $CLEAN_BUILD == true ]
then
    echo " [i] Cleaning build directory"
    cd $WORKSPACE
    rm -rf $BUILD_DIR/*
fi

# create a new build directory and change the current working directory
echo " [i] Create a new build directory and change the current working directory"
cd $WORKSPACE
mkdir -p $BUILD_DIR
cd $BUILD_DIR

echo " [i] Current working directory: $(pwd)"

echo ">>>>>> Start building iceoryx package <<<<<<"
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX=$ICEORYX_INSTALL_PREFIX -DTOML_CONFIG=on -Dtest=$TEST_FLAG -Droudi_environment=on -Dexamples=OFF -Dintrospection=$INTROSPECTION_FLAG -Ddds_gateway=$DDS_GATEWAY_FLAG -Dcyclonedds=$CYCLONEDDS_FLAG $WORKSPACE/iceoryx_meta
cmake --build . --target install
echo ">>>>>> finished building iceoryx package <<<<<<"

echo ">>>>>> Start building iceoryx examples <<<<<<"
cd $WORKSPACE/build
mkdir -p iceoryx_examples
echo ">>>>>>>> icedelivery"
cd $WORKSPACE/build/iceoryx_examples
mkdir -p icedelivery
cd icedelivery
cmake -DCMAKE_PREFIX_PATH=$ICEORYX_INSTALL_PREFIX $WORKSPACE/iceoryx_examples/icedelivery
cmake --build .
echo ">>>>>>>> iceperf"
cd $WORKSPACE/build/iceoryx_examples
mkdir -p iceperf
cd iceperf
cmake -DCMAKE_PREFIX_PATH=$ICEORYX_INSTALL_PREFIX $WORKSPACE/iceoryx_examples/iceperf
cmake --build .
echo ">>>>>> finished building iceoryx examples <<<<<<"

#====================================================================================================
#==== Step 2 : Run all Tests  =======================================================================
#====================================================================================================

if [ $TEST_FLAG == "on" ]
then

# The absolute path of the directory assigned to the build
cd $BUILD_DIR

# change the current working directory
mkdir -p tools
cp $WORKSPACE/tools/run_all_tests.sh $WORKSPACE/build/tools/run_all_tests.sh

echo " [i] Run all Tests:"
# call runAllTest shell script to run all tests for Iceoryx
$WORKSPACE/build/tools/run_all_tests.sh

for folder in $component_folder; do

    if [ ! -f testresults/"$folder"_ModuleTestResults.xml ]; then
        echo "xml:"$folder"_ModuletestTestResults.xml not found!"
        exit 1
    fi

    if [ ! -f testresults/"$folder"_ComponenttestTestResults.xml ]; then
        echo "xml:"$folder"_ComponenttestTestResults.xml not found!"
        exit 1
    fi

    if [ ! -f testresults/"$folder"_IntegrationTestResults.xml ]; then
        echo "xml:"$folder"_IntegrationTestResults.xml not found!"
        exit 1
    fi

done

fi
