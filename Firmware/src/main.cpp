// ###########################################################################################################################
// This code is for a Power Meter using ESP8266 (ESP12-F), ADS1115 ADC, LCD color display 0.96", and MQTT communication.
// It measures current using a current transformer (SCT-013), measures voltage using a voltage sensor transformer ZMPT107-1
// and calculates power consumption.
// It also uses MQTT to send data to a broker.
// It has a web interface to configure the device configuration.
//
// Author: CtrlAltJon
// Last Updated: June 2026
// Copyright (c) 2026 CtrlAltJon
// License: MIT
// The software is provided "as is", without warranty of any kind.
// This license notice must be included in any copy or portion of the software.
//
// ###########################################################################################################################

// Include Libraries
#include <Arduino.h>                                                      // Main Arduino Library
#include <Wire.h>                                                         // I2C Library
#include <Adafruit_ADS1X15.h>                                             // ADS1115 Library
#include <ESP8266WiFi.h>                                                  // WiFi Library for ESP8266
#include <WiFiUdp.h>                                                      // WiFi UDP Library
#include <PubSubClient.h>                                                 // MQTT Library
#include <ESP8266WebServer.h>                                             // Web Server Library for ESP8266
#include <Updater.h>                                                      // OTA Update Library
#include <EEPROM.h>                                                       // EEPROM Library
#include "config.h"                                                       // Master Configuration
#include "languages.h"                                                    // Language definitions
#include "webpages.h"                                                     // Web interface functions
#include "sys_globals.h"                                                  // Global structures
#include "display_manager.h"                                              // Display Manager

// ###########################################################################################################################
//   FUNCTIONS #############################################################################################################
// #######################################################################################################################

// --- Function Prototypes (Index of main logic) ---
void mqttCallback(char* topic, uint8_t* payload, unsigned int length);    // MQTT message callback function
bool pcfWrite();                                                          // Write the output byte to the PCF8574 I/O expander
void wifiStartAP();                                                       // Start the ESP in Access Point mode
void deviceFactoryReset();                                                // Perform a factory reset by erasing EEPROM and restarting the device
void wifiSetup();                                                         // Manage WiFi connection initialization
void mqttPublishDiscovery();                                              // Publishes MQTT discovery messages for Home Assistant auto-discovery
bool mqttReconnect();                                                     // Reconnect to MQTT broker if the connection is lost
void deviceHandleButtons();                                               // Handle button presses and update the display accordingly
void makeMqttSafeId(const char* input, char* output, size_t outputSize);  // Build a MQTT-safe identifier from device_name

// Build a stable MQTT-safe identifier from an arbitrary device name.
void makeMqttSafeId(const char* input, char* output, size_t outputSize) {
  if (outputSize == 0) return;
  size_t oi = 0;

  for (size_t i = 0; input[i] != '\0' && oi < outputSize - 1; i++) {
    char c = input[i];
    bool isLower = (c >= 'a' && c <= 'z');
    bool isUpper = (c >= 'A' && c <= 'Z');
    bool isDigit = (c >= '0' && c <= '9');
    bool isSafe = isLower || isUpper || isDigit || c == '_' || c == '-';

    output[oi++] = isSafe ? c : '_';
  }

  if (oi == 0) {
    strncpy(output, "PwrMtr", outputSize - 1);
    output[outputSize - 1] = '\0';
    return;
  }

  output[oi] = '\0';
}

// ------------------------------------------------------------------------------------------------------------- MQTT Callback
// This function is called when a message is received on a subscribed MQTT topic.
// It parses the incoming message to update the device's configuration (power threshold or buzzer mode)
// based on commands from Home Assistant or other MQTT clients.
void mqttCallback(char* topic, uint8_t* payload, unsigned int length) {
  Serial.print("MQTT Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  char message_buff[MQTT_BUFFER_SIZE];
  size_t copyLen = (length < (sizeof(message_buff) - 1)) ? length : (sizeof(message_buff) - 1);
  memcpy(message_buff, payload, copyLen);
  message_buff[copyLen] = '\0';
  if (length >= sizeof(message_buff)) {
    Serial.printf("MQTT payload truncated: %u -> %u bytes\n", length, (unsigned int)copyLen);
  }
  Serial.println(message_buff);

  char threshold_cmd_topic[128];
  char buzzer_cmd_topic[128];
  char mqttDeviceId[33];
  makeMqttSafeId(sysConfig.device_name, mqttDeviceId, sizeof(mqttDeviceId));
  snprintf(threshold_cmd_topic, sizeof(threshold_cmd_topic), MQTT_THRESHOLD_COMMAND_TOPIC, mqttDeviceId);
  snprintf(buzzer_cmd_topic, sizeof(buzzer_cmd_topic), MQTT_BUZZER_MODE_COMMAND_TOPIC, mqttDeviceId);

  bool config_changed = false;

  if (strcmp(topic, threshold_cmd_topic) == 0) {
    int newThreshold = atoi(message_buff);
    if (newThreshold >= 0 && newThreshold <= MAX_POWER_THRESHOLD) { // Basic validation
      if (sysConfig.powerThreshold != newThreshold) {
        sysConfig.powerThreshold = newThreshold;
        config_changed = true;
        Serial.printf("Updated powerThreshold to %d\n", newThreshold);
      }
    } else {
      Serial.printf("Invalid powerThreshold value received: %s\n", message_buff);
    }
  } else if (strcmp(topic, buzzer_cmd_topic) == 0) {
    BuzzerMode newMode = BUZZER_MODE_COUNT; // Using COUNT as invalid/null value
    
    if (strcmp_P(message_buff, (PGM_P)getText(ID_BUZZER_MODE_CONTINUOUS)) == 0) newMode = BUZZER_MODE_CONTINUOUS;
    else if (strcmp_P(message_buff, (PGM_P)getText(ID_BUZZER_MODE_PULSE_SLOW)) == 0) newMode = BUZZER_MODE_PULSE_SLOW;
    else if (strcmp_P(message_buff, (PGM_P)getText(ID_BUZZER_MODE_PULSE_FAST)) == 0) newMode = BUZZER_MODE_PULSE_FAST;

    if (newMode != BUZZER_MODE_COUNT && sysConfig.buzzerMode != newMode) {
      sysConfig.buzzerMode = newMode;
      config_changed = true;
      Serial.printf("Updated buzzerMode to %d\n", (int)newMode);
    } else if (newMode == BUZZER_MODE_COUNT) {
      Serial.printf("Invalid buzzerMode value received: %s\n", message_buff);
    }
  }

  if (config_changed) {
    EEPROM.put(1, sysConfig);
    if (EEPROM.commit()) { savedSysConfig = sysConfig; Serial.println("Configuration saved to EEPROM from MQTT."); }
    else { Serial.println("Failed to save configuration to EEPROM from MQTT."); }
    lastDisplayUpdate = 0; // Force display update if needed
  }
}

// ---------------------------------------------------------------------------------------------------------- Write to PCF8574
// Send the status By te to the PCF8574 I/O expander. The byte is inverted before sending to match the active LOW logic of the hardware.
bool pcfWrite() {                                                         // Returns true if transmission is successful
  Wire.beginTransmission(PCF8574_ADDR);
  Wire.write(~pcfOutputByte);                                             // Invert the byte for the PCF8574 logic (active LOW)
  byte error = Wire.endTransmission();                                    // Check for I2C transmission errors
  if (error != 0) {
    Serial.printf("PCF8574: I2C error %d at address 0x%02X\n", error, PCF8574_ADDR);
    return false; // Indicate failure
  }
  return true; // Indicate success
}
// ------------------------------------------------------------------------------------------------------------- Start AP Mode
// ---------------------------------------------------------------------------------------------------------------------------
// Starts the WiFi Access Point mode with an SSID based on the device's MAC address. This is used when no WiFi credentials are
// configured or when user press button to manually start AP mode, allowing the user to connect to the AP and configure the
// device through the web interface.
void wifiStartAP() {
  Serial.println(F("Starting Access Point..."));
  WiFi.mode(WIFI_AP);                                                     // Set WiFi to Access Point mode
  uint8_t mac[6];                                                         // Variable to hold the MAC address bytes
  WiFi.macAddress(mac);                                                   // Get the MAC address of the device
  // Create a unique SSID for the Access Point using the last three bytes of the MAC address (e.g., "PwrMtr-1A2B3C")
  snprintf(sysState.apSSID, sizeof(sysState.apSSID), "PwrMtr-%02X%02X%02X", mac[3], mac[4], mac[5]);
  WiFi.softAP(sysState.apSSID);                                           // Start the Access Point with the generated SSID
  Serial.print(F("AP Started. SSID: ")); Serial.println(sysState.apSSID);
  Serial.print(F("IP Address: ")); Serial.println(WiFi.softAPIP());
}

// ------------------------------------------------------------------------------------------------------------- Factory Reset
// ---------------------------------------------------------------------------------------------------------------------------
// Performs a factory reset by erasing the EEPROM and restarting the device. This is triggered when the user selects the Reset
// option. It ensures that all user configurations are cleared and the device is returned to its initial state.
void deviceFactoryReset() {
  initBootScreen();                                                       // Initialize the boot screen
  addBootMessage("Reset ", BOOT_WARN);                                    // Display a warning message about the reset process
  Serial.println("Factory Reset: Erasing EEPROM...");
  for (int i = 0; i < EEPROM_SIZE; i++) {                                 // Loop through the entire EEPROM
    EEPROM.write(i, 0);                                                   // and write 0 to each byte to clear it
  }
  EEPROM.commit();                                                        // Commit the changes to ensure they are saved
  addBootMessage("Erasing... ", BOOT_OK);                                 // Display a message indicating that the EEPROM is being erased
  delay(1000);
  ESP.restart();                                                          // Restart the device to complete the factory reset process
}

// ----------------------------------------------------------------------------------------------------------- Connect to WiFi
// ---------------------------------------------------------------------------------------------------------------------------
// Connects to the WiFi network using the credentials stored in the configuration. If static IP configuration is enabled, it applies
// the static IP settings. If no SSID is configured, it starts the Access Point mode immediately. It also includes a timeout mechanism
// to avoid getting stuck while trying to connect to WiFi, and it allows background tasks to run while waiting for the connection
// to succeed or fail.
void wifiSetup() {
  delay(10);                                                              // Short delay for stability
  IPAddress ip, gateway, subnet, dns;                                     // Variables for IP configuration if static IP is used
    if (strlen(sysConfig.ssid) == 0) {                                    // If no SSID is configured
    wifiStartAP();                                                        // start AP mode immediately
    return;
  }

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(sysConfig.ssid);
  WiFi.mode(WIFI_STA);                                                    // Set WiFi to station mode
  WiFi.hostname(sysConfig.device_name);                                   // Set hostname for router

  if (sysConfig.staticIP) {                                               // If static IP configuration is enabled, apply the static IP settings
    Serial.println("Using Static IP configuration.");
    ip.fromString(sysConfig.static_ip);
    gateway.fromString(sysConfig.static_gateway);
    subnet.fromString(sysConfig.static_subnet);
    dns.fromString(sysConfig.static_dns);
    WiFi.config(ip, gateway, subnet, dns);
  }
  
  WiFi.begin(sysConfig.ssid, sysConfig.password);                         // Start WiFi connection

  unsigned long connect_start = millis();                                 // Start time for connection attempt to implement timeout
  unsigned long last_dot_print = 0;                                       // Timer for printing dots in serial to indicate connection progress
  while (WiFi.status() != WL_CONNECTED) {                                 // Wait for connection
    if (millis() - connect_start > WIFI_CONNECT_TIMEOUT) {                // Use config.h timeout
      Serial.println("\nWiFi connection timed out.");                     // No fallback to AP Mode automatically to avoid security risks or unwanted APs
      break;
    }
    if (millis() - last_dot_print > 500) {                                // Print a dot every 500 ms to indicate connection progress to the serial monitor
      Serial.print(".");
      last_dot_print = millis();
    }
    yield();                                                              // Allow background tasks to run while waiting for WiFi connection
  }

  if (WiFi.status() == WL_CONNECTED) {                                    // If connected successfully, print the info to the serial monitor
    Serial.println("");
    Serial.println("WiFi connected");
    IPAddress ip = WiFi.localIP();
    Serial.printf("IP address: %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
  }
}

// ---------------------------------------------------------------------------------------------------- Publish MQTT discovery
// ---------------------------------------------------------------------------------------------------------------------------
// Publishes MQTT discovery messages for Home Assistant auto-discovery. This function sends configuration messages for each sensor
// (power, current, voltage, and buzzer mode) to the MQTT broker, allowing Home Assistant to automatically discover and integrate
// the device and its sensors without manual configuration.
void mqttPublishDiscovery() {
  static char payload[512];                                               // MQTT payload buffer
  char topic[128];                                                        // MQTT topic buffer
  const char* prefix = (strlen(sysConfig.ha_prefix) > 0) ? sysConfig.ha_prefix : DEFAULT_HA_PREFIX;
  char mqttDeviceId[33];
  makeMqttSafeId(sysConfig.device_name, mqttDeviceId, sizeof(mqttDeviceId));
  Serial.println(F("Sending MQTT discovery for Home Assistant..."));

  char nBuf[64]; // Buffer for localized entity names

  // --- Power Sensor ---
  // Construct the MQTT discovery topic using the user-defined prefix
  snprintf(topic, sizeof(topic), "%s/sensor/%s/power/config", prefix, mqttDeviceId);
  strncpy_P(nBuf, (PGM_P)MSG_WEB_POWER, sizeof(nBuf)); nBuf[sizeof(nBuf)-1] = '\0';
  // Construct the MQTT payload with the sensor configuration in JSON format, including device information for Home Assistant
  snprintf(payload, sizeof(payload),
    "{\"name\": \"%s\", \"unique_id\": \"%s_power\", \"state_topic\": \"%s/state\", \"unit_of_measurement\": \"W\", \"device_class\": \"power\", \"icon\": \"mdi:flash\", \"value_template\": \"{{ value_json.power }}\", \"device\": {\"identifiers\": [\"%s_id\"], \"name\": \"%s\", \"model\": \"ESP8266 Meter\", \"manufacturer\": \"%s\"}}",
    nBuf, mqttDeviceId, mqttDeviceId, mqttDeviceId, sysConfig.device_name, sysConfig.manufacturer);
  // Publish the MQTT discovery message for the power sensor with the retain flag set to true, ensuring that Home Assistant can
  // discover the sensor even if it connects after the message is sent
  mqttClient.publish(topic, payload, true);
 
  // --- Current Sensor ---
  // Construct the MQTT topic and payload for the current sensor in the same way as the power sensor
  snprintf(topic, sizeof(topic), "%s/sensor/%s/current/config", prefix, mqttDeviceId);
  strncpy_P(nBuf, (PGM_P)MSG_WEB_CURRENT, sizeof(nBuf)); nBuf[sizeof(nBuf)-1] = '\0';
  snprintf(payload, sizeof(payload),
    "{\"name\": \"%s\", \"unique_id\": \"%s_current\", \"state_topic\": \"%s/state\", \"unit_of_measurement\": \"A\", \"device_class\": \"current\", \"icon\": \"mdi:current-ac\", \"value_template\": \"{{ value_json.current }}\", \"device\": {\"identifiers\": [\"%s_id\"], \"name\": \"%s\", \"model\": \"ESP8266 Meter\", \"manufacturer\": \"%s\"}}",
    nBuf, mqttDeviceId, mqttDeviceId, mqttDeviceId, sysConfig.device_name, sysConfig.manufacturer);
  // Publish the MQTT discovery message for the current sensor
  mqttClient.publish(topic, payload, true);

  // --- Voltage Sensor ---
  // Construct the MQTT topic and payload for the voltage sensor in the same way as the previous
  snprintf(topic, sizeof(topic), "%s/sensor/%s/voltage/config", prefix, mqttDeviceId);
  strncpy_P(nBuf, (PGM_P)MSG_WEB_VOLTAGE, sizeof(nBuf)); nBuf[sizeof(nBuf)-1] = '\0';
  snprintf(payload, sizeof(payload),
    "{\"name\": \"%s\", \"unique_id\": \"%s_voltage\", \"state_topic\": \"%s/state\", \"unit_of_measurement\": \"V\", \"device_class\": \"voltage\", \"icon\": \"mdi:sine-wave\", \"value_template\": \"{{ value_json.voltage }}\", \"device\": {\"identifiers\": [\"%s_id\"], \"name\": \"%s\", \"model\": \"ESP8266 Meter\", \"manufacturer\": \"%s\"}}",
    nBuf, mqttDeviceId, mqttDeviceId, mqttDeviceId, sysConfig.device_name, sysConfig.manufacturer);
  // Publish the MQTT discovery message for the voltage sensor
  mqttClient.publish(topic, payload, true);

  // --- Temperature Sensor ---
  snprintf(topic, sizeof(topic), "%s/sensor/%s/temperature/config", prefix, mqttDeviceId);
  strncpy_P(nBuf, (PGM_P)MSG_WEB_TEMPERATURE, sizeof(nBuf)); nBuf[sizeof(nBuf)-1] = '\0';
  snprintf(payload, sizeof(payload),
    "{\"name\": \"%s\", \"unique_id\": \"%s_temp\", \"state_topic\": \"%s/state\", \"unit_of_measurement\": \"\\u00b0C\", \"device_class\": \"temperature\", \"icon\": \"mdi:thermometer\", \"value_template\": \"{{ value_json.temperature }}\", \"device\": {\"identifiers\": [\"%s_id\"], \"name\": \"%s\", \"model\": \"ESP8266 Meter\", \"manufacturer\": \"%s\"}}",
    nBuf, mqttDeviceId, mqttDeviceId, mqttDeviceId, sysConfig.device_name, sysConfig.manufacturer);
  // Publish the MQTT discovery message for the temperature sensor
  mqttClient.publish(topic, payload, true);

  // --- Alarm Threshold Sensor ---
  snprintf(topic, sizeof(topic), "%s/sensor/%s/threshold/config", prefix, mqttDeviceId);
  strncpy_P(nBuf, (PGM_P)MSG_DISPLAY_THRESHOLD, sizeof(nBuf)); nBuf[sizeof(nBuf)-1] = '\0';
  snprintf(payload, sizeof(payload),
    "{\"name\": \"%s\", \"unique_id\": \"%s_threshold\", \"state_topic\": \"%s/state\", \"unit_of_measurement\": \"W\", \"device_class\": \"power\", \"icon\": \"mdi:bell-ring\", \"value_template\": \"{{ value_json.threshold }}\", \"device\": {\"identifiers\": [\"%s_id\"], \"name\": \"%s\", \"model\": \"ESP8266 Meter\", \"manufacturer\": \"%s\"}}",
    nBuf, mqttDeviceId, mqttDeviceId, mqttDeviceId, sysConfig.device_name, sysConfig.manufacturer);
  // Publish the MQTT discovery message for the threshold sensor
  mqttClient.publish(topic, payload, true);

  // --- Buzzer Mode Sensor ---
  // Construct the MQTT topic and payload for the buzzer mode sensor, which is a custom sensor that reports the current buzzer mode as a string
  snprintf(topic, sizeof(topic), "%s/sensor/%s/buzzer_mode/config", prefix, mqttDeviceId);
  strncpy_P(nBuf, (PGM_P)MSG_CFG_BUZZER_MODE, sizeof(nBuf)); nBuf[sizeof(nBuf)-1] = '\0';
  snprintf(payload, sizeof(payload),
    "{\"name\": \"%s\", \"unique_id\": \"%s_buzzer_mode\", \"state_topic\": \"%s/state\", \"icon\": \"mdi:volume-high\", \"value_template\": \"{{ value_json.buzzer_mode }}\", \"device\": {\"identifiers\": [\"%s_id\"], \"name\": \"%s\", \"model\": \"ESP8266 Meter\", \"manufacturer\": \"%s\"}}",
    nBuf, mqttDeviceId, mqttDeviceId, mqttDeviceId, sysConfig.device_name, sysConfig.manufacturer);
  // Publish the MQTT discovery message for the buzzer mode sensor
  mqttClient.publish(topic, payload, true);

  // --- Alarm Threshold Number Entity (for setting threshold from HA) ---
  snprintf(topic, sizeof(topic), "%s/number/%s/threshold_set/config", prefix, mqttDeviceId);
  strncpy_P(nBuf, (PGM_P)MSG_DISPLAY_THRESHOLD, sizeof(nBuf));
  strncat(nBuf, " (Set)", sizeof(nBuf) - strlen(nBuf) - 1);
  snprintf(payload, sizeof(payload),
    "{\"name\": \"%s\", \"unique_id\": \"%s_threshold_set\", \"state_topic\": \"%s/state\", \"value_template\": \"{{ value_json.threshold }}\", \"command_topic\": \"%s/threshold/set\", \"min\": 0, \"max\": %d, \"step\": %d, \"unit_of_measurement\": \"W\", \"device_class\": \"power\", \"icon\": \"mdi:bell-cog\", \"mode\": \"box\", \"device\": {\"identifiers\": [\"%s_id\"], \"name\": \"%s\", \"model\": \"ESP8266 Meter\", \"manufacturer\": \"%s\"}}",
    nBuf, mqttDeviceId, mqttDeviceId, mqttDeviceId, MAX_POWER_THRESHOLD, THRESHOLD_STEP, mqttDeviceId, sysConfig.device_name, sysConfig.manufacturer);
  mqttClient.publish(topic, payload, true);

  // --- Buzzer Mode Select Entity (for setting buzzer mode from HA) ---
  // Need to get string representations of buzzer modes for the options and value_template
  char b0[32], b1[32], b2[32];
  strncpy_P(b0, (PGM_P)getText(ID_BUZZER_MODE_CONTINUOUS), sizeof(b0));
  strncpy_P(b1, (PGM_P)getText(ID_BUZZER_MODE_PULSE_SLOW), sizeof(b1));
  strncpy_P(b2, (PGM_P)getText(ID_BUZZER_MODE_PULSE_FAST), sizeof(b2));

  char options_json[128]; // Buffer for the JSON array of options
  snprintf(options_json, sizeof(options_json), "[\"%s\", \"%s\", \"%s\"]", b0, b1, b2);

  snprintf(topic, sizeof(topic), "%s/select/%s/buzzer_mode_set/config", prefix, mqttDeviceId);
  strncpy_P(nBuf, (PGM_P)MSG_CFG_BUZZER_MODE, sizeof(nBuf));
  strncat(nBuf, " (Set)", sizeof(nBuf) - strlen(nBuf) - 1);
  
  // We must escape '%' signs for the Jinja template logic (e.g. %%{ instead of {%) 
  // so snprintf doesn't mistake them for format specifiers.
  snprintf(payload, sizeof(payload),
    "{\"name\": \"%s\", \"unique_id\": \"%s_buzzer_mode_set\", \"state_topic\": \"%s/state\", \"value_template\": \"{{ value_json.buzzer_mode }}\", \"command_topic\": \"%s/buzzer_mode/set\", \"options\": %s, \"icon\": \"mdi:tune\", \"device\": {\"identifiers\": [\"%s_id\"], \"name\": \"%s\", \"model\": \"ESP8266 Meter\", \"manufacturer\": \"%s\"}}",
    nBuf, mqttDeviceId, mqttDeviceId, mqttDeviceId, options_json, mqttDeviceId, sysConfig.device_name, sysConfig.manufacturer);
  mqttClient.publish(topic, payload, true);
}

// --------------------------------------------------------------------------------------------------------- MQTT reconnection
// ---------------------------------------------------------------------------------------------------------------------------
// Attempts to reconnect to the MQTT broker if the connection is lost.
// It uses a backoff strategy to avoid flooding the broker with reconnection attempts.
bool mqttReconnect() {
  bool connected = false;                                                 // Flag to track if the connection was successful
  char mqttDeviceId[33];
  makeMqttSafeId(sysConfig.device_name, mqttDeviceId, sizeof(mqttDeviceId));
  if (strlen(sysConfig.mqtt_user) > 0) {                                  // If MQTT username is configured
    connected = mqttClient.connect(mqttDeviceId, sysConfig.mqtt_user, sysConfig.mqtt_pass);    // Attempt to connect with credentials
  } else {                                                                // If no MQTT username is configured
    connected = mqttClient.connect(mqttDeviceId);                         // Attempt to connect without credentials
  }

  if (connected) {                                                        // If MQTT connection is successful
    Serial.println("MQTT connected");
    mqttPublishDiscovery();                                               // Publish MQTT discovery messages for Home Assistant auto-discovery
    char threshold_cmd_topic[128];
    char buzzer_cmd_topic[128];
    snprintf(threshold_cmd_topic, sizeof(threshold_cmd_topic), MQTT_THRESHOLD_COMMAND_TOPIC, mqttDeviceId);
    snprintf(buzzer_cmd_topic, sizeof(buzzer_cmd_topic), MQTT_BUZZER_MODE_COMMAND_TOPIC, mqttDeviceId);
    mqttClient.subscribe(threshold_cmd_topic);
    mqttClient.subscribe(buzzer_cmd_topic);
    Serial.printf("Subscribed to %s and %s\n", threshold_cmd_topic, buzzer_cmd_topic);
  }
  return mqttClient.connected();                                          // Return the connection status
}

// ------------------------------------------------------------------------------ Button Handling with Debounce and Hold Logic
// ---------------------------------------------------------------------------------------------------------------------------
// Handles the button input with debounce logic to prevent false triggers and implements hold logic for long presses.
// It manages page navigation and actions based on the current page and the duration of the button press.
// Short presses are used for navigating between pages, while long presses trigger specific actions depending on the current page
// (e.g., incrementing values, starting AP mode, rebooting, or factory reset).
void deviceHandleButtons() {
  unsigned long now = millis();                                           // Get the current time for debounce and hold logic
  int currentBtnState = digitalRead(PIN_BTN);                             // Read the current state of the button
  if (currentBtnState != lastBtnState) {                                  // If the button state has changed
    lastDebounceTime = now;                                               // Reset the debounce timer
  }

  // --- Short Press Logic (Debounced) ---
  if ((now - lastDebounceTime) > DEBOUNCE_DELAY) {                        // If button pressed 
    if (currentBtnState != btnState) {                                    // If button state is different from the last stable state
      btnState = currentBtnState;                                         // Update the button state

      if (btnState == LOW) {                                              // If button is pressed (active LOW)
        sysState.btnPressedTime = now;                                    // Record the time when the button was pressed for hold logic
        sysState.actionTriggered = false;                                 // Reset the action triggered flag for hold logic
        Serial.println(F("Button Pressed"));
      } else {                                                            // If button is released
        unsigned long pressDuration = now - sysState.btnPressedTime;      // Calculate the duration of the button press
        Serial.printf("Button Released. Duration: %lu ms\n", pressDuration);

        if (!sysState.actionTriggered && pressDuration < HOLD_THRESHOLD) {     // If it's a short press.
          Serial.println(F("Short Press Action"));
          switch (sysState.displayPage) {                                 // Handle short press actions based on current page on LCD
            case 6:                                                       // Page 6: Cycle selected action
              sysState.selectedAction++;                                  // Increment the selected action index
              if (sysState.selectedAction > 2) {                          // If it exceeds the number of actions
                sysState.selectedAction = 0;                              // Reset to the first action
                sysState.displayPage = 1;                                 // Return to page 1 after cycling all actions
              }
              break;
            default:                                                      // Other pages: Increment display page
              sysState.displayPage++;                                     // Go to the next page
              if (sysState.displayPage == 6) sysState.selectedAction = 0; // If reached system page, reset selected action
              if (sysState.displayPage > 6) sysState.displayPage = 1;     // Wrap around at 6
              break;
          }
        }
        // If sysState.actionTriggered is true, a long press action was already handled
        // while the button was LOW, so nothing more to do on release.
      }
    }
  } // End of Short Press check

  lastBtnState = currentBtnState;                                         // Update the last button state for the next iteration

  // --- Hold Logic for Long Presses ---
  if (btnState == LOW) {                                                  // If button is currently pressed, check for hold actions.
    unsigned long holdTime = now - sysState.btnPressedTime;               // Calculate how long the button has been held down.
    
    if (sysState.displayPage == 6) {                                      // PAGE 6 - Special 5-second hold for system actions
      if (holdTime >= SYS_ACTION_HOLD && !sysState.actionTriggered) {     // Use config.h constant
        sysState.actionTriggered = true;                                  // Prevent re-triggering
        Serial.printf("Page 6 Special 5s Hold (Action %d) triggered\n", sysState.selectedAction);
        switch (sysState.selectedAction) {                                // Execute the selected action based on the index
          case 0:                                                         // Selected Action 0: first Button on Page 7: Start AP Mode
            wifiStartAP();                                                // Start the Access Point mode
            sysState.showAPModeMessage = true;                            // Show AP mode message on the display
            sysState.apModeMessageStart = millis();                       // Record the time when AP mode message is shown to manage its display duration
            break;
          case 1:                                                         // Selected Action 1: second Button on Page 7: Reboot
            initBootScreen();                                             // Initialize the boot screen before rebooting to show the reboot message
            addBootMessage("Reboot", BOOT_OK);                            // Display a reboot message on the boot screen
            ESP.restart();                                                // Restart the device to complete the reboot process
            break;
          case 2:                                                         // Selected Action 2: third Button on Page 7: Factory Reset
            deviceFactoryReset();                                         // Perform a factory reset by erasing the EEPROM and restarting the device
            break;
        }
      }
      return;                                                             // No other long press actions on page 7
    }

    if (holdTime > HOLD_THRESHOLD) {                                      // OTHER PAGES, handle long press actions after HOLD_THRESHOLD

      // PAGE 3 and 4 - one-shot long press actions
      if (!sysState.actionTriggered) {
        sysState.actionTriggered = true;                                  // Mark action as triggered to prevent multiple triggers
        Serial.printf("Long Press detected on Page %d\n", sysState.displayPage);
        switch (sysState.displayPage) {                                   // Handle long press actions based on the current page
          case 3:                                                         // Page 3: Buzzer Mode
          {
            int nextMode = (int)sysConfig.buzzerMode + 1;                 // Cast to int to allow increment
            if (nextMode >= BUZZER_MODE_COUNT) nextMode = 0;              // Wrap around
            sysConfig.buzzerMode = (BuzzerMode)nextMode;                  // Cast back to enum
            lastDisplayUpdate = 0;                                        // Force display update to show the new buzzer mode immediately
          }
            break;
          case 4:                                                         // Page 4: Save Settings
            EEPROM.put(1, sysConfig);                                     // Write the current configuration struct to EEPROM at address 1
            sysState.lastSaveResult = EEPROM.commit();                    // Commit the changes to EEPROM and store the result (true if successful)
            sysState.showSaveMessage = true;                              // Set flag to show save message on display
            if (sysState.lastSaveResult) {                                // If save was successful
              savedSysConfig = sysConfig;                                 // Update the saved configuration copy to the current configuration
            }
            sysState.saveMessageStart = now;                              // Record the time when the save message is shown to manage its display duration
            Serial.print(F("Settings saved to EEPROM with result: ")); Serial.println(sysState.lastSaveResult ? F("SUCCESS") : F("FAILURE"));
            break;
          default:
            // No one-shot long press action for this page, so reset actionTriggered
            sysState.actionTriggered = false;
            break;
        }
      }

      // PAGE 2 - continuous increment actions while holding the button
      if (sysState.displayPage == 2 && (now - lastThresholdChange > REPEAT_DELAY)) {
        sysState.actionTriggered = true;                                  // Ensure actionTriggered is true for continuous actions
        switch (sysState.displayPage) {                                   // Handle continuous increment actions
          case 2:                                                         // Page 2: Power Threshold
            sysConfig.powerThreshold += THRESHOLD_STEP;                   // Use config.h step
            if (sysConfig.powerThreshold > MAX_POWER_THRESHOLD) sysConfig.powerThreshold = 0; // Use config.h limit
            break;
        }
        lastThresholdChange = now;                                        // Update the last threshold change time
        lastDisplayUpdate = 0;                                            // Force LCD update
      }
    }
  }
}

// ###########################################################################################################################
//   SETUP  ################################################################################################################
// #######################################################################################################################
void setup() {
  Wire.begin();                                                                // I2C Initialization
  Serial.begin(115200);                                                        // Serial Initialization at 115200 baud rate
  while (!Serial);                                                             // Wait for Serial to be ready

  // --- I2C Bus Scanner (Serial Debug only) ---
  Serial.println(F("I2C Scanner starting..."));                                // Perform I2C bus scan at boot time only
  byte nDevices = 0;
  for (byte address = 1; address < 127; address++) {                           // Scan I2C bus for devices at boot time only
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();
    if (error == 0) {
      Serial.printf("I2C device found at address 0x%02X\n", address);          // Print found device address
      nDevices++;
    } else if (error == 4) {
      Serial.printf("Unknown error at address 0x%02X\n", address);             // Print unknown error at address
    }
  }
  if (nDevices == 0) Serial.println(F("No I2C devices found!\n"));             // If no devices found, print message
  else Serial.printf("I2C scan finished. %d devices found.\n\n", nDevices);    // If devices found, print number of devices found
  // -------------------------------------------

  Serial.println(F("\n\n##########################################"));
  Serial.println(F("## Peter The Power Meter Initialization ##"));
  Serial.println(F("##########################################"));
  
  uint8_t mac[6];
  WiFi.macAddress(mac);
  Serial.printf("MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  initDisplay();                                                          // Initialize the LCD display
  initBootScreen();                                                       // Launch the boot screen on the LCD to show boot messages during startup
  addBootMessage("Booting", BOOT_OK);                                     // Display a booting message on the boot screen (Boot start)
  sysState.displayPage = 1;                                               // Load the first page on the LCD at startup
  sysState.currentRMS = 0.0f;                                             // Initialize current state
  sysState.powerW = 0.0f;                                                 // Initialize power state
  measuredVoltageRMS = 0.0f;                                              // Initial state: 0V until first reading
  sysState.temperature = NAN;                                             // Initialize temperature state variable as NAN (invalid until first reading)
  sysState.ptcSum = 0;
  sysState.ptcCount = 0;
  addBootMessage("Serial", BOOT_OK);                                      // Serial initialized successfully, show message on boot screen
  
  EEPROM.begin(EEPROM_SIZE);                                              // Initialize EEPROM
  if (EEPROM.read(0) == EEPROM_MAGIC) {                                   // Check Magic Byte at address 0 to verify if EEPROM is initialized with valid data
    Serial.println("Loading configuration from EEPROM...");
    EEPROM.get(1, sysConfig);                                             // Load the configuration struct from EEPROM starting at address 1
    EEPROM.get(1, savedSysConfig);                                        // Load the saved configuration struct for later comparison to detect changes
    configSanitizeCalibration(sysConfig);                                 // Clamp calibration fields from EEPROM
    configSanitizeCalibration(savedSysConfig);
    addBootMessage("EEPROM", BOOT_OK);                                    // Show message on boot screen that EEPROM was read successfully
    
    setLanguage((Language)sysConfig.language);                            // Set the language for the device based on the loaded configuration

    Serial.print("Power Threshold loaded: "); Serial.println(sysConfig.powerThreshold);
    Serial.print("Device Name loaded: "); Serial.println(sysConfig.device_name);
    addBootMessage("Config", BOOT_OK);                                    // Show message on boot screen that configuration was loaded successfully
  } else {                                                                // EEPROM not initialized or invalid, set defaults and save to EEPROM
    Serial.println("No configuration found in EEPROM. Saving default values...");
    addBootMessage("Config", BOOT_WARN);                                  // Show warning message on boot screen that no valid configuration was found

    char defaultName[32];                                                 // Buffer to hold the default device name generated from the MAC address
    // Generate a default device name using the last three bytes of the MAC address (e.g., "PwrMtr-1A2B3C")
    snprintf(defaultName, sizeof(defaultName), "PwrMtr-%02X%02X%02X", mac[3], mac[4], mac[5]);
    
    // Set defaults
    strcpy(sysConfig.device_name, defaultName);                           // Set the default device name
    strcpy(sysConfig.manufacturer, DEFAULT_MANUFACTURER);                 // Use config.h default
    sysConfig.powerThreshold = DEFAULT_POWER_THRESHOLD;                   // Use config.h default
    sysConfig.ssid[0] = '\0';                                             // Set default SSID to empty
    sysConfig.password[0] = '\0';                                         // Set default WiFi password to empty
    sysConfig.staticIP = false;                                           // Set default to not use static IP
    strcpy(sysConfig.static_ip, "192.168.1.100");                         // Set default static IP address
    strcpy(sysConfig.static_gateway, "192.168.1.1");                      // Set default static gateway
    strcpy(sysConfig.static_subnet, "255.255.255.0");                     // Set default static subnet mask
    strcpy(sysConfig.static_dns, "192.168.1.1");                          // Set default static DNS server
    sysConfig.mqtt_server[0] = '\0';                                      // Set default MQTT server to empty
    sysConfig.mqtt_port = DEFAULT_MQTT_PORT;                              // Use config.h default
    sysConfig.mqtt_user[0] = '\0';                                        // Set default MQTT username to empty
    sysConfig.mqtt_pass[0] = '\0';                                        // Set default MQTT password to empty
    strcpy(sysConfig.ha_prefix, DEFAULT_HA_PREFIX);                       // Set default HA prefix from config.h
    sysConfig.buzzerMode = BUZZER_MODE_CONTINUOUS;                        // Set default buzzer mode to continuous
    sysConfig.language = LANG_IT;                                         // Set default language
    strcpy(sysConfig.web_user, DEFAULT_WEB_AUTH_USER);                    // Initialize default web user
    strcpy(sysConfig.web_pass, DEFAULT_WEB_AUTH_PASS);                    // Initialize default web password
    configSetCalibrationDefaults(sysConfig);                                // Per-device calibration defaults
    
    Serial.print("Default Device Name: "); Serial.println(sysConfig.device_name);

    EEPROM.write(0, EEPROM_MAGIC);                                        // Write Magic Byte at address 0 that means EEPROM has
                                                                          // been initialized with valid data
    EEPROM.put(1, sysConfig);                                             // Write the default configuration struct to EEPROM at address 1
    EEPROM.commit();                                                      // Commit changes to EEPROM
    savedSysConfig = sysConfig;                                           // Update the saved configuration copy to the current configuration
    addBootMessage("Data ", BOOT_OK);                                     // Show message on boot screen that default configuration was saved successfully

    setLanguage((Language)sysConfig.language);                            // Set the language for the device based on the default configuration
  }

  // --- ADS1115 Initialization ---
  ads.setGain(ADS_GAIN);                                                  // Use centralized gain setting
                                                                          // resolution in the 0-25A range with SCT-013 sensor (0-1V output)
  ads_ok = ads.begin(ADS1115_ADDR);                                       // Initialize the ADS1115 and check if it was successful
  if (ads_ok) {                                                           // If ADS1115 initialized successfully
    ads.setDataRate(ADS_SPS);                                             // Use centralized data rate
    Serial.println("ADS1115 initialized.");
    addBootMessage("ADS1115", BOOT_OK);                                   // Show boot emssage success for ADS1115 initialization
  } else {                                                                // If ADS1115 failed to initialize
    Serial.println("Can't initialize ADS1115. Check it! - Measures disabled.");
    addBootMessage("ADS1115", BOOT_FAIL);                                 // Show boot message failure for ADS1115. Measurements will be disabled
  }

  // --- GPIO Initialization (PCF8574 I/O expander, same I2C bus as ADS1115) ---
  pcfOutputByte = 0x00;                                                   // Output byte for PCF8574, initialized to all bits LOW (direct logic)
  pcf_ok = pcfWrite();                                                    // Send initial state (will write 0xFF to the chip)
  if (pcf_ok) {
    Serial.println("PCF8574 initialized.");
    addBootMessage("PCF8574", BOOT_OK);                                   // Show boot message success for PCF8574 initialization
  } else {
    Serial.println("Can't initialize PCF8574. Check it! - Buzzer disabled.");
    addBootMessage("PCF8574", BOOT_FAIL);                                 // Show boot message failure for PCF8574
  }

  pinMode(PIN_BTN, INPUT_PULLUP);                                         // Initialize button pin as input with internal pull-up resistor

  // --- WiFi & MQTT Initialization ---
  wifiSetup();                                                            // Lauch WiFi connection setup (or AP mode if no credentials)
  if (WiFi.getMode() == WIFI_AP) {                                        // If we are in AP mode
    addBootMessage("AP Mode", BOOT_OK);                                   // Show message on boot screen that AP mode was started successfully
  } else if (WiFi.status() == WL_CONNECTED) {                             // If WiFi is connected successfully
    addBootMessage("WiFi", BOOT_OK);                                      // Show message on boot screen that WiFi connection was successful
  } else {                                                                // If WiFi connection failed
    addBootMessage("WiFi", BOOT_FAIL);                                    // Show message on boot screen that WiFi connection failed
  }

  // --- Web Server Initialization ---
  webServer.on("/", handleRoot);                                          // Setup handler for root page
  webServer.on("/config", handleConfig);                                  // Setup handler for configuration page
  webServer.on("/save_config", handleSaveConfig);                         // Setup handler for saving configuration
  webServer.on("/reboot", HTTP_POST, handleReboot);                       // Setup handler for reboot action
  webServer.onNotFound(handleNotFound);                                   // Setup handler for 404 Not Found errors
  webServer.on("/update", HTTP_GET, handleUpdate);                             // Setup handler for OTA update page
  webServer.on("/update", HTTP_POST, handleUpdateResult, handleUpdateUpload);  // Setup handlers for OTA update form submission and file upload
  webServer.begin();                                                      // Start the web server
  Serial.println(F("Web server started on port 80."));
  addBootMessage("Server", BOOT_OK);                                      // Show message on boot screen that web server was started successfully

  // --- MQTT Client Initialization ---
  mqttClient.setServer(sysConfig.mqtt_server, sysConfig.mqtt_port);       // Set the MQTT broker server and port from the configuration
  mqttClient.setCallback(mqttCallback);                                   // Set the MQTT message callback function
  Serial.printf("MQTT Server: %s:%d\n", sysConfig.mqtt_server, sysConfig.mqtt_port);
  mqttClient.setBufferSize(MQTT_BUFFER_SIZE);                             // Use config.h buffer size
  mqttClient.setSocketTimeout(MQTT_TIMEOUT);                              // Use config.h timeout

  // Attempt initial MQTT connection to show status on boot screen if WiFi is connected and a server is defined
  if (WiFi.status() == WL_CONNECTED && strlen(sysConfig.mqtt_server) > 0) {
    if (mqttReconnect()) {
      addBootMessage("MQTT", BOOT_OK);                                    // MQTT connected successfully, show checkmark
    } else {
      addBootMessage("MQTT", BOOT_FAIL);                                  // MQTT connection failed, show red X
    }
  }

  Serial.println(F("##########################################\n\n"));

  delay(2000);                                                            // Short delay for user to read boot messages before the main loop starts
}

// ###########################################################################################################################
//   MAIN  #################################################################################################################
// #######################################################################################################################
void loop() {
  unsigned long now = millis();                                           // Capture current time at the very beginning of the loop

  if (shouldReboot) {                                                     // Handle reboot request
    delay(2000);                                                          // Short delay to allow any pending operations to complete before rebooting
    ESP.restart();                                                        // Restart the device to complete the reboot process
  }

  webServer.handleClient();                                               // Handle incoming web server clients (non-blocking)

  // Naked Block for WiFi and MQTT Reconnection with exponential Backoff Strategy.
  // If WiFi is connected but MQTT is not, it also attempts to reconnect to the MQTT broker using the same backoff strategy.
  {
    int wifiStatus = WiFi.status();                                       // Get the current WiFi connection status

    if (wifiStatus == WL_CONNECTED && lastKnownWifiStatus != WL_CONNECTED) {   // If WiFi just reconnected
      currentBackoff = 5000;                                              // Reset backoff to initial value
      lastReconnectAttempt = 0;                                           // Reset last reconnect attempt time to allow immediate MQTT reconnection attempt
      Serial.println("WiFi Reconnected, resetting backoff.");
    }
    lastKnownWifiStatus = wifiStatus;                                     // Update the last known WiFi status for the next iteration

    if (wifiStatus != WL_CONNECTED && WiFi.getMode() != WIFI_AP) {        // If WiFi is disconnected and device is not in AP mode
      if (now - lastReconnectAttempt > currentBackoff) {                  // Try to reconnect if the backoff time has passed since the last attempt
        lastReconnectAttempt = now;                                       // Update the last reconnect attempt time to the current time
        Serial.print("Reconnecting to WiFi... (Backoff: "); Serial.print(currentBackoff/1000); Serial.println("s)");
        WiFi.begin(sysConfig.ssid, sysConfig.password);                   // Attempt to reconnect to WiFi with the configured SSID and password
        if (currentBackoff < MAX_BACKOFF) currentBackoff *= 2;            // If the connection attempt fails, the backoff time will be doubled
                                                                          // for the next attempt, up to a maximum limit defined by MAX_BACKOFF
      }
    } else if (wifiStatus == WL_CONNECTED) {                              // If WiFi is connected, check MQTT connection
      if (!mqttClient.connected()) {                                      // If MQTT is not connected, attempt to reconnect using the same backoff strategy
        if (now - lastReconnectAttempt > currentBackoff) {                // Try to reconnect if the backoff time has passed since the last attempt
          lastReconnectAttempt = now;                                     // Update the last reconnect attempt time to the current time
          Serial.print("Reconnecting to MQTT... (Backoff: "); Serial.print(currentBackoff/1000); Serial.println("s)");
          if (mqttReconnect()) {                                          // If MQTT reconnection is successful
            currentBackoff = 5000;                                        // If MQTT connection is successful, reset backoff to initial value
            lastReconnectAttempt = 0;                                     // Reset last reconnect attempt time
          } else {                                                        // If MQTT reconnection fails
            if (currentBackoff < MAX_BACKOFF) currentBackoff *= 2;        // The backoff time will be doubled for the next attempt
                                                                          // up to a maximum limit defined by MAX_BACKOFF
          }
        }
      } else {                                                            // If MQTT is connected
        mqttClient.loop();                                                // Call the MQTT client loop to maintain the connection
        if (currentBackoff > 5000) currentBackoff = 5000;                 // Reset backoff
      }
    }
  }

  deviceHandleButtons();                                                  // Handle button input with debounce and hold logic

  // --- Auto-return to Page 1 ---
  // If the current display page is not 1 and the button is not being pressed and if the timeout has elapsed,
  // automatically return to page 1 and hide any save messages
  if (sysState.displayPage != 1 && btnState == HIGH && (now - lastDebounceTime > PAGE_TIMEOUT)) {
    sysState.displayPage = 1;
    sysState.showSaveMessage = false;
  }

  // Differential sampling on ADS1115 for both Current (Ch 0-1) and Voltage (Ch 2-3).
  // The loop processes both measurements at a high rate to ensure accurate RMS values over the 1s calculation window.
  if (ads_ok) {
    // Sample Current (Ch 0-1)
    int16_t resI = ads.readADC_Differential_0_1();
    float vI = (float)resI * ADS_MULTIPLIER;                              // Use centralized multiplier
    sumSamplesI += vI;
    sumSquaredI += (double)vI * vI;

    // Sample Voltage (Ch 2-3)
    int16_t resV = ads.readADC_Differential_2_3();
    float vV = (float)resV * ADS_MULTIPLIER;                              // Use centralized multiplier
    sumSamplesV += vV;
    sumSquaredV += (double)vV * vV;

    samplesCount++;
  }

  // Perform RMS calculation and update power state every second based on the accumulated samples. The RMS calculation also includes a noise filter to ignore very low voltage readings that are likely just noise, and it updates the system state with the calculated current and power values
  if (now - lastSample >= RMS_WINDOW_MS) {                                // Use config.h interval
    unsigned long dt = now - lastSample;                                  // Calculate the time difference since the last sample
    lastSample = now;                                                     // Update the last sample time to the current time
    if (samplesCount > 0) {
      // Current RMS calculation
      double meanI = sumSamplesI / samplesCount;
      double varI = (sumSquaredI / samplesCount) - (meanI * meanI);
      float vRmsI = (varI > 0) ? (float)sqrt(varI) : 0.0f;
      if (vRmsI < CURRENT_NOISE_FLOOR) vRmsI = 0.0f;                      // Use config.h noise floor
      sysState.currentRMS = (vRmsI * FACTOR_A_V * sysConfig.calScaleA) + sysConfig.calOffsetA;
      if (sysState.currentRMS < 0.0f) sysState.currentRMS = 0.0f;

      // Voltage RMS calculation
      double meanV = sumSamplesV / samplesCount;
      double varV = (sumSquaredV / samplesCount) - (meanV * meanV);
      float vRmsV = (varV > 0) ? (float)sqrt(varV) : 0.0f;
      float measuredV = (vRmsV * FACTOR_V_V * sysConfig.calScaleV) + sysConfig.calOffsetV;
      if (measuredV < 0.0f) measuredV = 0.0f;
      
      // Dynamic voltage management: use measurement if valid, otherwise fallback to nominal
      measuredVoltageRMS = (measuredV > VOLTAGE_MIN_THRESHOLD) ? measuredV : (float)DEFAULT_GRID_VOLTAGE;

      // Calculate Power using real-time measured voltage
      sysState.powerW = sysState.currentRMS * measuredVoltageRMS;
    } else {
      // No samples collected (e.g. ADS1115 missing or hardware error)
      sysState.currentRMS = 0.0f;
      sysState.powerW = 0.0f;
      measuredVoltageRMS = 0.0f;                                          // No sensor = 0V
    }

    // --- PTC Temperature Measurement (Averaged over 10 cycles) ---
    sysState.ptcSum += analogRead(PIN_PTC);                               // Use global state accumulator
    sysState.ptcCount++;                                                  // Use global state counter
    if (sysState.ptcCount >= PTC_AVG_SAMPLES) {                           // Use config.h constant
      float avgRaw = sysState.ptcSum / (float)PTC_AVG_SAMPLES;
      // Voltage measured at the ADC pin. 1.0V reference for bare ESP8266
      float vMeasuredADC = avgRaw * (1.0 / 1023.0);
      // Recover voltage at the main divider junction by compensating the drop across R21 (220 ohm) due to R22 (100k) load
      // V_main_divider = V_ADC * (R21 + R22) / R22
      float vDivider = vMeasuredADC * (PTC_R_DIVIDER_TOTAL / PTC_R_DIVIDER_LOAD);
      const float kEpsilon = 0.001f;                                          // Small guard to avoid near-zero denominators

      // Guard 1: divider node must stay inside physical range and away from 0-denominator zone.
      if (vDivider <= kEpsilon || vDivider >= (PTC_V_SOURCE - kEpsilon)) {
        sysState.temperature = NAN;
      } else {
        // Calculate the total equivalent resistance to GND from V_main_divider
        // R_equiv = (V_main_divider * R7) / (V_source - V_main_divider) where R7 = 33K and V_source = 3.3V
        float denom1 = (PTC_V_SOURCE - vDivider);
        float rEquivalentGND = (vDivider * PTC_R_PULLUP) / denom1;
        float denom2 = (PTC_R_DIVIDER_TOTAL - rEquivalentGND);

        // Guard 2: avoid singularity when removing the parallel measurement branch.
        if (fabsf(denom2) <= kEpsilon || rEquivalentGND <= 0.0f) {
          sysState.temperature = NAN;
        } else {
          // Isolate the PTC (R8) resistance by removing the parallel measurement branch (R21 + R22 = 100.22k).
          float rPTC = (rEquivalentGND * PTC_R_DIVIDER_TOTAL) / denom2;

          // Guard 3: invalid electrical state.
          if (rPTC <= 0.0f || isnan(rPTC) || isinf(rPTC)) {
            sysState.temperature = NAN;
          } else {
            // Conversion to Celsius (TFPT1206L1002DV: 10k @ 25°C). The formula used is derived from the linear approximation
            // of the PTC resistance-temperature characteristic around 25°C,
            // where 0.00411 is the temperature coefficient of resistance (TCR) for the PTC.
            sysState.temperature = ((rPTC / PTC_NOMINAL_RES) - 1.0f) / PTC_COEFF + 25.0f + sysConfig.calOffsetTemp;

            // Validation: if the sensor is disconnected (open circuit) or shorted, the calculated value will be unrealistic.
            // We accept a range from -30°C to +100°C for internal board temperature.
            if (isnan(sysState.temperature) || isinf(sysState.temperature) || sysState.temperature < -30.0f || sysState.temperature > 100.0f) {
              sysState.temperature = NAN;
            }
          }
        }
      }

      sysState.ptcSum = 0;                                                // Reset global accumulators
      sysState.ptcCount = 0;
    }
    accumulatedEnergy += sysState.powerW * dt;                            // Weighted Average for Energy = Power * Time
    accumulatedCharge += sysState.currentRMS * dt;                        // Weighted Average for Charge = Current * Time
    accumulatedTime += dt;                                                // Accumulate the total time for weighted average calculations

    // --- Serial output for debugging ---
    Serial.print("V_meas: "); Serial.print(measuredVoltageRMS, 1);
    Serial.print(" V | I: "); Serial.print(sysState.currentRMS, 3);
    Serial.print(" A | P: "); Serial.print(sysState.powerW, 0); 
    Serial.print(" W | Thr: "); Serial.print(sysConfig.powerThreshold);
    Serial.print(" | Temp: "); Serial.println(sysState.temperature);

    // --- Reset accumulators for the next RMS calculation cycle ---
    sumSquaredI = 0; sumSamplesI = 0;
    sumSquaredV = 0; sumSamplesV = 0;
    samplesCount = 0;
  }

  // --- Buzzer Control Logic (Non Blocking) ---
  uint8_t oldPcfByte = pcfOutputByte;                                     // Save previous state to check for changes at the end
  if (sysState.powerW >= sysConfig.powerThreshold) {                      // If the measured power exceeds the configured threshold
    if (!alarmActive) {                                                   // If the alarm is not already active
      alarmActive = true;                                                 // Activate the alarm
      buzzerState = true;                                                 // Start with buzzer ON
      pcfOutputByte |= (1 << PCF_BIT_BUZZER);                             // Set the bit for the buzzer to 1 (ON) (direct logic)
      lastBuzzerToggle = now;                                             // Record the time of the last buzzer toggle
    } else {                                                              // If the alarm is already active, manage the buzzer based on the configured mode
      if (sysConfig.buzzerMode == BUZZER_MODE_CONTINUOUS) {               // If the buzzer mode is continuous, ensure the buzzer is ON without toggling
        pcfOutputByte |= (1 << PCF_BIT_BUZZER);                           // Set the bit for the buzzer to 1 (ON) (direct logic)
      } else {                                                            // If the buzzer mode is pulsing
        unsigned long interval = (sysConfig.buzzerMode == BUZZER_MODE_PULSE_SLOW) ? 1000 : 500;    // Determine the total period for the pulse
        unsigned long halfPeriod = interval / 2;                          // Calculate the half period for toggling the buzzer state
        if (now - lastBuzzerToggle >= halfPeriod) {                       // If it's time to toggle the buzzer state based on the half period
          lastBuzzerToggle = now;                                         // Update the last buzzer toggle time to the current time
          buzzerState = !buzzerState;                                     // Toggle the buzzer state
          if (buzzerState) pcfOutputByte |= (1 << PCF_BIT_BUZZER);        // Set the bit for the buzzer to 1 (ON) (direct logic)
          else pcfOutputByte &= ~(1 << PCF_BIT_BUZZER);                   // Clear the bit for the buzzer to 0 (OFF) (direct logic)
        }
      }
    }
  } else {                                                                // If the measured power is below the threshold
    alarmActive = false;                                                  // Ensure the alarm is deactivated
    buzzerState = false;                                                  // Reset the buzzer state to OFF
    pcfOutputByte &= ~(1 << PCF_BIT_BUZZER);                              // Clear the bit for the buzzer to 0 (OFF) (direct logic)
  }
  if (pcf_ok && pcfOutputByte != oldPcfByte) pcfWrite();                  // Update hardware only if the state has actually changed

  // --- MQTT Publishing Logic ---
  // The Weighted Average window is defined here (20000ms). The Weighted Average represents the mean consumption since the last reset
  if (mqttClient.connected() && (now - lastMsg > MQTT_PUBLISH_INTERVAL)) { // Use config.h interval
    float avgPower, avgCurrent;                                           // Variables to hold the average power and current values for MQTT publishing
    char msg[256];                                                        // State JSON is small, 256 bytes is plenty
    char topic[128];                                                      // Increased to 128 for consistency with discovery topics
    lastMsg = now;                                                        // Update the last message time to the current time

    // Calculate the average power and current based on the accumulated energy, charge, and time.
    // If the accumulated time is greater than 0, calculate the averages; otherwise, use the current instantaneous
    // values from the system state to avoid division by zero
    avgPower = (accumulatedTime > 0) ? (float)(accumulatedEnergy / accumulatedTime) : sysState.powerW;
    avgCurrent = (accumulatedTime > 0) ? (float)(accumulatedCharge / accumulatedTime) : sysState.currentRMS;

    // Safe conversion of float values to strings to prevent empty JSON fields.
    // This ensures compatibility across all ESP8266 toolchains regardless of printf float support.
    float safePower = isnan(avgPower) ? 0.0f : avgPower;
    float safeCurrent = isnan(avgCurrent) ? 0.0f : avgCurrent;
    float safeTemp = isnan(sysState.temperature) ? 0.0f : sysState.temperature;
    float safeVoltage = isnan(measuredVoltageRMS) ? 0.0f : measuredVoltageRMS;

    char pBuf[10], vBuf[10], cBuf[10], tBuf[10];
    dtostrf(safePower, 1, 0, pBuf);                                       // Power: 0 decimals
    dtostrf(safeVoltage, 1, 0, vBuf);                                     // Voltage: 0 decimals
    dtostrf(safeCurrent, 1, 1, cBuf);                                     // Current: 1 decimal
    dtostrf(safeTemp, 1, 1, tBuf);                                        // Temp: 1 decimal

    // Construct the JSON payload using safe string buffers for float values
    char buzzerModeString[32];
    const __FlashStringHelper* fsh = nullptr;
    switch (sysConfig.buzzerMode) {
      case BUZZER_MODE_CONTINUOUS: fsh = getText(ID_BUZZER_MODE_CONTINUOUS); break;
      case BUZZER_MODE_PULSE_SLOW: fsh = getText(ID_BUZZER_MODE_PULSE_SLOW); break;
      case BUZZER_MODE_PULSE_FAST: fsh = getText(ID_BUZZER_MODE_PULSE_FAST); break;
      default: fsh = nullptr; break;
    }

    if (fsh) {
      strncpy_P(buzzerModeString, (PGM_P)fsh, sizeof(buzzerModeString));
      buzzerModeString[sizeof(buzzerModeString) - 1] = '\0';
    } else {
      strcpy(buzzerModeString, "Unknown");
    }

    snprintf(msg, sizeof(msg), "{\"power\": %s, \"voltage\": %s, \"current\": %s, \"temperature\": %s, \"threshold\": %d, \"buzzer_mode\": \"%s\"}",
      pBuf, vBuf, cBuf, tBuf, sysConfig.powerThreshold, buzzerModeString);


    // Publish the JSON payload to the MQTT topic in the format "<device_name>/state" (e.g., "PwrMtr-1A2B3C/state")
    char mqttDeviceId[33];
    makeMqttSafeId(sysConfig.device_name, mqttDeviceId, sizeof(mqttDeviceId));
    snprintf(topic, sizeof(topic), "%s/state", mqttDeviceId);             // Construct the MQTT topic string
    mqttClient.publish(topic, msg);                                       // Publish the message to the MQTT broker
    
    // Reset the accumulators for energy, charge, and time after publishing to start a new Weighted Average calculation window
    accumulatedEnergy = 0;                                                // Reset the accumulated energy for the next MQTT publishing window
    accumulatedCharge = 0;                                                // Reset the accumulated charge for the next MQTT publishing window
    accumulatedTime = 0;                                                  // Reset the accumulated time for the next MQTT publishing window
  }

  // --- LCD Display Update Logic ---
  if (now - lastDisplayUpdate > DISPLAY_UPDATE_INTERVAL) {                // Use config.h interval
    lastDisplayUpdate = now;                                              // Update the LCD display with the current system state and configuration
    updateDisplay();                                                      // Call the function to update the LCD display
  }
}
