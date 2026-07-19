# CMake Fetch example

![Static Badge](https://img.shields.io/badge/license-CeCILL_C-lightgrey)
![Static Badge](https://img.shields.io/badge/builds_on-Linux-brightgreen)
![Static Badge](https://img.shields.io/badge/builds_on-Windows-brightgreen)


Here is an example of the cmake fetch if the `find_package()` cannot be used. Fetch is useful when building custom version of popular libraries.

## Building

In the root directory launch the following:
```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## Service

```
sudo cp yoyo-temperature.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable yoyo-temperature
sudo systemctl start yoyo-temperature
```
