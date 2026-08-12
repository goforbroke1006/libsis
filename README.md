# libsis

Library for work with [Symbian Installation Source](https://en.wikipedia.org/wiki/Symbian#Software_Installation_Script) files *.sis *.sisx

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
