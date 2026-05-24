#ifndef SYS_GLOBALS_H
#define SYS_GLOBALS_H

// ###########################################################################################################################
// Global definitions.
// Defines global constants, enumerations, data structures (AppConfig, AppState), and extern declarations for objects shared
// across the application.
//
// Author: CtrlAltJon
// Last Updated: January 2026
// Copyright (c) 2026 CtrlAltJon
// License: MIT
// The software is provided "as is", without warranty of any kind.
// This license notice must be included in any copy or portion of the software.
//
// ###########################################################################################################################

#include <Arduino.h>                             // Main Arduino Library
#include <ESP8266WebServer.h>                    // Web Server Library for ESP8266
#include <PubSubClient.h>                        // MQTT library
#include <Adafruit_ADS1X15.h>                    // ADS1115 Library
#include "languages.h"                           // Languages
#include "config.h"                              // Constants and Pin definitions

// Buzzer alarm modes enumerator
enum BuzzerMode {
  BUZZER_MODE_CONTINUOUS,                        // Continuous mode
  BUZZER_MODE_PULSE_SLOW,                        // Slow pulse mode (1 second)
  BUZZER_MODE_PULSE_FAST,                        // Fast pulse mode (0.5 seconds)
  BUZZER_MODE_COUNT                              // Counter for selecting buzzer modes
};

// EEPROM structure for configuration settings
struct AppConfig {
  char device_name[33];                          // Device name
  char manufacturer[33];                         // Manufacturer name
  int powerThreshold;                            // Power threshold alarm trigger (W)
  char ssid[33];                                 // WiFi SSID
  char password[65];                             // WiFi password
  bool staticIP;                                 // Static IP configuration
  char static_ip[16];                            // Static IP address
  char static_gateway[16];                       // Static gateway
  char static_subnet[16];                        // Static netmask
  char static_dns[16];                           // Static DNS
  char mqtt_server[64];                          // MQTT server IP
  int mqtt_port;                                 // MQTT server port
  char mqtt_user[33];                            // MQTT username (optional)
  char mqtt_pass[65];                            // MQTT password (optional)
  char ha_prefix[33];                            // Home Assistant discovery prefix
  BuzzerMode buzzerMode;                         // Buzzer alarm mode
  Language language;                             // Selected language
  char web_user[25];                             // Web interface username (max 24)
  char web_pass[25];                             // Web interface password (max 24)
  float calOffsetV;                              // Voltage offset (V), applied after scale
  float calScaleV;                               // Voltage scale factor (1.0 = no correction)
  float calOffsetA;                              // Current offset (A), applied after scale
  float calScaleA;                               // Current scale factor (1.0 = no correction)
  float calOffsetTemp;                           // Temperature offset (°C)
};

// Reset or clamp per-device sensor calibration fields
void configSetCalibrationDefaults(AppConfig& cfg);
void configSanitizeCalibration(AppConfig& cfg);

// Volatile data for device status
struct AppState {
  float powerW;                                  // Power in Watt
  float currentRMS;                              // Current in RMS Ampere
  float temperature;                             // Temperature from PTC sensor on A0
  float ptcSum;                                  // PTC averaging accumulator
  int ptcCount;                                  // PTC sample counter
  uint8_t powerHistory[Layout::SCREEN_W];        // Power history for the trend graph in the LCD page 1
  bool historyInitialized;                       // Flag to initialize the power history graph (first run)
  char apSSID[33];                               // SSID of the Access Point when in AP mode (e.g., "PowerMeter-XXXX")
  
  // LCD UI state
  int displayPage;                               // Current page
  bool showSaveMessage;                          // Flag to display the settings save message confirm on LCD
  unsigned long saveMessageStart;                // Timestamp for the settings save message visualization
  bool lastSaveResult;                           // Flasg of the last settings save operation result
  int selectedAction;                            // 0: AP Mode, 1: Reboot, 2: Factory Reset
  unsigned long btnPressedTime;                  // Time pressure for bar loading
  bool actionTriggered;                          // Action triggered status
  bool showAPModeMessage;                        // Flag to show AP mode started
  unsigned long apModeMessageStart;              // Timestamp for the AP mode message visualization
};

// Structures for the state, comparison, data
extern AppConfig sysConfig;                      // Global instance of the current (working) configuration. Modified in real time by user actions
extern AppConfig savedSysConfig;                 // Copy of the last saved configuration. Used for comparisons in the UI (e.g. unsaved changes)
extern AppState sysState;                        // Global instance of the application's volatile state. Contains real-time data (sensors, UI status, etc.)

// --- Hardware Objects ---
extern Adafruit_ADS1115 ads;                     // ADS1115 ADC instance
extern bool ads_ok;                              // Initialization flag
extern bool pcf_ok;                              // PCF8574 initialization flag

// --- Sampling & RMS Accumulators ---
extern double sumSquaredI, sumSamplesI;          // Current accumulators
extern double sumSquaredV, sumSamplesV;          // Voltage accumulators
extern int samplesCount;                         // Global sample counter
extern float measuredVoltageRMS;                 // Real-time calculated voltage

// --- Weighted Average Accumulators (MQTT) ---
extern double accumulatedEnergy;                 // Watt-ms
extern double accumulatedCharge;                 // Ampere-ms
extern unsigned long accumulatedTime;            // Total window time

// --- Logic & Timer Variables ---
extern unsigned long lastMsg;                    // Last MQTT publish
extern unsigned long lastThresholdChange;        // Last UI threshold adjustment
extern unsigned long lastReconnectAttempt;       // Reconnection timer
extern unsigned long lastDisplayUpdate;          // LCD refresh timer
extern unsigned long lastSample;                 // RMS calculation timer
extern unsigned long currentBackoff;             // Exponential backoff status
extern int lastKnownWifiStatus;                  // WiFi state tracker
extern unsigned long lastBuzzerToggle;           // Pulsing buzzer timer
extern bool buzzerState;                         // Logical buzzer state
extern bool alarmActive;                         // Alarm active flag

// --- Button State ---
extern int btnState;                             // Current stable button state
extern int lastBtnState;                         // Last raw button state
extern unsigned long lastDebounceTime;           // Debounce timer

// External shared objects
extern ESP8266WebServer webServer;               // Global instance of the web server for the configuration interface
extern PubSubClient mqttClient;                  // Global instance of the MQTT client for data publishing
extern bool shouldReboot;                        // Flag to manage device reboot
extern uint8_t pcfOutputByte;                    // Actual byte to be sent to the PCF8574 I/O expander (direct logic)

#endif
