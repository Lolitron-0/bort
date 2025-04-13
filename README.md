# bort

**bort** is a cross-platform Small-C language cross compiler for RISC-V architecture.

## Global TODOs
- switch
- compound variable declaration (`int a = 5, b, c = d;`)
- testing

## Build
bort uses cmake build system, so building pipeline is pretty standard. All dependencies are downloaded during cmake configuration via `FetchContent` (~1G by now).

```bash
git clone https://github.com/Lolitron-0/bort
cd bort
cmake -S . -B build
сmake --build build
```

To build bort with tests use `cmake -S . -B build -DBORT_BUILD_TESTS=ON` as configuration command (requires `python3` to be installed).

## Usage
After building, you can find binaries directly in build directory. You can start with:
```bash
./build/bort -h
```
To explore available command line options.
