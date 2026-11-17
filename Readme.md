# Smart-HumTemp

**A smart WiFi temperature & humidity sensor for the LOLIN C3 Mini (ESP32-C3) and the Wemos D1 mini (ESP8266) – with web interface, OLED display, MQTT and Home Assistant auto discovery.**

Smart-HumTemp reads a DHT sensor (or, alternatively, the OpenWeatherMap API), calculates the dew point and publishes the values to your smart home. Everything is configured in the browser – no recompiling needed to set up WiFi, MQTT or sensor calibration.

---

## Features

- 🌡️ **Temperature, humidity, dew point and absolute humidity** – from a DHT11 / DHT21 / DHT22 sensor
- ☁️ **OpenWeatherMap as virtual sensor** – use outdoor weather data instead of a physical sensor
- 🎯 **Calibration** – offsets for temperature and humidity, configurable in the web UI
- 🖥️ **OLED display** (SH1106, 128×64) – start logo, access point info, large temperature and humidity pages
- 🚦 **RGB status LED** – shows the device state at a glance
- 🌐 **Web interface** – responsive UI (English / German) for status, configuration, backup and firmware update
- 📡 **MQTT** – state, availability (last will) and heartbeat messages
- 🏠 **Home Assistant auto discovery** – the device and all its sensors appear automatically in Home Assistant
- 🔄 **Firmware update over the air** via the web interface
- 📶 **Access point mode** for first setup – no hard coded WiFi credentials
- ⏱️ **NTP time synchronisation**

---

## Hardware

| Component | Description |
|-----------|-------------|
| Microcontroller | LOLIN C3 Mini (ESP32-C3, 4 MB flash) **or** Wemos D1 mini (ESP8266, 4 MB flash) |
| Sensor | DHT11 by default – DHT12 / DHT21 (AM2301) / DHT22 selectable in the web UI (build default: `-D LSC_DHT_DEFAULT_SENSOR=…` in `platformio.ini`) |
| Display *(optional)* | 1.3" OLED, SH1106 controller, 128×64, I²C (address `0x3C`) |
| Status LED *(optional)* | RGB LED (common cathode) |

### Wiring

Both boards have the same form factor – the connections are at the same header positions, so the same wiring (or shield) fits both.

| Header position | D1 mini (ESP8266) | C3 Mini (ESP32-C3) | Connected to |
|-----------------|-------------------|--------------------|--------------|
| `D4` | GPIO2  | GPIO6  | DHT data line |
| `D1` | GPIO5  | GPIO10 | Display SCL |
| `D2` | GPIO4  | GPIO8  | Display SDA |
| `D6` | GPIO12 | GPIO3  | Status LED – red |
| `D5` | GPIO14 | GPIO2  | Status LED – green |
| `D0` | GPIO16 | GPIO1  | Status LED – blue |
| `3V3` / `GND` | – | – | Power for sensor and display |

The pins are defined in [include/BoardConfig.h](include/BoardConfig.h) and can be overridden with build flags (e.g. `-D PIN_DHT_SENSOR=7`).

> **Tip:** Most DHT modules already have a pull-up resistor on the data line. If you use a bare sensor, add a 10 kΩ resistor between data and 3.3 V.

---

## Getting started

### 1. Flash the firmware

**Using a prebuilt binary:** ready-to-use firmware files are located in [bin/](bin/). Flash them with a tool of your choice (e.g. [esptool](https://github.com/espressif/esptool) or the ESP Web Flasher).

**Building from source:** see [Building from source](#building-from-source).

### 2. First setup

1. After the first start the device opens its own WiFi access point named **`LSC-xxxxxxxx`**.
2. Connect to this access point and open **<http://192.168.4.1>** in your browser.
3. Log in with the default password **`admin`** – *please change it right away* on the **Device** page.
4. Enter your WiFi credentials on the **WiFi** page and save. The device restarts and connects to your network.

### 3. Configure the sensor

On the **Sensor** page you can:

- choose the sensor type (DHT11 / DHT21 / DHT22) or **OpenWeatherMap**
- set offsets to calibrate temperature and humidity
- for OpenWeatherMap: enter your API key (App ID), latitude and longitude

---

## Home Assistant integration

Smart-HumTemp uses the **MQTT device discovery** of Home Assistant. Once MQTT is configured, the device appears automatically – no YAML needed.

1. Make sure the [MQTT integration](https://www.home-assistant.io/integrations/mqtt/) is set up in Home Assistant.
2. On the **MQTT** page of the device enter the broker address, port and credentials, set a topic prefix and enable **Home Assistant**.
3. The device publishes its discovery to `homeassistant/device/<hostname>/config`.

The following entities are created:

| Entity | Device class | Unit | Notes |
|--------|--------------|------|-------|
| Temperature | `temperature` | °C | long-term statistics enabled |
| Humidity | `humidity` | % | long-term statistics enabled |
| Dew point | `temperature` | °C | name can be changed in the configuration |
| Absolute humidity | `absolute_humidity` | g/m³ | water content of the air – useful to decide whether ventilating helps |
| WiFi signal | `signal_strength` | dBm | diagnostic entity |

Home Assistant converts the temperature to °F automatically, if your system uses imperial units.

### MQTT topics

| Topic | Content |
|-------|---------|
| `<prefix>/state` | JSON with the current sensor values (e.g. `{"sensor":{"temp_c":21.3,"hum":54.2,"dew_c":11.6,"abs_hum":10.1}}`) |
| `<prefix>/status` | Availability: `online` / `offline` (retained, last will) |
| `<prefix>/info` | Periodic heartbeat with device status (default every 60 s) |
| `<prefix>/cmd/#` | Commands to the device |

When Home Assistant restarts (birth message `online` on `homeassistant/status`), the device resends its discovery automatically.

---

## Building from source

### Requirements

- [Visual Studio Code](https://code.visualstudio.com/) with the [PlatformIO](https://platformio.org/) extension
- [Node.js](https://nodejs.org/) (for building the web pages)
- Git

### Clone and initialise

```bash
git clone --recurse-submodules https://github.com/LSC-Labs/Smart-HumTemp.git
cd Smart-HumTemp
npm install
```

If you cloned without `--recurse-submodules`, run the [initialize](initialize) script – it fetches the submodules (runtime library and wiki) and installs the npm packages.

### Build the web interface

The web UI is located in [src/web/](src/web/). It is minified, gzipped and compiled into C header files ([include/web/](include/web/)), so it is part of the firmware image.

```bash
npm run create-WebPages
```

Run this again whenever you change files in `src/web/` or the pages of the runtime library. The pages used by the project are listed in [pages.json](pages.json).

### Build and upload the firmware

| Environment | Purpose |
|-------------|---------|
| `lolin_c3_mini_debug` | **Default** – LOLIN C3 Mini, debug output on the serial console (USB, 115200 baud) |
| `lolin_c3_mini` | LOLIN C3 Mini – release build |
| `d1_mini_debug` | Wemos D1 mini – debug output on the serial console (115200 baud) |
| `d1_mini` | Wemos D1 mini – release build |

```bash
pio run                             # build the default (lolin_c3_mini_debug)
pio run -e lolin_c3_mini -t upload  # build and flash the C3 Mini release
pio run -e d1_mini -t upload        # build and flash the D1 mini release
pio device monitor                  # serial monitor
```

> **C3 Mini:** the board uses its native USB port for flashing and the serial console. If the upload does not start, hold the **BOOT** (`9`) button while pressing **RST** once to enter the download mode.

### Versioning

The version is maintained in [package.json](package.json) / [data/version.json](data/version.json) and compiled into the firmware ([include/ProgVersion.h](include/ProgVersion.h)). Use the npm scripts `incr-Build-Version`, `incr-Patch-Version`, `incr-Minor-Version` or `incr-Major-Version` to increase it.

---

## Project structure

```text
├── src/                    Application source code
│   ├── main.cpp            Setup – registers all modules
│   ├── CDHTSensor.cpp      Sensor module (DHT / OpenWeatherMap, dew point, HA discovery)
│   ├── CAppControl.cpp     Application logic – publishes the state via MQTT
│   ├── CDisplay*.cpp       OLED display and display pages
│   └── web/                Web interface (HTML, JS, CSS, i18n)
├── include/                Header files and generated web headers
├── lib/PLibESPV1/          ESP runtime library (git submodule)
├── scripts/                Build scripts (web pages, version, library patches)
├── tools/wsemulator/       WebSocket emulator to test the web UI without hardware
├── test/data/              Sample data for the emulator
├── bin/                    Prebuilt firmware binaries
└── platformio.ini          PlatformIO configuration
```

### Testing the web UI without hardware

The WebSocket emulator in [tools/wsemulator/](tools/wsemulator/) simulates the device, so you can work on the web interface in your browser:

```bash
cd tools/wsemulator
npm install
node wserver.js          # listens on ws://localhost:8080
```

---

## Built with

- [PLibESPV1](https://github.com/LSC-Labs/PLibESPV1) – modular ESP runtime (WiFi, web server, MQTT, configuration, security)
- [ESPAsyncWebServer](https://github.com/ESP32Async/ESPAsyncWebServer) & [ESPAsyncTCP](https://github.com/ESP32Async/ESPAsyncTCP)
- [AsyncMqttClient](https://github.com/marvinroger/async-mqtt-client)
- [Adafruit DHT sensor library](https://github.com/adafruit/DHT-sensor-library) & [Adafruit SH110X](https://github.com/adafruit/Adafruit_SH110X)
- [Bootstrap](https://getbootstrap.com/) for the web interface

More details about the runtime and the module concept can be found in the [ESP-ProjectTemplate wiki](https://github.com/LSC-Labs/ESP-ProjectTemplate/wiki).

---

## License

MIT © LSC Labs
