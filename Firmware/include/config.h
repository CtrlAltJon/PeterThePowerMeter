#ifndef CONFIG_H
#define CONFIG_H

// ###########################################################################################################################
// Centralized Project Configuration (Hardware, Logic, Physics and UI Layout)
//
// Author: CtrlAltJon
// Last Updated: June 2026
// Copyright (c) 2026 CtrlAltJon
// License: MIT
// The software is provided "as is", without warranty of any kind.
// This license notice must be included in any copy or portion of the software.
//
// ###########################################################################################################################
 
#include <Arduino.h>                                                      // Include Arduino core library for basic types and functions

// --- Firmware & System ---
#define FIRMWARE_VERSION         "0.0"                                    // Firmware version string
#define EEPROM_SIZE               1024                                    // EEPROM size in bytes
#define EEPROM_MAGIC              0x4A                                    // Magic byte to validate EEPROM data (ASCII 'J' for Jon)

// --- GPIO & Addresses ---
#define PIN_BTN                   12                                      // Button pin
#define PIN_PTC                   A0                                      // Analog input for PTC
#define PCF8574_ADDR              0x21                                    // I2C address for PCF8574 (Detected by scan)
#define ADS1115_ADDR              0x48                                    // I2C address for ADS1115 (Detected by scan)
#define PCF_BIT_BUZZER            0                                       // Bit index for Buzzer on PCF8574

// --- Logic & Timing (ms) ---
#define DEBOUNCE_DELAY            100                                     // Button debounce delay
#define HOLD_THRESHOLD            900                                     // Time to hold button for long press (ms)
#define REPEAT_DELAY              350                                     // Delay before repeating action when button is held (ms)
#define SYS_ACTION_HOLD           5000                                    // Time to hold button for system actions (s)
#define PAGE_TIMEOUT              30000                                   // OLED auto-return timeout
#define MQTT_PUBLISH_INTERVAL     20000                                   // Standard publish interval
#define DISPLAY_UPDATE_INTERVAL   500                                     // LCD refresh rate (ms)
#define RMS_WINDOW_MS             1000                                    // RMS window calculation interval (ms)
#define WIFI_CONNECT_TIMEOUT      30000                                   // WiFi connection timeout (ms)
#define MAX_BACKOFF               900000                                  // Max reconnection backoff (15 min)

// --- UI & Web Timing ---
#define SAVE_MSG_DURATION         2000                                    // Duration of "Saved" popup (ms)
#define UI_SCROLL_DELAY           2000                                    // Pause before text scroll (ms)
#define UI_SCROLL_SPEED           2                                       // Pixels per frame
#define AP_MSG_DURATION           3000                                    // AP Mode confirmation duration (ms)
#define WEB_REFRESH_STATUS        10                                      // Web root refresh (s)
#define WEB_REFRESH_SAVE          2                                       // Web redirect after save (s)
#define WEB_REFRESH_REBOOT        5                                       // Web redirect after reboot (s)
#define DEFAULT_WEB_AUTH_USER     "admin"                                 // Default Basic Auth username
#define DEFAULT_WEB_AUTH_PASS     "admin"                                 // Default Basic Auth password

// --- Physics & Calibration ---
#define FACTOR_A_V                25.0f                                   // SCT-013 100A/1V factor
#define FACTOR_V_V                246.0f                                  // ZMPT107-1 Voltage divider factor
#define ADS_MULTIPLIER            0.0000625f                              // GAIN_TWO: 2.048V / 32768
#define ADS_GAIN                  GAIN_TWO                                // ADS1115 Gain setting
#define ADS_SPS                   RATE_ADS1115_860SPS                     // ADS1115 Data rate
#define PTC_AVG_SAMPLES           10                                      // Number of PTC samples for averaging
#define TEMP_WARNING_THRESHOLD    45.0f                                   // Temp color change trigger (°C)
#define CURRENT_NOISE_FLOOR       0.002f                                  // Noise floor filter for RMS current calculation
#define VOLTAGE_MIN_THRESHOLD     50.0f                                   // Minimum voltage to use real reading (otherwise fallback to 230V)

// --- Per-device calibration limits (stored in EEPROM, applied at runtime) ---
#define CAL_SCALE_DEFAULT         1.0f                                    // Default multiplicative scale (no correction)
#define CAL_SCALE_MIN             0.1f                                    // Minimum allowed scale factor
#define CAL_SCALE_MAX             10.0f                                   // Maximum allowed scale factor
#define CAL_OFFSET_V_MIN          -500.0f                                 // Voltage offset limits (V)
#define CAL_OFFSET_V_MAX          500.0f
#define CAL_OFFSET_A_MIN          -100.0f                                 // Current offset limits (A)
#define CAL_OFFSET_A_MAX          100.0f
#define CAL_OFFSET_TEMP_MIN       -50.0f                                  // Temperature offset limits (°C)
#define CAL_OFFSET_TEMP_MAX       50.0f

// --- PTC Calibration & Hardware ---
#define PTC_R_DIVIDER_TOTAL       100220.0f                               // R21 (220) + R22 (100k)
#define PTC_R_DIVIDER_LOAD        100000.0f                               // R22 (100k)
#define PTC_R_PULLUP              33000.0f                                // R7 (33k pull-up to 3.3V)
#define PTC_V_SOURCE              3.3f                                    // Logic source voltage
#define PTC_NOMINAL_RES           10000.0f                                // PTC resistance at 25°C (10k)
#define PTC_COEFF                 0.00411f                                // PTC Temperature Coefficient (TCR)

// --- Default Configuration Values ---
#define DEFAULT_POWER_THRESHOLD   3100                                    // Default power threshold for alerts (W)
#define MAX_POWER_THRESHOLD       6000                                    // UI wrap-around for power threshold
#define THRESHOLD_STEP            100                                     // UI increment step for power threshold
#define DEFAULT_GRID_VOLTAGE      230                                     // Default grid voltage for power calculation if measurement is invalid (V)
#define DEFAULT_MQTT_PORT         1883                                    // Default MQTT port
#define DEFAULT_MANUFACTURER      "DIY"                                   // Default manufacturer string for device information
#define DEFAULT_HA_PREFIX         "homeassistant"                         // Default Home Assistant discovery prefix
#define MQTT_BUFFER_SIZE          512                                     // MQTT message buffer size
#define MQTT_THRESHOLD_COMMAND_TOPIC "%s/threshold/set"                   // MQTT command topic for power threshold
#define MQTT_BUZZER_MODE_COMMAND_TOPIC "%s/buzzer_mode/set"               // MQTT command topic for buzzer mode
#define MQTT_TIMEOUT              5                                       // MQTT connection timeout (s)

namespace Layout {                                                        // Namespace to encapsulate all layout-related constants and definitions
  // --- Screen Dimensions ---
  const int SCREEN_W              = 80;                                   // Width of the display in pixels
  const int SCREEN_H              = 160;                                  // Height of the display in pixels
  const int SCREEN_CX             = SCREEN_W / 2;                         // Center X
    
  // --- Standard & Customized Colors (RGB565) ---
  const uint16_t COL_BLACK        = 0x0000;
  const uint16_t COL_WHITE        = 0xFFFF;
  const uint16_t COL_RED          = 0xF800;
  const uint16_t COL_GREEN        = 0x3666;                               // Custom Green for badges
  const uint16_t COL_BLUE         = 0x001F;
  const uint16_t COL_CYAN         = 0x07FF;
  const uint16_t COL_YELLOW       = 0xFFE0;
  const uint16_t COL_YELLOW_DARK  = 0xFE80;                               // Custom Dark Yellow
  const uint16_t COL_ORANGE       = 0xFD20;
  const uint16_t COL_BG_BADGE_OFF = 0x2965;                               // Dark Gray for OFF badges
  const uint16_t COL_TEXT_UNIT    = 0xBDF7;                               // Light Blue for units (kW)
  const uint16_t COL_INFO_LABEL   = 0x1D58;                               // Dark Blue for info labels (Host, SSID, IP)
  const uint16_t COL_GRAY_TEXT    = 0x9492;                               // Medium Gray for secondary text
  const uint16_t COL_TREND_BLUE   = 0x19D7;                               // Graph line color
  const uint16_t COL_TREND_CYAN   = 0x433E;                               // Graph current value point color

  // --- Page 1: Main ---
  const int BADGE_W               = 38;                                   // Badges width
  const int BADGE_H               = 20;                                   // Badges height
  const int BADGE_SPACING         = 2;                                    // Spacing between badges
  const int BADGE_GROUP_W         = (BADGE_W * 2) + BADGE_SPACING;        // Total width of the group of badges (WiFi + MQTT + spacing)
  const int BADGE_WIFI_X          = (SCREEN_W - BADGE_GROUP_W) / 2;       // X coordinate to center the group of badges
  const int BADGE_MQTT_X          = BADGE_WIFI_X + BADGE_W + BADGE_SPACING;    // X coordinate for the MQTT badge
  const int BADGE_Y               = SCREEN_H - BADGE_H - 5;               // Y coordinate for the badges (aligned to the bottom with a small margin)
  const int GRAPH_Y               = 75;                                   // Y coordinate for the top of the power trend graph
  const int GRAPH_H               = 54;                                   // Height of the power trend graph area (from GRAPH_Y to just above the badges)
  const int GRAPH_BOTTOM          = GRAPH_Y + GRAPH_H;                    // Y coordinate for the bottom of the graph area (used for scaling)

  // --- Page 4: Buzzer ---
  const int BTN_MODE_W            = 70;                                   // Button width
  const int BTN_MODE_H            = 28;                                   // Button height
  const int BTN_MODE_X            = (SCREEN_W - BTN_MODE_W) / 2;          // X coordinate to center the button
  const int BTN_MODE_Y[3]         = {50, 86, 122};                        // Y coordinates for the three buzzer mode badges (evenly spaced vertically)

  // --- Page 5: Save Settings ---
  const int ICON_W                = 16;                                   // Icon width
  const int ICON_H                = 16;                                   // Icon height
  const int ICON_X                = 4;                                    // X coordinate for the icons in the save settings page (left margin)
  const int TEXT_X                = ICON_X + ICON_W + 4;                  // X coordinate for the text in the save settings page
  const int ROW_Y[3]              = {60, 85, 110};                        // Y coordinates for 3 rows (Power Threshold, Buzzer Mode, Spare)

  // --- Page 6: Info ---
  const int INFO_VIEW_W           = SCREEN_W - 10;                        // Width of the view area for scrolling text

  // --- Page 7: System Actions ---
  const int BTN_SYS_W             = 78;                                   // Button width for system actions
  const int BTN_SYS_H             = 32;                                   // Button height for system actions
  const int BTN_SYS_X             = (SCREEN_W - BTN_SYS_W) / 2;           // X coordinate to center the system action buttons
  const int BAR_X                 = 1;                                    // X coordinate for the progress bar
  const int BAR_W                 = SCREEN_W - 1;                         // Width of the progress bar
  const int BAR_H                 = BTN_SYS_H;                            // Height of the progress bar (same as button height for visual integration)
  const int BAR_Y                 = 125;                                  // Y coordinate for the progress bar (aligned with the system action buttons)
}

#endif // CONFIG_H
