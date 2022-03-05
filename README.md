# wifi_reconnect

[![test](https://github.com/mdvorak/esp-wifi-reconnect/actions/workflows/test.yml/badge.svg)](https://github.com/mdvorak/esp-wifi-reconnect/actions/workflows/test.yml)

Manages Wi-Fi connection, with incremental back-off.

## Usage

To reference this library by your project, add it as git submodule, using command

```shell
git submodule add https://github.com/mdvorak/esp-wifi-reconnect.git components/wifi_reconnect
```

and include either of the header files

```c
#include <wifi_reconnect.h>
```

For full example, see [wifi_reconnect_example_main.c](./example/main/wifi_reconnect_example_main.c).

## Development

Prepare [ESP-IDF development environment](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html#get-started-get-prerequisites)
.

Configure example application with

```
cd example/
idf.py menuconfig
```

Flash it via (in the example dir)

```
idf.py build flash monitor
```

As an alternative, you can use [PlatformIO](https://docs.platformio.org/en/latest/core/installation.html) to build and
flash the example project.
