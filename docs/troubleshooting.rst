.. _troubleshooting:

===============
Troubleshooting
===============

This guide helps you diagnose and solve common issues with the QuectelEC200U library.

.. contents:: Common Issues
   :local:
   :depth: 2

Modem Communication Issues
==========================

Modem Not Responding to AT Commands
------------------------------------

**Symptoms:**
- ``testAT()`` returns false
- All commands timeout
- No response from modem

**Diagnostic Steps:**

1. **Check Physical Connections**

   .. code-block:: cpp

      // Test with simple AT command
      String response;
      int result = modem.sendRawATCommand("AT", response);
      Serial.print("Result: ");
      Serial.println(result);
      Serial.print("Response: ");
      Serial.println(response);

2. **Verify Power Supply**

   - Measure voltage: Should be 3.3V-4.2V
   - Check current capability: Needs 2A peak
   - Look for brown-out during transmission

3. **Test Different Baud Rates**

   .. code-block:: cpp

      int baudRates[] = {9600, 19200, 38400, 57600, 115200};

      for (int i = 0; i < 5; i++) {
          Serial.print("Testing ");
          Serial.print(baudRates[i]);
          Serial.println(" baud...");

          modemSerial.begin(baudRates[i], SERIAL_8N1, 16, 17);
          delay(100);

          if (modem.testAT()) {
              Serial.print("Modem responds at ");
              Serial.print(baudRates[i]);
              Serial.println(" baud!");
              break;
          }
      }

**Solutions:**

- **Wrong Pins:** Swap TX/RX connections
- **Bad Power:** Use dedicated power supply with adequate current
- **Wrong Baud:** Try auto-baud detection or different rates
- **Modem Sleeping:** Send "AT" multiple times to wake up

Intermittent Communication
--------------------------

**Symptoms:**
- Commands work sometimes
- Random timeouts
- Corrupted responses

**Solutions:**

1. **Add Pull-up Resistors**

   .. code-block:: text

      ESP32 TX ----[10kΩ]---- VCC
                |
                └─────────── EC200U RX

2. **Increase Timeout**

   .. code-block:: cpp

      modem.setTimeout(10000);  // 10 seconds

3. **Clear Buffer Before Commands**

   .. code-block:: cpp

      modem.clearBuffer();
      // Then send command

GPS/GNSS Issues
===============

GPS Cannot Get Fix
-------------------

**Symptoms:**
- Constant ``CME_NOT_FIXED_NOW`` errors
- ``getPosition()`` always fails
- No satellite data

**Diagnostic Code:**

.. code-block:: cpp

    void diagnoseGPS() {
        Serial.println("=== GPS Diagnostics ===");

        // 1. Check if GNSS is on
        String response;
        modem.sendRawATCommand("AT+QGPS?", response);
        Serial.print("GNSS Status: ");
        Serial.println(response);

        // 2. Check satellite info
        modem.sendRawATCommand("AT+QGPSGNMEA=\"GSV\"", response);
        Serial.print("Satellites: ");
        Serial.println(response);

        // 3. Check antenna
        modem.sendRawATCommand("AT+QGPSANT?", response);
        Serial.print("Antenna: ");
        Serial.println(response);

        // 4. Try different modes
        Serial.println("\nTrying different GNSS modes:");

        // Mode 1: GPS only
        modem.gnssOff();
        delay(1000);
        modem.sendRawATCommand("AT+QGPSCFG=\"gnssconfig\",1", response);
        modem.gnssOn();
        delay(5000);

        GNSSPosition pos;
        if (modem.getPosition(pos, GNSS_FORMAT_DECIMAL_DEGREES, 3, 2000)) {
            Serial.println("GPS-only mode works!");
        }

        // Mode 2: GPS+GLONASS
        modem.gnssOff();
        delay(1000);
        modem.sendRawATCommand("AT+QGPSCFG=\"gnssconfig\",3", response);
        modem.gnssOn();
        delay(5000);

        if (modem.getPosition(pos, GNSS_FORMAT_DECIMAL_DEGREES, 3, 2000)) {
            Serial.println("GPS+GLONASS mode works!");
        }
    }

**Solutions:**

1. **Check Antenna Connection**
   - Use active GPS antenna for better reception
   - Ensure antenna cable is not damaged
   - Place antenna with clear sky view

2. **Increase Timeout and Retries**

   .. code-block:: cpp

      GNSSPosition position;
      // Try up to 60 times with 2 second delay (2 minutes total)
      if (modem.getPosition(position, GNSS_FORMAT_DECIMAL_DEGREES, 60, 2000)) {
          // Success
      }

3. **Configure AGPS (Assisted GPS)**

   .. code-block:: cpp

      // Enable AGPS for faster fix
      modem.sendRawATCommand("AT+QGPSCFG=\"agpsposmode\",1", response);

4. **Check for Interference**
   - Move away from buildings
   - Avoid electromagnetic interference
   - Don't place near WiFi/Bluetooth antennas

Wrong Coordinates
-----------------

**Symptoms:**
- Coordinates are 0,0
- Location is incorrect
- Coordinates don't change

**Diagnostic:**

.. code-block:: cpp

    void validateCoordinates(GNSSPosition& pos) {
        Serial.println("=== Coordinate Validation ===");

        // Check if coordinates are valid
        if (pos.latitude == 0 && pos.longitude == 0) {
            Serial.println("ERROR: Null island (0,0) - No fix");
            return;
        }

        // Check coordinate range
        if (abs(pos.latitude) > 90) {
            Serial.println("ERROR: Invalid latitude (>90)");
            return;
        }

        if (abs(pos.longitude) > 180) {
            Serial.println("ERROR: Invalid longitude (>180)");
            return;
        }

        // Check HDOP (accuracy)
        if (pos.hdop > 5) {
            Serial.println("WARNING: Poor accuracy (HDOP > 5)");
        }

        // Check number of satellites
        if (pos.numSatellites < 4) {
            Serial.println("WARNING: Few satellites (<4)");
        }

        // Check fix mode
        if (pos.fixMode < 2) {
            Serial.println("WARNING: No valid fix");
        }

        Serial.println("Coordinates appear valid");
    }

**Solutions:**

- Wait for better signal (more satellites)
- Use decimal degree format for accuracy
- Check coordinate format conversion
- Verify with known location

SSL/HTTPS Connection Issues
===========================

SSL Handshake Failures
----------------------

**Symptoms:**
- ``SSL_HANDSHAKE_FAIL`` error
- Cannot connect to HTTPS servers
- Connection drops immediately

**Diagnostic Code:**

.. code-block:: cpp

    void diagnoseSSL() {
        Serial.println("=== SSL Diagnostics ===");

        // 1. Check PDP context
        String response;
        modem.sendRawATCommand("AT+QIACT?", response);
        Serial.print("PDP Context: ");
        Serial.println(response);

        // 2. Check SSL configuration
        modem.sendRawATCommand("AT+QSSLCFG=\"sslversion\",1", response);
        Serial.print("SSL Version: ");
        Serial.println(response);

        // 3. Try different SSL versions
        int versions[] = {0, 1, 2, 3, 4};  // SSL3, TLS1.0, 1.1, 1.2, All
        String versionNames[] = {"SSL3.0", "TLS1.0", "TLS1.1", "TLS1.2", "All"};

        for (int i = 0; i < 5; i++) {
            Serial.print("Testing ");
            Serial.print(versionNames[i]);
            Serial.print("...");

            String cmd = "AT+QSSLCFG=\"sslversion\",1," + String(versions[i]);
            modem.sendRawATCommand(cmd, response);

            SSLConnectionState state;
            if (modem.httpsConnect("httpbin.org", 443, state)) {
                Serial.println(" SUCCESS!");
                modem.httpsDisconnect(state.clientID);
                break;
            } else {
                Serial.print(" Failed: ");
                Serial.println(state.sslError);
            }
        }
    }

**Solutions:**

1. **Initialize PDP Context**

   .. code-block:: cpp

      // Ensure PDP is active
      if (!modem.sslBegin()) {
          // Configure APN if needed
          modem.sendRawATCommand("AT+QICSGP=1,1,\"your.apn\",\"\",\"\",1", response);
          modem.sslBegin();
      }

2. **Try Different SSL Versions**

   .. code-block:: cpp

      // Try TLS 1.2 specifically
      modem.sslConfigure(1, "", 300);
      modem.sendRawATCommand("AT+QSSLCFG=\"sslversion\",1,3", response);

3. **Adjust Cipher Suites**

   .. code-block:: cpp

      // Use all available ciphers
      modem.sendRawATCommand("AT+QSSLCFG=\"ciphersuite\",1,0xFFFF", response);

Transparent Mode Issues
-----------------------

**Symptoms:**
- Cannot exit transparent mode
- Data not transmitting
- Stuck in CONNECT state

**Solutions:**

1. **Proper Escape Sequence**

   .. code-block:: cpp

      bool exitTransparentModeSafely() {
          // Ensure guard time
          delay(1500);

          // Send escape sequence
          modemSerial.print("+++");

          // Wait for response
          delay(1500);

          // Check for OK
          String response = "";
          unsigned long start = millis();
          while (millis() - start < 2000) {
              if (modemSerial.available()) {
                  response += (char)modemSerial.read();
                  if (response.indexOf("OK") >= 0) {
                      return true;
                  }
              }
          }

          // If failed, try DTR method
          // (requires AT&D1 configuration)
          return false;
      }

2. **Force Reset Connection**

   .. code-block:: cpp

      // Last resort - reset modem
      modem.reset();
      delay(10000);
      modem.begin();

Network and Time Issues
=======================

Network Registration Fails
---------------------------

**Symptoms:**
- Never registers to network
- Status remains 0 or 2
- No signal strength

**Diagnostic:**

.. code-block:: cpp

    void diagnoseNetwork() {
        String response;

        // Check SIM
        modem.sendRawATCommand("AT+CPIN?", response);
        Serial.print("SIM Status: ");
        Serial.println(response);

        // Check operator
        modem.sendRawATCommand("AT+COPS?", response);
        Serial.print("Operator: ");
        Serial.println(response);

        // Check available networks
        Serial.println("Scanning networks (may take 30s)...");
        modem.sendRawATCommand("AT+COPS=?", response, 30000);
        Serial.print("Available: ");
        Serial.println(response);

        // Check band configuration
        modem.sendRawATCommand("AT+QCFG=\"band\"", response);
        Serial.print("Bands: ");
        Serial.println(response);
    }

**Solutions:**

- Check SIM card insertion and PIN
- Verify APN settings with carrier
- Try manual network selection
- Check antenna connections

Time Sync Issues
----------------

**Symptoms:**
- Network time returns empty
- RTC loses time on power cycle
- Incorrect timezone

**Solutions:**

.. code-block:: cpp

    class TimeManager {
    private:
        QuectelEC200U* modem;
        bool timeValid = false;
        unsigned long lastSync = 0;

    public:
        TimeManager(QuectelEC200U* m) : modem(m) {}

        bool ensureTimeSync() {
            // Try network time first
            NetworkTime time;
            if (modem->getNetworkTime(time, TIME_MODE_LOCAL)) {
                if (time.valid) {
                    timeValid = true;
                    lastSync = millis();
                    return true;
                }
            }

            // Fallback: Set manual time
            // Get from external source (GPS, NTP, etc)
            if (setTimeFromGPS()) {
                return true;
            }

            // Last resort: Set a default time
            setDefaultTime();
            return false;
        }

        bool setTimeFromGPS() {
            GNSSPosition pos;
            if (modem->getPosition(pos)) {
                // Parse UTC time from GPS
                // Format: hhmmss.sss
                String utc = pos.utcTime;
                int hour = utc.substring(0, 2).toInt();
                int min = utc.substring(2, 4).toInt();
                int sec = utc.substring(4, 6).toInt();

                // Parse date
                // Format: ddmmyy
                String date = pos.date;
                int day = date.substring(0, 2).toInt();
                int month = date.substring(2, 4).toInt();
                int year = date.substring(4, 6).toInt();

                return modem->setRTCTime(year, month, day, hour, min, sec, 0);
            }
            return false;
        }

        void setDefaultTime() {
            // Set to compile time as fallback
            modem->setRTCTime(24, 1, 1, 0, 0, 0, 0);
        }
    };

Performance Issues
==================

Slow Response Times
-------------------

**Symptoms:**
- Commands take long to execute
- Data transfer is slow
- Timeouts frequent

**Solutions:**

1. **Optimize Baud Rate**

   .. code-block:: cpp

      // Use highest stable baud rate
      modemSerial.begin(921600, SERIAL_8N1, RX_PIN, TX_PIN);

2. **Batch Operations**

   .. code-block:: cpp

      // Instead of multiple commands
      // Combine into single session
      modem.sendRawATCommand("AT+QGPSLOC;+CSQ;+CREG?", response);

3. **Use Appropriate Timeouts**

   .. code-block:: cpp

      // Short timeout for simple commands
      modem.setTimeout(1000);
      modem.testAT();

      // Long timeout for network operations
      modem.setTimeout(30000);
      modem.sslBegin();

Memory Issues
-------------

**Symptoms:**
- ESP32 crashes
- Out of memory errors
- Stack overflow

**Solutions:**

1. **Reduce Buffer Sizes**

   .. code-block:: cpp

      // Read data in smaller chunks
      SSLReceiveData data;
      while (modem.httpsReceive(data, 512)) {  // Smaller buffer
          // Process chunk
          data.data = "";  // Clear after processing
      }

2. **Use PROGMEM for Constants**

   .. code-block:: cpp

      const char API_HOST[] PROGMEM = "api.example.com";

3. **Monitor Memory Usage**

   .. code-block:: cpp

      void printMemoryStats() {
          Serial.print("Free Heap: ");
          Serial.println(ESP.getFreeHeap());
          Serial.print("Largest Block: ");
          Serial.println(ESP.getMaxAllocHeap());
      }

Debug Techniques
================

Enable Debug Output
-------------------

.. code-block:: cpp

    // In your code, before including library
    #define QUECTEL_DEBUG 1
    #include <QuectelEC200U.h>

AT Command Monitor
------------------

.. code-block:: cpp

    class ATMonitor {
    public:
        static void monitor(HardwareSerial* modemSerial) {
            Serial.println("AT Command Monitor (type 'EXIT' to quit)");

            while (true) {
                // Forward from Serial to Modem
                if (Serial.available()) {
                    String cmd = Serial.readStringUntil('\n');
                    if (cmd == "EXIT") break;

                    Serial.print(">> ");
                    Serial.println(cmd);
                    modemSerial->println(cmd);
                }

                // Forward from Modem to Serial
                if (modemSerial->available()) {
                    String response = modemSerial->readStringUntil('\n');
                    Serial.print("<< ");
                    Serial.println(response);
                }
            }
        }
    };

Signal Quality Monitor
----------------------

.. code-block:: cpp

    void monitorSignalQuality() {
        while (true) {
            int rssi, ber;
            if (modem.getSignalQuality(rssi, ber)) {
                // Convert to dBm
                int dbm = -113 + (rssi * 2);

                // Visual bar graph
                Serial.print("Signal: ");
                int bars = map(rssi, 0, 31, 0, 10);
                for (int i = 0; i < 10; i++) {
                    Serial.print(i < bars ? "█" : "░");
                }
                Serial.print(" ");
                Serial.print(dbm);
                Serial.println(" dBm");
            }

            delay(1000);
        }
    }

Getting Help
============

If you're still experiencing issues:

1. **Check the Examples**: Review the :doc:`examples` section
2. **Review Error Codes**: See :doc:`error_codes` for specific errors
3. **Enable Debug Mode**: Set ``QUECTEL_DEBUG 1`` for verbose output
4. **Check Connections**: Use a multimeter to verify power and signals
5. **Test with AT Commands**: Use raw AT commands to isolate issues
6. **Report Issues**: File a bug report with debug logs

Common AT Commands for Testing
-------------------------------

.. code-block:: text

    AT              - Basic test
    AT+CSQ          - Signal quality
    AT+CREG?        - Network registration
    AT+CPIN?        - SIM status
    AT+COPS?        - Network operator
    AT+QGPS=1       - Turn on GPS
    AT+QGPSLOC?     - Get GPS location
    AT+QSSLCFG=?    - SSL configuration
    AT+QIACT?       - PDP context status
    AT+QLTS         - Network time