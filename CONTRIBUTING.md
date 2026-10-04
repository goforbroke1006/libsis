# libsis

## Preparation

```shell
cmake -S . -B build-debug \
    -DBUILD_TESTING=OFF \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_INSTALL_PREFIX=/usr/local

cmake --build build-debug -j$(nproc)

sudo cmake --install build-debug

# Обновить кэш динамических библиотек
sudo ldconfig

# ...

# sudo cmake --build build-debug --target uninstall
```
