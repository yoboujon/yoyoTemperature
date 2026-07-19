# Yoyo Temperature

[Website here](https://climat.etheryo.fr/)

Launch `setup.sh` to create the basic environment variables needed for the project. (Later, a config file will be given to the service that will configurate the ESP32 automatically.)

You will need the following packages:
```
sudo pacman -S npm sqlite
```

For ESPIDF, you can either install the vscode extension that will automate everything for you. Or download the Espressif tools [here](https://github.com/espressif/esp-idf).

## Build the Server

After building using CMake with the following commands:
```
cd server
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```
You can copy the created executable `yoyo_temperature` to `/opt`. Then start the service `yoyo-temperature` and the server will be served.