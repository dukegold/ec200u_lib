.. _api_reference:

=============
API Reference
=============

This section provides detailed documentation for all classes, methods, and data structures in the QuectelEC200U library.

.. contents:: Table of Contents
   :local:
   :depth: 3

QuectelEC200U Class
===================

Main class for interfacing with the Quectel EC200U modem.

Constructor
-----------

.. cpp:function:: QuectelEC200U(HardwareSerial* serial, uint32_t baud = 115200)

   Creates a new QuectelEC200U instance.

   :param serial: Pointer to HardwareSerial object for modem communication
   :param baud: Baud rate for serial communication (default: 115200)

   **Example:**

   .. code-block:: cpp

      HardwareSerial modemSerial(1);
      QuectelEC200U modem(&modemSerial, 115200);

Basic Modem Control
===================

.. cpp:function:: bool begin()

   Initializes the modem and establishes communication.

   :returns: ``true`` if initialization successful, ``false`` otherwise

   **Description:**

   * Tests AT command communication
   * Disables command echo (ATE0)
   * Enables verbose error reporting (AT+CMEE=2)
   * Clears serial buffers

   **Example:**

   .. code-block:: cpp

      if (!modem.begin()) {
          Serial.println("Modem initialization failed!");
      }

.. cpp:function:: bool testAT()

   Tests if modem is responding to AT commands.

   :returns: ``true`` if modem responds, ``false`` otherwise

   **Note:** Automatically retries 3 times with 500ms delay between attempts.

.. cpp:function:: bool reset()

   Performs a software reset of the modem.

   :returns: ``true`` if reset successful, ``false`` otherwise

   **Warning:** Reset takes approximately 10 seconds to complete.

.. cpp:function:: bool getSignalQuality(int& rssi, int& ber)

   Retrieves current signal quality metrics.

   :param rssi: Reference to store RSSI value (0-31, 99=unknown)
   :param ber: Reference to store Bit Error Rate (0-7, 99=unknown)
   :returns: ``true`` if successful, ``false`` otherwise

   **RSSI Values:**

   * 0: -113 dBm or less
   * 1: -111 dBm
   * 2-30: -109 to -53 dBm (2 dBm steps)
   * 31: -51 dBm or greater
   * 99: Unknown

.. cpp:function:: bool getIMEI(String& imei)

   Retrieves the modem's IMEI number.

   :param imei: Reference to store IMEI string
   :returns: ``true`` if successful, ``false`` otherwise

.. cpp:function:: bool getNetworkStatus(int& status)

   Gets current network registration status.

   :param status: Reference to store status code
   :returns: ``true`` if successful, ``false`` otherwise

   **Status Codes:**

   * 0: Not registered, not searching
   * 1: Registered, home network
   * 2: Not registered, searching
   * 3: Registration denied
   * 4: Unknown
   * 5: Registered, roaming

GPS/GNSS Functions
==================

.. cpp:function:: bool gnssBegin()

   Initializes GNSS module and configures NMEA output.

   :returns: ``true`` if successful, ``false`` otherwise

.. cpp:function:: bool gnssOn(int mode = 1, int fixMaxTime = 30)

   Turns on GNSS receiver.

   :param mode: GNSS work mode (1=normal, default)
   :param fixMaxTime: Maximum positioning time in seconds (1-255, default 30)
   :returns: ``true`` if successful, ``false`` otherwise

.. cpp:function:: bool gnssOff()

   Turns off GNSS receiver to save power.

   :returns: ``true`` if successful, ``false`` otherwise

.. cpp:function:: bool getPosition(GNSSPosition& position, GNSSCoordFormat format = GNSS_FORMAT_DECIMAL_DEGREES, int maxRetries = 10, unsigned long retryDelay = 2000)

   Acquires current GPS position with automatic retry mechanism.

   :param position: Reference to GNSSPosition structure for results
   :param format: Coordinate format (see GNSSCoordFormat enum)
   :param maxRetries: Maximum retry attempts if not fixed (default 10)
   :param retryDelay: Delay between retries in milliseconds (default 2000)
   :returns: ``true`` if position acquired, ``false`` otherwise

   **Example:**

   .. code-block:: cpp

      GNSSPosition position;
      if (modem.getPosition(position)) {
          Serial.print("Latitude: ");
          Serial.println(position.latitude, 6);
          Serial.print("Longitude: ");
          Serial.println(position.longitude, 6);
          Serial.print("Altitude: ");
          Serial.println(position.altitude);
          Serial.print("Satellites: ");
          Serial.println(position.numSatellites);
      } else {
          Serial.print("Error: ");
          Serial.println(position.lastError);
      }

.. cpp:function:: bool getCoordinates(double& latitude, double& longitude)

   Simple method to get only latitude and longitude.

   :param latitude: Reference to store latitude in decimal degrees
   :param longitude: Reference to store longitude in decimal degrees
   :returns: ``true`` if successful, ``false`` otherwise

.. cpp:function:: bool isGNSSFixed()

   Checks if GNSS has a valid position fix.

   :returns: ``true`` if fixed, ``false`` otherwise

SSL/HTTPS Functions
===================

.. cpp:function:: bool sslBegin(int contextID = 1, int sslContextID = 1, int sslVersion = 4)

   Initializes SSL module and activates PDP context.

   :param contextID: PDP context ID (1-7, default 1)
   :param sslContextID: SSL context ID (0-5, default 1)
   :param sslVersion: SSL version (0=SSL3.0, 1=TLS1.0, 2=TLS1.1, 3=TLS1.2, 4=All)
   :returns: ``true`` if successful, ``false`` otherwise

   **Note:** This function may take up to 30 seconds for network activation.

.. cpp:function:: bool sslConfigure(int sslContextID = 1, const String& cipherSuite = "", int negotiateTime = 300)

   Configures SSL parameters.

   :param sslContextID: SSL context ID (0-5)
   :param cipherSuite: Cipher suite specification (empty = all available)
   :param negotiateTime: SSL negotiation timeout in seconds (10-300)
   :returns: ``true`` if successful, ``false`` otherwise

.. cpp:function:: bool httpsConnect(const String& serverAddress, int port, SSLConnectionState& state, int contextID = 1, int sslContextID = 1, int clientID = 0)

   Establishes HTTPS connection in transparent mode.

   :param serverAddress: Server IP address or domain name
   :param port: Server port number (typically 443 for HTTPS)
   :param state: Reference to SSLConnectionState for connection info
   :param contextID: PDP context ID (1-7)
   :param sslContextID: SSL context ID (0-5)
   :param clientID: Socket index (0-11)
   :returns: ``true`` if connected, ``false`` otherwise

   **Example:**

   .. code-block:: cpp

      SSLConnectionState state;
      if (modem.httpsConnect("api.example.com", 443, state)) {
          Serial.println("Connected securely!");
          // Now in transparent mode - data flows directly
      } else {
          Serial.print("SSL Error: ");
          Serial.println(state.sslError);
      }

.. cpp:function:: bool httpsSend(const String& data)

   Sends string data over HTTPS connection (transparent mode).

   :param data: Data string to send
   :returns: ``true`` if sent, ``false`` if not in transparent mode

.. cpp:function:: bool httpsSendBytes(const uint8_t* data, size_t length)

   Sends binary data over HTTPS connection.

   :param data: Pointer to byte array
   :param length: Number of bytes to send
   :returns: ``true`` if sent, ``false`` otherwise

.. cpp:function:: bool httpsReceive(SSLReceiveData& receiveData, int maxLength = 1500)

   Receives data from HTTPS connection.

   :param receiveData: Reference to SSLReceiveData structure
   :param maxLength: Maximum bytes to read (1-1500)
   :returns: ``true`` if data received, ``false`` otherwise

.. cpp:function:: bool httpsDataAvailable(int clientID, int& availableBytes)

   Checks if data is available in receive buffer.

   :param clientID: Socket index (0-11)
   :param availableBytes: Reference to store available byte count
   :returns: ``true`` if data available, ``false`` otherwise

.. cpp:function:: bool exitTransparentMode()

   Exits transparent mode using +++ escape sequence.

   :returns: ``true`` if successful, ``false`` otherwise

   **Note:** Requires 1 second guard time before and after +++

.. cpp:function:: bool httpsDisconnect(int clientID = 0)

   Closes SSL connection.

   :param clientID: Socket index to close (0-11)
   :returns: ``true`` if successful, ``false`` otherwise

.. cpp:function:: bool httpsGET(const String& host, const String& path, String& response)

   Performs HTTP GET request over SSL.

   :param host: Host name (e.g., "api.example.com")
   :param path: Request path (e.g., "/data")
   :param response: Reference to store response
   :returns: ``true`` if successful, ``false`` otherwise

   **Example:**

   .. code-block:: cpp

      String response;
      if (modem.httpsGET("api.example.com", "/users/123", response)) {
          Serial.println("Response received:");
          Serial.println(response);
      }

.. cpp:function:: bool httpsPOST(const String& host, const String& path, const String& contentType, const String& body, String& response)

   Performs HTTP POST request over SSL.

   :param host: Host name
   :param path: Request path
   :param contentType: Content-Type header value
   :param body: POST request body
   :param response: Reference to store response
   :returns: ``true`` if successful, ``false`` otherwise

Time Functions
==============

.. cpp:function:: bool getNetworkTime(NetworkTime& time, TimeQueryMode mode = TIME_MODE_LOCAL)

   Gets time from network synchronization.

   :param time: Reference to NetworkTime structure
   :param mode: Query mode (see TimeQueryMode enum)
   :returns: ``true`` if successful, ``false`` otherwise

   **Modes:**

   * ``TIME_MODE_LAST_SYNC``: Last synchronized time
   * ``TIME_MODE_GMT``: Current GMT time
   * ``TIME_MODE_LOCAL``: Current local time

.. cpp:function:: bool getCurrentTime(String& timeStr, TimeQueryMode mode = TIME_MODE_LOCAL)

   Gets current time as formatted string.

   :param timeStr: Reference to store time string
   :param mode: Query mode
   :returns: ``true`` if successful, ``false`` otherwise

.. cpp:function:: bool setRTCTime(int year, int month, int day, int hour, int minute, int second, int timezone = 0)

   Sets the modem's RTC time.

   :param year: Year (00-99, where 00=2000)
   :param month: Month (1-12)
   :param day: Day (1-31)
   :param hour: Hour (0-23)
   :param minute: Minute (0-59)
   :param second: Second (0-59)
   :param timezone: Timezone in quarters of hour from GMT (-48 to +56)
   :returns: ``true`` if successful, ``false`` otherwise

   **Timezone Examples:**

   * GMT+0: timezone = 0
   * GMT+8: timezone = 32 (8 * 4)
   * GMT-5: timezone = -20 (-5 * 4)

.. cpp:function:: bool getRTCTime(NetworkTime& time)

   Reads current RTC time.

   :param time: Reference to NetworkTime structure
   :returns: ``true`` if successful, ``false`` otherwise

.. cpp:function:: bool syncTimeFromNetwork()

   Synchronizes RTC with network time.

   :returns: ``true`` if successful, ``false`` otherwise

Utility Functions
=================

.. cpp:function:: void setTimeout(unsigned long ms)

   Sets default timeout for AT commands.

   :param ms: Timeout in milliseconds

.. cpp:function:: unsigned long getTimeout()

   Gets current timeout setting.

   :returns: Timeout in milliseconds

.. cpp:function:: String getErrorDescription(int errorCode)

   Gets human-readable error description.

   :param errorCode: Error code to describe
   :returns: String description of error

.. cpp:function:: int getLastSSLError()

   Retrieves last SSL error code.

   :returns: SSL error code (0 if no error)

.. cpp:function:: void clearBuffer()

   Clears modem serial receive buffer.

.. cpp:function:: int sendRawATCommand(const String& command, String& response, unsigned long customTimeout = 0)

   Sends raw AT command and gets response.

   :param command: AT command to send
   :param response: Reference to store response
   :param customTimeout: Custom timeout (0 = use default)
   :returns: Response code (see ATResponseCode enum)

   **Example:**

   .. code-block:: cpp

      String response;
      int result = modem.sendRawATCommand("AT+CSQ", response);
      if (result == AT_OK) {
          Serial.println("Response: " + response);
      }

Data Structures
===============

GNSSPosition Structure
----------------------

.. cpp:struct:: GNSSPosition

   Contains complete GPS/GNSS position data.

   .. cpp:member:: bool valid

      True if position data is valid

   .. cpp:member:: String utcTime

      UTC time in format "hhmmss.sss"

   .. cpp:member:: double latitude

      Latitude in decimal degrees

   .. cpp:member:: double longitude

      Longitude in decimal degrees

   .. cpp:member:: String latitudeStr

      Original latitude string from modem

   .. cpp:member:: String longitudeStr

      Original longitude string from modem

   .. cpp:member:: float hdop

      Horizontal Dilution of Precision (0.5-99.9)

   .. cpp:member:: float altitude

      Altitude in meters above sea level

   .. cpp:member:: uint8_t fixMode

      Fix mode (2=2D, 3=3D)

   .. cpp:member:: float courseOverGround

      Course over ground in degrees (0-359)

   .. cpp:member:: float speedKmh

      Speed in kilometers per hour

   .. cpp:member:: float speedKnots

      Speed in knots

   .. cpp:member:: String date

      Date in format "ddmmyy"

   .. cpp:member:: uint8_t numSatellites

      Number of satellites in use

   .. cpp:member:: int lastError

      Last error code if position acquisition failed

NetworkTime Structure
---------------------

.. cpp:struct:: NetworkTime

   Contains time and date information.

   .. cpp:member:: bool valid

      True if time data is valid

   .. cpp:member:: String dateTime

      Complete datetime string

   .. cpp:member:: int year

      Year (4 digits)

   .. cpp:member:: int month

      Month (1-12)

   .. cpp:member:: int day

      Day (1-31)

   .. cpp:member:: int hour

      Hour (0-23)

   .. cpp:member:: int minute

      Minute (0-59)

   .. cpp:member:: int second

      Second (0-59)

   .. cpp:member:: int timezone

      Timezone in quarters of hour from GMT

   .. cpp:member:: int timezoneHours

      Calculated timezone in hours

   .. cpp:member:: bool daylightSaving

      True if daylight saving active

   .. cpp:member:: int lastError

      Last error code if time query failed

SSLConnectionState Structure
-----------------------------

.. cpp:struct:: SSLConnectionState

   SSL connection information.

   .. cpp:member:: bool connected

      True if connected

   .. cpp:member:: int clientID

      Socket index (0-11)

   .. cpp:member:: int sslError

      SSL error code (0 = no error)

   .. cpp:member:: SSLAccessMode mode

      Current access mode

   .. cpp:member:: String serverAddr

      Server address

   .. cpp:member:: int serverPort

      Server port number

SSLReceiveData Structure
------------------------

.. cpp:struct:: SSLReceiveData

   Received SSL data information.

   .. cpp:member:: bool dataAvailable

      True if data was received

   .. cpp:member:: String data

      Received data string

   .. cpp:member:: int dataLength

      Length of received data

   .. cpp:member:: int totalReceived

      Total bytes received (buffer mode)

   .. cpp:member:: int alreadyRead

      Bytes already read (buffer mode)

   .. cpp:member:: int unreadLength

      Unread bytes in buffer (buffer mode)

Enumerations
============

ATResponseCode
--------------

.. cpp:enum:: ATResponseCode

   AT command response codes.

   .. cpp:enumerator:: AT_OK = 0

      Command successful

   .. cpp:enumerator:: AT_ERROR = -1

      Command failed

   .. cpp:enumerator:: AT_TIMEOUT = -2

      Response timeout

   .. cpp:enumerator:: AT_CONNECT = -3

      Connected (transparent mode)

   .. cpp:enumerator:: AT_NO_CARRIER = -4

      Connection lost

   .. cpp:enumerator:: AT_SEND_OK = -5

      Data sent successfully

   .. cpp:enumerator:: AT_SEND_FAIL = -6

      Data send failed

   .. cpp:enumerator:: AT_CME_ERROR = -100

      Base for CME errors

GNSSCoordFormat
---------------

.. cpp:enum:: GNSSCoordFormat

   GPS coordinate formats.

   .. cpp:enumerator:: GNSS_FORMAT_DEGREES_MINUTES = 0

      Format: ddmm.mmmmN/S,dddmm.mmmmE/W

   .. cpp:enumerator:: GNSS_FORMAT_DEGREES_MINUTES_EXT = 1

      Format: ddmm.mmmmmm,N/S,dddmm.mmmmmm,E/W

   .. cpp:enumerator:: GNSS_FORMAT_DECIMAL_DEGREES = 2

      Format: (-)dd.ddddd,(-)ddd.ddddd

GNSSFixMode
-----------

.. cpp:enum:: GNSSFixMode

   GNSS positioning modes.

   .. cpp:enumerator:: GNSS_FIX_NONE = 0

      No fix

   .. cpp:enumerator:: GNSS_FIX_2D = 2

      2D positioning

   .. cpp:enumerator:: GNSS_FIX_3D = 3

      3D positioning

SSLAccessMode
-------------

.. cpp:enum:: SSLAccessMode

   SSL data access modes.

   .. cpp:enumerator:: SSL_MODE_BUFFER = 0

      Buffer access mode

   .. cpp:enumerator:: SSL_MODE_DIRECT_PUSH = 1

      Direct push mode

   .. cpp:enumerator:: SSL_MODE_TRANSPARENT = 2

      Transparent access mode

TimeQueryMode
-------------

.. cpp:enum:: TimeQueryMode

   Time query modes.

   .. cpp:enumerator:: TIME_MODE_LAST_SYNC = 0

      Last synchronized time

   .. cpp:enumerator:: TIME_MODE_GMT = 1

      Current GMT time

   .. cpp:enumerator:: TIME_MODE_LOCAL = 2

      Current local time