#!/bin/bash

# Check for the required project name
if [[ $# -lt 1 || ${1:0:1} == "-" ]]; then
  echo "Usage: $0 PROJECT_NAME [-l libdaisy_location] [-d daisysp_location]" >&2
  exit 1
fi

# Set the project name value
PROJECT="$1"

shift

# Parse command line options
while [[ $# -gt 0 ]]; do
  case $1 in
    -l)
      LIBDAISY_DIR="$2"
      shift 2
      ;;
    -d)
      DAISYSP_DIR="$2"
      shift 2
      ;;
    *)
      echo "Usage: $0 PROJECT_NAME [-l libdaisy_location] [-d daisysp_location]" >&2
      exit;
      ;;
  esac
done

# Default variable value
: "${LIBDAISY_DIR:="../../../libDaisy"}"
: "${DAISYSP_DIR:="../../../DaisySP"}"

# Make the folders if they don't exist
mkdir -p lgpl/
mkdir -p lgpl/resource

# copy .a files
cp $DAISYSP_DIR/build/libdaisysp.a lgpl/resource
cp $DAISYSP_DIR/DaisySP-LGPL/build/libdaisysp-lgpl.a lgpl/resource
cp $LIBDAISY_DIR/build/libdaisy.a lgpl/resource

# copy .o and .map files
cp build/*.o lgpl/resource
cp build/*.map lgpl/resource

# create the local makefile
makefile="# Project Name\n
TARGET = $PROJECT\n
\n
USE_DAISYSP_LGPL = 1\n
\n
# Library Locations\n
LIBDAISY_DIR ?= $LIBDAISY_DIR\n
DAISYSP_DIR ?= $DAISYSP_DIR\n
\n
# Core location, and generic Makefile.\n
SYSTEM_FILES_DIR = \$(LIBDAISY_DIR)/core\n
include \$(SYSTEM_FILES_DIR)/Makefile_LGPL"

echo -e $makefile > lgpl/Makefile

# make a little readme

readme="To rebuild with an updated or modified version of any of the static libraries:\n
1. Build the library, for example DaisySP-LGPL\n
2. Copy the generated .a file to the resources folder. In this case it would be libdaisy-lgpl.a\n
3. Run the make command. Your .bin file will be in the build folder"

echo -e $readme > lgpl/Readme.md