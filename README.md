# EPD
C driver for EPD panels

## Driver Design
The driver model is linux-ish: each driver defines an API that platform needs to implement. Functions such as GPIO, SPI, time, are then abstracted away from the driver itself, making it easier to add new platforms or port the driver for use in existing code.

The driver is designed to be customized for a specific platform using a BSP. BSPs live in the `platforms` directory. Every platform should define the minimum required functionality (console, gpio, spi, time) and bind them by declaring a struct (defined in each driver's header file). The platform file should also define any platform-specific pins that are required. See the example platform (the Adafruit RP2040 THINKINK using pico-sdk) for more info.

Panels follows the same pattern: each panel includes the top-level `panel.h` header, and then is responsible for implementing the panel API it defines. Panels should not use any platform-specific code (GPIO, SPI, printf), but rather the driver implementations of each so that they can be platform-agnostic and easily ported to different platforms. See the `E2B98FS081` for an example.

## Building
```
mkdir -p build && cd build
cmake -DPICO_BOARD="adafruit_rp2040_thinkink" .. && make -j$(nproc)
```
