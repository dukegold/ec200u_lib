.. _installation:

============
Installation
============

This guide will walk you through installing the QuectelEC200U library in your Arduino development environment.

Prerequisites
=============

Before installing the library, ensure you have:

**Hardware Requirements:**

* ESP32 development board (any variant)
* Quectel EC200U modem module
* Appropriate power supply for the modem (3.3V-4.2V, minimum 2A capability)
* UART connection cables
* (Optional) GPS antenna for positioning features
* (Optional) LTE antenna for cellular connectivity

**Software Requirements:**

* Arduino IDE 1.8.x or higher (or PlatformIO)
* ESP32 board support package installed
* Serial monitor or terminal program

Installing ESP32 Board Support
===============================

If you haven't already installed ESP32 support in Arduino IDE:

1. Open Arduino IDE
2. Go to **File** → **Preferences**
3. Add the following URL to **Additional Boards Manager URLs**:

   .. code-block:: text

      https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json

4. Go to **Tools** → **Board** → **Boards Manager**
5. Search for "ESP32"
6. Install "ESP32 by Espressif Systems"

Library Installation Methods
=============================

Method 1: Manual Installation (Recommended)
--------------------------------------------

1. Download the library files:

   * ``QuectelEC200U.h``
   * ``QuectelEC200U.cpp``

2. Create a new folder in your Arduino libraries directory:

   * **Windows**: ``Documents\Arduino\libraries\QuectelEC200U\``
   * **macOS**: ``~/Documents/Arduino/libraries/QuectelEC200U/``
   * **Linux**: ``~/Arduino/libraries/QuectelEC200U/``

3. Copy the files into this folder:

   .. code-block:: text

      Arduino/
      └── libraries/
          └── QuectelEC200U/
              ├── QuectelEC200U.h
              ├── QuectelEC200U.cpp
              └── examples/
                  └── QuectelEC200U_Example/
                      └── QuectelEC200U_Example.ino

4. Restart Arduino IDE

5. Verify installation by going to **File** → **Examples** → **QuectelEC200U**

Method 2: ZIP Library Installation
-----------------------------------

1. Download the library as a ZIP file
2. Open Arduino IDE
3. Go to **Sketch** → **Include Library** → **Add .ZIP Library**
4. Select the downloaded ZIP file
5. The library will be installed automatically

Method 3: PlatformIO Installation
----------------------------------

For PlatformIO users, add to your ``platformio.ini``:

.. code-block:: ini

    [env:esp32dev]
    platform = espressif32
    board = esp32dev
    framework = arduino
    lib_deps =
        QuectelEC200U
    monitor_speed = 115200

Or install via PlatformIO CLI:

.. code-block:: bash

    pio lib install "QuectelEC200U"

Hardware Connections
====================

Basic UART Connection
---------------------

Connect the EC200U modem to your ESP32:

.. code-block:: text

    EC200U          ESP32
    ------          -----
    TX      →       RX (GPIO16)
    RX      ←       TX (GPIO17)
    GND     ---     GND
    VCC     ---     3.3V-4.2V (2A supply)

**Important Notes:**

* The EC200U requires a stable power supply capable of providing at least 2A peak current
* Do NOT power the modem directly from ESP32's 3.3V pin (insufficient current)
* Use a separate power supply or a high-current voltage regulator

Recommended Connection Diagram
-------------------------------

.. code-block:: text

    +----------------+          +----------------+
    |                |          |                |
    |     ESP32      |          |    EC200U      |
    |                |          |                |
    |          GPIO16|<---------|TX              |
    |          GPIO17|--------->|RX              |
    |            GND |----------| GND            |
    |                |          |                |
    +----------------+          |                |
                                | VCC            |
                                |                |
                                +-------+--------+
                                        |
                                        | 3.3V-4.2V
                                        | (2A capable)
                                        |
                                  +-----+-----+
                                  |           |
                                  |  Power    |
                                  |  Supply   |
                                  |           |
                                  +-----------+

Additional Connections
----------------------

**For GPS/GNSS functionality:**

.. code-block:: text

    EC200U
    ------
    GPS_ANT --- GPS Antenna (active or passive)

**For LTE connectivity:**

.. code-block:: text

    EC200U
    ------
    MAIN_ANT --- LTE Main Antenna
    DIV_ANT  --- LTE Diversity Antenna (optional)

**For SIM card:**

.. code-block:: text

    EC200U
    ------
    SIM_VDD  --- SIM Card VDD
    SIM_DATA --- SIM Card DATA
    SIM_CLK  --- SIM Card CLK
    SIM_RST  --- SIM Card RST
    SIM_DET  --- SIM Card DETECT (optional)

Verifying Installation
======================

1. **Test Sketch**

   Create a simple test sketch to verify the installation:

   .. code-block:: cpp

      #include <QuectelEC200U.h>

      HardwareSerial modemSerial(1);
      QuectelEC200U modem(&modemSerial, 115200);

      void setup() {
          Serial.begin(115200);
          Serial.println("QuectelEC200U Test");

          // Initialize modem serial
          modemSerial.begin(115200, SERIAL_8N1, 16, 17);

          // Initialize modem
          if (modem.begin()) {
              Serial.println("✓ Library installed correctly");
              Serial.println("✓ Modem communication established");

              // Get modem info
              String imei;
              if (modem.getIMEI(imei)) {
                  Serial.print("✓ Modem IMEI: ");
                  Serial.println(imei);
              }
          } else {
              Serial.println("✗ Failed to initialize modem");
              Serial.println("Check connections and power supply");
          }
      }

      void loop() {
          // Empty
      }

2. **Upload and Monitor**

   * Select your ESP32 board: **Tools** → **Board** → **ESP32 Dev Module**
   * Select the correct port: **Tools** → **Port**
   * Upload the sketch
   * Open Serial Monitor at 115200 baud
   * You should see successful initialization messages

Troubleshooting Installation
=============================

**Library Not Found**

If you get "No such file or directory" error:

* Verify files are in correct location
* Restart Arduino IDE
* Check folder names (case-sensitive on Linux/macOS)

**Compilation Errors**

If you get compilation errors:

* Ensure ESP32 board package is installed
* Verify you've selected an ESP32 board
* Check that both .h and .cpp files are present

**Modem Not Responding**

If the modem doesn't respond:

* Check TX/RX connections (try swapping them)
* Verify power supply provides sufficient current
* Ensure modem is properly powered (check LED indicators)
* Try different baud rates (9600, 115200)

**Serial Port Issues**

For serial port problems:

* Install appropriate drivers:

  - **CP2102**: Silicon Labs CP210x drivers
  - **CH340**: CH340 drivers
  - **FTDI**: FTDI VCP drivers

* On Linux, add user to dialout group:

  .. code-block:: bash

      sudo usermod -a -G dialout $USER

* On macOS, check for /dev/cu.* devices

Configuration for Different ESP32 Boards
=========================================

**ESP32 DevKit V1**

.. code-block:: cpp

    // Use UART1 on default pins
    HardwareSerial modemSerial(1);
    modemSerial.begin(115200, SERIAL_8N1, 16, 17);

**ESP32-WROOM-32**

.. code-block:: cpp

    // Use UART2 with custom pins
    HardwareSerial modemSerial(2);
    modemSerial.begin(115200, SERIAL_8N1, 26, 27);

**ESP32-C3**

.. code-block:: cpp

    // ESP32-C3 has different pin mapping
    HardwareSerial modemSerial(1);
    modemSerial.begin(115200, SERIAL_8N1, 4, 5);

**ESP32-S3**

.. code-block:: cpp

    // ESP32-S3 with USB CDC
    HardwareSerial modemSerial(1);
    modemSerial.begin(115200, SERIAL_8N1, 18, 17);

Power Supply Recommendations
=============================

The EC200U modem requires careful power supply design:

**Minimum Requirements:**

* Voltage: 3.3V - 4.2V
* Current: 2A peak, 500mA average
* Low ripple: < 50mV
* Fast transient response

**Recommended Solutions:**

1. **Dedicated LDO Regulator**

   Use a high-current LDO like AMS1117-3.3 with adequate capacitors:

   .. code-block:: text

      5V ---[AMS1117-3.3]--- 3.3V/2A --- EC200U
              |        |
            100uF   470uF

2. **Buck Converter**

   For battery-powered applications, use an efficient buck converter

3. **Laboratory Power Supply**

   For development, use a bench power supply set to 3.8V with 2A current limit

Next Steps
==========

After successful installation:

* Review the :doc:`getting_started` guide
* Explore the :doc:`examples`
* Consult the :doc:`api_reference` for detailed documentation
* Check :doc:`troubleshooting` if you encounter issues