#!/usr/bin/env bash
set -e

# compile every .cpp into .o
for src in *.cpp; do
  obj="${src%.cpp}.o"
  echo "➜ Compiling $src → $obj"
  g++ -c "$src" -o "$obj" \
      -I.             # so #include "header.h" works if you have headers
done

echo "➜ $(basename "$(pwd)") module compiled."

