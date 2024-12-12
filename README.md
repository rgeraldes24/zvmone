# zvmone

[![ethereum badge]][ethereum]
[![readme style standard badge]][standard readme]
[![codecov badge]][codecov]
[![circleci badge]][circleci]
[![appveyor badge]][appveyor]
[![license badge]][Apache License, Version 2.0]

> Fast Zond Virtual Machine implementation

_zvmone_ is a C++ implementation of the Zond Virtual Machine (ZVM). 
Created by members of the [Ipsilon] (ex-[Ewasm]) team, the project aims for clean, standalone ZVM implementation 
that can be imported as an execution module by Zond Client projects. 
The codebase of _zvmone_ is optimized to provide fast and efficient execution of ZVM smart contracts.

### Characteristic of zvmone

1. Exposes the [ZVMC] API.
2. Requires C++20 standard.
3. The [intx] library is used to provide 256-bit integer precision.
4. The [ethash] library is used to provide Keccak hash function implementation
   needed for the special `KECCAK256` instruction.
5. Contains two interpreters: 
   - **Baseline** (default)
   - **Advanced** (select with the `advanced` option)

### Baseline Interpreter

1. Provides relatively straight-forward but efficient ZVM implementation.
2. Performs only minimalistic `JUMPDEST` analysis.

### Advanced Interpreter

1. The _indirect call threading_ is the dispatch method used -
   a loaded ZVM program is a table with pointers to functions implementing virtual instructions.
2. The gas cost and stack requirements of block of instructions is precomputed 
   and applied once per block during execution.
3. Performs extensive and expensive bytecode analysis before execution.


## Usage

### As gzond plugin

zvmone implements the [ZVMC] API for Zond Virtual Machines.
It can be used as a plugin replacing gzond's internal ZVM. But for that a modified
version of gzond is needed. The [Ewasm]'s fork
of go-zond provides [binary releases of gzond with ZVMC support](https://github.com/ewasm/go-ethereum/releases).

Next, download zvmone from [Releases].

Start the downloaded gzond with `--vm.zvm` option pointing to the zvmone shared library.

```bash
gzond --vm.zvm=./libzvmone.so
```

### Building from source

To build the zvmone ZVMC module (shared library), test, and benchmark:

1. Fetch the source code:
   ```
   git clone --recursive https://github.com/theqrl/zvmone
   cd zvmone
   ```

2. Configure the project build and dependencies:
   ##### Linux / OSX
   ```
   cmake -S . -B build -DZVMONE_TESTING=ON
   ```

   ##### Windows
   ```
   cmake -S . -B build -DZVMONE_TESTING=ON -G "Visual Studio 16 2019" -A x64
   ```
   
3. Build:
   ```
   cmake --build build --parallel
   ```


3. Run the unit tests or benchmarking tool:
   ```
   build/bin/zvmone-unittests
   build/bin/zvmone-bench test/zvm-benchmarks/benchmarks
   ```

### Tools

#### zvm-test

The **zvm-test** executes a collection of unit tests on 
any ZVMC-compatible Zond Virtual Machine implementation.
The collection of tests comes from the zvmone project.

```bash
zvm-test ./zvmone.so
```

### Docker

Docker images with zvmone are available on Docker Hub:
https://hub.docker.com/r/ethereum/zvmone.

Having the zvmone shared library inside a docker is not very useful on its own,
but the image can be used as the base of another one or you can run benchmarks 
with it.

```bash
docker run --entrypoint zvmone-bench ethereum/zvmone /src/test/benchmarks
```

## References

1. [Efficient gas calculation algorithm for ZVM](docs/efficient_gas_calculation_algorithm.md)

## Maintainer

Paweł Bylica [@chfast]

## License

[![license badge]][Apache License, Version 2.0]

Licensed under the [Apache License, Version 2.0].


[@chfast]: https://github.com/chfast
[appveyor]: https://ci.appveyor.com/project/chfast/evmone/branch/master
[circleci]: https://circleci.com/gh/ethereum/evmone/tree/master
[codecov]: https://codecov.io/gh/ethereum/evmone/
[Apache License, Version 2.0]: LICENSE
[ethereum]: https://ethereum.org
[ZVMC]: https://github.com/ethereum/evmc
[Ipsilon]: https://github.com/ipsilon
[Ewasm]: https://github.com/ewasm
[intx]: https://github.com/chfast/intx
[ethash]: https://github.com/chfast/ethash
[Releases]: https://github.com/ethereum/evmone/releases
[standard readme]: https://github.com/RichardLitt/standard-readme

[appveyor badge]: https://img.shields.io/appveyor/ci/chfast/evmone/master.svg?logo=appveyor
[circleci badge]: https://img.shields.io/circleci/project/github/ethereum/evmone/master.svg?logo=circleci
[codecov badge]: https://img.shields.io/codecov/c/github/ethereum/evmone.svg?logo=codecov
[ethereum badge]: https://img.shields.io/badge/ethereum-EVM-informational.svg?logo=ethereum
[license badge]: https://img.shields.io/github/license/ethereum/evmone.svg?logo=apache
[readme style standard badge]: https://img.shields.io/badge/readme%20style-standard-brightgreen.svg
