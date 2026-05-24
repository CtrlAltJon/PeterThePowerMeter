// ###########################################################################################################################
// Definition of global variables and data structures used throughout the application.
// Includes configuration structs and shared object instances like the WebServer and MQTT Client.
//
// Author: CtrlAltJon
// Last Updated: January 2026
// Copyright (c) 2026 CtrlAltJon
// License: MIT
// The software is provided "as is", without warranty of any kind.
// This license notice must be included in any copy or portion of the software.
//
// ###########################################################################################################################

#include "sys_globals.h"                                                  // Include the header file that declares the global variables and structures
#include <ESP8266WiFi.h>                                                  // For WiFi status constants
#include <math.h>                                                         // isnan, isinf

void configSetCalibrationDefaults(AppConfig& cfg) {
  cfg.calOffsetV = 0.0f;
  cfg.calScaleV = CAL_SCALE_DEFAULT;
  cfg.calOffsetA = 0.0f;
  cfg.calScaleA = CAL_SCALE_DEFAULT;
  cfg.calOffsetTemp = 0.0f;
}

static float clampCalFloat(float value, float minVal, float maxVal, float fallback) {
  if (isnan(value) || isinf(value)) return fallback;
  if (value < minVal) return minVal;
  if (value > maxVal) return maxVal;
  return value;
}

void configSanitizeCalibration(AppConfig& cfg) {
  cfg.calScaleV = clampCalFloat(cfg.calScaleV, CAL_SCALE_MIN, CAL_SCALE_MAX, CAL_SCALE_DEFAULT);
  cfg.calScaleA = clampCalFloat(cfg.calScaleA, CAL_SCALE_MIN, CAL_SCALE_MAX, CAL_SCALE_DEFAULT);
  cfg.calOffsetV = clampCalFloat(cfg.calOffsetV, CAL_OFFSET_V_MIN, CAL_OFFSET_V_MAX, 0.0f);
  cfg.calOffsetA = clampCalFloat(cfg.calOffsetA, CAL_OFFSET_A_MIN, CAL_OFFSET_A_MAX, 0.0f);
  cfg.calOffsetTemp = clampCalFloat(cfg.calOffsetTemp, CAL_OFFSET_TEMP_MIN, CAL_OFFSET_TEMP_MAX, 0.0f);
}

// sysConfig is the main configuration structure that holds all the user-configurable settings for the device. This includes WiFi credentials, MQTT settings, device name, power threshold, grid voltage, buzzer mode, and other relevant configuration parameters. This structure is typically loaded from EEPROM at startup and can be modified through the web interface or other means during runtime.
AppConfig sysConfig;

// savedSysConfig is used to store the last saved configuration loaded from EEPROM. This allows us to compare the current configuration (sysConfig) with the saved one to determine if there are unsaved changes, which is useful for UI indications and decision-making in the code.
AppConfig savedSysConfig;

// sysState is a structure that holds the current state of the system, including runtime information such as the current WiFi SSID, AP mode SSID, selected action in the UI, and other dynamic state variables that are relevant during the operation of the device. This structure is used to manage the state of the application and provide information to the display manager for rendering the UI.
AppState sysState;

// --- Hardware Objects ---
Adafruit_ADS1115 ads;                                                     // Creation of ADS1115 object
bool ads_ok = false;                                                      // Flag to indicate if ADS1115 is initialized
bool pcf_ok = false;                                                      // Flag to indicate if PCF8574 is initialized

// --- Sampling & RMS Accumulators ---
double sumSquaredI = 0, sumSamplesI = 0;                                  // Current sampling accumulators
double sumSquaredV = 0, sumSamplesV = 0;                                  // Voltage sampling accumulators
int samplesCount = 0;                                                     // Number of samples in the current window
float measuredVoltageRMS = 0.0;                                           // Real-time measured voltage

// --- Weighted Average Accumulators (MQTT) ---
double accumulatedEnergy = 0.0;                                           // Accumulated Energy (Watt-ms) for weighted average
double accumulatedCharge = 0.0;                                           // Accumulated Charge (Ampere-ms) for weighted average
unsigned long accumulatedTime = 0;                                        // Total time elapsed for the averaging window (ms)

// --- Logic & Timer Variables ---
unsigned long lastMsg = 0;                                                // Last MQTT message sent time
unsigned long lastThresholdChange = 0;                                    // Last time the threshold was modified by the user
unsigned long lastReconnectAttempt = 0;                                   // Last MQTT/WiFi reconnect attempt time
unsigned long lastDisplayUpdate = 0;                                      // Last Display update time
unsigned long lastSample = 0;                                             // Last RMS calculation start time
unsigned long currentBackoff = 5000;                                      // Exponential backoff current delay (starts at 5s)
int lastKnownWifiStatus = WL_IDLE_STATUS;                                 // To track WiFi state changes

// --- Buzzer Control ---
unsigned long lastBuzzerToggle = 0;                                       // Timer for pulsing buzzer
bool buzzerState = false;                                                 // Logical state (true = ON/Sounding)
bool alarmActive = false;                                                 // Flag to track alarm state transition

// --- Button State ---
int btnState = HIGH;                                                      // Button state
int lastBtnState = HIGH;                                                  // Previous button state
unsigned long lastDebounceTime = 0;                                       // Last debounce time

// Web server instance running on port 80, used to serve the configuration web pages and handle user interactions for configuring the device settings.
ESP8266WebServer webServer(80);

// MQTT client instance that uses the ESP8266 WiFi client to connect to an MQTT broker and publish/subscribe to topics for sending sensor data and receiving commands. This client is configured with the MQTT server address, port, and credentials from sysConfig and is used throughout the application to manage MQTT communication.
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Flag to indicate if the device should perform a safe reboot. This is set to true when the user selects the reboot option in the UI, allowing the main loop to handle the reboot process gracefully, ensuring that any necessary cleanup or state saving can occur before restarting the device.
bool shouldReboot = false;

// Output status Byte for the PCF8574 I/O expander. Direct logic.
uint8_t pcfOutputByte = 0x00;
