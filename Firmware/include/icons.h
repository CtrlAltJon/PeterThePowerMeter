#ifndef ICONS_H
#define ICONS_H

// ###########################################################################################################################
// Header for icons definitions.
// Defines bitmap data for various icons used in the display.
//
// Author: CtrlAltJon
// Last Updated: June 2026
// Copyright (c) 2026 CtrlAltJon
// License: MIT
// The software is provided "as is", without warranty of any kind.
// This license notice must be included in any copy or portion of the software.
//
// ###########################################################################################################################

#include <Arduino.h>
#include "config.h"
 
// Macro to simplify drawing bitmaps (K=Black, W=White, G=Green, R=Grey, etc.)
// They refer to the colors centralized in layout.h
#define K Layout::COL_BLACK
#define W Layout::COL_WHITE
#define R Layout::COL_BG_BADGE_OFF
#define E Layout::COL_RED
#define Y Layout::COL_YELLOW
#define C Layout::COL_CYAN
#define B Layout::COL_BLUE
#define G Layout::COL_GREEN

// Badge WiFi ON (Green) and connected to network
static const uint16_t badge_wifi_ok_38x20[760] PROGMEM = {
  K,K,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,K,K,
  K,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,K,
  G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,
  G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,
  G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,
  G,G,G,G,G,W,W,G,G,G,G,W,W,G,G,W,W,G,G,G,G,G,G,G,W,W,W,W,W,G,G,W,W,G,G,G,G,G,
  G,G,G,G,G,W,W,G,G,G,G,W,W,G,G,W,W,G,G,G,G,G,G,G,W,W,W,W,W,G,G,W,W,G,G,G,G,G,
  G,G,G,G,G,W,W,G,G,G,G,W,W,G,G,G,G,G,G,G,G,G,G,G,W,W,G,G,G,G,G,G,G,G,G,G,G,G,
  G,G,G,G,G,W,W,G,G,G,G,W,W,G,G,W,W,G,G,G,G,G,G,G,W,W,W,W,G,G,G,W,W,G,G,G,G,G,
  G,G,G,G,G,W,W,G,W,W,G,W,W,G,G,W,W,G,G,W,W,W,W,G,W,W,W,W,G,G,G,W,W,G,G,G,G,G,
  G,G,G,G,G,W,W,G,W,W,G,W,W,G,G,W,W,G,G,W,W,W,W,G,W,W,G,G,G,G,G,W,W,G,G,G,G,G,
  G,G,G,G,G,G,W,W,W,W,W,W,G,G,G,W,W,G,G,G,G,G,G,G,W,W,G,G,G,G,G,W,W,G,G,G,G,G,
  G,G,G,G,G,G,W,W,G,G,W,W,G,G,G,W,W,G,G,G,G,G,G,G,W,W,G,G,G,G,G,W,W,G,G,G,G,G,
  G,G,G,G,G,G,W,W,G,G,W,W,G,G,G,W,W,G,G,G,G,G,G,G,W,W,G,G,G,G,G,W,W,G,G,G,G,G,
  G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,
  G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,
  G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,
  G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,
  K,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,K,
  K,K,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,K,K,
};

// Badge WiFi OFF (Dark Grey) or not connected to network
static const uint16_t badge_wifi_off_38x20[760] PROGMEM = {
  K,K,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,K,K,
  K,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,K,
  R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,
  R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,
  R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,
  R,R,R,R,R,W,W,R,R,R,R,W,W,R,R,W,W,R,R,R,R,R,R,R,W,W,W,W,W,R,R,W,W,R,R,R,R,R,
  R,R,R,R,R,W,W,R,R,R,R,W,W,R,R,W,W,R,R,R,R,R,R,R,W,W,W,W,W,R,R,W,W,R,R,R,R,R,
  R,R,R,R,R,W,W,R,R,R,R,W,W,R,R,R,R,R,R,R,R,R,R,R,W,W,R,R,R,R,R,R,R,R,R,R,R,R,
  R,R,R,R,R,W,W,R,R,R,R,W,W,R,R,W,W,R,R,R,R,R,R,R,W,W,W,W,R,R,R,W,W,R,R,R,R,R,
  R,R,R,R,R,W,W,R,W,W,R,W,W,R,R,W,W,R,R,W,W,W,W,R,W,W,W,W,R,R,R,W,W,R,R,R,R,R,
  R,R,R,R,R,W,W,R,W,W,R,W,W,R,R,W,W,R,R,W,W,W,W,R,W,W,R,R,R,R,R,W,W,R,R,R,R,R,
  R,R,R,R,R,R,W,W,W,W,W,W,W,R,R,W,W,R,R,R,R,R,R,R,W,W,R,R,R,R,R,W,W,R,R,R,R,R,
  R,R,R,R,R,R,W,W,R,R,W,W,R,R,R,W,W,R,R,R,R,R,R,R,W,W,R,R,R,R,R,W,W,R,R,R,R,R,
  R,R,R,R,R,R,W,W,R,R,W,W,R,R,R,W,W,R,R,R,R,R,R,R,W,W,R,R,R,R,R,W,W,R,R,R,R,R,
  R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,
  R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,
  R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,
  R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,
  K,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,K,
  K,K,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,K,K,
};

// Badge AP mode ON (Blue) and no client connected
static const uint16_t badge_wifi_ap_38x20[760] PROGMEM = {
  K,K,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,K,K,
  K,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,K,
  B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,
  B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,
  B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,
  B,B,B,B,B,B,B,B,B,B,B,W,W,W,W,B,B,B,B,B,B,W,W,W,W,W,W,B,B,B,B,B,B,B,B,B,B,B,
  B,B,B,B,B,B,B,B,B,B,B,W,W,W,W,B,B,B,B,B,B,W,W,W,W,W,W,B,B,B,B,B,B,B,B,B,B,B,
  B,B,B,B,B,B,B,B,B,B,W,W,B,B,W,W,B,B,B,B,B,W,W,B,B,B,B,W,W,B,B,B,B,B,B,B,B,B,
  B,B,B,B,B,B,B,B,B,B,W,W,B,B,W,W,B,B,B,B,B,W,W,B,B,B,B,W,W,B,B,B,B,B,B,B,B,B,
  B,B,B,B,B,B,B,B,B,B,W,W,W,W,W,W,B,B,B,B,B,W,W,W,W,W,W,B,B,B,B,B,B,B,B,B,B,B,
  B,B,B,B,B,B,B,B,B,B,W,W,W,W,W,W,B,B,B,B,B,W,W,W,W,W,W,B,B,B,B,B,B,B,B,B,B,B,
  B,B,B,B,B,B,B,B,B,W,W,B,B,B,B,W,W,B,B,B,B,W,W,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,
  B,B,B,B,B,B,B,B,B,W,W,B,B,B,B,W,W,B,B,B,B,W,W,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,
  B,B,B,B,B,B,B,B,B,W,W,B,B,B,B,W,W,B,B,B,B,W,W,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,
  B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,
  B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,
  B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,
  B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,
  K,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,K,
  K,K,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,K,K,
};

// Badge AP mode ON (Green) and client connected
static const uint16_t badge_wifi_ap_green_38x20[760] PROGMEM = {
  K,K,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,K,K,
  K,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,K,
  G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,
  G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,
  G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,
  G,G,G,G,G,G,G,G,G,G,G,W,W,W,W,G,G,G,G,G,G,W,W,W,W,W,W,G,G,G,G,G,G,G,G,G,G,G,
  G,G,G,G,G,G,G,G,G,G,G,W,W,W,W,G,G,G,G,G,G,W,W,W,W,W,W,G,G,G,G,G,G,G,G,G,G,G,
  G,G,G,G,G,G,G,G,G,G,W,W,G,G,W,W,G,G,G,G,G,W,W,G,G,G,G,W,W,G,G,G,G,G,G,G,G,G,
  G,G,G,G,G,G,G,G,G,G,W,W,G,G,W,W,G,G,G,G,G,W,W,G,G,G,G,W,W,G,G,G,G,G,G,G,G,G,
  G,G,G,G,G,G,G,G,G,G,W,W,W,W,W,W,G,G,G,G,G,W,W,W,W,W,W,G,G,G,G,G,G,G,G,G,G,G,
  G,G,G,G,G,G,G,G,G,G,W,W,W,W,W,W,G,G,G,G,G,W,W,W,W,W,W,G,G,G,G,G,G,G,G,G,G,G,
  G,G,G,G,G,G,G,G,G,W,W,G,G,G,G,W,W,G,G,G,G,W,W,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,
  G,G,G,G,G,G,G,G,G,W,W,G,G,G,G,W,W,G,G,G,G,W,W,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,
  G,G,G,G,G,G,G,G,G,W,W,G,G,G,G,W,W,G,G,G,G,W,W,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,
  G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,
  G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,
  G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,
  G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,
  K,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,K,
  K,K,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,K,K,
};

// Badge MQTT ON (Green) (connected to broker)
static const uint16_t badge_mqtt_ok_38x20[760] PROGMEM = {
  K,K,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,K,K,
  K,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,K,
  G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,
  G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,
  G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,
  G,G,G,G,G,G,W,W,G,G,G,W,W,G,G,W,W,W,W,G,G,W,W,W,W,W,W,G,W,W,W,W,W,W,G,G,G,G,
  G,G,G,G,G,G,W,W,W,G,W,W,W,G,W,W,G,G,W,W,G,W,W,W,W,W,W,G,W,W,W,W,W,W,G,G,G,G,
  G,G,G,G,G,G,W,W,W,G,W,W,W,G,W,W,G,G,W,W,G,G,G,W,W,G,G,G,G,G,W,W,G,G,G,G,G,G,
  G,G,G,G,G,G,W,W,G,W,G,W,W,G,W,W,G,G,W,W,G,G,G,W,W,G,G,G,G,G,W,W,G,G,G,G,G,G,
  G,G,G,G,G,G,W,W,G,W,G,W,W,G,W,W,G,G,W,W,G,G,G,W,W,G,G,G,G,G,W,W,G,G,G,G,G,G,
  G,G,G,G,G,G,W,W,G,G,G,W,W,G,W,W,G,G,W,W,G,G,G,W,W,G,G,G,G,G,W,W,G,G,G,G,G,G,
  G,G,G,G,G,G,W,W,G,G,G,W,W,G,W,W,G,W,W,W,G,G,G,W,W,G,G,G,G,G,W,W,G,G,G,G,G,G,
  G,G,G,G,G,G,W,W,G,G,G,W,W,G,G,W,W,W,W,G,G,G,G,W,W,G,G,G,G,G,W,W,G,G,G,G,G,G,
  G,G,G,G,G,G,W,W,G,G,G,W,W,G,G,G,G,G,W,W,G,G,G,W,W,G,G,G,G,G,W,W,G,G,G,G,G,G,
  G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,
  G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,
  G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,
  G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,
  K,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,K,
  K,K,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,K,K,
};

// Badge MQTT OFF (Dark Grey) (not connected to broker)
static const uint16_t badge_mqtt_off_38x20[760] PROGMEM = {
  K,K,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,K,K,
  K,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,K,
  R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,
  R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,
  R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,
  R,R,R,R,R,R,W,W,R,R,R,W,W,R,R,W,W,W,W,R,R,W,W,W,W,W,W,R,W,W,W,W,W,W,R,R,R,R,
  R,R,R,R,R,R,W,W,W,R,W,W,W,R,W,W,R,R,W,W,R,W,W,W,W,W,W,R,W,W,W,W,W,W,R,R,R,R,
  R,R,R,R,R,R,W,W,W,R,W,W,W,R,W,W,R,R,W,W,R,R,R,W,W,R,R,R,R,R,W,W,R,R,R,R,R,R,
  R,R,R,R,R,R,W,W,R,W,R,W,W,R,W,W,R,R,W,W,R,R,R,W,W,R,R,R,R,R,W,W,R,R,R,R,R,R,
  R,R,R,R,R,R,W,W,R,W,R,W,W,R,W,W,R,R,W,W,R,R,R,W,W,R,R,R,R,R,W,W,R,R,R,R,R,R,
  R,R,R,R,R,R,W,W,R,R,R,W,W,R,W,W,R,R,W,W,R,R,R,W,W,R,R,R,R,R,W,W,R,R,R,R,R,R,
  R,R,R,R,R,R,W,W,R,R,R,W,W,R,W,W,R,W,W,W,R,R,R,W,W,R,R,R,R,R,W,W,R,R,R,R,R,R,
  R,R,R,R,R,R,W,W,R,R,R,W,W,R,R,W,W,W,W,R,R,R,R,W,W,R,R,R,R,R,W,W,R,R,R,R,R,R,
  R,R,R,R,R,R,W,W,R,R,R,W,W,R,R,R,R,R,W,W,R,R,R,W,W,R,R,R,R,R,W,W,R,R,R,R,R,R,
  R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,
  R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,
  R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,
  R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,
  K,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,K,
  K,K,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,K,K,
};

// Boot status icon OK (16x16 pixel)
static const uint16_t icon_check_16x16[256] PROGMEM = {
  K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,
  K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,
  K,K,K,K,K,K,K,K,K,K,K,K,G,G,K,K,
  K,K,K,K,K,K,K,K,K,K,K,G,G,G,K,K,
  K,K,K,K,K,K,K,K,K,K,G,G,G,K,K,K,
  K,G,G,K,K,K,K,K,K,G,G,G,K,K,K,K,
  K,G,G,K,K,K,K,K,G,G,G,K,K,K,K,K,
  K,K,G,G,K,K,K,G,G,G,K,K,K,K,K,K,
  K,K,G,G,K,K,G,G,G,K,K,K,K,K,K,K,
  K,K,K,G,G,G,G,G,K,K,K,K,K,K,K,K,
  K,K,K,G,G,G,G,K,K,K,K,K,K,K,K,K,
  K,K,K,K,G,G,K,K,K,K,K,K,K,K,K,K,
  K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,
  K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,
  K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,
  K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,
};

// Boot status icon ERROR (16x16 pixel)
static const uint16_t icon_error_16x16[256] PROGMEM = {
  K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,
  K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,
  K,K,E,E,K,K,K,K,K,K,K,K,E,E,K,K,
  K,K,K,E,E,K,K,K,K,K,K,E,E,K,K,K,
  K,K,K,K,E,E,K,K,K,K,E,E,K,K,K,K,
  K,K,K,K,K,E,E,K,K,E,E,K,K,K,K,K,
  K,K,K,K,K,K,E,E,E,E,K,K,K,K,K,K,
  K,K,K,K,K,K,K,E,E,K,K,K,K,K,K,K,
  K,K,K,K,K,K,E,E,E,E,K,K,K,K,K,K,
  K,K,K,K,K,E,E,K,K,E,E,K,K,K,K,K,
  K,K,K,K,E,E,K,K,K,K,E,E,K,K,K,K,
  K,K,K,E,E,K,K,K,K,K,K,E,E,K,K,K,
  K,K,E,E,K,K,K,K,K,K,K,K,E,E,K,K,
  K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,
  K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,
  K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,
};

// Boot status icon WARNING (16x16 pixel)
static const uint16_t icon_warning_16x16[256] PROGMEM = {
  K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,
  K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,
  K,K,K,K,K,Y,Y,Y,Y,Y,K,K,K,K,K,K,
  K,K,K,K,K,K,Y,Y,Y,K,K,K,K,K,K,K,
  K,K,K,K,K,K,Y,Y,Y,K,K,K,K,K,K,K,
  K,K,K,K,K,K,Y,Y,Y,K,K,K,K,K,K,K,
  K,K,K,K,K,K,Y,Y,Y,K,K,K,K,K,K,K,
  K,K,K,K,K,K,K,Y,K,K,K,K,K,K,K,K,
  K,K,K,K,K,K,K,Y,K,K,K,K,K,K,K,K,
  K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,
  K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,
  K,K,K,K,K,K,K,Y,K,K,K,K,K,K,K,K,
  K,K,K,K,K,K,Y,Y,Y,K,K,K,K,K,K,K,
  K,K,K,K,K,K,K,Y,K,K,K,K,K,K,K,K,
  K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,
  K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,
};

// Power icon (16x16 pixel)
static const uint16_t icon_power_16x16[256] PROGMEM = {
  K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,
  K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,
  K,K,K,K,K,K,K,K,K,K,E,E,E,K,K,K,
  K,K,K,K,K,K,K,K,K,K,E,E,E,K,K,K,
  K,K,K,K,K,K,K,K,K,K,E,E,E,K,K,K,
  K,K,K,K,K,K,K,K,K,K,E,E,E,K,K,K,
  K,K,K,K,K,K,K,K,K,K,E,E,E,K,K,K,
  K,K,E,E,E,K,K,K,K,K,E,E,E,K,K,K,
  K,K,E,E,E,K,K,K,K,K,E,E,E,K,K,K,
  K,K,E,E,E,K,K,K,K,K,E,E,E,K,K,K,
  K,K,E,E,E,K,E,E,E,K,E,E,E,K,K,K,
  K,K,E,E,E,K,E,E,E,K,E,E,E,K,K,K,
  K,K,E,E,E,K,E,E,E,K,E,E,E,K,K,K,
  K,K,E,E,E,K,E,E,E,K,E,E,E,K,K,K,
  K,K,E,E,E,K,E,E,E,K,E,E,E,K,K,K,
  K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,
};

// Voltage icon (16x16 pixel)
static const uint16_t icon_voltage_16x16[256] PROGMEM = {
  K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,
  K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,
  K,K,K,K,K,K,K,K,K,K,Y,Y,Y,Y,Y,K,
  K,K,K,K,K,K,K,K,K,Y,Y,Y,Y,Y,K,K,
  K,K,K,K,K,K,K,K,Y,Y,Y,Y,K,K,K,K,
  K,K,K,K,K,K,K,Y,Y,Y,Y,K,K,K,K,K,
  K,K,K,K,K,K,Y,Y,Y,K,K,K,K,K,K,K,
  K,K,K,K,K,Y,Y,Y,Y,Y,Y,Y,Y,K,K,K,
  K,K,K,K,K,K,K,K,Y,Y,Y,Y,K,K,K,K,
  K,K,K,K,K,K,K,Y,Y,Y,Y,K,K,K,K,K,
  K,K,K,K,K,K,Y,Y,Y,Y,K,K,K,K,K,K,
  K,K,K,K,K,Y,Y,Y,Y,K,K,K,K,K,K,K,
  K,K,K,K,Y,Y,Y,K,K,K,K,K,K,K,K,K,
  K,K,K,Y,Y,K,K,K,K,K,K,K,K,K,K,K,
  K,K,Y,Y,K,K,K,K,K,K,K,K,K,K,K,K,
  K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,
};

// Speaker icon (16x16 pixel)
static const uint16_t icon_speaker_16x16[256] PROGMEM = {
  K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,
  K,K,K,K,K,K,K,K,K,K,K,C,C,K,K,K,
  K,K,K,K,K,K,K,K,K,K,C,C,C,K,K,K,
  K,K,K,K,K,K,K,K,K,C,C,C,C,C,K,K,
  K,K,K,K,K,K,K,C,C,C,C,C,C,C,K,K,
  K,K,C,C,C,K,C,C,C,C,C,C,C,C,K,K,
  K,K,C,C,C,K,C,C,C,C,C,C,C,C,K,K,
  K,K,C,C,C,K,C,C,C,C,C,C,C,C,K,K,
  K,K,C,C,C,K,C,C,C,C,C,C,C,C,K,K,
  K,K,C,C,C,K,C,C,C,C,C,C,C,C,K,K,
  K,K,K,K,K,K,K,C,C,C,C,C,C,C,K,K,
  K,K,K,K,K,K,K,K,K,C,C,C,C,C,K,K,
  K,K,K,K,K,K,K,K,K,K,C,C,C,K,K,K,
  K,K,K,K,K,K,K,K,K,K,K,C,C,K,K,K,
  K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,
};

// Macro cleanup
#undef K
#undef W
#undef G
#undef R
#undef E
#undef Y
#undef C

#endif