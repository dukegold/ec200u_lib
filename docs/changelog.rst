.. _changelog:

=========
Changelog
=========

All notable changes to the QuectelEC200U library will be documented in this file.

The format is based on `Keep a Changelog <https://keepachangelog.com/en/1.0.0/>`_,
and this project adheres to `Semantic Versioning <https://semver.org/spec/v2.0.0.html>`_.

[1.0.0] - 2024-11-26
====================

Initial Release
---------------

Added
^^^^^

**Core Features:**

* Complete ESP32 Arduino library for Quectel EC200U modem
* Comprehensive AT command response handling
* Automatic error recovery and retry mechanisms
* Debug output control with ``QUECTEL_DEBUG`` macro

**GPS/GNSS Functions:**

* ``gnssBegin()`` - Initialize GNSS module
* ``gnssOn()`` / ``gnssOff()`` - Control GNSS power
* ``getPosition()`` - Get complete position data with auto-retry
* ``getCoordinates()`` - Simple latitude/longitude retrieval
* ``isGNSSFixed()`` - Check GPS fix status
* Support for multiple coordinate formats:

  - Degrees and minutes (``ddmm.mmmmN/S``)
  - Decimal degrees (``±dd.ddddd``)
  - Extended precision (``ddmm.mmmmmm``)

* Complete NMEA data parsing (time, altitude, speed, satellites)
* Automatic session management with error recovery

**SSL/HTTPS Functions:**

* ``sslBegin()`` - Initialize SSL with PDP context activation
* ``sslConfigure()`` - Configure SSL parameters
* ``httpsConnect()`` - Establish secure connections
* ``httpsSend()`` / ``httpsSendBytes()`` - Send data
* ``httpsReceive()`` - Receive data with buffering
* ``httpsDataAvailable()`` - Check receive buffer
* ``exitTransparentMode()`` - Proper escape sequence handling
* ``httpsDisconnect()`` - Clean connection closure
* ``httpsGET()`` / ``httpsPOST()`` - HTTP helper methods
* Support for three access modes:

  - Buffer mode (controlled data flow)
  - Direct push mode (immediate data)
  - Transparent mode (streaming)

**Time Functions:**

* ``getNetworkTime()`` - Get network synchronized time
* ``getCurrentTime()`` - Simple time string retrieval
* ``setRTCTime()`` / ``getRTCTime()`` - RTC management
* ``syncTimeFromNetwork()`` - Automatic time synchronization
* Support for multiple time modes:

  - Last synchronized time
  - Current GMT time
  - Current local time

* Timezone handling (quarters of hour from GMT)
* Daylight saving time support

**Error Handling:**

* Complete CME error code handling (50+ codes)
* Complete SSL error code handling (30+ codes)
* ``getErrorDescription()`` - Human-readable error messages
* ``getLastSSLError()`` - Detailed SSL error information
* Structured error codes in enums
* Error codes in data structures for debugging

**Utility Functions:**

* ``testAT()`` - Modem communication test
* ``reset()`` - Software reset
* ``getSignalQuality()`` - RSSI and BER metrics
* ``getIMEI()`` - Retrieve modem IMEI
* ``getNetworkStatus()`` - Network registration status
* ``setTimeout()`` / ``getTimeout()`` - Timeout management
* ``clearBuffer()`` - Serial buffer management
* ``sendRawATCommand()`` - Direct AT command access

**Data Structures:**

* ``GNSSPosition`` - Complete GPS data structure
* ``NetworkTime`` - Time and date information
* ``SSLConnectionState`` - Connection status tracking
* ``SSLReceiveData`` - Received data information

**Documentation:**

* Comprehensive API reference
* Getting started guide
* Installation instructions
* Multiple working examples
* Error code reference
* Troubleshooting guide
* Sphinx documentation ready

Known Issues
^^^^^^^^^^^^

* Transparent mode exit may occasionally require hardware reset
* AGPS configuration requires additional AT commands
* Some networks may not provide time synchronization service

Roadmap
=======

Future Versions
---------------

[1.1.0] - Planned
^^^^^^^^^^^^^^^^^

**Planned Features:**

* SMS support (send/receive)
* Voice call support
* FTP/FTPS client
* MQTT client integration
* Power saving modes
* Firmware update support
* Extended GPS features (AGPS, geofencing)

[1.2.0] - Planned
^^^^^^^^^^^^^^^^^

**Planned Features:**

* HTTP/2 support
* WebSocket client
* Cell tower location
* Network diagnostics
* Automatic APN detection
* Connection pooling

[2.0.0] - Future
^^^^^^^^^^^^^^^^

**Major Changes:**

* Async/await support
* FreeRTOS integration
* Multi-modem support
* Event-driven architecture
* Hardware abstraction layer

Contributing
============

We welcome contributions! Please see our contributing guidelines:

1. **Bug Reports**: Include debug logs and minimal reproduction code
2. **Feature Requests**: Describe use case and proposed API
3. **Pull Requests**: Follow existing code style and include tests
4. **Documentation**: Help improve docs and examples

Development Setup
-----------------

.. code-block:: bash

    # Clone repository
    git clone https://github.com/yourusername/QuectelEC200U.git

    # Install dependencies
    pip install -r docs/requirements.txt

    # Build documentation
    cd docs
    make html

    # Run tests (Arduino CLI)
    arduino-cli compile --fqbn esp32:esp32:esp32 examples/QuectelEC200U_Example

Testing Checklist
-----------------

Before submitting PR:

* [ ] Code compiles without warnings
* [ ] Tested on actual hardware
* [ ] Documentation updated
* [ ] Examples work correctly
* [ ] Error handling tested
* [ ] Memory leaks checked

Version History
===============

Versioning Scheme
-----------------

This project uses Semantic Versioning:

* **MAJOR** version for incompatible API changes
* **MINOR** version for backwards-compatible new features
* **PATCH** version for backwards-compatible bug fixes

Version Support
---------------

.. list-table:: Version Support Status
   :widths: 20 20 20 40
   :header-rows: 1

   * - Version
     - Status
     - Released
     - Notes
   * - 1.0.0
     - Current
     - 2024-11-26
     - Initial stable release
   * - 0.9.x
     - Beta
     - 2024-11-01
     - Pre-release testing
   * - 0.1.x
     - Alpha
     - 2024-10-01
     - Development versions

Migration Guides
================

From Raw AT Commands
--------------------

If migrating from raw AT command usage:

**Before (Raw AT):**

.. code-block:: cpp

    Serial1.println("AT+QGPSLOC=2");
    delay(5000);
    String response = Serial1.readString();
    // Manual parsing required

**After (Library):**

.. code-block:: cpp

    GNSSPosition position;
    if (modem.getPosition(position)) {
        // Data already parsed
        Serial.println(position.latitude);
    }

From Other Libraries
--------------------

If migrating from TinyGSM or similar:

**Key Differences:**

1. **Error Handling**: All errors are captured and returned
2. **Data Structures**: Responses are parsed into structures
3. **Retry Logic**: Built-in retry mechanisms
4. **Mode Support**: Multiple operating modes supported

**Migration Example:**

.. code-block:: cpp

    // TinyGSM style
    modem.getGPS(&lat, &lon);

    // QuectelEC200U style
    double lat, lon;
    modem.getCoordinates(lat, lon);
    // Or with full data:
    GNSSPosition pos;
    modem.getPosition(pos);

License
=======

This library is released under the MIT License.

.. code-block:: text

    MIT License

    Copyright (c) 2024 ESP32 Arduino Development

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

Acknowledgments
===============

* Quectel Wireless Solutions for modem documentation
* ESP32 Arduino Core contributors
* The open source community for testing and feedback

Contact
=======

* **GitHub**: https://github.com/yourusername/QuectelEC200U
* **Issues**: https://github.com/yourusername/QuectelEC200U/issues
* **Email**: support@example.com