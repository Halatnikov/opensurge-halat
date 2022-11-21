mkdir -p build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../src/misc/toolchain-mingw.cmake -DCMAKE_SYSROOT=$ALLEGRO_DEPS_FOLDER -DALLEGRO_STATIC=ON -DALLEGRO_MONOLITH=ON -DSURGESCRIPT_STATIC=ON
make
cd ~
exec bash