.. QuectelEC200U documentation master file

====================================
QuectelEC200U Arduino Library
====================================

.. image:: https://img.shields.io/badge/version-1.0.0-blue.svg
   :alt: Version

.. image:: https://img.shields.io/badge/platform-ESP32-orange.svg
   :alt: Platform

.. image:: https://img.shields.io/badge/license-MIT-green.svg
   :alt: License

A comprehensive Arduino library for ESP32 to interface with Quectel EC200U cellular modem, providing robust APIs for GPS positioning, HTTPS connections, and time synchronization.

.. toctree::
   :maxdepth: 2
   :caption: Contents:

   introduction
   installation
   getting_started
   api_reference
   examples
   error_codes
   troubleshooting
   changelog

Features
========

* **GPS/GNSS Positioning** - Get accurate location data with comprehensive error handling
* **HTTPS/SSL Communications** - Secure data transmission with transparent mode support
* **Time Synchronization** - Network and RTC time management
* **Robust Error Handling** - Complete AT command response parsing
* **Multiple Operating Modes** - Buffer, Direct Push, and Transparent modes for SSL
* **Automatic Retry Mechanisms** - Built-in retry logic for transient failures

Key Capabilities
================

GPS/GNSS Functions
------------------
* Multiple coordinate formats (Decimal degrees, Degrees/Minutes)
* Complete NMEA data parsing
* Automatic GPS fix acquisition with retry
* Support for 2D/3D positioning
* Satellite count and signal quality metrics

SSL/HTTPS Functions
-------------------
* SSL/TLS secure connections
* Transparent mode for streaming data
* HTTP GET/POST helper methods
* Multiple SSL contexts support
* Comprehensive SSL error handling

Time Management
---------------
* Network time synchronization
* RTC (Real-Time Clock) control
* Multiple timezone support
* Daylight saving time handling
* GMT and local time queries

Quick Start
===========

.. code-block:: cpp

    #include <QuectelEC200U.h>

    // Initialize modem on Serial1
    QuectelEC200U modem(&Serial1, 115200);

    void setup() {
        // Initialize modem
        modem.begin();

        // Get GPS coordinates
        double latitude, longitude;
        if (modem.getCoordinates(latitude, longitude)) {
            Serial.print("Location: ");
            Serial.print(latitude, 6);
            Serial.print(", ");
            Serial.println(longitude, 6);
        }
    }

Hardware Requirements
=====================

* ESP32 development board
* Quectel EC200U modem module
* Appropriate power supply (3.3V-4.2V for modem)
* UART connections between ESP32 and EC200U
* (Optional) GPS antenna for positioning functions
* (Optional) LTE antenna for cellular connectivity

Software Requirements
=====================

* Arduino IDE 1.8.x or higher
* ESP32 board support package
* This QuectelEC200U library

Support
=======

For bug reports and feature requests, please visit our `GitHub repository <https://github.com/yourusername/QuectelEC200U>`_.

License
=======

This library is released under the MIT License. See LICENSE file for details.

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`