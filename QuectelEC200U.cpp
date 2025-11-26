/**
 * QuectelEC200U.cpp - ESP32 Arduino Library Implementation for Quectel EC200U Modem
 *
 * This library provides comprehensive APIs for:
 * - GNSS/GPS positioning (latitude/longitude)
 * - HTTPS connections using SSL transparent mode
 * - Time synchronization from network and RTC
 *
 * Author: ESP32 Arduino Library
 * Date: 2024
 */

#include "QuectelEC200U.h"

// Constructor
QuectelEC200U::QuectelEC200U(HardwareSerial* serial, uint32_t baud) {
    modemSerial = serial;
    baudRate = baud;
    timeout = 5000;  // Default 5 second timeout
    transparentMode = false;
    currentSSLClient = -1;
}

// ========== Basic Modem Control ==========

bool QuectelEC200U::begin() {
    modemSerial->begin(baudRate);
    delay(1000);  // Give modem time to initialize

    // Clear any pending data
    clearBuffer();

    // Test AT command
    if (!testAT()) {
        DEBUG_PRINTLN("Modem not responding to AT commands");
        return false;
    }

    // Disable echo
    sendATCommand("ATE0");

    // Set error reporting to verbose
    sendATCommand("AT+CMEE=2");

    return true;
}

bool QuectelEC200U::testAT() {
    for (int i = 0; i < 3; i++) {
        if (sendATCommand("AT")) {
            return true;
        }
        delay(500);
    }
    return false;
}

bool QuectelEC200U::reset() {
    return sendATCommand("AT+CFUN=1,1", 10000);  // Reset with 10s timeout
}

bool QuectelEC200U::getSignalQuality(int& rssi, int& ber) {
    String response;
    if (sendRawATCommand("AT+CSQ", response) == AT_OK) {
        int idx = response.indexOf("+CSQ: ");
        if (idx >= 0) {
            idx += 6;
            int commaIdx = response.indexOf(',', idx);
            if (commaIdx > idx) {
                rssi = response.substring(idx, commaIdx).toInt();
                ber = response.substring(commaIdx + 1).toInt();
                return true;
            }
        }
    }
    return false;
}

bool QuectelEC200U::getIMEI(String& imei) {
    String response;
    if (sendRawATCommand("AT+GSN", response) == AT_OK) {
        int idx = response.indexOf("\r\n");
        if (idx > 0) {
            imei = response.substring(0, idx);
            imei.trim();
            return true;
        }
    }
    return false;
}

bool QuectelEC200U::getNetworkStatus(int& status) {
    String response;
    if (sendRawATCommand("AT+CREG?", response) == AT_OK) {
        int idx = response.indexOf("+CREG: ");
        if (idx >= 0) {
            idx = response.indexOf(',', idx) + 1;
            status = response.substring(idx, idx + 1).toInt();
            return true;
        }
    }
    return false;
}

// ========== Helper Functions ==========

bool QuectelEC200U::sendATCommand(const String& command, unsigned long customTimeout) {
    clearBuffer();

    DEBUG_PRINT(">> ");
    DEBUG_PRINTLN(command);

    modemSerial->println(command);

    unsigned long timeoutMs = (customTimeout > 0) ? customTimeout : timeout;
    String response = readResponse(timeoutMs);

    DEBUG_PRINT("<< ");
    DEBUG_PRINTLN(response);

    return (response.indexOf("OK") >= 0);
}

String QuectelEC200U::readResponse(unsigned long customTimeout) {
    unsigned long timeoutMs = (customTimeout > 0) ? customTimeout : timeout;
    unsigned long startTime = millis();
    String response = "";

    while (millis() - startTime < timeoutMs) {
        while (modemSerial->available()) {
            char c = modemSerial->read();
            response += c;

            // Check for completion patterns
            if (response.endsWith("OK\r\n") ||
                response.endsWith("ERROR\r\n") ||
                response.endsWith("CONNECT\r\n") ||
                response.endsWith("NO CARRIER\r\n") ||
                response.endsWith("SEND OK\r\n") ||
                response.endsWith("SEND FAIL\r\n") ||
                response.indexOf("+CME ERROR:") >= 0) {
                return response;
            }
        }
        delay(10);
    }

    return response;
}

bool QuectelEC200U::waitForResponse(const String& expected, unsigned long customTimeout) {
    String response = readResponse(customTimeout);
    return (response.indexOf(expected) >= 0);
}

int QuectelEC200U::parseATResponse(const String& response) {
    if (response.indexOf("OK") >= 0) return AT_OK;
    if (response.indexOf("ERROR") >= 0) return AT_ERROR;
    if (response.indexOf("CONNECT") >= 0) return AT_CONNECT;
    if (response.indexOf("NO CARRIER") >= 0) return AT_NO_CARRIER;
    if (response.indexOf("SEND OK") >= 0) return AT_SEND_OK;
    if (response.indexOf("SEND FAIL") >= 0) return AT_SEND_FAIL;
    if (response.indexOf("+CME ERROR:") >= 0) {
        return AT_CME_ERROR - parseCMEError(response);
    }
    return AT_TIMEOUT;
}

int QuectelEC200U::parseCMEError(const String& response) {
    int idx = response.indexOf("+CME ERROR: ");
    if (idx >= 0) {
        idx += 12;
        int endIdx = response.indexOf('\r', idx);
        if (endIdx > idx) {
            return response.substring(idx, endIdx).toInt();
        }
    }
    return -1;
}

int QuectelEC200U::parseSSLError(const String& response) {
    int idx = response.indexOf("+QSSLOPEN: ");
    if (idx >= 0) {
        idx = response.indexOf(',', idx) + 1;
        int endIdx = response.indexOf('\r', idx);
        if (endIdx > idx) {
            return response.substring(idx, endIdx).toInt();
        }
    }
    return -1;
}

void QuectelEC200U::clearBuffer() {
    while (modemSerial->available()) {
        modemSerial->read();
    }
}

// ========== GNSS/GPS Functions ==========

bool QuectelEC200U::gnssBegin() {
    // Configure GNSS parameters if needed
    return sendATCommand("AT+QGPSCFG=\"nmeasrc\",1");  // Enable NMEA output
}

bool QuectelEC200U::gnssOn(int mode, int fixMaxTime) {
    String cmd = "AT+QGPS=" + String(mode);
    if (fixMaxTime != 30) {
        cmd += "," + String(fixMaxTime);
    }
    return sendATCommand(cmd);
}

bool QuectelEC200U::gnssOff() {
    return sendATCommand("AT+QGPSEND");
}

bool QuectelEC200U::getPosition(GNSSPosition& position, GNSSCoordFormat format,
                                int maxRetries, unsigned long retryDelay) {
    position.valid = false;
    position.lastError = 0;

    for (int retry = 0; retry < maxRetries; retry++) {
        String response;
        String cmd = "AT+QGPSLOC=" + String(format);
        int result = sendRawATCommand(cmd, response, 5000);

        if (result == AT_OK) {
            if (parseGNSSResponse(response, position, format)) {
                position.valid = true;
                return true;
            }
        } else if (result <= AT_CME_ERROR) {
            int cmeError = AT_CME_ERROR - result;
            position.lastError = cmeError;

            // Check if error is temporary (not fixed yet)
            if (cmeError == CME_NOT_FIXED_NOW) {
                DEBUG_PRINTLN("GNSS not fixed yet, retrying...");
                delay(retryDelay);
                continue;
            } else if (cmeError == CME_SESSION_NOT_ACTIVE) {
                DEBUG_PRINTLN("GNSS session not active, turning on GNSS...");
                if (gnssOn()) {
                    delay(2000);  // Give GNSS time to start
                    continue;
                }
            }
            // For other errors, stop retrying
            break;
        }

        delay(retryDelay);
    }

    return false;
}

bool QuectelEC200U::getCoordinates(double& latitude, double& longitude) {
    GNSSPosition position;
    if (getPosition(position, GNSS_FORMAT_DECIMAL_DEGREES)) {
        latitude = position.latitude;
        longitude = position.longitude;
        return true;
    }
    return false;
}

bool QuectelEC200U::isGNSSFixed() {
    GNSSPosition position;
    // Try to get position with just 1 retry
    return getPosition(position, GNSS_FORMAT_DECIMAL_DEGREES, 1, 100);
}

bool QuectelEC200U::parseGNSSResponse(const String& response, GNSSPosition& position,
                                      GNSSCoordFormat format) {
    int idx = response.indexOf("+QGPSLOC: ");
    if (idx < 0) return false;

    idx += 10;  // Skip "+QGPSLOC: "

    // Parse UTC time
    int nextComma = response.indexOf(',', idx);
    if (nextComma <= idx) return false;
    position.utcTime = response.substring(idx, nextComma);
    idx = nextComma + 1;

    // Parse latitude
    nextComma = response.indexOf(',', idx);
    if (nextComma <= idx) return false;
    position.latitudeStr = response.substring(idx, nextComma);
    position.latitude = convertCoordinateToDecimal(position.latitudeStr, false, format);
    idx = nextComma + 1;

    // Parse longitude
    nextComma = response.indexOf(',', idx);
    if (nextComma <= idx) return false;
    position.longitudeStr = response.substring(idx, nextComma);
    position.longitude = convertCoordinateToDecimal(position.longitudeStr, true, format);
    idx = nextComma + 1;

    // Parse HDOP
    nextComma = response.indexOf(',', idx);
    if (nextComma <= idx) return false;
    position.hdop = response.substring(idx, nextComma).toFloat();
    idx = nextComma + 1;

    // Parse altitude
    nextComma = response.indexOf(',', idx);
    if (nextComma <= idx) return false;
    position.altitude = response.substring(idx, nextComma).toFloat();
    idx = nextComma + 1;

    // Parse fix mode
    nextComma = response.indexOf(',', idx);
    if (nextComma <= idx) return false;
    position.fixMode = response.substring(idx, nextComma).toInt();
    idx = nextComma + 1;

    // Parse course over ground
    nextComma = response.indexOf(',', idx);
    if (nextComma <= idx) return false;
    String cog = response.substring(idx, nextComma);
    position.courseOverGround = cog.toFloat();
    idx = nextComma + 1;

    // Parse speed in km/h
    nextComma = response.indexOf(',', idx);
    if (nextComma <= idx) return false;
    position.speedKmh = response.substring(idx, nextComma).toFloat();
    idx = nextComma + 1;

    // Parse speed in knots
    nextComma = response.indexOf(',', idx);
    if (nextComma <= idx) return false;
    position.speedKnots = response.substring(idx, nextComma).toFloat();
    idx = nextComma + 1;

    // Parse date
    nextComma = response.indexOf(',', idx);
    if (nextComma <= idx) return false;
    position.date = response.substring(idx, nextComma);
    idx = nextComma + 1;

    // Parse number of satellites
    int endIdx = response.indexOf('\r', idx);
    if (endIdx > idx) {
        position.numSatellites = response.substring(idx, endIdx).toInt();
    }

    return true;
}

double QuectelEC200U::convertCoordinateToDecimal(const String& coord, bool isLongitude,
                                                 GNSSCoordFormat format) {
    double result = 0.0;

    if (format == GNSS_FORMAT_DECIMAL_DEGREES) {
        // Already in decimal format
        result = coord.toDouble();
    } else if (format == GNSS_FORMAT_DEGREES_MINUTES || format == GNSS_FORMAT_DEGREES_MINUTES_EXT) {
        // Parse degrees and minutes
        String coordStr = coord;
        bool negative = false;

        // Check for hemisphere indicator
        if (format == GNSS_FORMAT_DEGREES_MINUTES) {
            // Format: ddmm.mmmmN/S or dddmm.mmmmE/W
            char hemisphere = coordStr.charAt(coordStr.length() - 1);
            coordStr = coordStr.substring(0, coordStr.length() - 1);
            negative = (hemisphere == 'S' || hemisphere == 'W');
        } else {
            // Format: ddmm.mmmmmm,N/S or dddmm.mmmmmm,E/W
            int commaIdx = coordStr.indexOf(',');
            if (commaIdx > 0) {
                String hemisphere = coordStr.substring(commaIdx + 1);
                coordStr = coordStr.substring(0, commaIdx);
                negative = (hemisphere == "S" || hemisphere == "W");
            }
        }

        // Parse degrees and minutes
        int dotIdx = coordStr.indexOf('.');
        if (dotIdx > 0) {
            String degMin = coordStr.substring(0, dotIdx);
            String minFrac = coordStr.substring(dotIdx + 1);

            int degrees, minutes;
            if (isLongitude) {
                // Longitude: dddmm
                degrees = degMin.substring(0, degMin.length() - 2).toInt();
                minutes = degMin.substring(degMin.length() - 2).toInt();
            } else {
                // Latitude: ddmm
                degrees = degMin.substring(0, degMin.length() - 2).toInt();
                minutes = degMin.substring(degMin.length() - 2).toInt();
            }

            double minutesFull = minutes + (minFrac.toDouble() / pow(10, minFrac.length()));
            result = degrees + (minutesFull / 60.0);

            if (negative) result = -result;
        }
    }

    return result;
}

// ========== SSL/HTTPS Functions ==========

bool QuectelEC200U::sslBegin(int contextID, int sslContextID, int sslVersion) {
    // Activate PDP context first
    String activateCmd = "AT+QIACT=" + String(contextID);
    if (!sendATCommand(activateCmd, 30000)) {  // 30s timeout for network activation
        DEBUG_PRINTLN("Failed to activate PDP context");
        return false;
    }

    // Configure SSL version
    String configCmd = "AT+QSSLCFG=\"sslversion\"," + String(sslContextID) + "," + String(sslVersion);
    if (!sendATCommand(configCmd)) {
        DEBUG_PRINTLN("Failed to configure SSL version");
        return false;
    }

    // Configure cipher suite (use all available)
    configCmd = "AT+QSSLCFG=\"ciphersuite\"," + String(sslContextID) + ",0xFFFF";
    if (!sendATCommand(configCmd)) {
        DEBUG_PRINTLN("Failed to configure cipher suite");
        return false;
    }

    // Configure negotiation time
    configCmd = "AT+QSSLCFG=\"negotiatetime\"," + String(sslContextID) + ",300";
    if (!sendATCommand(configCmd)) {
        DEBUG_PRINTLN("Failed to configure negotiation time");
        return false;
    }

    return true;
}

bool QuectelEC200U::sslConfigure(int sslContextID, const String& cipherSuite, int negotiateTime) {
    // Configure negotiation time
    String configCmd = "AT+QSSLCFG=\"negotiatetime\"," + String(sslContextID) + "," + String(negotiateTime);
    if (!sendATCommand(configCmd)) {
        return false;
    }

    // Configure cipher suite if provided
    if (cipherSuite.length() > 0) {
        configCmd = "AT+QSSLCFG=\"ciphersuite\"," + String(sslContextID) + "," + cipherSuite;
        if (!sendATCommand(configCmd)) {
            return false;
        }
    }

    return true;
}

bool QuectelEC200U::httpsConnect(const String& serverAddress, int port,
                                 SSLConnectionState& state, int contextID,
                                 int sslContextID, int clientID) {
    state.connected = false;
    state.clientID = clientID;
    state.mode = SSL_MODE_TRANSPARENT;
    state.serverAddr = serverAddress;
    state.serverPort = port;
    state.sslError = 0;

    // Build connection command for transparent mode
    String cmd = "AT+QSSLOPEN=" + String(contextID) + "," + String(sslContextID) + "," +
                 String(clientID) + ",\"" + serverAddress + "\"," + String(port) + ",2";

    clearBuffer();
    DEBUG_PRINT(">> ");
    DEBUG_PRINTLN(cmd);
    modemSerial->println(cmd);

    // Wait for CONNECT response (up to 150s + negotiation time)
    unsigned long startTime = millis();
    unsigned long timeoutMs = 450000;  // 150s + 300s negotiation
    String response = "";

    while (millis() - startTime < timeoutMs) {
        while (modemSerial->available()) {
            char c = modemSerial->read();
            response += c;

            if (response.indexOf("CONNECT") >= 0) {
                DEBUG_PRINTLN("<< CONNECT");
                state.connected = true;
                transparentMode = true;
                currentSSLClient = clientID;
                return true;
            }

            if (response.indexOf("ERROR") >= 0) {
                DEBUG_PRINT("<< ");
                DEBUG_PRINTLN(response);
                return false;
            }

            if (response.indexOf("+QSSLOPEN:") >= 0) {
                // Parse SSL error
                int commaIdx = response.indexOf(',');
                if (commaIdx > 0) {
                    state.sslError = response.substring(commaIdx + 1).toInt();
                }
                DEBUG_PRINT("<< SSL Error: ");
                DEBUG_PRINTLN(state.sslError);
                return false;
            }
        }
        delay(10);
    }

    return false;
}

bool QuectelEC200U::httpsSend(const String& data) {
    if (!transparentMode) {
        DEBUG_PRINTLN("Not in transparent mode");
        return false;
    }

    modemSerial->print(data);
    return true;
}

bool QuectelEC200U::httpsSendBytes(const uint8_t* data, size_t length) {
    if (!transparentMode) {
        DEBUG_PRINTLN("Not in transparent mode");
        return false;
    }

    modemSerial->write(data, length);
    return true;
}

bool QuectelEC200U::httpsReceive(SSLReceiveData& receiveData, int maxLength) {
    receiveData.dataAvailable = false;
    receiveData.data = "";
    receiveData.dataLength = 0;

    if (transparentMode) {
        // In transparent mode, data comes directly
        unsigned long startTime = millis();
        while (modemSerial->available() && receiveData.dataLength < maxLength) {
            char c = modemSerial->read();
            receiveData.data += c;
            receiveData.dataLength++;

            // Check for disconnection
            if (receiveData.data.endsWith("NO CARRIER")) {
                transparentMode = false;
                currentSSLClient = -1;
                return false;
            }
        }

        if (receiveData.dataLength > 0) {
            receiveData.dataAvailable = true;
        }
    } else {
        // Buffer mode - use AT+QSSLRECV
        String cmd = "AT+QSSLRECV=" + String(currentSSLClient) + "," + String(maxLength);
        String response;

        if (sendRawATCommand(cmd, response) == AT_OK) {
            int idx = response.indexOf("+QSSLRECV: ");
            if (idx >= 0) {
                idx += 11;
                int endIdx = response.indexOf("\r\n", idx);
                if (endIdx > idx) {
                    int receivedLen = response.substring(idx, endIdx).toInt();
                    if (receivedLen > 0) {
                        receiveData.dataAvailable = true;
                        receiveData.dataLength = receivedLen;
                        // Extract actual data after the length line
                        int dataStart = endIdx + 2;
                        receiveData.data = response.substring(dataStart, dataStart + receivedLen);
                    }
                }
            }
        }
    }

    return receiveData.dataAvailable;
}

bool QuectelEC200U::httpsDataAvailable(int clientID, int& availableBytes) {
    if (transparentMode) {
        availableBytes = modemSerial->available();
        return (availableBytes > 0);
    }

    // Query buffer status
    String cmd = "AT+QSSLRECV=" + String(clientID) + ",0";
    String response;

    if (sendRawATCommand(cmd, response) == AT_OK) {
        int idx = response.indexOf("+QSSLRECV: ");
        if (idx >= 0) {
            idx += 11;
            // Parse: total_receive_length,have_read_length,unread_length
            int comma1 = response.indexOf(',', idx);
            int comma2 = response.indexOf(',', comma1 + 1);
            int endIdx = response.indexOf('\r', comma2);

            if (comma2 > comma1 && endIdx > comma2) {
                availableBytes = response.substring(comma2 + 1, endIdx).toInt();
                return (availableBytes > 0);
            }
        }
    }

    return false;
}

bool QuectelEC200U::exitTransparentMode() {
    if (!transparentMode) {
        return true;
    }

    // Wait 1 second of no data
    delay(1000);

    // Send +++ escape sequence
    modemSerial->print("+++");

    // Wait 1 second after
    delay(1000);

    // Check for OK response
    String response = readResponse(2000);
    if (response.indexOf("OK") >= 0) {
        transparentMode = false;
        return true;
    }

    return false;
}

bool QuectelEC200U::httpsDisconnect(int clientID) {
    // Exit transparent mode first if needed
    if (transparentMode && clientID == currentSSLClient) {
        if (!exitTransparentMode()) {
            DEBUG_PRINTLN("Failed to exit transparent mode");
        }
    }

    String cmd = "AT+QSSLCLOSE=" + String(clientID);
    bool result = sendATCommand(cmd, 10000);

    if (result && clientID == currentSSLClient) {
        currentSSLClient = -1;
    }

    return result;
}

bool QuectelEC200U::httpsGET(const String& host, const String& path, String& response) {
    // Build HTTP GET request
    String request = "GET " + path + " HTTP/1.1\r\n";
    request += "Host: " + host + "\r\n";
    request += "User-Agent: QuectelEC200U/1.0\r\n";
    request += "Accept: */*\r\n";
    request += "Connection: close\r\n\r\n";

    // Send request
    if (!httpsSend(request)) {
        return false;
    }

    // Read response
    response = "";
    unsigned long startTime = millis();
    unsigned long timeoutMs = 30000;  // 30 second timeout

    while (millis() - startTime < timeoutMs) {
        SSLReceiveData receiveData;
        if (httpsReceive(receiveData, 1500)) {
            response += receiveData.data;

            // Check if we've received the complete response
            if (response.indexOf("\r\n\r\n") >= 0) {
                // Headers complete, check for content
                int contentLengthIdx = response.indexOf("Content-Length: ");
                if (contentLengthIdx >= 0) {
                    contentLengthIdx += 16;
                    int endIdx = response.indexOf('\r', contentLengthIdx);
                    int contentLength = response.substring(contentLengthIdx, endIdx).toInt();

                    int headerEndIdx = response.indexOf("\r\n\r\n") + 4;
                    int currentBodyLength = response.length() - headerEndIdx;

                    if (currentBodyLength >= contentLength) {
                        return true;
                    }
                } else if (response.indexOf("Transfer-Encoding: chunked") >= 0) {
                    // For chunked encoding, check for end marker
                    if (response.endsWith("0\r\n\r\n")) {
                        return true;
                    }
                }
            }
        }
        delay(100);
    }

    return (response.length() > 0);
}

bool QuectelEC200U::httpsPOST(const String& host, const String& path,
                              const String& contentType, const String& body,
                              String& response) {
    // Build HTTP POST request
    String request = "POST " + path + " HTTP/1.1\r\n";
    request += "Host: " + host + "\r\n";
    request += "User-Agent: QuectelEC200U/1.0\r\n";
    request += "Content-Type: " + contentType + "\r\n";
    request += "Content-Length: " + String(body.length()) + "\r\n";
    request += "Accept: */*\r\n";
    request += "Connection: close\r\n\r\n";
    request += body;

    // Send request
    if (!httpsSend(request)) {
        return false;
    }

    // Read response (same as GET)
    response = "";
    unsigned long startTime = millis();
    unsigned long timeoutMs = 30000;

    while (millis() - startTime < timeoutMs) {
        SSLReceiveData receiveData;
        if (httpsReceive(receiveData, 1500)) {
            response += receiveData.data;

            // Check if we've received the complete response
            if (response.indexOf("\r\n\r\n") >= 0) {
                int contentLengthIdx = response.indexOf("Content-Length: ");
                if (contentLengthIdx >= 0) {
                    contentLengthIdx += 16;
                    int endIdx = response.indexOf('\r', contentLengthIdx);
                    int contentLength = response.substring(contentLengthIdx, endIdx).toInt();

                    int headerEndIdx = response.indexOf("\r\n\r\n") + 4;
                    int currentBodyLength = response.length() - headerEndIdx;

                    if (currentBodyLength >= contentLength) {
                        return true;
                    }
                } else if (response.indexOf("Transfer-Encoding: chunked") >= 0) {
                    if (response.endsWith("0\r\n\r\n")) {
                        return true;
                    }
                }
            }
        }
        delay(100);
    }

    return (response.length() > 0);
}

// ========== Time Functions ==========

bool QuectelEC200U::getNetworkTime(NetworkTime& time, TimeQueryMode mode) {
    time.valid = false;
    time.lastError = 0;

    String cmd = "AT+QLTS=" + String(mode);
    String response;
    int result = sendRawATCommand(cmd, response);

    if (result == AT_OK) {
        return parseNetworkTime(response, time);
    } else if (result <= AT_CME_ERROR) {
        time.lastError = AT_CME_ERROR - result;
    }

    return false;
}

bool QuectelEC200U::getCurrentTime(String& timeStr, TimeQueryMode mode) {
    NetworkTime time;
    if (getNetworkTime(time, mode)) {
        timeStr = time.dateTime;
        return true;
    }
    return false;
}

bool QuectelEC200U::setRTCTime(int year, int month, int day,
                               int hour, int minute, int second,
                               int timezone) {
    // Format: "yy/MM/dd,hh:mm:ss±zz"
    String timeStr = String(year % 100) + "/" +
                    String(month < 10 ? "0" : "") + String(month) + "/" +
                    String(day < 10 ? "0" : "") + String(day) + "," +
                    String(hour < 10 ? "0" : "") + String(hour) + ":" +
                    String(minute < 10 ? "0" : "") + String(minute) + ":" +
                    String(second < 10 ? "0" : "") + String(second);

    if (timezone >= 0) {
        timeStr += "+" + String(timezone < 10 ? "0" : "") + String(timezone);
    } else {
        timeStr += String(timezone);
    }

    String cmd = "AT+CCLK=\"" + timeStr + "\"";
    return sendATCommand(cmd);
}

bool QuectelEC200U::getRTCTime(NetworkTime& time) {
    time.valid = false;

    String response;
    if (sendRawATCommand("AT+CCLK?", response) == AT_OK) {
        int idx = response.indexOf("+CCLK: \"");
        if (idx >= 0) {
            idx += 8;
            int endIdx = response.indexOf('\"', idx);
            if (endIdx > idx) {
                String timeStr = response.substring(idx, endIdx);
                time.dateTime = timeStr;

                // Parse the time string
                // Format: "yy/MM/dd,hh:mm:ss±zz"
                if (timeStr.length() >= 17) {
                    time.year = 2000 + timeStr.substring(0, 2).toInt();
                    time.month = timeStr.substring(3, 5).toInt();
                    time.day = timeStr.substring(6, 8).toInt();
                    time.hour = timeStr.substring(9, 11).toInt();
                    time.minute = timeStr.substring(12, 14).toInt();
                    time.second = timeStr.substring(15, 17).toInt();

                    if (timeStr.length() >= 20) {
                        String tzStr = timeStr.substring(17);
                        time.timezone = tzStr.toInt();
                        time.timezoneHours = time.timezone / 4;  // Convert quarters to hours
                    }

                    time.valid = true;
                    return true;
                }
            }
        }
    }

    return false;
}

bool QuectelEC200U::syncTimeFromNetwork() {
    // Try to get network time
    NetworkTime time;
    if (getNetworkTime(time, TIME_MODE_LOCAL)) {
        // Set RTC with network time
        return setRTCTime(time.year % 100, time.month, time.day,
                         time.hour, time.minute, time.second,
                         time.timezone);
    }
    return false;
}

bool QuectelEC200U::parseNetworkTime(const String& response, NetworkTime& time) {
    int idx = response.indexOf("+QLTS: \"");
    if (idx < 0) return false;

    idx += 8;
    int endIdx = response.indexOf('\"', idx);
    if (endIdx <= idx) {
        // Empty response means never synchronized
        time.dateTime = "";
        return false;
    }

    time.dateTime = response.substring(idx, endIdx);

    // Parse: "YYYY/MM/dd,hh:mm:ss±zz,d"
    if (time.dateTime.length() >= 22) {
        time.year = time.dateTime.substring(0, 4).toInt();
        time.month = time.dateTime.substring(5, 7).toInt();
        time.day = time.dateTime.substring(8, 10).toInt();
        time.hour = time.dateTime.substring(11, 13).toInt();
        time.minute = time.dateTime.substring(14, 16).toInt();
        time.second = time.dateTime.substring(17, 19).toInt();

        // Parse timezone
        String tzStr = time.dateTime.substring(19, 22);
        time.timezone = tzStr.toInt();
        time.timezoneHours = time.timezone / 4;  // Convert quarters to hours

        // Parse DST if present
        if (time.dateTime.length() >= 24) {
            time.daylightSaving = (time.dateTime.charAt(23) == '1');
        }

        time.valid = true;
        return true;
    }

    return false;
}

// ========== Error Handling ==========

String QuectelEC200U::getErrorDescription(int errorCode) {
    if (errorCode == AT_OK) return "OK";
    if (errorCode == AT_ERROR) return "ERROR";
    if (errorCode == AT_TIMEOUT) return "Timeout";
    if (errorCode == AT_CONNECT) return "Connected";
    if (errorCode == AT_NO_CARRIER) return "No carrier";
    if (errorCode == AT_SEND_OK) return "Send OK";
    if (errorCode == AT_SEND_FAIL) return "Send failed";

    if (errorCode <= AT_CME_ERROR) {
        int cmeError = AT_CME_ERROR - errorCode;
        switch (cmeError) {
            case CME_PHONE_FAILURE: return "Phone failure";
            case CME_NO_CONNECTION: return "No connection to phone";
            case CME_NOT_ALLOWED: return "Operation not allowed";
            case CME_NOT_SUPPORTED: return "Operation not supported";
            case CME_SIM_NOT_INSERTED: return "SIM not inserted";
            case CME_SIM_PIN_REQUIRED: return "SIM PIN required";
            case CME_SIM_FAILURE: return "SIM failure";
            case CME_SIM_BUSY: return "SIM busy";
            case CME_MEMORY_FULL: return "Memory full";
            case CME_INVALID_PARAMS: return "Invalid parameters";
            case CME_GNSS_BUSY: return "GNSS subsystem busy";
            case CME_SESSION_NOT_ACTIVE: return "GNSS session not active";
            case CME_OP_TIMEOUT: return "Operation timeout";
            case CME_NOT_FIXED_NOW: return "GNSS not fixed now";
            default: return "CME Error " + String(cmeError);
        }
    }

    // SSL errors
    switch (errorCode) {
        case SSL_UNKNOWN_ERROR: return "SSL unknown error";
        case SSL_OP_BLOCKED: return "SSL operation blocked";
        case SSL_INVALID_PARAM: return "SSL invalid parameter";
        case SSL_MEMORY_NOT_ENOUGH: return "SSL memory not enough";
        case SSL_CREATE_SOCKET_FAILED: return "SSL create socket failed";
        case SSL_DNS_PARSE_FAILED: return "SSL DNS parse failed";
        case SSL_SOCKET_CONN_FAILED: return "SSL connection failed";
        case SSL_SOCKET_CLOSED: return "SSL socket closed";
        case SSL_OP_TIMEOUT: return "SSL operation timeout";
        case SSL_HANDSHAKE_FAIL: return "SSL handshake failed";
        default: return "Unknown error " + String(errorCode);
    }
}

int QuectelEC200U::getLastSSLError() {
    String response;
    if (sendRawATCommand("AT+QIGETERROR", response) == AT_OK) {
        // Parse error response
        int idx = response.indexOf("+QIGETERROR: ");
        if (idx >= 0) {
            idx += 13;
            int endIdx = response.indexOf('\r', idx);
            if (endIdx > idx) {
                return response.substring(idx, endIdx).toInt();
            }
        }
    }
    return 0;
}

int QuectelEC200U::sendRawATCommand(const String& command, String& response, unsigned long customTimeout) {
    clearBuffer();

    DEBUG_PRINT(">> ");
    DEBUG_PRINTLN(command);

    modemSerial->println(command);

    unsigned long timeoutMs = (customTimeout > 0) ? customTimeout : timeout;
    response = readResponse(timeoutMs);

    DEBUG_PRINT("<< ");
    DEBUG_PRINTLN(response);

    return parseATResponse(response);
}