/**
 * QuectelEC200U_Example.ino - Example sketch for QuectelEC200U library
 *
 * This example demonstrates:
 * 1. Getting current GPS location (latitude/longitude)
 * 2. Connecting to HTTPS server using SSL transparent mode
 * 3. Getting current time from the modem
 *
 * Hardware connections:
 * - Connect EC200U TX to ESP32 RX pin (e.g., GPIO16)
 * - Connect EC200U RX to ESP32 TX pin (e.g., GPIO17)
 * - Connect EC200U GND to ESP32 GND
 * - Power EC200U with appropriate voltage (3.3V-4.2V)
 *
 * Author: ESP32 Arduino Library
 * Date: 2024
 */

#include <HardwareSerial.h>
#include "QuectelEC200U.h"

// Define serial pins for ESP32
#define MODEM_RX_PIN 16
#define MODEM_TX_PIN 17
#define MODEM_BAUD 115200

// Create hardware serial instance
HardwareSerial modemSerial(1);  // Use UART1

// Create modem instance
QuectelEC200U modem(&modemSerial, MODEM_BAUD);

// HTTPS server configuration
const char* HTTPS_SERVER = "httpbin.org";  // Example HTTPS test server
const int HTTPS_PORT = 443;
const char* API_ENDPOINT = "/post";

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }

    Serial.println("\n=====================================");
    Serial.println("Quectel EC200U Example Starting...");
    Serial.println("=====================================\n");

    // Initialize modem serial with specific pins
    modemSerial.begin(MODEM_BAUD, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN);

    // Initialize modem
    Serial.println("Initializing modem...");
    if (!modem.begin()) {
        Serial.println("Failed to initialize modem!");
        while (1) {
            delay(1000);
        }
    }
    Serial.println("Modem initialized successfully!\n");

    // Get modem info
    String imei;
    if (modem.getIMEI(imei)) {
        Serial.print("Modem IMEI: ");
        Serial.println(imei);
    }

    // Check signal quality
    int rssi, ber;
    if (modem.getSignalQuality(rssi, ber)) {
        Serial.print("Signal Quality - RSSI: ");
        Serial.print(rssi);
        Serial.print(", BER: ");
        Serial.println(ber);
    }

    // Check network registration
    Serial.println("\nChecking network registration...");
    int status;
    int retries = 0;
    while (retries < 30) {
        if (modem.getNetworkStatus(status)) {
            Serial.print("Network status: ");
            Serial.print(status);
            if (status == 1 || status == 5) {  // 1=registered home, 5=registered roaming
                Serial.println(" - Registered!");
                break;
            } else {
                Serial.println(" - Not registered, waiting...");
            }
        }
        delay(2000);
        retries++;
    }

    Serial.println("\n=====================================");
    Serial.println("Starting main demo tasks...");
    Serial.println("=====================================\n");
}

void loop() {
    Serial.println("\n--- Task Menu ---");
    Serial.println("1. Get GPS Location");
    Serial.println("2. Connect to HTTPS Server and Send Data");
    Serial.println("3. Get Current Time");
    Serial.println("4. Run All Tasks");
    Serial.println("5. Exit");
    Serial.println("\nEnter your choice: ");

    // Wait for user input
    while (!Serial.available()) {
        delay(100);
    }

    char choice = Serial.read();
    // Clear any remaining characters
    while (Serial.available()) {
        Serial.read();
    }

    switch (choice) {
        case '1':
            demoGPSLocation();
            break;
        case '2':
            demoHTTPSConnection();
            break;
        case '3':
            demoGetTime();
            break;
        case '4':
            runAllDemos();
            break;
        case '5':
            Serial.println("Exiting...");
            while (1) {
                delay(1000);
            }
            break;
        default:
            Serial.println("Invalid choice. Please try again.");
    }

    delay(2000);
}

// ========== Demo Functions ==========

void demoGPSLocation() {
    Serial.println("\n=== GPS Location Demo ===");

    // Turn on GNSS
    Serial.println("Turning on GNSS...");
    if (!modem.gnssOn()) {
        Serial.println("Failed to turn on GNSS!");
        // Try to check error
        return;
    }
    Serial.println("GNSS turned on successfully!");

    // Get position
    Serial.println("\nGetting GPS position (this may take 30-60 seconds for first fix)...");

    GNSSPosition position;
    if (modem.getPosition(position, GNSS_FORMAT_DECIMAL_DEGREES, 30, 2000)) {
        Serial.println("\n--- GPS Position Acquired ---");
        Serial.print("Latitude: ");
        Serial.println(position.latitude, 6);
        Serial.print("Longitude: ");
        Serial.println(position.longitude, 6);
        Serial.print("Altitude: ");
        Serial.print(position.altitude);
        Serial.println(" meters");
        Serial.print("Speed: ");
        Serial.print(position.speedKmh);
        Serial.println(" km/h");
        Serial.print("Fix Mode: ");
        Serial.println(position.fixMode == 2 ? "2D" : "3D");
        Serial.print("Satellites: ");
        Serial.println(position.numSatellites);
        Serial.print("HDOP: ");
        Serial.println(position.hdop);
        Serial.print("UTC Time: ");
        Serial.println(position.utcTime);
        Serial.print("Date: ");
        Serial.println(position.date);
    } else {
        Serial.println("Failed to get GPS position!");
        if (position.lastError != 0) {
            Serial.print("Error: ");
            Serial.println(modem.getErrorDescription(AT_CME_ERROR - position.lastError));
        }
    }

    // Alternative simple method
    Serial.println("\nTrying simple coordinate method...");
    double lat, lon;
    if (modem.getCoordinates(lat, lon)) {
        Serial.print("Simple Coordinates - Lat: ");
        Serial.print(lat, 6);
        Serial.print(", Lon: ");
        Serial.println(lon, 6);
    }

    // Turn off GNSS to save power
    Serial.println("\nTurning off GNSS...");
    modem.gnssOff();
}

void demoHTTPSConnection() {
    Serial.println("\n=== HTTPS Connection Demo ===");

    // Initialize SSL
    Serial.println("Initializing SSL...");
    if (!modem.sslBegin(1, 1, 4)) {  // Context 1, SSL context 1, TLS all versions
        Serial.println("Failed to initialize SSL!");
        return;
    }
    Serial.println("SSL initialized!");

    // Connect to HTTPS server
    SSLConnectionState sslState;
    Serial.print("Connecting to ");
    Serial.print(HTTPS_SERVER);
    Serial.print(":");
    Serial.print(HTTPS_PORT);
    Serial.println(" in transparent mode...");

    if (modem.httpsConnect(HTTPS_SERVER, HTTPS_PORT, sslState)) {
        Serial.println("Connected successfully!");
        Serial.print("Client ID: ");
        Serial.println(sslState.clientID);

        // Prepare JSON data to send
        String jsonData = "{\"device\":\"EC200U\",\"message\":\"Hello from ESP32\",\"timestamp\":12345}";

        // Build HTTP POST request
        String httpRequest = "POST ";
        httpRequest += API_ENDPOINT;
        httpRequest += " HTTP/1.1\r\n";
        httpRequest += "Host: ";
        httpRequest += HTTPS_SERVER;
        httpRequest += "\r\n";
        httpRequest += "User-Agent: QuectelEC200U/1.0\r\n";
        httpRequest += "Content-Type: application/json\r\n";
        httpRequest += "Content-Length: ";
        httpRequest += String(jsonData.length());
        httpRequest += "\r\n";
        httpRequest += "Connection: close\r\n\r\n";
        httpRequest += jsonData;

        Serial.println("\nSending HTTP POST request...");
        Serial.println("Request:");
        Serial.println(httpRequest);

        if (modem.httpsSend(httpRequest)) {
            Serial.println("Request sent successfully!");

            // Read response
            Serial.println("\nReading response...");
            delay(1000);  // Give server time to respond

            SSLReceiveData receiveData;
            String fullResponse = "";
            int attempts = 0;

            while (attempts < 50) {  // Try for up to 5 seconds
                if (modem.httpsReceive(receiveData, 1500)) {
                    if (receiveData.dataAvailable) {
                        fullResponse += receiveData.data;
                        Serial.print("Received ");
                        Serial.print(receiveData.dataLength);
                        Serial.println(" bytes");

                        // Check if we have complete response
                        if (fullResponse.indexOf("0\r\n\r\n") >= 0 ||  // Chunked encoding end
                            (fullResponse.indexOf("\r\n\r\n") >= 0 &&
                             fullResponse.indexOf("Content-Length:") >= 0)) {
                            break;
                        }
                    }
                }
                delay(100);
                attempts++;
            }

            if (fullResponse.length() > 0) {
                Serial.println("\n--- Server Response ---");
                Serial.println(fullResponse);
            } else {
                Serial.println("No response received!");
            }
        } else {
            Serial.println("Failed to send request!");
        }

        // Exit transparent mode
        Serial.println("\nExiting transparent mode...");
        if (modem.exitTransparentMode()) {
            Serial.println("Exited transparent mode successfully!");
        }

        // Disconnect
        Serial.println("Disconnecting...");
        modem.httpsDisconnect(sslState.clientID);
        Serial.println("Disconnected!");

    } else {
        Serial.println("Failed to connect!");
        Serial.print("SSL Error: ");
        Serial.println(sslState.sslError);
        Serial.print("Description: ");
        Serial.println(modem.getErrorDescription(sslState.sslError));
    }
}

void demoGetTime() {
    Serial.println("\n=== Time Functions Demo ===");

    // Get network synchronized time
    Serial.println("\n1. Getting Network Time...");
    NetworkTime netTime;

    // Try different modes
    Serial.println("\n   a. Last synchronized time:");
    if (modem.getNetworkTime(netTime, TIME_MODE_LAST_SYNC)) {
        if (netTime.valid) {
            printTimeInfo(netTime);
        } else {
            Serial.println("      No time synchronized yet");
        }
    } else {
        Serial.println("      Failed to get time");
        if (netTime.lastError != 0) {
            Serial.print("      Error: ");
            Serial.println(modem.getErrorDescription(AT_CME_ERROR - netTime.lastError));
        }
    }

    Serial.println("\n   b. Current GMT time:");
    if (modem.getNetworkTime(netTime, TIME_MODE_GMT)) {
        if (netTime.valid) {
            printTimeInfo(netTime);
        } else {
            Serial.println("      No time available");
        }
    }

    Serial.println("\n   c. Current local time:");
    if (modem.getNetworkTime(netTime, TIME_MODE_LOCAL)) {
        if (netTime.valid) {
            printTimeInfo(netTime);
        } else {
            Serial.println("      No time available");
        }
    }

    // Get RTC time
    Serial.println("\n2. Getting RTC Time...");
    NetworkTime rtcTime;
    if (modem.getRTCTime(rtcTime)) {
        printTimeInfo(rtcTime);
    } else {
        Serial.println("   Failed to get RTC time");
    }

    // Set RTC time manually
    Serial.println("\n3. Setting RTC Time manually...");
    // Set to Nov 26, 2024, 14:30:00, GMT+8
    if (modem.setRTCTime(24, 11, 26, 14, 30, 0, 32)) {  // 32 quarters = 8 hours
        Serial.println("   RTC time set successfully!");

        // Read it back
        if (modem.getRTCTime(rtcTime)) {
            Serial.println("   New RTC time:");
            printTimeInfo(rtcTime);
        }
    } else {
        Serial.println("   Failed to set RTC time");
    }

    // Sync time from network
    Serial.println("\n4. Syncing time from network...");
    if (modem.syncTimeFromNetwork()) {
        Serial.println("   Time synchronized successfully!");

        // Read the synced time
        if (modem.getRTCTime(rtcTime)) {
            Serial.println("   Synchronized time:");
            printTimeInfo(rtcTime);
        }
    } else {
        Serial.println("   Failed to sync time from network");
        Serial.println("   (Network may not provide time sync)");
    }

    // Simple time string method
    Serial.println("\n5. Getting time as string...");
    String timeStr;
    if (modem.getCurrentTime(timeStr, TIME_MODE_LOCAL)) {
        Serial.print("   Current time string: ");
        Serial.println(timeStr);
    }
}

void printTimeInfo(const NetworkTime& time) {
    Serial.print("      Date: ");
    Serial.print(time.year);
    Serial.print("/");
    Serial.print(time.month < 10 ? "0" : "");
    Serial.print(time.month);
    Serial.print("/");
    Serial.print(time.day < 10 ? "0" : "");
    Serial.println(time.day);

    Serial.print("      Time: ");
    Serial.print(time.hour < 10 ? "0" : "");
    Serial.print(time.hour);
    Serial.print(":");
    Serial.print(time.minute < 10 ? "0" : "");
    Serial.print(time.minute);
    Serial.print(":");
    Serial.print(time.second < 10 ? "0" : "");
    Serial.println(time.second);

    Serial.print("      Timezone: GMT");
    if (time.timezoneHours >= 0) Serial.print("+");
    Serial.println(time.timezoneHours);

    if (time.daylightSaving) {
        Serial.println("      Daylight Saving: Yes");
    }

    Serial.print("      Raw: ");
    Serial.println(time.dateTime);
}

void runAllDemos() {
    Serial.println("\n=== Running All Demos ===");

    Serial.println("\n[1/3] GPS Location Demo");
    demoGPSLocation();

    delay(2000);

    Serial.println("\n[2/3] HTTPS Connection Demo");
    demoHTTPSConnection();

    delay(2000);

    Serial.println("\n[3/3] Time Functions Demo");
    demoGetTime();

    Serial.println("\n=== All Demos Complete ===");
}