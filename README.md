# bort

**bort** is a cross-platform Small-C language cross compiler for RISC-V architecture.

## Build
bort uses cmake build system, so building pipeline is pretty standard. All dependencies are downloaded during cmake configuration via `FetchContent` (~1G by now).

```bash
git clone https://github.com/Lolitron-0/bort
cd bort
cmake -S . -B build 
```

## Usage
After building, you can find binaries directly in build directory. You can start with:
```bash
./build/bort -h
```
To explore available command line options.
