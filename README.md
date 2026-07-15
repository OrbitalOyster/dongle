# Dongle
## Prerequisites
* git
* cmake
* clang
## Building
```bash
git clone https://github.com/OrbitalOyster/dongle.git
cd dongle
cmake -B build -DCMAKE_CXX_COMPILER=clang++
# To generate compile_commands.json (for editor hints):
bear -- cmake --build build
cmake --build build
```
## Running
```bash
./build/dongle
```
