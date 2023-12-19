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
cp $DAISYSP_DIR/build/libdaisysp.a lgpl/resource || { echo 'Invalid libDaisy location. Try -l flag' ; exit 1; }
cp $DAISYSP_DIR/DaisySP-LGPL/build/libdaisysp-lgpl.a lgpl/resource || { echo 'Invalid DaisySP location. Try -d flag' ; exit 1; }
cp $LIBDAISY_DIR/build/libdaisy.a lgpl/resource || { echo 'Invalid DaisySP location. Try -d flag' ; exit 1; }

# copy bootloader files
cp $LIBDAISY_DIR/core/dsy_bootloader* lgpl/resource || { echo 'Invalid libDaisy location. Try -l flag' ; exit 1; }
cp $LIBDAISY_DIR/core/STM32H750IB_flash.lds lgpl/resource || { echo 'Invalid libDaisy location. Try -l flag' ; exit 1; }
cp $LIBDAISY_DIR/core/STM32H750IB_sram.lds lgpl/resource || { echo 'Invalid libDaisy location. Try -l flag' ; exit 1; }
cp $LIBDAISY_DIR/core/STM32H750IB_qspi.lds lgpl/resource || { echo 'Invalid libDaisy location. Try -l flag' ; exit 1; }

# copy and prepend vars to local makefile
cp $LIBDAISY_DIR/core/Makefile_LGPL lgpl/Makefile || { echo 'Invalid libDaisy location. Try -l flag' ; exit 1; }

prepend="# Project Name\nTARGET = $PROJECT\n"
sed -i -e "1i $prepend" lgpl/Makefile

# copy .o and .map files
cp build/*.o lgpl/resource
cp build/*.map lgpl/resource

# make a little readme
readme="To rebuild with an updated or modified version of any of the static libraries:\n
1. Build the library you would like to replace, for example by running make inside of DaisySP-LGPL.\n
2. Copy the generated .a file to the resources folder. In this case it would be libdaisy-lgpl.a \n
3. Run the make command in this folder. Your .bin file will be in the build folder. "

echo -e $readme > lgpl/Readme.md