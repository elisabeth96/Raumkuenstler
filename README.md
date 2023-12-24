# Raumkünstler
[![Linux](https://github.com/elisabeth96/Raumkuenstler/actions/workflows/cmake-single-platform.yml/badge.svg)](https://github.com/elisabeth96/Raumkuenstler/actions/workflows/cmake-single-platform.yml)

Raumkünstler is a little side project I have been working on to tinker with implicit modelling. It's a playground for exploring procedural modelling and fast implicit meshing.
![Smooth Union of a Box and a Torus. The box is animated.](Raumkuenstler.gif)

### Features

- **Node-Based Modeling**: Design procedural geometry via a node-based visual scripting language.
- **Implicit Modeling Approach**: Instead of an explicit boundary representation Raumkünstler describes shapes implicitly using formulas.
- **LLVM-Powered JIT Compilation**: We use LLVM to turn the node graph into a native function for blazing fast performance.
- **Fast Dual Contouring Implementation**: Raumkünstler uses a fast dual contouring implementation using empty space skipping and support for sharp features.

## Building

Raumkünstler uses a standard cmake build. The only required dependancy which is not automatically fetched is LLVM. On Mac you can install LLVM via brew
   ```bash
   brew intall llvm
   ```
On Ubuntu it can installed via apt
   ```bash
   sudo apt-get install libllvm-dev
   ```
Once llvm is installed, you can grab that code

   ```bash
   git clone https://github.com/elisabeth96/Raumkuenstler.git
   ```
Then setup an out of tree build
   ```bash
   cd Raumkuenstler && mkdir build && cd build
   ```
and finally build via cmake/make
   ```bash
   cmake .. -DCMAKE_BUILD_TYPE=Release
   make- j
   ```
