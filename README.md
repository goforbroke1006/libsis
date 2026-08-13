# libsis

Library for work with [Symbian Installation Source](https://en.wikipedia.org/wiki/Symbian#Software_Installation_Script)
files *.sis *.sisx

## Build status

| Platform       | Status                                                                                     |
|----------------|--------------------------------------------------------------------------------------------|
| Ubuntu / GCC   | ![Ubuntu GCC](https://github.com/USER/REPO/actions/workflows/ubuntu-gcc.yml/badge.svg)     |
| Ubuntu / Clang | ![Ubuntu Clang](https://github.com/USER/REPO/actions/workflows/ubuntu-clang.yml/badge.svg) |
| Windows / MSVC | ![Windows MSVC](https://github.com/USER/REPO/actions/workflows/windows-msvc.yml/badge.svg) |

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

## Docs

https://www.cryer.co.uk/file-types/s/sis/softwareinstallsis.pdf
