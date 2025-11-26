.. _examples:

========
Examples
========

This section provides complete, working examples demonstrating various features of the QuectelEC200U library.

.. contents:: Examples
   :local:
   :depth: 2

GPS Tracking Application
=========================

This example creates a GPS tracker that logs positions to Serial and can send them to a server.

.. code-block:: cpp

    /**
     * GPS Tracking Application
     * Continuously tracks GPS position and sends to server
     */

    #include <QuectelEC200U.h>

    #define MODEM_RX_PIN 16
    #define MODEM_TX_PIN 17
    #define UPDATE_INTERVAL 30000  // 30 seconds

    HardwareSerial modemSerial(1);
    QuectelEC200U modem(&modemSerial, 115200);

    struct TrackingData {
        double latitude;
        double longitude;
        float altitude;
        float speed;
        uint8_t satellites;
        String timestamp;
    };

    TrackingData lastPosition;
    bool trackingEnabled = true;

    void setup() {
        Serial.begin(115200);
        Serial.println("GPS Tracker Starting...");

        modemSerial.begin(115200, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN);

        if (!modem.begin()) {
            Serial.println("Modem initialization failed!");
            while (1);
        }

        // Start GPS
        if (!modem.gnssOn()) {
            Serial.println("Failed to start GPS!");
            while (1);
        }

        Serial.println("GPS Tracker Ready!");
    }

    void loop() {
        static unsigned long lastUpdate = 0;

        if (trackingEnabled && (millis() - lastUpdate > UPDATE_INTERVAL)) {
            lastUpdate = millis();

            if (updatePosition()) {
                displayPosition();
                sendPositionToServer();
            }
        }

        // Check for commands
        if (Serial.available()) {
            handleCommand();
        }
    }

    bool updatePosition() {
        GNSSPosition position;

        Serial.println("Getting GPS position...");

        if (modem.getPosition(position, GNSS_FORMAT_DECIMAL_DEGREES, 5, 2000)) {
            lastPosition.latitude = position.latitude;
            lastPosition.longitude = position.longitude;
            lastPosition.altitude = position.altitude;
            lastPosition.speed = position.speedKmh;
            lastPosition.satellites = position.numSatellites;
            lastPosition.timestamp = position.utcTime;
            return true;
        }

        Serial.println("Failed to get GPS fix");
        return false;
    }

    void displayPosition() {
        Serial.println("\n=== GPS Position Update ===");
        Serial.print("Time: ");
        Serial.println(lastPosition.timestamp);
        Serial.print("Latitude: ");
        Serial.println(lastPosition.latitude, 6);
        Serial.print("Longitude: ");
        Serial.println(lastPosition.longitude, 6);
        Serial.print("Altitude: ");
        Serial.print(lastPosition.altitude);
        Serial.println(" m");
        Serial.print("Speed: ");
        Serial.print(lastPosition.speed);
        Serial.println(" km/h");
        Serial.print("Satellites: ");
        Serial.println(lastPosition.satellites);

        // Generate Google Maps link
        Serial.print("Map: https://maps.google.com/?q=");
        Serial.print(lastPosition.latitude, 6);
        Serial.print(",");
        Serial.println(lastPosition.longitude, 6);
        Serial.println("===========================\n");
    }

    void sendPositionToServer() {
        // Initialize SSL if not already done
        if (!modem.sslBegin()) {
            Serial.println("Failed to initialize SSL");
            return;
        }

        // Connect to server
        SSLConnectionState state;
        if (!modem.httpsConnect("your-server.com", 443, state)) {
            Serial.println("Failed to connect to server");
            return;
        }

        // Create JSON payload
        String json = "{";
        json += "\"lat\":" + String(lastPosition.latitude, 6) + ",";
        json += "\"lon\":" + String(lastPosition.longitude, 6) + ",";
        json += "\"alt\":" + String(lastPosition.altitude) + ",";
        json += "\"speed\":" + String(lastPosition.speed) + ",";
        json += "\"sats\":" + String(lastPosition.satellites) + ",";
        json += "\"time\":\"" + lastPosition.timestamp + "\"";
        json += "}";

        // Send POST request
        String request = "POST /api/tracking HTTP/1.1\r\n";
        request += "Host: your-server.com\r\n";
        request += "Content-Type: application/json\r\n";
        request += "Content-Length: " + String(json.length()) + "\r\n";
        request += "Connection: close\r\n\r\n";
        request += json;

        if (modem.httpsSend(request)) {
            Serial.println("Position sent to server");
        }

        // Clean up
        modem.exitTransparentMode();
        modem.httpsDisconnect(state.clientID);
    }

    void handleCommand() {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();

        if (cmd == "START") {
            trackingEnabled = true;
            Serial.println("Tracking started");
        } else if (cmd == "STOP") {
            trackingEnabled = false;
            Serial.println("Tracking stopped");
        } else if (cmd == "STATUS") {
            displayPosition();
        } else if (cmd == "HELP") {
            Serial.println("Commands:");
            Serial.println("  START - Start tracking");
            Serial.println("  STOP  - Stop tracking");
            Serial.println("  STATUS - Show last position");
            Serial.println("  HELP  - Show this help");
        }
    }

IoT Sensor Data Logger
======================

This example reads sensor data and sends it to an HTTPS endpoint periodically.

.. code-block:: cpp

    /**
     * IoT Sensor Data Logger
     * Reads sensors and posts data to cloud service
     */

    #include <QuectelEC200U.h>
    #include <ArduinoJson.h>

    #define MODEM_RX_PIN 16
    #define MODEM_TX_PIN 17
    #define SENSOR_PIN 34  // Analog sensor pin
    #define SEND_INTERVAL 60000  // 1 minute

    HardwareSerial modemSerial(1);
    QuectelEC200U modem(&modemSerial, 115200);

    // Cloud configuration
    const char* API_HOST = "api.thingspeak.com";
    const int API_PORT = 443;
    const char* API_KEY = "YOUR_API_KEY";

    void setup() {
        Serial.begin(115200);
        Serial.println("IoT Data Logger Starting...");

        modemSerial.begin(115200, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN);

        if (!modem.begin()) {
            Serial.println("Modem initialization failed!");
            while (1);
        }

        // Wait for network
        waitForNetwork();

        // Initialize SSL
        if (!modem.sslBegin()) {
            Serial.println("SSL initialization failed!");
            while (1);
        }

        // Sync time for accurate timestamps
        modem.syncTimeFromNetwork();

        Serial.println("IoT Logger Ready!");
    }

    void loop() {
        static unsigned long lastSend = 0;

        if (millis() - lastSend > SEND_INTERVAL) {
            lastSend = millis();

            // Read sensors
            float temperature = readTemperature();
            float humidity = readHumidity();
            float pressure = readPressure();
            int lightLevel = analogRead(SENSOR_PIN);

            // Get location if available
            double lat = 0, lon = 0;
            modem.getCoordinates(lat, lon);

            // Send to cloud
            if (sendToCloud(temperature, humidity, pressure, lightLevel, lat, lon)) {
                Serial.println("Data sent successfully!");
            } else {
                Serial.println("Failed to send data!");
            }
        }

        // Low power delay
        delay(1000);
    }

    void waitForNetwork() {
        Serial.println("Waiting for network...");
        int status;
        while (true) {
            if (modem.getNetworkStatus(status)) {
                if (status == 1 || status == 5) {
                    Serial.println("Network connected!");
                    break;
                }
            }
            delay(2000);
        }
    }

    bool sendToCloud(float temp, float humid, float press, int light, double lat, double lon) {
        // Connect to server
        SSLConnectionState state;
        if (!modem.httpsConnect(API_HOST, API_PORT, state)) {
            Serial.println("Connection failed");
            return false;
        }

        // Build query string
        String query = "api_key=" + String(API_KEY);
        query += "&field1=" + String(temp, 2);
        query += "&field2=" + String(humid, 2);
        query += "&field3=" + String(press, 2);
        query += "&field4=" + String(light);
        if (lat != 0 && lon != 0) {
            query += "&lat=" + String(lat, 6);
            query += "&long=" + String(lon, 6);
        }

        // Send HTTP POST
        String request = "POST /update HTTP/1.1\r\n";
        request += "Host: " + String(API_HOST) + "\r\n";
        request += "Content-Type: application/x-www-form-urlencoded\r\n";
        request += "Content-Length: " + String(query.length()) + "\r\n";
        request += "Connection: close\r\n\r\n";
        request += query;

        bool success = modem.httpsSend(request);

        // Read response
        if (success) {
            delay(1000);
            SSLReceiveData data;
            String response = "";

            while (modem.httpsReceive(data, 1500)) {
                if (data.dataAvailable) {
                    response += data.data;
                }
                if (response.indexOf("\r\n\r\n") > 0) break;
            }

            Serial.println("Response: " + response.substring(0, 100));
        }

        // Cleanup
        modem.exitTransparentMode();
        modem.httpsDisconnect(state.clientID);

        return success;
    }

    float readTemperature() {
        // Simulate temperature reading
        return 20.0 + random(0, 100) / 10.0;
    }

    float readHumidity() {
        // Simulate humidity reading
        return 40.0 + random(0, 400) / 10.0;
    }

    float readPressure() {
        // Simulate pressure reading
        return 1000.0 + random(0, 300) / 10.0;
    }

REST API Client
===============

This example demonstrates making various REST API calls.

.. code-block:: cpp

    /**
     * REST API Client Example
     * Demonstrates GET, POST, PUT, DELETE operations
     */

    #include <QuectelEC200U.h>

    #define MODEM_RX_PIN 16
    #define MODEM_TX_PIN 17

    HardwareSerial modemSerial(1);
    QuectelEC200U modem(&modemSerial, 115200);

    const char* API_HOST = "jsonplaceholder.typicode.com";
    const int API_PORT = 443;

    void setup() {
        Serial.begin(115200);
        Serial.println("REST API Client Example");

        modemSerial.begin(115200, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN);

        if (!modem.begin()) {
            Serial.println("Modem initialization failed!");
            while (1);
        }

        // Initialize SSL
        if (!modem.sslBegin()) {
            Serial.println("SSL initialization failed!");
            while (1);
        }

        Serial.println("\nReady for API calls!\n");
    }

    void loop() {
        Serial.println("Select operation:");
        Serial.println("1. GET request");
        Serial.println("2. POST request");
        Serial.println("3. PUT request");
        Serial.println("4. DELETE request");
        Serial.println("5. Get all posts");

        while (!Serial.available());
        char choice = Serial.read();
        while (Serial.available()) Serial.read();  // Clear buffer

        switch (choice) {
            case '1':
                performGET();
                break;
            case '2':
                performPOST();
                break;
            case '3':
                performPUT();
                break;
            case '4':
                performDELETE();
                break;
            case '5':
                getAllPosts();
                break;
            default:
                Serial.println("Invalid choice");
        }

        Serial.println("\n-------------------\n");
        delay(2000);
    }

    void performGET() {
        Serial.println("\n=== GET Request ===");

        String response;
        if (makeRequest("GET", "/posts/1", "", response)) {
            Serial.println("Response:");
            Serial.println(response);
        } else {
            Serial.println("GET request failed");
        }
    }

    void performPOST() {
        Serial.println("\n=== POST Request ===");

        String json = "{";
        json += "\"title\":\"Test Post\",";
        json += "\"body\":\"This is a test post from EC200U\",";
        json += "\"userId\":1";
        json += "}";

        String response;
        if (makeRequest("POST", "/posts", json, response)) {
            Serial.println("Response:");
            Serial.println(response);
        } else {
            Serial.println("POST request failed");
        }
    }

    void performPUT() {
        Serial.println("\n=== PUT Request ===");

        String json = "{";
        json += "\"id\":1,";
        json += "\"title\":\"Updated Post\",";
        json += "\"body\":\"This post has been updated\",";
        json += "\"userId\":1";
        json += "}";

        String response;
        if (makeRequest("PUT", "/posts/1", json, response)) {
            Serial.println("Response:");
            Serial.println(response);
        } else {
            Serial.println("PUT request failed");
        }
    }

    void performDELETE() {
        Serial.println("\n=== DELETE Request ===");

        String response;
        if (makeRequest("DELETE", "/posts/1", "", response)) {
            Serial.println("Response:");
            Serial.println(response);
        } else {
            Serial.println("DELETE request failed");
        }
    }

    void getAllPosts() {
        Serial.println("\n=== Get All Posts ===");

        String response;
        if (makeRequest("GET", "/posts", "", response)) {
            // Parse and display first 500 chars
            Serial.println("Response (truncated):");
            Serial.println(response.substring(0, 500));
            Serial.println("...");
        } else {
            Serial.println("Request failed");
        }
    }

    bool makeRequest(const String& method, const String& path,
                     const String& body, String& response) {
        // Connect to server
        SSLConnectionState state;
        if (!modem.httpsConnect(API_HOST, API_PORT, state)) {
            Serial.println("Failed to connect");
            return false;
        }

        // Build request
        String request = method + " " + path + " HTTP/1.1\r\n";
        request += "Host: " + String(API_HOST) + "\r\n";
        request += "User-Agent: EC200U/1.0\r\n";
        request += "Accept: application/json\r\n";

        if (body.length() > 0) {
            request += "Content-Type: application/json\r\n";
            request += "Content-Length: " + String(body.length()) + "\r\n";
        }

        request += "Connection: close\r\n\r\n";

        if (body.length() > 0) {
            request += body;
        }

        // Send request
        if (!modem.httpsSend(request)) {
            Serial.println("Failed to send request");
            modem.exitTransparentMode();
            modem.httpsDisconnect(state.clientID);
            return false;
        }

        // Read response
        response = "";
        unsigned long start = millis();
        SSLReceiveData data;

        while (millis() - start < 10000) {  // 10 second timeout
            if (modem.httpsReceive(data, 1500)) {
                if (data.dataAvailable) {
                    response += data.data;

                    // Check if we have complete response
                    if (response.indexOf("\r\n\r\n") > 0) {
                        int headerEnd = response.indexOf("\r\n\r\n") + 4;

                        // Check for Content-Length
                        int clIdx = response.indexOf("Content-Length: ");
                        if (clIdx > 0) {
                            int clEnd = response.indexOf("\r\n", clIdx);
                            int contentLength = response.substring(clIdx + 16, clEnd).toInt();
                            int currentLength = response.length() - headerEnd;

                            if (currentLength >= contentLength) {
                                break;  // Complete response received
                            }
                        } else if (response.indexOf("Transfer-Encoding: chunked") > 0) {
                            if (response.endsWith("0\r\n\r\n")) {
                                break;  // Chunked response complete
                            }
                        }
                    }
                }
            }
            delay(100);
        }

        // Cleanup
        modem.exitTransparentMode();
        modem.httpsDisconnect(state.clientID);

        // Extract body from response
        int bodyStart = response.indexOf("\r\n\r\n");
        if (bodyStart > 0) {
            response = response.substring(bodyStart + 4);
        }

        return response.length() > 0;
    }

Time Synchronization Example
=============================

This example demonstrates all time-related functions.

.. code-block:: cpp

    /**
     * Time Synchronization Example
     * Shows network time sync, RTC management, and timezone handling
     */

    #include <QuectelEC200U.h>

    #define MODEM_RX_PIN 16
    #define MODEM_TX_PIN 17

    HardwareSerial modemSerial(1);
    QuectelEC200U modem(&modemSerial, 115200);

    void setup() {
        Serial.begin(115200);
        Serial.println("Time Synchronization Example");

        modemSerial.begin(115200, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN);

        if (!modem.begin()) {
            Serial.println("Failed to initialize modem!");
            while (1);
        }

        Serial.println("Modem ready!\n");
    }

    void loop() {
        Serial.println("Select option:");
        Serial.println("1. Get network time (all modes)");
        Serial.println("2. Get RTC time");
        Serial.println("3. Set RTC manually");
        Serial.println("4. Sync time from network");
        Serial.println("5. Continuous time display");

        while (!Serial.available());
        char choice = Serial.read();
        while (Serial.available()) Serial.read();

        switch (choice) {
            case '1':
                showNetworkTime();
                break;
            case '2':
                showRTCTime();
                break;
            case '3':
                setRTCManually();
                break;
            case '4':
                syncFromNetwork();
                break;
            case '5':
                continuousTimeDisplay();
                break;
        }

        Serial.println("\n-------------------\n");
        delay(2000);
    }

    void showNetworkTime() {
        Serial.println("\n=== Network Time ===");

        NetworkTime time;

        // Last synchronized time
        Serial.println("\n1. Last Synchronized Time:");
        if (modem.getNetworkTime(time, TIME_MODE_LAST_SYNC)) {
            if (time.valid) {
                displayTime(time);
            } else {
                Serial.println("   No time synchronized yet");
            }
        } else {
            Serial.println("   Failed to get time");
        }

        // GMT time
        Serial.println("\n2. Current GMT Time:");
        if (modem.getNetworkTime(time, TIME_MODE_GMT)) {
            if (time.valid) {
                displayTime(time);
            } else {
                Serial.println("   Not available");
            }
        }

        // Local time
        Serial.println("\n3. Current Local Time:");
        if (modem.getNetworkTime(time, TIME_MODE_LOCAL)) {
            if (time.valid) {
                displayTime(time);
            } else {
                Serial.println("   Not available");
            }
        }
    }

    void showRTCTime() {
        Serial.println("\n=== RTC Time ===");

        NetworkTime time;
        if (modem.getRTCTime(time)) {
            displayTime(time);
        } else {
            Serial.println("Failed to read RTC");
        }
    }

    void setRTCManually() {
        Serial.println("\n=== Set RTC Manually ===");

        // Set to current date/time (adjust as needed)
        int year = 24;    // 2024
        int month = 11;
        int day = 26;
        int hour = 14;
        int minute = 30;
        int second = 0;
        int timezone = 32;  // GMT+8

        Serial.print("Setting RTC to: ");
        Serial.print("20");
        Serial.print(year);
        Serial.print("/");
        Serial.print(month);
        Serial.print("/");
        Serial.print(day);
        Serial.print(" ");
        Serial.print(hour);
        Serial.print(":");
        Serial.print(minute);
        Serial.print(":");
        Serial.print(second);
        Serial.print(" GMT+");
        Serial.println(timezone / 4);

        if (modem.setRTCTime(year, month, day, hour, minute, second, timezone)) {
            Serial.println("RTC set successfully!");

            // Read back
            NetworkTime time;
            if (modem.getRTCTime(time)) {
                Serial.println("\nNew RTC time:");
                displayTime(time);
            }
        } else {
            Serial.println("Failed to set RTC");
        }
    }

    void syncFromNetwork() {
        Serial.println("\n=== Sync from Network ===");

        Serial.println("Synchronizing time from network...");
        if (modem.syncTimeFromNetwork()) {
            Serial.println("Time synchronized successfully!");

            NetworkTime time;
            if (modem.getRTCTime(time)) {
                Serial.println("\nSynchronized time:");
                displayTime(time);
            }
        } else {
            Serial.println("Failed to sync from network");
            Serial.println("(Network may not provide time)");
        }
    }

    void continuousTimeDisplay() {
        Serial.println("\n=== Continuous Time Display ===");
        Serial.println("Press any key to stop...\n");

        while (!Serial.available()) {
            NetworkTime time;
            if (modem.getRTCTime(time)) {
                Serial.print("\r");
                Serial.print(time.year);
                Serial.print("/");
                if (time.month < 10) Serial.print("0");
                Serial.print(time.month);
                Serial.print("/");
                if (time.day < 10) Serial.print("0");
                Serial.print(time.day);
                Serial.print(" ");
                if (time.hour < 10) Serial.print("0");
                Serial.print(time.hour);
                Serial.print(":");
                if (time.minute < 10) Serial.print("0");
                Serial.print(time.minute);
                Serial.print(":");
                if (time.second < 10) Serial.print(" 0");
                Serial.print(time.second);
                Serial.print(" GMT");
                if (time.timezoneHours >= 0) Serial.print("+");
                Serial.print(time.timezoneHours);
                Serial.print("    ");
            }
            delay(1000);
        }

        while (Serial.available()) Serial.read();
        Serial.println("\n");
    }

    void displayTime(const NetworkTime& time) {
        Serial.print("   Date: ");
        Serial.print(time.year);
        Serial.print("/");
        if (time.month < 10) Serial.print("0");
        Serial.print(time.month);
        Serial.print("/");
        if (time.day < 10) Serial.print("0");
        Serial.println(time.day);

        Serial.print("   Time: ");
        if (time.hour < 10) Serial.print("0");
        Serial.print(time.hour);
        Serial.print(":");
        if (time.minute < 10) Serial.print("0");
        Serial.print(time.minute);
        Serial.print(":");
        if (time.second < 10) Serial.print("0");
        Serial.println(time.second);

        Serial.print("   Timezone: GMT");
        if (time.timezoneHours >= 0) Serial.print("+");
        Serial.println(time.timezoneHours);

        if (time.daylightSaving) {
            Serial.println("   Daylight Saving: Yes");
        }

        Serial.print("   Raw: ");
        Serial.println(time.dateTime);
    }

Advanced Error Handling
========================

This example shows comprehensive error handling and recovery.

.. code-block:: cpp

    /**
     * Advanced Error Handling Example
     * Demonstrates error detection, logging, and recovery
     */

    #include <QuectelEC200U.h>

    #define MODEM_RX_PIN 16
    #define MODEM_TX_PIN 17

    HardwareSerial modemSerial(1);
    QuectelEC200U modem(&modemSerial, 115200);

    // Error statistics
    struct ErrorStats {
        int totalErrors = 0;
        int cmeErrors = 0;
        int sslErrors = 0;
        int timeouts = 0;
        int recoveries = 0;
    } stats;

    void setup() {
        Serial.begin(115200);
        Serial.println("Error Handling Example");

        modemSerial.begin(115200, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN);

        // Initialize with retry
        if (!initializeModemWithRetry()) {
            Serial.println("Fatal: Could not initialize modem after retries");
            while (1);
        }

        Serial.println("System ready!\n");
    }

    void loop() {
        // Demonstrate various error scenarios
        testGPSErrors();
        delay(2000);

        testSSLErrors();
        delay(2000);

        testTimeErrors();
        delay(2000);

        // Display statistics
        displayErrorStats();
        delay(5000);
    }

    bool initializeModemWithRetry() {
        int retries = 3;

        while (retries > 0) {
            Serial.print("Initializing modem (attempt ");
            Serial.print(4 - retries);
            Serial.println("/3)...");

            if (modem.begin()) {
                Serial.println("Modem initialized successfully!");
                return true;
            }

            Serial.println("Failed, retrying...");
            retries--;
            delay(2000);
        }

        return false;
    }

    void testGPSErrors() {
        Serial.println("\n=== Testing GPS Errors ===");

        GNSSPosition position;

        // Try to get position without turning on GNSS
        if (!modem.getPosition(position, GNSS_FORMAT_DECIMAL_DEGREES, 1, 100)) {
            handleGPSError(position.lastError);
        }

        // Turn on GNSS
        if (modem.gnssOn()) {
            // Try with short timeout (likely to fail)
            if (!modem.getPosition(position, GNSS_FORMAT_DECIMAL_DEGREES, 2, 1000)) {
                handleGPSError(position.lastError);

                // Recover: Try with longer timeout
                Serial.println("Recovering: Trying with longer timeout...");
                if (modem.getPosition(position, GNSS_FORMAT_DECIMAL_DEGREES, 10, 3000)) {
                    Serial.println("Recovery successful - Position acquired!");
                    stats.recoveries++;
                }
            }

            modem.gnssOff();
        }
    }

    void testSSLErrors() {
        Serial.println("\n=== Testing SSL Errors ===");

        SSLConnectionState state;

        // Try to connect without initializing SSL
        if (!modem.httpsConnect("example.com", 443, state)) {
            handleSSLError(state.sslError);

            // Recover: Initialize SSL and retry
            Serial.println("Recovering: Initializing SSL...");
            if (modem.sslBegin()) {
                if (modem.httpsConnect("example.com", 443, state)) {
                    Serial.println("Recovery successful - Connected!");
                    stats.recoveries++;
                    modem.httpsDisconnect(state.clientID);
                } else {
                    handleSSLError(state.sslError);
                }
            }
        }
    }

    void testTimeErrors() {
        Serial.println("\n=== Testing Time Errors ===");

        NetworkTime time;

        // Try to get network time (may fail if never synced)
        if (!modem.getNetworkTime(time, TIME_MODE_LAST_SYNC)) {
            handleTimeError(time.lastError);

            // Recover: Try to sync from network
            Serial.println("Recovering: Syncing from network...");
            if (modem.syncTimeFromNetwork()) {
                Serial.println("Recovery successful - Time synced!");
                stats.recoveries++;
            } else {
                Serial.println("Recovery failed - Network may not provide time");
            }
        }
    }

    void handleGPSError(int error) {
        stats.totalErrors++;
        stats.cmeErrors++;

        Serial.print("GPS Error ");
        Serial.print(error);
        Serial.print(": ");

        switch (error) {
            case CME_SESSION_NOT_ACTIVE:
                Serial.println("GNSS not active - Need to turn on GNSS");
                break;
            case CME_NOT_FIXED_NOW:
                Serial.println("No GPS fix - Need more time or better signal");
                break;
            case CME_OP_TIMEOUT:
                Serial.println("Operation timeout - Increase timeout value");
                break;
            case CME_INVALID_PARAMS:
                Serial.println("Invalid parameters - Check command syntax");
                break;
            default:
                Serial.println(modem.getErrorDescription(AT_CME_ERROR - error));
        }
    }

    void handleSSLError(int error) {
        stats.totalErrors++;
        stats.sslErrors++;

        Serial.print("SSL Error ");
        Serial.print(error);
        Serial.print(": ");

        switch (error) {
            case SSL_HANDSHAKE_FAIL:
                Serial.println("Handshake failed - Check SSL configuration");
                break;
            case SSL_SOCKET_CONN_FAILED:
                Serial.println("Connection failed - Check network and server");
                break;
            case SSL_OP_TIMEOUT:
                Serial.println("Timeout - Check network connectivity");
                stats.timeouts++;
                break;
            case SSL_DNS_PARSE_FAILED:
                Serial.println("DNS failed - Check server address");
                break;
            default:
                Serial.println(modem.getErrorDescription(error));
        }
    }

    void handleTimeError(int error) {
        stats.totalErrors++;
        stats.cmeErrors++;

        Serial.print("Time Error ");
        Serial.print(error);
        Serial.print(": ");
        Serial.println(modem.getErrorDescription(AT_CME_ERROR - error));
    }

    void displayErrorStats() {
        Serial.println("\n=== Error Statistics ===");
        Serial.print("Total Errors: ");
        Serial.println(stats.totalErrors);
        Serial.print("CME Errors: ");
        Serial.println(stats.cmeErrors);
        Serial.print("SSL Errors: ");
        Serial.println(stats.sslErrors);
        Serial.print("Timeouts: ");
        Serial.println(stats.timeouts);
        Serial.print("Successful Recoveries: ");
        Serial.println(stats.recoveries);
        Serial.println("========================\n");
    }