.. _getting_started:

===============
Getting Started
===============

This guide will help you write your first sketch with the QuectelEC200U library and understand the basic workflow.

Quick Start Example
===================

Here's a minimal example to get you started:

.. code-block:: cpp

    #include <QuectelEC200U.h>

    // Configure serial pins
    #define MODEM_RX 16
    #define MODEM_TX 17

    // Create modem instance
    HardwareSerial modemSerial(1);
    QuectelEC200U modem(&modemSerial, 115200);

    void setup() {
        Serial.begin(115200);
        delay(1000);

        Serial.println("Initializing modem...");

        // Initialize modem serial
        modemSerial.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);

        // Initialize modem
        if (modem.begin()) {
            Serial.println("Modem ready!");

            // Get signal quality
            int rssi, ber;
            if (modem.getSignalQuality(rssi, ber)) {
                Serial.print("Signal strength: ");
                Serial.print(rssi);
                Serial.println("/31");
            }
        } else {
            Serial.println("Failed to initialize modem!");
        }
    }

    void loop() {
        // Your application code here
    }

Step-by-Step Tutorial
=====================

Step 1: Include the Library
----------------------------

.. code-block:: cpp

    #include <QuectelEC200U.h>

Step 2: Configure Serial Communication
---------------------------------------

The EC200U communicates via UART. Configure the serial pins:

.. code-block:: cpp

    // Define pins (adjust for your board)
    #define MODEM_RX_PIN 16  // ESP32 RX <- EC200U TX
    #define MODEM_TX_PIN 17  // ESP32 TX -> EC200U RX

    // Create hardware serial instance
    HardwareSerial modemSerial(1);  // Use UART1

Step 3: Create Modem Instance
------------------------------

.. code-block:: cpp

    // Create modem object with serial and baud rate
    QuectelEC200U modem(&modemSerial, 115200);

Step 4: Initialize in Setup
----------------------------

.. code-block:: cpp

    void setup() {
        // Initialize debug serial
        Serial.begin(115200);
        while (!Serial) delay(10);

        // Initialize modem serial
        modemSerial.begin(115200, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN);

        // Initialize modem
        if (!modem.begin()) {
            Serial.println("Modem initialization failed!");
            while (1) delay(1000);
        }

        Serial.println("Modem initialized successfully!");
    }

Step 5: Use Modem Features
---------------------------

Now you can use any of the modem's features:

.. code-block:: cpp

    void loop() {
        // Example: Get GPS coordinates every 10 seconds
        static unsigned long lastCheck = 0;

        if (millis() - lastCheck > 10000) {
            lastCheck = millis();

            double lat, lon;
            if (modem.getCoordinates(lat, lon)) {
                Serial.print("Location: ");
                Serial.print(lat, 6);
                Serial.print(", ");
                Serial.println(lon, 6);
            }
        }
    }

Common Use Cases
================

1. Getting GPS Location
-----------------------

.. code-block:: cpp

    void getGPSLocation() {
        // Turn on GNSS
        if (!modem.gnssOn()) {
            Serial.println("Failed to turn on GNSS!");
            return;
        }

        Serial.println("Waiting for GPS fix...");

        // Get position (with retry)
        GNSSPosition position;
        if (modem.getPosition(position, GNSS_FORMAT_DECIMAL_DEGREES, 30, 2000)) {
            Serial.println("=== GPS Position ===");
            Serial.print("Latitude: ");
            Serial.println(position.latitude, 6);
            Serial.print("Longitude: ");
            Serial.println(position.longitude, 6);
            Serial.print("Altitude: ");
            Serial.print(position.altitude);
            Serial.println(" m");
            Serial.print("Speed: ");
            Serial.print(position.speedKmh);
            Serial.println(" km/h");
            Serial.print("Satellites: ");
            Serial.println(position.numSatellites);
        } else {
            Serial.println("Failed to get GPS fix!");
            Serial.print("Error: ");
            Serial.println(modem.getErrorDescription(position.lastError));
        }

        // Turn off GNSS to save power
        modem.gnssOff();
    }

2. Sending HTTPS Request
-------------------------

.. code-block:: cpp

    void sendHTTPSRequest() {
        // Initialize SSL
        if (!modem.sslBegin()) {
            Serial.println("Failed to initialize SSL!");
            return;
        }

        // Connect to server
        SSLConnectionState state;
        if (!modem.httpsConnect("httpbin.org", 443, state)) {
            Serial.println("Failed to connect!");
            return;
        }

        // Send GET request
        String request = "GET /get HTTP/1.1\r\n";
        request += "Host: httpbin.org\r\n";
        request += "Connection: close\r\n\r\n";

        if (modem.httpsSend(request)) {
            Serial.println("Request sent!");

            // Read response
            delay(1000);
            SSLReceiveData data;
            String response = "";

            while (modem.httpsReceive(data, 1500)) {
                if (data.dataAvailable) {
                    response += data.data;
                }
                delay(100);
            }

            Serial.println("Response:");
            Serial.println(response);
        }

        // Exit transparent mode and disconnect
        modem.exitTransparentMode();
        modem.httpsDisconnect(state.clientID);
    }

3. Getting Network Time
------------------------

.. code-block:: cpp

    void getNetworkTime() {
        NetworkTime time;

        // Get current local time
        if (modem.getNetworkTime(time, TIME_MODE_LOCAL)) {
            Serial.println("=== Current Time ===");
            Serial.print("Date: ");
            Serial.print(time.year);
            Serial.print("/");
            Serial.print(time.month);
            Serial.print("/");
            Serial.println(time.day);

            Serial.print("Time: ");
            Serial.print(time.hour);
            Serial.print(":");
            Serial.print(time.minute);
            Serial.print(":");
            Serial.println(time.second);

            Serial.print("Timezone: GMT");
            if (time.timezoneHours >= 0) Serial.print("+");
            Serial.println(time.timezoneHours);
        } else {
            Serial.println("Failed to get network time!");

            // Try to sync from network
            if (modem.syncTimeFromNetwork()) {
                Serial.println("Time synchronized from network!");
            }
        }
    }

4. Monitoring Signal Quality
-----------------------------

.. code-block:: cpp

    void monitorSignal() {
        int rssi, ber;

        if (modem.getSignalQuality(rssi, ber)) {
            Serial.print("Signal Quality: ");

            // Convert RSSI to dBm
            int dbm;
            if (rssi == 0) {
                dbm = -113;
            } else if (rssi == 1) {
                dbm = -111;
            } else if (rssi >= 2 && rssi <= 30) {
                dbm = -109 + (rssi - 2) * 2;
            } else if (rssi == 31) {
                dbm = -51;
            } else {
                dbm = -999;  // Unknown
            }

            if (dbm != -999) {
                Serial.print(dbm);
                Serial.print(" dBm");

                // Signal quality indicator
                if (dbm >= -70) {
                    Serial.println(" [Excellent]");
                } else if (dbm >= -85) {
                    Serial.println(" [Good]");
                } else if (dbm >= -100) {
                    Serial.println(" [Fair]");
                } else {
                    Serial.println(" [Poor]");
                }
            } else {
                Serial.println("Unknown");
            }

            // Bit error rate
            Serial.print("Bit Error Rate: ");
            if (ber == 99) {
                Serial.println("Unknown");
            } else {
                Serial.print(ber);
                Serial.println("/7");
            }
        }
    }

Best Practices
==============

1. Power Management
-------------------

**Turn off unused features:**

.. code-block:: cpp

    // Turn off GNSS when not needed
    modem.gnssOff();

    // Disconnect SSL when done
    modem.httpsDisconnect();

**Use appropriate timeouts:**

.. code-block:: cpp

    // Set shorter timeout for simple commands
    modem.setTimeout(2000);  // 2 seconds

    // Use longer timeout for network operations
    modem.setTimeout(30000);  // 30 seconds

2. Error Handling
-----------------

**Always check return values:**

.. code-block:: cpp

    if (!modem.begin()) {
        Serial.println("Initialization failed!");
        // Handle error appropriately
        return;
    }

**Use error codes for debugging:**

.. code-block:: cpp

    GNSSPosition position;
    if (!modem.getPosition(position)) {
        Serial.print("Error code: ");
        Serial.println(position.lastError);
        Serial.print("Description: ");
        Serial.println(modem.getErrorDescription(position.lastError));
    }

3. Network Operations
---------------------

**Check network registration:**

.. code-block:: cpp

    void waitForNetwork() {
        Serial.println("Waiting for network...");

        int status;
        int attempts = 0;

        while (attempts < 30) {
            if (modem.getNetworkStatus(status)) {
                if (status == 1 || status == 5) {
                    Serial.println("Network registered!");
                    return;
                }
            }
            delay(2000);
            attempts++;
        }

        Serial.println("Network registration failed!");
    }

**Implement retry logic:**

.. code-block:: cpp

    bool connectWithRetry(const String& server, int port, int maxRetries = 3) {
        for (int i = 0; i < maxRetries; i++) {
            SSLConnectionState state;
            if (modem.httpsConnect(server, port, state)) {
                return true;
            }

            Serial.print("Connection attempt ");
            Serial.print(i + 1);
            Serial.println(" failed, retrying...");
            delay(5000);
        }
        return false;
    }

4. Serial Communication
-----------------------

**Clear buffers before operations:**

.. code-block:: cpp

    // Clear any pending data
    modem.clearBuffer();

    // Then send command
    String response;
    modem.sendRawATCommand("AT+CSQ", response);

**Use debug output wisely:**

.. code-block:: cpp

    // Enable debug output during development
    #define QUECTEL_DEBUG 1

    // Disable for production
    #define QUECTEL_DEBUG 0

Complete Working Example
========================

Here's a complete example that demonstrates all major features:

.. code-block:: cpp

    #include <QuectelEC200U.h>

    // Pin definitions
    #define MODEM_RX_PIN 16
    #define MODEM_TX_PIN 17

    // Create instances
    HardwareSerial modemSerial(1);
    QuectelEC200U modem(&modemSerial, 115200);

    // State variables
    bool networkReady = false;
    bool gpsFixed = false;
    unsigned long lastGPSCheck = 0;
    unsigned long lastTimeSync = 0;

    void setup() {
        Serial.begin(115200);
        while (!Serial) delay(10);

        Serial.println("\n=== QuectelEC200U Demo ===\n");

        // Initialize modem serial
        modemSerial.begin(115200, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN);

        // Initialize modem
        Serial.println("Initializing modem...");
        if (!modem.begin()) {
            Serial.println("Failed to initialize modem!");
            while (1) delay(1000);
        }

        // Get modem info
        String imei;
        if (modem.getIMEI(imei)) {
            Serial.print("Modem IMEI: ");
            Serial.println(imei);
        }

        // Wait for network
        Serial.println("\nWaiting for network registration...");
        waitForNetwork();

        // Initialize GPS
        Serial.println("\nInitializing GPS...");
        if (modem.gnssOn()) {
            Serial.println("GPS turned on!");
        }

        // Initialize SSL
        Serial.println("\nInitializing SSL...");
        if (modem.sslBegin()) {
            Serial.println("SSL ready!");
        }

        Serial.println("\n=== Setup Complete ===\n");
    }

    void loop() {
        // Check GPS every 30 seconds
        if (millis() - lastGPSCheck > 30000) {
            lastGPSCheck = millis();
            checkGPS();
        }

        // Sync time every hour
        if (millis() - lastTimeSync > 3600000) {
            lastTimeSync = millis();
            syncTime();
        }

        // Monitor signal quality
        static unsigned long lastSignalCheck = 0;
        if (millis() - lastSignalCheck > 60000) {
            lastSignalCheck = millis();
            checkSignal();
        }

        // Your application logic here
        delay(1000);
    }

    void waitForNetwork() {
        int status;
        int attempts = 0;

        while (attempts < 30) {
            if (modem.getNetworkStatus(status)) {
                Serial.print("Status: ");
                Serial.print(status);

                if (status == 1 || status == 5) {
                    Serial.println(" - Registered!");
                    networkReady = true;
                    return;
                } else {
                    Serial.println(" - Not registered");
                }
            }
            delay(2000);
            attempts++;
        }
    }

    void checkGPS() {
        Serial.println("\n--- GPS Check ---");

        double lat, lon;
        if (modem.getCoordinates(lat, lon)) {
            Serial.print("Position: ");
            Serial.print(lat, 6);
            Serial.print(", ");
            Serial.println(lon, 6);
            gpsFixed = true;
        } else {
            Serial.println("No GPS fix yet");
        }
    }

    void syncTime() {
        Serial.println("\n--- Time Sync ---");

        NetworkTime time;
        if (modem.getNetworkTime(time, TIME_MODE_LOCAL)) {
            Serial.print("Current time: ");
            Serial.print(time.hour);
            Serial.print(":");
            Serial.print(time.minute);
            Serial.print(":");
            Serial.println(time.second);
        }
    }

    void checkSignal() {
        int rssi, ber;
        if (modem.getSignalQuality(rssi, ber)) {
            Serial.print("Signal: ");
            Serial.print(rssi);
            Serial.println("/31");
        }
    }

Next Steps
==========

Now that you understand the basics:

* Explore more :doc:`examples`
* Review the complete :doc:`api_reference`
* Learn about :doc:`error_codes`
* Check :doc:`troubleshooting` for common issues