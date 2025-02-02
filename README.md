# chip8
Simple chip8 emulator in C++. Requires [SDL3](https://github.com/libsdl-org/SDL/releases/tag/release-3.2.2).

To build on windows with [CMake](https://cmake.org/download/):
```
cmake . -D /build
cmake --build ./build --config release -DCMAKE_PREFIX_PATH=<SDL3-dir>
```

Run chip8 with a rom file:
```
./build/release/chip8 <path-to-rom>
```

A nice collection of chip8 roms can be found in the [Octojam archive](https://johnearnest.github.io/chip8Archive/)