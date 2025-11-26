/**
 * QuectelEC200U.h - ESP32 Arduino Library for Quectel EC200U Modem
 *
 * This library provides comprehensive APIs for:
 * - GNSS/GPS positioning (latitude/longitude)
 * - HTTPS connections using SSL transparent mode
 * - Time synchronization from network and RTC
 *
 * Author: ESP32 Arduino Library
 * Date: 2024
 */

#ifndef QUECTEL_EC200U_H
#define QUECTEL_EC200U_H

#include <Arduino.h>
#include <HardwareSerial.h>

// Debug output control
#define QUECTEL_DEBUG 1

#if QUECTEL_DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif

// AT Command Response Codes
enum ATResponseCode {
    AT_OK = 0,
    AT_ERROR = -1,
    AT_TIMEOUT = -2,
    AT_CONNECT = -3,
    AT_NO_CARRIER = -4,
    AT_SEND_OK = -5,
    AT_SEND_FAIL = -6,
    AT_CME_ERROR = -100  // Base for CME errors (actual error = AT_CME_ERROR - error_code)
};

// CME Error Codes
enum CMEErrorCode {
    CME_PHONE_FAILURE = 0,
    CME_NO_CONNECTION = 1,
    CME_LINK_RESERVED = 2,
    CME_NOT_ALLOWED = 3,
    CME_NOT_SUPPORTED = 4,
    CME_PH_SIM_PIN_REQUIRED = 5,
    CME_SIM_NOT_INSERTED = 10,
    CME_SIM_PIN_REQUIRED = 11,
    CME_SIM_PUK_REQUIRED = 12,
    CME_SIM_FAILURE = 13,
    CME_SIM_BUSY = 14,
    CME_SIM_WRONG = 15,
    CME_INCORRECT_PASSWORD = 16,
    CME_MEMORY_FULL = 20,
    CME_INVALID_INDEX = 21,
    CME_NOT_FOUND = 22,
    // GNSS specific errors
    CME_INVALID_PARAMS = 501,
    CME_OP_NOT_SUPPORTED = 502,
    CME_GNSS_BUSY = 503,
    CME_SESSION_ONGOING = 504,
    CME_SESSION_NOT_ACTIVE = 505,
    CME_OP_TIMEOUT = 506,
    CME_FUNC_NOT_ENABLED = 507,
    CME_TIME_INFO_ERROR = 508,
    CME_VALIDITY_OUT_RANGE = 512,
    CME_INTERNAL_RES_ERROR = 513,
    CME_GNSS_LOCKED = 514,
    CME_END_BY_E911 = 515,
    CME_NOT_FIXED_NOW = 516,
    CME_CMUX_NOT_OPENED = 517
};

// SSL Error Codes
enum SSLErrorCode {
    SSL_OK = 0,
    SSL_UNKNOWN_ERROR = 550,
    SSL_OP_BLOCKED = 551,
    SSL_INVALID_PARAM = 552,
    SSL_MEMORY_NOT_ENOUGH = 553,
    SSL_CREATE_SOCKET_FAILED = 554,
    SSL_OP_NOT_SUPPORTED = 555,
    SSL_SOCKET_BIND_FAILED = 556,
    SSL_SOCKET_LISTEN_FAILED = 557,
    SSL_SOCKET_WRITE_FAILED = 558,
    SSL_SOCKET_READ_FAILED = 559,
    SSL_SOCKET_ACCEPT_FAILED = 560,
    SSL_OPEN_PDP_FAILED = 561,
    SSL_CLOSE_PDP_FAILED = 562,
    SSL_SOCKET_ID_USED = 563,
    SSL_DNS_BUSY = 564,
    SSL_DNS_PARSE_FAILED = 565,
    SSL_SOCKET_CONN_FAILED = 566,
    SSL_SOCKET_CLOSED = 567,
    SSL_OP_BUSY = 568,
    SSL_OP_TIMEOUT = 569,
    SSL_PDP_BROKEN = 570,
    SSL_CANCEL_SEND = 571,
    SSL_OP_NOT_ALLOWED = 572,
    SSL_APN_NOT_CONFIGURED = 573,
    SSL_PORT_BUSY = 574,
    SSL_HANDSHAKE_FAIL = 579
};

// GNSS Position Mode
enum GNSSFixMode {
    GNSS_FIX_NONE = 0,
    GNSS_FIX_2D = 2,
    GNSS_FIX_3D = 3
};

// GNSS Coordinate Format
enum GNSSCoordFormat {
    GNSS_FORMAT_DEGREES_MINUTES = 0,    // ddmm.mmmmN/S,dddmm.mmmmE/W
    GNSS_FORMAT_DEGREES_MINUTES_EXT = 1, // ddmm.mmmmmm,N/S,dddmm.mmmmmm,E/W
    GNSS_FORMAT_DECIMAL_DEGREES = 2      // (-)dd.ddddd,(-)ddd.ddddd
};

// SSL Access Mode
enum SSLAccessMode {
    SSL_MODE_BUFFER = 0,
    SSL_MODE_DIRECT_PUSH = 1,
    SSL_MODE_TRANSPARENT = 2
};

// Time Query Mode
enum TimeQueryMode {
    TIME_MODE_LAST_SYNC = 0,  // Last synchronized time
    TIME_MODE_GMT = 1,         // Current GMT time
    TIME_MODE_LOCAL = 2        // Current local time
};

// GNSS Position Data Structure
struct GNSSPosition {
    bool valid;
    String utcTime;        // hhmmss.sss
    double latitude;       // Decimal degrees
    double longitude;      // Decimal degrees
    String latitudeStr;    // Original format string
    String longitudeStr;   // Original format string
    float hdop;           // Horizontal dilution of precision
    float altitude;       // Meters above sea level
    uint8_t fixMode;      // 2=2D, 3=3D
    float courseOverGround; // Degrees
    float speedKmh;       // Speed in km/h
    float speedKnots;     // Speed in knots
    String date;          // ddmmyy
    uint8_t numSatellites; // Number of satellites
    int lastError;        // Last error code if failed
};

// Network Time Data Structure
struct NetworkTime {
    bool valid;
    String dateTime;      // YYYY/MM/dd,hh:mm:ss±zz
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    int timezone;         // In quarters of hour from GMT
    int timezoneHours;    // Calculated hours from GMT
    bool daylightSaving;  // DST adjustment
    int lastError;        // Last error code if failed
};

// SSL Connection State
struct SSLConnectionState {
    bool connected;
    int clientID;
    int sslError;
    SSLAccessMode mode;
    String serverAddr;
    int serverPort;
};

// SSL Receive Data
struct SSLReceiveData {
    bool dataAvailable;
    String data;
    int dataLength;
    int totalReceived;
    int alreadyRead;
    int unreadLength;
};

class QuectelEC200U {
private:
    HardwareSerial* modemSerial;
    uint32_t baudRate;
    unsigned long timeout;
    bool transparentMode;
    int currentSSLClient;

    // Internal buffer for AT responses
    String responseBuffer;

    // Helper functions
    bool sendATCommand(const String& command, unsigned long customTimeout = 0);
    String readResponse(unsigned long customTimeout = 0);
    bool waitForResponse(const String& expected, unsigned long customTimeout = 0);
    int parseATResponse(const String& response);
    int parseCMEError(const String& response);
    int parseSSLError(const String& response);

    // Parse helper functions
    bool parseGNSSResponse(const String& response, GNSSPosition& position, GNSSCoordFormat format);
    bool parseNetworkTime(const String& response, NetworkTime& time);
    double convertCoordinateToDecimal(const String& coord, bool isLongitude, GNSSCoordFormat format);

public:
    // Constructor
    QuectelEC200U(HardwareSerial* serial, uint32_t baud = 115200);

    // Basic modem control
    bool begin();
    bool testAT();
    bool reset();
    bool getSignalQuality(int& rssi, int& ber);
    bool getIMEI(String& imei);
    bool getNetworkStatus(int& status);

    // Configuration
    void setTimeout(unsigned long ms) { timeout = ms; }
    unsigned long getTimeout() { return timeout; }

    // ========== GNSS/GPS Functions ==========

    /**
     * Initialize GNSS module
     * @return true if successful, false otherwise
     */
    bool gnssBegin();

    /**
     * Turn on GNSS
     * @param mode GNSS work mode (1=normal, default)
     * @param fixMaxTime Maximum positioning time in seconds (1-255, default 30)
     * @return true if successful, false otherwise
     */
    bool gnssOn(int mode = 1, int fixMaxTime = 30);

    /**
     * Turn off GNSS
     * @return true if successful, false otherwise
     */
    bool gnssOff();

    /**
     * Get current position (latitude and longitude)
     * @param position Reference to GNSSPosition structure to store results
     * @param format Coordinate format (default: decimal degrees)
     * @param maxRetries Maximum number of retries if not fixed (default: 10)
     * @param retryDelay Delay between retries in ms (default: 2000)
     * @return true if position obtained, false otherwise
     */
    bool getPosition(GNSSPosition& position,
                     GNSSCoordFormat format = GNSS_FORMAT_DECIMAL_DEGREES,
                     int maxRetries = 10,
                     unsigned long retryDelay = 2000);

    /**
     * Get only latitude and longitude as doubles
     * @param latitude Reference to store latitude
     * @param longitude Reference to store longitude
     * @return true if successful, false otherwise
     */
    bool getCoordinates(double& latitude, double& longitude);

    /**
     * Check if GNSS has a valid fix
     * @return true if fixed, false otherwise
     */
    bool isGNSSFixed();

    // ========== SSL/HTTPS Functions ==========

    /**
     * Initialize SSL module and configure context
     * @param contextID PDP context ID (1-7)
     * @param sslContextID SSL context ID (0-5)
     * @param sslVersion SSL version (0=SSL3.0, 1=TLS1.0, 2=TLS1.1, 3=TLS1.2, 4=All)
     * @return true if successful, false otherwise
     */
    bool sslBegin(int contextID = 1, int sslContextID = 1, int sslVersion = 4);

    /**
     * Configure SSL parameters
     * @param sslContextID SSL context ID (0-5)
     * @param cipherSuite Cipher suite (optional)
     * @param negotiateTime Negotiation timeout in seconds (10-300, default 300)
     * @return true if successful, false otherwise
     */
    bool sslConfigure(int sslContextID = 1, const String& cipherSuite = "", int negotiateTime = 300);

    /**
     * Connect to HTTPS server using SSL transparent mode
     * @param serverAddress Server IP or domain name
     * @param port Server port
     * @param state Reference to store connection state
     * @param contextID PDP context ID (1-7)
     * @param sslContextID SSL context ID (0-5)
     * @param clientID Socket index (0-11)
     * @return true if connected, false otherwise
     */
    bool httpsConnect(const String& serverAddress,
                      int port,
                      SSLConnectionState& state,
                      int contextID = 1,
                      int sslContextID = 1,
                      int clientID = 0);

    /**
     * Send data over HTTPS connection (transparent mode)
     * @param data Data to send
     * @return true if sent successfully, false otherwise
     */
    bool httpsSend(const String& data);

    /**
     * Send raw bytes over HTTPS connection
     * @param data Byte array to send
     * @param length Length of data
     * @return true if sent successfully, false otherwise
     */
    bool httpsSendBytes(const uint8_t* data, size_t length);

    /**
     * Receive data from HTTPS connection (buffer mode)
     * @param receiveData Reference to store received data
     * @param maxLength Maximum bytes to read (1-1500)
     * @return true if successful, false otherwise
     */
    bool httpsReceive(SSLReceiveData& receiveData, int maxLength = 1500);

    /**
     * Check if data is available in receive buffer
     * @param clientID Socket index
     * @param availableBytes Reference to store available bytes count
     * @return true if data available, false otherwise
     */
    bool httpsDataAvailable(int clientID, int& availableBytes);

    /**
     * Exit transparent mode
     * @return true if successful, false otherwise
     */
    bool exitTransparentMode();

    /**
     * Close SSL connection
     * @param clientID Socket index (0-11)
     * @return true if successful, false otherwise
     */
    bool httpsDisconnect(int clientID = 0);

    /**
     * Send HTTP GET request over SSL
     * @param host Host name
     * @param path Request path
     * @param response Reference to store response
     * @return true if successful, false otherwise
     */
    bool httpsGET(const String& host, const String& path, String& response);

    /**
     * Send HTTP POST request over SSL
     * @param host Host name
     * @param path Request path
     * @param contentType Content type header
     * @param body POST body data
     * @param response Reference to store response
     * @return true if successful, false otherwise
     */
    bool httpsPOST(const String& host, const String& path,
                   const String& contentType, const String& body,
                   String& response);

    // ========== Time Functions ==========

    /**
     * Get network synchronized time
     * @param time Reference to store time data
     * @param mode Query mode (last sync, GMT, or local)
     * @return true if successful, false otherwise
     */
    bool getNetworkTime(NetworkTime& time, TimeQueryMode mode = TIME_MODE_LOCAL);

    /**
     * Get current time as formatted string
     * @param timeStr Reference to store time string
     * @param mode Query mode
     * @return true if successful, false otherwise
     */
    bool getCurrentTime(String& timeStr, TimeQueryMode mode = TIME_MODE_LOCAL);

    /**
     * Set RTC time
     * @param year Year (00-99)
     * @param month Month (1-12)
     * @param day Day (1-31)
     * @param hour Hour (0-23)
     * @param minute Minute (0-59)
     * @param second Second (0-59)
     * @param timezone Timezone in quarters of hour from GMT (-48 to +56)
     * @return true if successful, false otherwise
     */
    bool setRTCTime(int year, int month, int day,
                    int hour, int minute, int second,
                    int timezone = 0);

    /**
     * Get RTC time
     * @param time Reference to store time data
     * @return true if successful, false otherwise
     */
    bool getRTCTime(NetworkTime& time);

    /**
     * Sync time from network
     * @return true if successful, false otherwise
     */
    bool syncTimeFromNetwork();

    // ========== Error Handling ==========

    /**
     * Get last error description
     * @param errorCode Error code to describe
     * @return String description of error
     */
    String getErrorDescription(int errorCode);

    /**
     * Get last SSL error details
     * @return SSL error code
     */
    int getLastSSLError();

    /**
     * Clear modem serial buffer
     */
    void clearBuffer();

    // ========== Raw AT Command Access ==========

    /**
     * Send raw AT command and get response
     * @param command AT command to send
     * @param response Reference to store response
     * @param customTimeout Custom timeout in ms (0 = use default)
     * @return Response code
     */
    int sendRawATCommand(const String& command, String& response, unsigned long customTimeout = 0);
};

#endif // QUECTEL_EC200U_H