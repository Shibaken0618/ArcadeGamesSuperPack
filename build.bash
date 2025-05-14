#!/usr/bin/env bash
set -e

# 1) Clean & prepare
rm -rf build
mkdir build

# 2) Compile the main launcher
echo "➜ Compiling ArcadeMain.cpp → ArcadeMain.o"
g++ -c src/ArcadeMain.cpp -o build/ArcadeMain.o \
    -Isrc

# 3) Invoke each module’s build and collect .o files
for module in pongGame snakeGame asteroidsGame conwayGame spaceInvadersGame; do
  echo "=== Building $module ==="
  (cd src/$module && bash build.bash)
  cp src/$module/*.o build/
done

# 4) Link everything into one binary
echo "➜ Linking all .o → ArcadeGames"
g++ build/*.o -o ArcadeGames \
    -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

echo "Built ./ArcadeGames"
