#ifndef LANGUAGES_H
#define LANGUAGES_H

// ###########################################################################################################################
// Header file for language management.
// Defines the Language enum, MsgId enum, and function prototypes for text retrieval.
//
// Author: CtrlAltJon
// Last Updated: January 2026
// Copyright (c) 2026 CtrlAltJon
// License: MIT
// The software is provided "as is", without warranty of any kind.
// This license notice must be included in any copy or portion of the software.
//
// ###########################################################################################################################

#include <Arduino.h>                                                      // Arduino library

enum Language {                                                           // Languages
  LANG_IT,                                                                // Italian
  LANG_EN,                                                                // English
  LANG_COUNT                                                              // Counter for selecting languages
};

extern Language currentLanguage;                                          // Current Language

void setLanguage(Language lang);                                          // Set language

enum MsgId {                                                              // Message IDs
    // OLED Display
  ID_DISPLAY_WIFI,                                                        // WiFi
  ID_DISPLAY_MQTT,                                                        // MQTT
  ID_DISPLAY_THRESHOLD,                                                   // Threshold
  ID_DISPLAY_SAVED,                                                       // Saved
  ID_DISPLAY_ERROR,                                                       // Error
  ID_DISPLAY_SAVE_SETTINGS_1,                                             // Save settings Line 1
  ID_DISPLAY_SAVE_SETTINGS_2,                                             // Save settings Line 2
  ID_DISPLAY_BUZZER_MODE_1,                                               // Buzzer mode Line 1
  ID_DISPLAY_BUZZER_MODE_2,                                               // Buzzer mode Line 2
    // Page 6 (System) Descriptions
  ID_DISPLAY_AP_ON,                                                       // AP ON message
  ID_DESC_AP_1,                                                           // AP description Line 1
  ID_DESC_AP_2,                                                           // AP description Line 2
  ID_DESC_REBOOT_1,                                                       // Reboot description Line 1
  ID_DESC_REBOOT_2,                                                       // Reboot description Line 2
  ID_DESC_RESET_1,                                                        // Reset description Line 1
  ID_DESC_RESET_2,                                                        // Reset description Line 2
    // Web Interface - Root page
  ID_WEB_TITLE,                                                           // Web title
  ID_WEB_STATUS,                                                          // Web status
  ID_WEB_POWER,                                                           // Web power
  ID_WEB_VOLTAGE,                                                         // Web grid voltage label
  ID_WEB_CURRENT,                                                         // Web current
  ID_GEN_TEMPERATURE,                                                     // Web Temperature
  ID_WEB_UPTIME,                                                          // Web uptime
  ID_WEB_BTN_CONFIG,                                                      // Web button config
  ID_WEB_BTN_REBOOT,                                                      // Web button reboot
  ID_WEB_LINK_REFRESH,                                                    // Web link refresh
  ID_WEB_LINK_UPDATE,                                                     // Web link update
    // Web Interface - Config page
  ID_CFG_TITLE,                                                           // Web config title
  ID_CFG_HEADER,                                                          // Web config header
  ID_CFG_GEN_SETTINGS,                                                    // Web config general settings
  ID_CFG_DEV_NAME,                                                        // Web config device name
  ID_CFG_MANUFACTURER,                                                    // Web config manufacturer
  ID_CFG_LANGUAGE,                                                        // Web config language
  ID_CFG_BUZZER_MODE,                                                     // Web config buzzer mode
  ID_CFG_THRESHOLD,                                                       // Web config threshold
  ID_CFG_WIFI_SETTINGS,                                                   // Web config WiFi settings
  ID_CFG_SSID,                                                            // Web config SSID
  ID_CFG_PASSWORD,                                                        // Web config password
  ID_CFG_STATIC_IP_CHECK,                                                 // Web config static IP check
  ID_CFG_IP_ADDR,                                                         // Web config IP address
  ID_CFG_SUBNET,                                                          // Web config subnet
  ID_CFG_GATEWAY,                                                         // Web config gateway
  ID_CFG_DNS,                                                             // Web config DNS
  ID_CFG_MQTT_SETTINGS,                                                   // Web config MQTT settings
  ID_CFG_BROKER_IP,                                                       // Web config MQTT broker IP
  ID_CFG_PORT,                                                            // Web config MQTT port
  ID_CFG_USER,                                                            // Web config MQTT user
  ID_CFG_PASS,                                                            // Web config MQTT password
  ID_CFG_HA_SETTINGS,                                                     // Web config HA settings
  ID_CFG_HA_PREFIX,                                                       // Web config HA prefix
  ID_CFG_CALIB_SETTINGS,                                                  // Web config calibration section
  ID_CFG_CALIB_HINT,                                                      // Web config calibration hint
  ID_CFG_CAL_OFFSET_V,                                                    // Web config voltage offset
  ID_CFG_CAL_SCALE_V,                                                     // Web config voltage scale
  ID_CFG_CAL_OFFSET_A,                                                    // Web config current offset
  ID_CFG_CAL_SCALE_A,                                                     // Web config current scale
  ID_CFG_CAL_OFFSET_TEMP,                                                 // Web config temperature offset
  ID_CFG_KEEP_CURRENT_PLACEHOLDER,                                        // Web config placeholder for keeping current password
  ID_CFG_BTN_SAVE,                                                        // Web config save button
  ID_CFG_BTN_BACK,                                                        // Web config back button
    // Web Interface - Save/Error
  ID_SAVE_OK_TITLE,                                                       // Web save ok title
  ID_SAVE_ERR_TITLE,                                                      // Web save error title
  ID_SAVE_REBOOTING,                                                      // Web save rebooting
    // Language Names
  ID_LANG_IT,                                                             // Italian
  ID_LANG_EN,                                                             // English
    // Buzzer Modes
  ID_BUZZER_MODE_CONTINUOUS,                                              // Continuous
  ID_BUZZER_MODE_PULSE_SLOW,                                              // Slow pulse
  ID_BUZZER_MODE_PULSE_FAST,                                              // Fast pulse
    // Web Access Settings
  ID_WEB_ACCESS_SETTINGS,                                                 // Web access header
  ID_WEB_USERNAME,                                                        // Username label
  ID_WEB_PASSWORD,                                                        // Password label
    // Language counter
  MSG_COUNT                                                               // Language counter in enum
};

const __FlashStringHelper* getText(MsgId id);                             // Get text message by ID (Standard Arduino Flash String)

// Display LCD
#define MSG_DISPLAY_WIFI            getText(ID_DISPLAY_WIFI)              // WiFi
#define MSG_DISPLAY_MQTT            getText(ID_DISPLAY_MQTT)              // MQTT
#define MSG_DISPLAY_THRESHOLD       getText(ID_DISPLAY_THRESHOLD)         // Threshold
#define MSG_DISPLAY_SAVED           getText(ID_DISPLAY_SAVED)             // Saved
#define MSG_DISPLAY_ERROR           getText(ID_DISPLAY_ERROR)             // Error
#define MSG_DISPLAY_SAVE_SETTINGS_1 getText(ID_DISPLAY_SAVE_SETTINGS_1)   // Save settings Line 1
#define MSG_DISPLAY_SAVE_SETTINGS_2 getText(ID_DISPLAY_SAVE_SETTINGS_2)   // Save settings Line 2
#define MSG_DISPLAY_BUZZER_MODE_1   getText(ID_DISPLAY_BUZZER_MODE_1)     // Buzzer mode Line 1
#define MSG_DISPLAY_BUZZER_MODE_2   getText(ID_DISPLAY_BUZZER_MODE_2)     // Buzzer mode Line 2
#define MSG_DISPLAY_AP_ON           getText(ID_DISPLAY_AP_ON)             // AP ON message
#define MSG_DESC_AP_1               getText(ID_DESC_AP_1)                 // AP description Line 1
#define MSG_DESC_AP_2               getText(ID_DESC_AP_2)                 // AP description Line 2
#define MSG_DESC_REBOOT_1           getText(ID_DESC_REBOOT_1)             // Reboot description Line 1
#define MSG_DESC_REBOOT_2           getText(ID_DESC_REBOOT_2)             // Reboot description Line 2
#define MSG_DESC_RESET_1            getText(ID_DESC_RESET_1)              // Reset description Line 1
#define MSG_DESC_RESET_2            getText(ID_DESC_RESET_2)              // Reset description Line 2
// Web Root
#define MSG_WEB_TITLE               getText(ID_WEB_TITLE)                 // Web title
#define MSG_WEB_STATUS              getText(ID_WEB_STATUS)                // Web status
#define MSG_WEB_POWER               getText(ID_WEB_POWER)                 // Web power
#define MSG_WEB_VOLTAGE             getText(ID_WEB_VOLTAGE)               // Web grid voltage
#define MSG_WEB_CURRENT             getText(ID_WEB_CURRENT)               // Web current
#define MSG_WEB_TEMPERATURE         getText(ID_GEN_TEMPERATURE)           // Temperature
#define MSG_WEB_UPTIME              getText(ID_WEB_UPTIME)                // Web uptime
#define MSG_WEB_BTN_CONFIG          getText(ID_WEB_BTN_CONFIG)            // Web button config
#define MSG_WEB_BTN_REBOOT          getText(ID_WEB_BTN_REBOOT)            // Web button reboot
#define MSG_WEB_LINK_REFRESH        getText(ID_WEB_LINK_REFRESH)          // Web link refresh
#define MSG_WEB_LINK_UPDATE         getText(ID_WEB_LINK_UPDATE)           // Web link update
// Web Config
#define MSG_CFG_TITLE               getText(ID_CFG_TITLE)                 // Web config title
#define MSG_CFG_HEADER              getText(ID_CFG_HEADER)                // Web config header
#define MSG_CFG_GEN_SETTINGS        getText(ID_CFG_GEN_SETTINGS)          // Web config general settings
#define MSG_CFG_DEV_NAME            getText(ID_CFG_DEV_NAME)              // Web config device name
#define MSG_CFG_MANUFACTURER        getText(ID_CFG_MANUFACTURER)          // Web config manufacturer
#define MSG_CFG_LANGUAGE            getText(ID_CFG_LANGUAGE)              // Web config language
#define MSG_CFG_BUZZER_MODE         getText(ID_CFG_BUZZER_MODE)           // Web config buzzer mode
#define MSG_CFG_THRESHOLD           getText(ID_CFG_THRESHOLD)             // Web config threshold
#define MSG_CFG_WIFI_SETTINGS       getText(ID_CFG_WIFI_SETTINGS)         // Web config WiFi settings
#define MSG_CFG_SSID                getText(ID_CFG_SSID)                  // Web config SSID
#define MSG_CFG_PASSWORD            getText(ID_CFG_PASSWORD)              // Web config password
#define MSG_CFG_STATIC_IP_CHECK     getText(ID_CFG_STATIC_IP_CHECK)       // Web config static IP check
#define MSG_CFG_IP_ADDR             getText(ID_CFG_IP_ADDR)               // Web config IP address
#define MSG_CFG_SUBNET              getText(ID_CFG_SUBNET)                // Web config subnet
#define MSG_CFG_GATEWAY             getText(ID_CFG_GATEWAY)               // Web config gateway
#define MSG_CFG_DNS                 getText(ID_CFG_DNS)                   // Web config DNS
#define MSG_CFG_MQTT_SETTINGS       getText(ID_CFG_MQTT_SETTINGS)         // Web config MQTT settings
#define MSG_CFG_BROKER_IP           getText(ID_CFG_BROKER_IP)             // Web config MQTT broker IP
#define MSG_CFG_PORT                getText(ID_CFG_PORT)                  // Web config MQTT port
#define MSG_CFG_USER                getText(ID_CFG_USER)                  // Web config MQTT user
#define MSG_CFG_PASS                getText(ID_CFG_PASS)                  // Web config MQTT password
#define MSG_CFG_HA_SETTINGS         getText(ID_CFG_HA_SETTINGS)           // Web config HA settings
#define MSG_CFG_HA_PREFIX           getText(ID_CFG_HA_PREFIX)             // Web config HA prefix
#define MSG_CFG_CALIB_SETTINGS      getText(ID_CFG_CALIB_SETTINGS)        // Web config calibration section
#define MSG_CFG_CALIB_HINT          getText(ID_CFG_CALIB_HINT)            // Web config calibration hint
#define MSG_CFG_CAL_OFFSET_V        getText(ID_CFG_CAL_OFFSET_V)          // Web config voltage offset
#define MSG_CFG_CAL_SCALE_V         getText(ID_CFG_CAL_SCALE_V)           // Web config voltage scale
#define MSG_CFG_CAL_OFFSET_A        getText(ID_CFG_CAL_OFFSET_A)          // Web config current offset
#define MSG_CFG_CAL_SCALE_A         getText(ID_CFG_CAL_SCALE_A)           // Web config current scale
#define MSG_CFG_CAL_OFFSET_TEMP     getText(ID_CFG_CAL_OFFSET_TEMP)       // Web config temperature offset
#define MSG_CFG_KEEP_CURRENT_PLACEHOLDER getText(ID_CFG_KEEP_CURRENT_PLACEHOLDER) // Keep current password placeholder
#define MSG_CFG_BTN_SAVE            getText(ID_CFG_BTN_SAVE)              // Web config save button
#define MSG_CFG_BTN_BACK            getText(ID_CFG_BTN_BACK)              // Web config back button
// Save/Error
#define MSG_SAVE_OK_TITLE           getText(ID_SAVE_OK_TITLE)             // Web save ok title
#define MSG_SAVE_ERR_TITLE          getText(ID_SAVE_ERR_TITLE)            // Web save error title
#define MSG_SAVE_REBOOTING          getText(ID_SAVE_REBOOTING)            // Web save rebooting
// Language Names
#define MSG_LANG_IT                 getText(ID_LANG_IT)                   // Italian
#define MSG_LANG_EN                 getText(ID_LANG_EN)                   // English
// Buzzer Modes
#define MSG_BUZZER_MODE_CONTINUOUS  getText(ID_BUZZER_MODE_CONTINUOUS)    // Continuous
#define MSG_BUZZER_MODE_PULSE_SLOW  getText(ID_BUZZER_MODE_PULSE_SLOW)    // Slow pulse
#define MSG_BUZZER_MODE_PULSE_FAST  getText(ID_BUZZER_MODE_PULSE_FAST)    // Fast pulse
// Web Access Settings
#define MSG_WEB_ACCESS_SETTINGS     getText(ID_WEB_ACCESS_SETTINGS)       // Web access header
#define MSG_WEB_USERNAME            getText(ID_WEB_USERNAME)              // Username label
#define MSG_WEB_PASSWORD            getText(ID_WEB_PASSWORD)              // Password label


#endif