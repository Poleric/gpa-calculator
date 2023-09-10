# GPA Calculator
A GPA calculator and interface written in C.

## Building

### Linux

Requires ncurses and CMake

#### Debian

- Installing ncurses

```shell
sudo apt-get update
sudo apt-get install libncurses5-dev libncursesw5-dev
```

### Windows

For compiling and building on Windows, it's recommended to use `vcpkg` to manage library, in this case for `pdcurses`.

```cmd
vcpkg install pdcurses:x64-windows
vcpkg integrate install
```
