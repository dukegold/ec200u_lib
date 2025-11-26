.. _error_codes:

===========
Error Codes
===========

This section provides a comprehensive reference for all error codes used in the QuectelEC200U library.

.. contents:: Error Categories
   :local:
   :depth: 2

AT Response Codes
=================

Basic response codes returned by AT commands:

.. list-table:: AT Response Codes
   :widths: 20 15 65
   :header-rows: 1

   * - Code
     - Value
     - Description
   * - AT_OK
     - 0
     - Command executed successfully
   * - AT_ERROR
     - -1
     - Command failed with generic error
   * - AT_TIMEOUT
     - -2
     - No response received within timeout period
   * - AT_CONNECT
     - -3
     - Connected (entered transparent mode)
   * - AT_NO_CARRIER
     - -4
     - Connection lost or disconnected
   * - AT_SEND_OK
     - -5
     - Data sent successfully
   * - AT_SEND_FAIL
     - -6
     - Data send operation failed
   * - AT_CME_ERROR
     - -100
     - Base value for CME errors (actual = -100 - error_code)

CME Error Codes
===============

CME (Mobile Equipment) errors are standard 3GPP error codes:

General CME Errors
------------------

.. list-table:: General CME Error Codes
   :widths: 15 10 75
   :header-rows: 1

   * - Error
     - Code
     - Description
   * - CME_PHONE_FAILURE
     - 0
     - General phone failure
   * - CME_NO_CONNECTION
     - 1
     - No connection to phone/modem
   * - CME_LINK_RESERVED
     - 2
     - Phone-adapter link reserved
   * - CME_NOT_ALLOWED
     - 3
     - Operation not allowed
   * - CME_NOT_SUPPORTED
     - 4
     - Operation not supported
   * - CME_PH_SIM_PIN_REQUIRED
     - 5
     - PH-SIM PIN required
   * - CME_SIM_NOT_INSERTED
     - 10
     - SIM card not inserted
   * - CME_SIM_PIN_REQUIRED
     - 11
     - SIM PIN required
   * - CME_SIM_PUK_REQUIRED
     - 12
     - SIM PUK required
   * - CME_SIM_FAILURE
     - 13
     - SIM card failure
   * - CME_SIM_BUSY
     - 14
     - SIM card busy
   * - CME_SIM_WRONG
     - 15
     - Wrong SIM card
   * - CME_INCORRECT_PASSWORD
     - 16
     - Incorrect password
   * - CME_MEMORY_FULL
     - 20
     - Memory full
   * - CME_INVALID_INDEX
     - 21
     - Invalid index
   * - CME_NOT_FOUND
     - 22
     - Not found

GNSS-Specific CME Errors
-------------------------

.. list-table:: GNSS CME Error Codes
   :widths: 20 10 70
   :header-rows: 1

   * - Error
     - Code
     - Description & Solution
   * - CME_INVALID_PARAMS
     - 501
     - **Invalid parameters** - Check command syntax and parameter values
   * - CME_OP_NOT_SUPPORTED
     - 502
     - **Operation not supported** - Feature not available on this modem
   * - CME_GNSS_BUSY
     - 503
     - **GNSS subsystem busy** - Wait and retry
   * - CME_SESSION_ONGOING
     - 504
     - **Session is ongoing** - GNSS operation already in progress
   * - CME_SESSION_NOT_ACTIVE
     - 505
     - **Session not active** - Call gnssOn() before getting position
   * - CME_OP_TIMEOUT
     - 506
     - **Operation timeout** - Increase timeout or check antenna
   * - CME_FUNC_NOT_ENABLED
     - 507
     - **Function not enabled** - Enable GNSS feature first
   * - CME_TIME_INFO_ERROR
     - 508
     - **Time information error** - System time invalid
   * - CME_VALIDITY_OUT_RANGE
     - 512
     - **Validity time out of range** - Check time parameters
   * - CME_INTERNAL_RES_ERROR
     - 513
     - **Internal resource error** - Modem internal error, reset may help
   * - CME_GNSS_LOCKED
     - 514
     - **GNSS locked** - GNSS is locked by another process
   * - CME_END_BY_E911
     - 515
     - **Ended by E911** - Emergency call interrupted GNSS
   * - CME_NOT_FIXED_NOW
     - 516
     - **Not fixed now** - No GPS fix yet, wait or move to open sky
   * - CME_CMUX_NOT_OPENED
     - 517
     - **CMUX port not opened** - Multiplexer port issue

SSL Error Codes
===============

SSL/TLS connection and operation errors:

.. list-table:: SSL Error Codes
   :widths: 20 10 70
   :header-rows: 1

   * - Error
     - Code
     - Description & Solution
   * - SSL_OK
     - 0
     - Operation successful
   * - SSL_UNKNOWN_ERROR
     - 550
     - **Unknown error** - Generic SSL error
   * - SSL_OP_BLOCKED
     - 551
     - **Operation blocked** - Resource busy, retry later
   * - SSL_INVALID_PARAM
     - 552
     - **Invalid parameter** - Check function parameters
   * - SSL_MEMORY_NOT_ENOUGH
     - 553
     - **Memory not enough** - Close unused connections
   * - SSL_CREATE_SOCKET_FAILED
     - 554
     - **Create socket failed** - Check network configuration
   * - SSL_OP_NOT_SUPPORTED
     - 555
     - **Operation not supported** - Feature not available
   * - SSL_SOCKET_BIND_FAILED
     - 556
     - **Socket bind failed** - Port may be in use
   * - SSL_SOCKET_LISTEN_FAILED
     - 557
     - **Socket listen failed** - Server mode error
   * - SSL_SOCKET_WRITE_FAILED
     - 558
     - **Socket write failed** - Connection issue during send
   * - SSL_SOCKET_READ_FAILED
     - 559
     - **Socket read failed** - Connection issue during receive
   * - SSL_SOCKET_ACCEPT_FAILED
     - 560
     - **Socket accept failed** - Server mode error
   * - SSL_OPEN_PDP_FAILED
     - 561
     - **Open PDP context failed** - Call sslBegin() first
   * - SSL_CLOSE_PDP_FAILED
     - 562
     - **Close PDP context failed** - PDP context close error
   * - SSL_SOCKET_ID_USED
     - 563
     - **Socket ID already used** - Use different clientID
   * - SSL_DNS_BUSY
     - 564
     - **DNS busy** - DNS resolver busy, retry
   * - SSL_DNS_PARSE_FAILED
     - 565
     - **DNS parse failed** - Check server address/domain
   * - SSL_SOCKET_CONN_FAILED
     - 566
     - **Socket connection failed** - Check server and network
   * - SSL_SOCKET_CLOSED
     - 567
     - **Socket has been closed** - Connection already closed
   * - SSL_OP_BUSY
     - 568
     - **Operation busy** - Another operation in progress
   * - SSL_OP_TIMEOUT
     - 569
     - **Operation timeout** - Network timeout, check connectivity
   * - SSL_PDP_BROKEN
     - 570
     - **PDP context broken** - Data connection lost
   * - SSL_CANCEL_SEND
     - 571
     - **Send cancelled** - Send operation was cancelled
   * - SSL_OP_NOT_ALLOWED
     - 572
     - **Operation not allowed** - Invalid state for operation
   * - SSL_APN_NOT_CONFIGURED
     - 573
     - **APN not configured** - Configure APN settings
   * - SSL_PORT_BUSY
     - 574
     - **Port busy** - Port already in use
   * - SSL_HANDSHAKE_FAIL
     - 579
     - **SSL handshake failed** - Certificate or cipher issue

Error Handling Examples
=======================

Checking Error Types
--------------------

.. code-block:: cpp

    int result = modem.sendRawATCommand("AT+QGPSLOC", response);

    if (result == AT_OK) {
        // Success
    } else if (result == AT_ERROR) {
        // Generic error
    } else if (result == AT_TIMEOUT) {
        // Timeout - increase timeout or check connection
    } else if (result <= AT_CME_ERROR) {
        // CME error - extract actual error code
        int cmeError = AT_CME_ERROR - result;
        Serial.print("CME Error: ");
        Serial.println(cmeError);
    }

Handling GNSS Errors
--------------------

.. code-block:: cpp

    GNSSPosition position;
    if (!modem.getPosition(position)) {
        switch (position.lastError) {
            case CME_SESSION_NOT_ACTIVE:
                // GNSS not turned on
                modem.gnssOn();
                delay(2000);
                // Retry
                break;

            case CME_NOT_FIXED_NOW:
                // No GPS fix yet
                Serial.println("Waiting for GPS fix...");
                delay(5000);
                // Retry with longer timeout
                break;

            case CME_OP_TIMEOUT:
                // Timeout - may need more time
                // Retry with increased timeout
                break;

            default:
                // Other error
                Serial.print("GPS Error: ");
                Serial.println(modem.getErrorDescription(position.lastError));
        }
    }

Handling SSL Errors
-------------------

.. code-block:: cpp

    SSLConnectionState state;
    if (!modem.httpsConnect(server, port, state)) {
        switch (state.sslError) {
            case SSL_OPEN_PDP_FAILED:
                // Need to initialize SSL first
                modem.sslBegin();
                // Retry connection
                break;

            case SSL_DNS_PARSE_FAILED:
                // Invalid server address
                Serial.println("Check server address");
                break;

            case SSL_HANDSHAKE_FAIL:
                // SSL/TLS negotiation failed
                Serial.println("SSL handshake failed");
                // May need different SSL version or cipher
                break;

            case SSL_OP_TIMEOUT:
                // Network timeout
                Serial.println("Connection timeout");
                // Check network connectivity
                break;

            case SSL_SOCKET_ID_USED:
                // Socket already in use
                modem.httpsDisconnect(state.clientID);
                // Retry with different clientID
                break;

            default:
                Serial.print("SSL Error: ");
                Serial.println(modem.getErrorDescription(state.sslError));
        }
    }

Error Recovery Strategies
==========================

Automatic Retry with Backoff
-----------------------------

.. code-block:: cpp

    template<typename Func>
    bool retryWithBackoff(Func operation, int maxRetries = 3) {
        int delay_ms = 1000;  // Start with 1 second

        for (int i = 0; i < maxRetries; i++) {
            if (operation()) {
                return true;  // Success
            }

            Serial.print("Attempt ");
            Serial.print(i + 1);
            Serial.print(" failed, waiting ");
            Serial.print(delay_ms);
            Serial.println("ms...");

            delay(delay_ms);
            delay_ms *= 2;  // Exponential backoff
        }

        return false;  // All retries failed
    }

    // Usage:
    bool success = retryWithBackoff([]() {
        return modem.testAT();
    });

Connection Recovery
-------------------

.. code-block:: cpp

    class ConnectionManager {
    private:
        QuectelEC200U* modem;
        bool networkReady = false;
        bool sslInitialized = false;

    public:
        ConnectionManager(QuectelEC200U* m) : modem(m) {}

        bool ensureNetwork() {
            if (networkReady) {
                // Quick check if still registered
                int status;
                if (modem->getNetworkStatus(status)) {
                    if (status == 1 || status == 5) {
                        return true;
                    }
                }
                networkReady = false;
            }

            // Try to register
            Serial.println("Registering network...");
            for (int i = 0; i < 30; i++) {
                int status;
                if (modem->getNetworkStatus(status)) {
                    if (status == 1 || status == 5) {
                        networkReady = true;
                        return true;
                    }
                }
                delay(2000);
            }

            return false;
        }

        bool ensureSSL() {
            if (!sslInitialized) {
                if (modem->sslBegin()) {
                    sslInitialized = true;
                }
            }
            return sslInitialized;
        }

        bool connect(const String& server, int port, SSLConnectionState& state) {
            // Ensure prerequisites
            if (!ensureNetwork()) {
                Serial.println("No network");
                return false;
            }

            if (!ensureSSL()) {
                Serial.println("SSL init failed");
                return false;
            }

            // Try to connect
            return modem->httpsConnect(server, port, state);
        }
    };

Error Logging
-------------

.. code-block:: cpp

    class ErrorLogger {
    private:
        struct ErrorEntry {
            unsigned long timestamp;
            int errorCode;
            String context;
        };

        ErrorEntry errors[50];
        int errorCount = 0;
        int errorIndex = 0;

    public:
        void logError(int code, const String& context) {
            errors[errorIndex].timestamp = millis();
            errors[errorIndex].errorCode = code;
            errors[errorIndex].context = context;

            errorIndex = (errorIndex + 1) % 50;  // Circular buffer
            if (errorCount < 50) errorCount++;

            // Also print to serial
            Serial.print("[ERROR] ");
            Serial.print(context);
            Serial.print(": ");
            Serial.println(code);
        }

        void printErrorHistory() {
            Serial.println("\n=== Error History ===");
            int start = (errorCount == 50) ? errorIndex : 0;

            for (int i = 0; i < errorCount; i++) {
                int idx = (start + i) % 50;
                Serial.print(errors[idx].timestamp);
                Serial.print("ms: ");
                Serial.print(errors[idx].context);
                Serial.print(" - Error ");
                Serial.println(errors[idx].errorCode);
            }
        }

        int getMostFrequentError() {
            // Analysis function to find most common error
            // Implementation depends on needs
            return 0;
        }
    };

Common Error Scenarios
======================

Scenario 1: GPS Won't Fix
--------------------------

**Symptoms:** Constant CME_NOT_FIXED_NOW errors

**Solutions:**

1. Ensure GPS antenna is connected
2. Move to open sky location
3. Increase timeout and retry count
4. Check if GNSS is actually turned on
5. Allow 30-60 seconds for cold start

Scenario 2: SSL Connection Fails
---------------------------------

**Symptoms:** SSL_HANDSHAKE_FAIL or SSL_SOCKET_CONN_FAILED

**Solutions:**

1. Verify PDP context is active (sslBegin())
2. Check server address and port
3. Ensure network registration
4. Try different SSL/TLS versions
5. Check if server certificate is valid

Scenario 3: Network Time Not Available
---------------------------------------

**Symptoms:** Empty response from AT+QLTS

**Solutions:**

1. Network may not provide time service
2. Wait for network synchronization
3. Use manual RTC setting as fallback
4. Implement NTP client as alternative

Scenario 4: Modem Not Responding
---------------------------------

**Symptoms:** AT_TIMEOUT on all commands

**Solutions:**

1. Check power supply (needs 2A capability)
2. Verify TX/RX connections (try swapping)
3. Check baud rate settings
4. Perform hardware reset
5. Check if modem is in sleep mode