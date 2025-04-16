#!/bin/bash

echo "Cleaning build and output files..."

# Rimuove la cartella di build
rm -rf build

# Rimuove cartelle binarie e oggetti (create da CMakeLists.txt)
rm -rf src/bin
rm -rf src/obj

echo "Done."
