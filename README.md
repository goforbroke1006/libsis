# libsis

Library for work with [Symbian Installation Source](https://en.wikipedia.org/wiki/Symbian#Software_Installation_Script)
files *.sis *.sisx

## Build status

| Platform       | Status                                                                                                 |
|----------------|--------------------------------------------------------------------------------------------------------|
| Ubuntu / GCC   | ![Ubuntu GCC](https://github.com/goforbroke1006/libsis/actions/workflows/ubuntu-gcc.yml/badge.svg)     |
| Ubuntu / Clang | ![Ubuntu Clang](https://github.com/goforbroke1006/libsis/actions/workflows/ubuntu-clang.yml/badge.svg) |
| Windows / MSVC | ![Windows MSVC](https://github.com/goforbroke1006/libsis/actions/workflows/windows-msvc.yml/badge.svg) |

## Usage

### As Cmake subdirectory

```shell
git modules init
git modules add https://github.com/goforbroke1006/libsis.git ./third_party/libsis
```

```cmake
add_subdirectory(./third_party/libsis)

# ...

target_link_libraries(${PROJECT_NAME} PRIVATE sis)
```

### Install

```shell
cmake -S . -B build \
    -DBUILD_TESTING=OFF \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local

cmake --build build -j$(nproc)

sudo cmake --install build

# Обновить кэш динамических библиотек
sudo ldconfig

# ...

# sudo cmake --build build --target uninstall
```

## Docs

https://www.cryer.co.uk/file-types/s/sis/softwareinstallsis.pdf
