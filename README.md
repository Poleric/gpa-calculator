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

For compiling and building on Windows, it's recommended to use `vcpkg` to manage library.

- Installing vcpkg

```cmd
> git clone https://github.com/microsoft/vcpkg
> cd vcpkg
> bootstrap-vcpkg.bat
```

- Installing required dependencies

```cmd
> vcpkg install pdcurses:x64-windows
> vcpkg install gettext:x64-windows
> vcpkg integrate install
```
