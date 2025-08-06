#!/bin/bash

echo "Cleaning..."
./clean.sh

echo "Rebuilding..."
mkdir -p ../build
cd ../build
cmake ../OpenMP_src
make
