#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

// ###########################################################################################################################
// Header for the display manager.
// Defines function prototypes for display control.
//
// Author: CtrlAltJon
// Last Updated: January 2026
// Copyright (c) 2026 CtrlAltJon
// License: MIT
// The software is provided "as is", without warranty of any kind.
// This license notice must be included in any copy or portion of the software.
//
// ###########################################################################################################################

#include <Arduino.h>                                       // Main Arduino Library

enum BootStatus {                                          // Enumeration for boot status icons
  BOOT_OK,                                                 // Indicates a successful boot step
  BOOT_WARN,                                               // Indicates a warning during a boot step
  BOOT_FAIL                                                // Indicates a failure during a boot step
};

void initDisplay();                                        // Initializes the display hardware and settings
void updateDisplay();                                      // Refreshes the display with the latest information
void initBootScreen();                                     // Sets up the display for showing boot messages
void addBootMessage(const char* text, BootStatus status);  // Adds a boot message with a status icon to the display

#endif
