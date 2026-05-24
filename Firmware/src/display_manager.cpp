// ###########################################################################################################################
// Manages the ST7735 TFT display.
// It handles initialization, page rendering, status updates, and visual feedback for the user interface.
//
// Author: CtrlAltJon
// Last Updated: January 2026
// Copyright (c) 2026 CtrlAltJon
// License: MIT
// The software is provided "as is", without warranty of any kind.
// This license notice must be included in any copy or portion of the software.
//
// ###########################################################################################################################

#include "display_manager.h"                                              // Header for display management functions
#include <TFT_eSPI.h>                                                     // TFT library for ST7735 display
#include "sys_globals.h"                                                  // Global variables and configuration across the application
#include "languages.h"                                                    // Language strings used across the application
#include "config.h"                                                       // Master Configuration and Layout
#include <ESP8266WiFi.h>                                                  // WiFi library for ESP8266
#include "icons.h"                                                        // Bitmap icons for display (WiFi, MQTT, Status, etc.)
#include <EEPROM.h>                                                       // EEPROM library for reading/writing configuration data

// Font available in TFT_eSPI (included in library):
// FreeMonoBold9pt7b
// FreeSans9pt7b
// FreeSerifItalic12pt7b
// FreeSansOblique24pt7b

// Images:
// For the conversion of images to byte arrays I used the online tool linked below.
// I selected the RGB565 format (16-bit color) for compatibility with the ST7735 display.
// http://www.rinkydinkelectronics.com/t_imageconverter565.php
//
// To calculate memory usage: Width * Height * 2 byte
// Example 32x32 px: 32 * 32 * 2 = 2048 byte (2 KB)
// Example full screen 80x160: 80 * 160 * 2 = 25600 byte (25 KB)

static TFT_eSPI tft = TFT_eSPI();                                         // Global instance of the TFT display object
static bool display_ok = false;                                           // Flag to indicate if the display is initialized and ready

// ---------------------------------------------------------------------------------------------------------------------------
// Draw a badge (rounded rectangle) for the buzzer mode selection, with different
// colors and internal graphics based on the mode and whether it's selected/saved:
// Dark Gray: Not selected, White: Selected and matches saved config, Orange: Selected but not yet saved.
static void drawBuzzerBadge(int index, int x, int y, int w, int h) {
// index = Badge type: 0=Continuous, 1=Pulse Slow, 2=Pulse Fast
// x and y = top-left coordinates; w and h = width and height
  uint16_t bgColor, lnColor;                                              // Background color and line color for the internal graphics
  int cy = y + (h / 2);                                                   // Central Y coordinate (used for drawing the internal graphics)
  int lx = x + 10;                                                        // Left X coordinate for the internal graphics (10px padding from the left edge)
  int lw = w - 20;                                                        // Width available for the internal graphics (padding of 10px on both sides)
  const int thickness = 3;                                                // Thickness of the lines
  if (index == sysConfig.buzzerMode) {                                    // Badge is selected (matches current config)
    if (sysConfig.buzzerMode == savedSysConfig.buzzerMode) {              // Selected and matches saved config: WHITE
      bgColor = Layout::COL_WHITE;
      lnColor = Layout::COL_BLACK;
    } else {                                                              // Selected but does not match saved config: ORANGE
      bgColor = Layout::COL_ORANGE;
      lnColor = Layout::COL_BLACK;
    }
  } else {                                                                // Badge is not selected: DARK GRAY
    bgColor = Layout::COL_BG_BADGE_OFF; 
    lnColor = Layout::COL_WHITE;
  }

  tft.fillRoundRect(x, y, w, h, 6, bgColor);                              // Draw the badge background (rounded rectangle)

  // Draw the internal graphics based on the mode
  if (index == 0) {                                                       // Continuous: 1 line full width
    tft.fillRect(lx, cy - 1, lw, thickness, lnColor);
  } else if (index == 1) {                                                // Slow Pulse: 2 segments with a gap in the middle
    int segLen = (lw - 10) / 2;                                           // Length of each segment (10px gap in the middle)
    tft.fillRect(lx, cy - 1, segLen, thickness, lnColor);                 // First segment
    tft.fillRect(lx + segLen + 10, cy - 1, segLen, thickness, lnColor);   // Second segment
  } else if (index == 2) {                                                // Fast Pulse: 4 segments with equal gaps
    int gap = 6;                                                          // Gap between segments
    int segLen = (lw - (gap * 3)) / 4;                                    // Length of each segment (4 segments, 3 gaps)
    for (int k = 0; k < 4; k++) {                                         // Loop to draw the 4 segments
      tft.fillRect(lx + (segLen + gap) * k, cy - 1, segLen, thickness, lnColor);
    }
  }
}

// ---------------------------------------------------------------------------------------------------------------------------
// Draw buttons for the system actions (AP mode, Reboot, Factory Reset) with different styles based on selection
static void drawSystemBtn(const char* label, int x, int y, int w, int h, uint16_t color, bool selected) {
// label = Button text; x and y = top-left coordinates; w and h = width and height
// color = base color for the button; selected = whether the button is currently selected (true) or not (false)
  if (selected) {                                                         // If selected, draw a filled button
    tft.fillRoundRect(x, y, w, h, 6, color);                              // Draw filled rounded rectangle
    tft.setTextColor(Layout::COL_BLACK, color);                           // Set text color
  } else {                                                                // If not selected, draw an outlined button
    tft.drawRoundRect(x, y, w, h, 6, color);                              // Draw rounded rectangle outline
    tft.setTextColor(color, Layout::COL_BLACK);                           // Set text color
  }
  tft.setFreeFont(&FreeSans9pt7b);                                        // Set a readable font for the button labels
  tft.setTextDatum(MC_DATUM);                                             // Set text datum to middle center for easy centering
  tft.drawString(label, x + (w / 2), y + (h / 2));                        // Draw the button label centered within the button
}

// ---------------------------------------------------------------------------------------------------------------------------
void initDisplay() {                                                      // Initialize the TFT display and set it up for use
  tft.init();                                                             // Initialize the display
  tft.setRotation(2);                                                     // 0=Portrait, 1=Landscape, 2=Portrait Inverted, 3=Landscape Inverted
  tft.fillScreen(Layout::COL_BLACK);                                      // Clear the screen with black background
  display_ok = true;                                                      // Set the flag to indicate the display is ready for use
  Serial.println(F("TFT ST7735 Initialized."));                           // Debug message to indicate successful initialization
}
// Y coordinate for the next boot message (used in initBootScreen and addBootMessage to manage vertical spacing of messages)
static int boot_msg_y = 0;

// ---------------------------------------------------------------------------------------------------------------------------
// Initialize the boot screen by clearing it and resetting the Y coordinate for messages
void initBootScreen() {
  if (!display_ok) return;                                                // If the display is not initialized, exit the function
  tft.fillScreen(Layout::COL_BLACK);                                      // Clear the screen with black background
  boot_msg_y = 5;                                                         // Start Y coordinate for the first boot message (with a small margin from the top)
}

// ---------------------------------------------------------------------------------------------------------------------------
// Adds a boot message line with a status icon to the display. It manages vertical spacing and pagination of messages.
void addBootMessage(const char* text, BootStatus status) {
// text = Message text to display; status = Status of the message (BOOT_OK, BOOT_WARN, BOOT_FAIL) which determines the icon to show
  if (!display_ok) return;                                                // If the display is not initialized, exit the function
  if (boot_msg_y > tft.height()) {                                        // If the messages exceed the screen height
    tft.fillScreen(Layout::COL_BLACK);                                    // Clear the screen
    boot_msg_y = 5;                                                       // Reset Y coordinate to start from the top again
  }
  tft.setTextDatum(TL_DATUM);                                             // Set text datum to top-left for message alignment
  tft.setTextFont(2);                                                     // Set a readable font for boot messages
  tft.setTextColor(Layout::COL_WHITE, Layout::COL_BLACK);                 // Set text color
  tft.drawString(text, 5, boot_msg_y);                                    // Draw the message text with a small margin from the left edge
  
  const int icon_w = 16, icon_h = 16;                                     // Define the width and height of the status icons
  int icon_x = tft.width() - icon_w - 5;                                  // X coordinate for the icon (right-aligned with a small margin)
  const uint16_t* icon_data = nullptr;                                    // Pointer to the icon bitmap data, initialized to nullptr
  if (status == BOOT_OK) icon_data = icon_check_16x16;                    // Select the appropriate icon based on the status
  else if (status == BOOT_WARN) icon_data = icon_warning_16x16;           
  else if (status == BOOT_FAIL) icon_data = icon_error_16x16;

  if (icon_data) {                                                        // If an icon is selected, draw it on the display
    tft.setSwapBytes(true);                                               // Set byte swapping for correct color rendering of the bitmap
    tft.pushImage(icon_x, boot_msg_y, icon_w, icon_h, icon_data);         // Draw the icon at the calculated position
    tft.setSwapBytes(false);                                              // Reset byte swapping to default for future drawing operations
  }
  boot_msg_y += 20;                                                       // Increment the Y coordinate for the next message
  delay(1000);                                                            // Pause for 1 second to allow the user to read the message
}

// ---------------------------------------------------------------------------------------------------------------------------
// Update the display based on the current page and system state.
// It handles rendering of different pages, status messages, and visual feedback for user actions.
void updateDisplay() {
  if (!display_ok) return;                                                // If the display is not initialized, exit the function
  unsigned long now = millis();                                           // Capture current time for consistent rendering logic
  tft.fillScreen(Layout::COL_BLACK);                                      // Clear the screen before redrawing the current page
  tft.setTextColor(Layout::COL_WHITE, Layout::COL_BLACK);                 // Set the text color

  if (sysState.showSaveMessage) {                                         // If the flag to show the save message is set, display a
                                                                          // confirmation message at the bottom of the screen for a short duration
    unsigned long elapsedTime = now - sysState.saveMessageStart;          // Timer of message visualization

    if (elapsedTime > SAVE_MSG_DURATION) {                                // Use config.h constant
      sysState.showSaveMessage = false;                                   // Reset flag to hide message
    } else {                                                              // Display the message
      int rectWidth = Layout::SCREEN_W * 0.8;                             // Calculate the width of the message rectangle (80% of screen width)
      int rectHeight = 25;                                                // Height of the message rectangle
      char buf[32];
      int rectX = (Layout::SCREEN_W - rectWidth) / 2;                     // X coordinate to center the rectangle horizontally
      int rectY = Layout::SCREEN_H - rectHeight - 5;                      // Align the rectangle at the bottom of the screen with a small margin

      // Background color based on the result of the save operation (green for success, red for error)
      uint16_t bgColor = sysState.lastSaveResult ? Layout::COL_GREEN : Layout::COL_RED;
      // Draw the background rectangle with rounded corners
      tft.fillRoundRect(rectX, rectY, rectWidth, rectHeight, 5, bgColor);
      // Set text color
      tft.setTextColor(Layout::COL_WHITE, bgColor);
      // Display the message text centered in the middle of the rectangle
      const __FlashStringHelper* fMsg = sysState.lastSaveResult ? MSG_DISPLAY_SAVED : MSG_DISPLAY_ERROR;
      strncpy_P(buf, (PGM_P)fMsg, sizeof(buf));
      buf[sizeof(buf)-1] = '\0';
      tft.drawCentreString(buf, Layout::SCREEN_CX, rectY + (rectHeight / 2) - 8, 2);
    }
  }
    
  switch (sysState.displayPage) {                                         // Draw the current page based on the index
    case 1: {                                                             // Page 1: Main status page with power, temperature, trend graph, and status badges
      float powerKw;                                                      // Power in kW with one decimal place
      char powerStr[10];                                                  // Buffer for the power string to be displayed
      const uint16_t* wifi_badge;                                         // Pointer to the WiFi status badge bitmap
      const uint16_t* mqtt_badge;                                         // Pointer to the MQTT status badge bitmap
                                                                          // --- Power Value ---
      tft.setTextFont(7);                                                 // Set the 7 segments font for the power
      tft.setTextDatum(TC_DATUM);                                         // Alignment for the power value (top center)
      powerKw = sysState.powerW / 1000.0;                                 // Convert power to kW
      dtostrf(powerKw, 1, 1, powerStr);                                   // Convert the power value to a string with one decimal place
                                                                          // (dtostrf is more reliable than snprintf for floats on ESP8266)
      tft.drawString(powerStr, Layout::SCREEN_CX, 5);                     // Draw the power value with a small margin from the top edge
      tft.setFreeFont(&FreeSans9pt7b);                                    // Set a smaller font for the unit "kW"
      tft.setTextColor(Layout::COL_GRAY_TEXT, Layout::COL_BLACK);         // Set text color for the unit
      tft.setTextDatum(TR_DATUM);                                         // Alignment for the unit (top right)
      tft.drawString("kW", Layout::SCREEN_W - 5, 57);           // Draw the unit "kW" under the power value, aligned to the right with a small margin
                                                                          // --- Temperature ---
      char tempStr[10];                                                   // Buffer for the temperature string to be displayed
      char tempVal[10];                                                   // Buffer for the numeric values
      uint16_t tempColor = Layout::COL_GREEN;                       // Color of the temperature text

      if (isnan(sysState.temperature)) {
        tempColor = Layout::COL_RED;                                      // Red color to indicate sensor error
        snprintf(tempStr, sizeof(tempStr), "---C");                       // Display dashes if sensor is faulty
      } else {
        // Use centralized threshold for warning color
        tempColor = (sysState.temperature >= TEMP_WARNING_THRESHOLD) ? Layout::COL_RED : Layout::COL_GREEN;
        dtostrf(sysState.temperature, 1, 0, tempVal);                     // Convert value to string
        snprintf(tempStr, sizeof(tempStr), "%sC", tempVal);               // Format with unit for Celsius. The degree symbol is represented by the
      }                                                                   // character code 223 in the font used.
      tft.setFreeFont(&FreeSans9pt7b);                                    // Set a small font for the temperature
      tft.setTextColor(tempColor, Layout::COL_BLACK);                     // Set text color
      tft.setTextDatum(TL_DATUM);                                         // Alignment for the temperature (top left)
      tft.drawString(tempStr, 5, 57);                                     // Draw the temperature string with a small margin from the
                                                                          // left edge and aligned under the power value

                                                                          // --- Power Trend Graph ---
      uint8_t graphBottom = Layout::BADGE_Y - 24;                         // Nuova base del grafico, alzata per far spazio a V/I
      if (!sysState.historyInitialized) {                                 // If the power history is not initialized
        for (int i = 0; i < Layout::SCREEN_W; ++i) {                      // fill it with
          sysState.powerHistory[i] = graphBottom;                         // flat line at the bottom of the graph area
        }
        sysState.historyInitialized = true;                               // and set the flag to indicate history initialized
      }

      // Flag to slow down the graph update to every 2 display refreshes (to reduce flickering and improve readability)
      static bool update_graph = false;
      update_graph = !update_graph;                                       // Turn update graph every 2 calls
      if (update_graph) {                                                 // Update the power history for the trend graph
        for (int i = 0; i < Layout::SCREEN_W - 1; ++i) {                  // Shift all values to the left by one position
          sysState.powerHistory[i] = sysState.powerHistory[i+1];
        }
        // Calculate the new Y value for the current power and add it to the end of the history array
        // The Y value is calculated based on the current power relative to a maximum power that is set to 100W
        // above the threshold for better visualization of values around the threshold. If the current power exceeds
        // this maximum, it is capped to avoid a flat graph or division by zero.
        float max_graph_power = (float)sysConfig.powerThreshold + 100.0f;
        if (max_graph_power < 500.0f) max_graph_power = 500.0f;           // Minimum scale, to maintain readability even with low thresholds

        float current_power = sysState.powerW < 0 ? 0 : sysState.powerW;       // Ensure current power is not negative
        if (current_power > max_graph_power) current_power = max_graph_power;  // Cap current power to the maximum graph power for scaling

        // Calculate the Y coordinate for the current power value and add it to the end of the history array.
        // The Y coordinate is calculated by scaling the current power to the graph height and inverting it
        // (since higher power should be lower on the screen).
        uint8_t compressedH = (uint8_t)(Layout::GRAPH_H * 0.70f);         // Height reduction of the graph to 70% of the defined height
        uint8_t new_y = graphBottom - (uint8_t)((current_power / max_graph_power) * compressedH);
        sysState.powerHistory[Layout::SCREEN_W - 1] = new_y;              // Add the new Y value to the end of the history array
      }

      for (int i = 0; i < Layout::SCREEN_W - 1; ++i) {                    // Draw the trend graph by connecting the points
        tft.drawLine(i, sysState.powerHistory[i], i + 1, sysState.powerHistory[i+1], Layout::COL_TREND_BLUE);
      }
      // Draw a point 2x2px at the end of the graph to indicate the current power value more clearly, using a cyan color for better visibility
      tft.fillRect(Layout::SCREEN_W - 2, sysState.powerHistory[Layout::SCREEN_W - 1] - 1, 2, 2, Layout::COL_TREND_CYAN);
        
      // --- Voltage and Current Display ---
      tft.setFreeFont(&FreeSans9pt7b);                                    // Set a small font for voltage and current values
      uint16_t vColor = (measuredVoltageRMS == (float)DEFAULT_GRID_VOLTAGE) ? Layout::COL_RED : Layout::COL_CYAN;
      tft.setTextColor(vColor, Layout::COL_BLACK);
      tft.setTextDatum(TL_DATUM);                                         // Alignment for voltage (top left)
      dtostrf(measuredVoltageRMS, 1, 0, tempVal);                         // Convert voltage to string (0 decimals)
      tft.drawString(tempVal, 5, Layout::BADGE_Y - 21);                   // Draw the voltage string
      tft.setTextColor(Layout::COL_YELLOW_DARK, Layout::COL_BLACK);       // Current in light orange (0xD6BA)
      tft.setTextDatum(TR_DATUM);                                         // Alignment for current (top right)
      dtostrf(sysState.currentRMS, 1, 1, tempVal);                        // Reuse tempVal buffer to convert current to a string with one decimal place
      tft.drawString(tempVal, Layout::SCREEN_W - 5, Layout::BADGE_Y - 21); // Draw the current string

      // --- WiFi e MQTT Badge ---
      tft.setSwapBytes(true);                                             // Set byte swapping for correct color rendering of the bitmap badges

      if (WiFi.getMode() == WIFI_AP) {                                    // If the WiFi mode is Access Point
        if (WiFi.softAPgetStationNum() > 0) {                             // If at least one client is connected
          wifi_badge = badge_wifi_ap_green_38x20;                         // Use AP green badge (client connected)
        } else {                                                          // Otherwise
          wifi_badge = badge_wifi_ap_38x20;                               // Use AP standard badge (waiting for client)
        }
      } else {                                                            // In Station mode, show Green if connected, Gray if not connected
        wifi_badge = (WiFi.status() == WL_CONNECTED) ? badge_wifi_ok_38x20 : badge_wifi_off_38x20;
      }
      // Draw the WiFi badge at the top right corner of the screen
      tft.pushImage(Layout::BADGE_WIFI_X, Layout::BADGE_Y, Layout::BADGE_W, Layout::BADGE_H, wifi_badge);
      // For MQTT, show Green if connected, Gray if not connected
      mqtt_badge = (mqttClient.connected()) ? badge_mqtt_ok_38x20 : badge_mqtt_off_38x20;
      // Draw the MQTT badge next to the WiFi badge
      tft.pushImage(Layout::BADGE_MQTT_X, Layout::BADGE_Y, Layout::BADGE_W, Layout::BADGE_H, mqtt_badge);

      tft.setSwapBytes(false);                                            // Reset byte swapping to default for future drawing operations
      break;
    }

    case 2: {                                                             // Page 2: Configuration page for power threshold
      char tempBuf[10];                                                   // Temporary buffer for converting float values to strings
      float savedKw, currentKw;                                           // Variables to hold the saved and current power threshold values
      char savedStr[16];                                                  // Buffer for the saved threshold string to be displayed (e.g., "1.5 kW")
      char thresholdStr[16];                                              // Buffer for the current threshold string to be displayed (e.g., "1.5 kW")

      char buf[32];
      tft.setFreeFont(&FreeSans9pt7b);                                    // Set a readable font for the page
      tft.setTextColor(Layout::COL_WHITE, Layout::COL_BLACK);             // Set text color for the page
      tft.setTextDatum(TL_DATUM);                                         // Alignment for the header (top left)
      strncpy_P(buf, (PGM_P)MSG_DISPLAY_THRESHOLD, sizeof(buf));
      buf[sizeof(buf)-1] = '\0';
      tft.drawString(buf, 2, 2);                // Draw the header for the threshold configuration page

      savedKw = savedSysConfig.powerThreshold / 1000.0;                   // Convert the saved power threshold to kW
      dtostrf(savedKw, 1, 1, tempBuf);                                    // Convert the saved power threshold to a string with one decimal place
      snprintf(savedStr, sizeof(savedStr), "%s kW", tempBuf);             // Format the saved threshold string with "kW" unit
        
      tft.setTextFont(4);                                                 // Set a larger font for the threshold values
      tft.setTextColor(Layout::COL_WHITE, Layout::COL_BLACK);             // Set text color for the saved threshold
      tft.setTextDatum(TR_DATUM);                                         // Alignment for the saved threshold (top right)
      tft.drawString(savedStr, Layout::SCREEN_W - 2, 24);                 // Draw the saved threshold value at the top right corner, with a small margin
        
      tft.setFreeFont(&FreeSans9pt7b);                                    // Set a readable font for the "SET" label
      tft.setTextColor(Layout::COL_ORANGE, Layout::COL_BLACK);            // Set text color for the "SET" label
      tft.setTextDatum(TL_DATUM);                                         // Alignment for the "SET" label (top left)
      tft.drawString("SET", 2, 70);                            // Draw the "SET" label on the left side
        
      currentKw = sysConfig.powerThreshold / 1000.0;                      // Convert the current power threshold to kW
      dtostrf(currentKw, 1, 1, tempBuf);                                  // Convert the current power threshold to a string with one decimal place
      snprintf(thresholdStr, sizeof(thresholdStr), "%s kW", tempBuf);     // Format the current threshold string with "kW" unit
        
      tft.setTextFont(4);                                                 // Set a larger font for the current threshold value
      tft.setTextColor(Layout::COL_ORANGE, Layout::COL_BLACK);            // Set text color for the current threshold value
      tft.setTextDatum(TR_DATUM);                                         // Alignment for the current threshold value (top right)
      tft.drawString(thresholdStr, Layout::SCREEN_W - 2, 92);             // Draw the current threshold value on the right side, with a small margin
      break;
    }

    case 3: {                                                             // Page 3: Configuration page for buzzer mode
      tft.setFreeFont(&FreeSans9pt7b);                                    // Set a readable font for the page
      tft.setTextColor(Layout::COL_WHITE, Layout::COL_BLACK);             // Set text color for the page
      tft.setTextDatum(TL_DATUM);                                         // Alignment for the header (top left)
      char buf[32];
      strncpy_P(buf, (PGM_P)MSG_DISPLAY_BUZZER_MODE_1, sizeof(buf));
      buf[sizeof(buf)-1] = '\0';
      tft.drawString(buf, 2, 2);
      strncpy_P(buf, (PGM_P)MSG_DISPLAY_BUZZER_MODE_2, sizeof(buf));
      buf[sizeof(buf)-1] = '\0';
      tft.drawString(buf, 2, 20);

      for (int i = 0; i < 3; i++) {                                       // Draw buzzer mode badges (Continuous, Pulse Slow, Pulse Fast)
        drawBuzzerBadge(i, Layout::BTN_MODE_X, Layout::BTN_MODE_Y[i], Layout::BTN_MODE_W, Layout::BTN_MODE_H);
      }
      break;
    }

    case 4: {                                                             // Page 4: Summary page showing the current settings
      float threshKw;                                                     // Variable to hold the power threshold in kW for display
      char settingsStr1[16], tempBuf[10];                                 // Buffers for the settings strings to be displayed
      int gfx_x, gfx_y, thickness, gfx_w;                                 // Variables for drawing the visual indicator for the buzzer mode (position, thickness, width)

      tft.setFreeFont(&FreeSans9pt7b);                                    // Set a readable font for the page
      tft.setTextColor(Layout::COL_WHITE, Layout::COL_BLACK);             // Set text color for the page
      tft.setTextDatum(TL_DATUM);                                         // Alignment for the header (top left)
      char buf[32];
      strncpy_P(buf, (PGM_P)MSG_DISPLAY_SAVE_SETTINGS_1, sizeof(buf));
      buf[sizeof(buf)-1] = '\0';
      tft.drawString(buf, 2, 2);
      strncpy_P(buf, (PGM_P)MSG_DISPLAY_SAVE_SETTINGS_2, sizeof(buf));
      buf[sizeof(buf)-1] = '\0';
      tft.drawString(buf, 2, 20);

      tft.setSwapBytes(true);                                             // Set byte swapping for correct color rendering of the bitmap icons
      // --- Row 1: Power Threshold ---
      tft.pushImage(Layout::ICON_X, Layout::ROW_Y[0] - (Layout::ICON_H/2), Layout::ICON_W, Layout::ICON_H, icon_power_16x16);
      // --- Row 2: Buzzer Mode ---
      tft.pushImage(Layout::ICON_X, Layout::ROW_Y[1] - (Layout::ICON_H/2), Layout::ICON_W, Layout::ICON_H, icon_speaker_16x16);
      tft.setSwapBytes(false);                                            // Reset byte swapping to default for future drawing operations

      tft.setTextColor(Layout::COL_ORANGE, Layout::COL_BLACK);            // Set text color for the settings values
      tft.setTextDatum(ML_DATUM);                                         // Alignment for the settings values (middle left, aligned with the icons)
      threshKw = sysConfig.powerThreshold / 1000.0;                       // Convert the power threshold to kW for display
      dtostrf(threshKw, 1, 1, tempBuf);                                   // Convert the power threshold to a string with one decimal place
      snprintf(settingsStr1, sizeof(settingsStr1), "%s kW", tempBuf);     // Format the power threshold string with "kW" unit
      tft.drawString(settingsStr1, Layout::TEXT_X, Layout::ROW_Y[0]);     // Draw the power threshold string on the first row, aligned with the first icon

      gfx_x = Layout::TEXT_X;                                             // X coordinate for the visual indicator of the buzzer mode
      gfx_y = Layout::ROW_Y[1];                                           // Y coordinate for the visual indicator of the buzzer mode
      thickness = 3;                                                      // Thickness of the lines for the visual indicator
      gfx_w = Layout::SCREEN_W - gfx_x - 5;                               // Width of the visual indicator area, calculated as the
                                                                          // remaining width of the screen after the text and a small margin
      if (sysConfig.buzzerMode == BUZZER_MODE_CONTINUOUS) {               // For Continuous mode, draw a full-width line
        tft.fillRect(gfx_x, gfx_y - 1, gfx_w, thickness, Layout::COL_ORANGE);  // Draw a filled rectangle as a line for the continuous mode
      } else if (sysConfig.buzzerMode == BUZZER_MODE_PULSE_SLOW) {        // For Slow Pulse mode, draw two segments with a gap in the middle
        int segLen = (gfx_w - 8) / 2;                                     // Length of each segment, calculated as half of the
                                                                          // available width minus the gap (8 pixels)
        tft.fillRect(gfx_x, gfx_y - 1, segLen, thickness, Layout::COL_ORANGE); // Draw the first segment for the slow pulse mode
        tft.fillRect(gfx_x + segLen + 8, gfx_y - 1, segLen, thickness, Layout::COL_ORANGE);   // Draw the second segment for the slow pulse mode,
                                                                          // positioned after the first segment and the gap
      } else if (sysConfig.buzzerMode == BUZZER_MODE_PULSE_FAST) {        // For Fast Pulse mode, draw four segments with equal gaps
        int gap = 6;                                                      // Gap between segments for the fast pulse mode
        int segLen = (gfx_w - (gap * 3)) / 4;                             // Length of each segment, calculated as a quarter of the
                                                                          // available width minus the total gap space (3 gaps for 4 segments)
        for (int k = 0; k < 4; k++) {                                     // Loop to draw the four segments for the fast pulse mode
          tft.fillRect(gfx_x + (segLen + gap) * k, gfx_y - 1, segLen, thickness, Layout::COL_ORANGE);   // Draw each segment, positioned based on
                                                                          // its index, the segment length, and the gap
        }
      }
      break;
    }

    case 5: {                                                             // Page 5: Information page showing the device hostname,
                                                                          // connected WiFi SSID, IP address, and firmware version with scrolling for long text
      bool isAP;                                                          // Variable to indicate if the device is in Access Point mode
      char newHN[33], newSSID[33], newIP[16], currentFW[20];              // Buffers for the new hostname, SSID, IP address, and current
                                                                          // firmware version to be displayed. The sizes are set to accommodate
                                                                          // typical maximum lengths (32 characters for hostname and SSID,
                                                                          // 15 characters for IP address plus null terminator, and
                                                                          // 20 characters for firmware version).
      int viewW, hnW, ssidW, ipW;                                         // Variables for the width of the view area for scrolling and
                                                                          // the actual width of the hostname, SSID, and IP address text
      static char currentHN[33] = "";                                     // Static buffers to store the currently displayed hostname, SSID, and IP address.
                                                                          // These are used to detect changes and reset scrolling when the information
                                                                          // changes or when the user returns to this page.
      static char currentSSID[33] = "";                                   // Note: The SSID can be up to 32 characters, but we use 3
                                                                          // to include the null terminator
      static char currentIP[16] = "";                                     // Note: An IPv4 address can be up to 15 characters
                                                                          // plus the null terminator,hence 16
      static int scrollHN = 0, scrollSSID = 0, scrollIP = 0;              // Static variables to manage the scrolling position for the hostname,
                                                                          // SSID, and IP address. These are used to create a scrolling effect
                                                                          // when the text exceeds the width of the view area.
      static unsigned long timerHN = 0, timerSSID = 0, timerIP = 0;       // Static timers to manage the timing of the scrolling effect for
                                                                          // the hostname, SSID, and IP address. These timers are used to create
                                                                          // pauses before and after scrolling, as well as to control
                                                                          // the speed of the scrolling.
      static int stateHN = 0, stateSSID = 0, stateIP = 0;                 // Static state variables to manage the state of the scrolling effect
                                                                          // for the hostname, SSID, and IP address. The states can be:
                                                                          // 0 = waiting before scroll, 1 = scrolling, 2 = waiting after scroll.
                                                                          // These states are used in conjunction with the timers to create
                                                                          // a smooth scrolling effect with pauses.
      isAP = (WiFi.getMode() == WIFI_AP);                                 // Determine if the device is in Access Point mode
      snprintf(newHN, sizeof(newHN), "%s", sysConfig.device_name);        // Get the device hostname from the configuration and store it in newHN
      if (isAP) {                                                         // If in Access Point mode
        snprintf(newSSID, sizeof(newSSID), "%s", sysState.apSSID);        // Get the AP SSID from the system state and store it in newSSID
        IPAddress ip = WiFi.softAPIP();
        snprintf(newIP, sizeof(newIP), "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
      } else {                                                            // Else (if in Station mode)
        strncpy(newSSID, WiFi.SSID().c_str(), sizeof(newSSID));           // Get the connected WiFi SSID and store it in newSSID
        newSSID[sizeof(newSSID) - 1] = '\0';                              // Ensure null termination in case of overflow
        if (strlen(newSSID) == 0) strcpy(newSSID, "---");                 // If not connected to any WiFi network, set SSID to "---"
        IPAddress ip = WiFi.localIP();
        snprintf(newIP, sizeof(newIP), "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
      }
      snprintf(currentFW, sizeof(currentFW), "Firmw. %s", FIRMWARE_VERSION);      // Get the current firmware version directly

      // If any of the displayed information (hostname, SSID, IP address) has changed since the last time this page was displayed,
      // update the current values and reset the scrolling state and timers.
      if (strcmp(newHN, currentHN) != 0 || strcmp(newSSID, currentSSID) != 0 || strcmp(newIP, currentIP) != 0) {
        strcpy(currentHN, newHN); strcpy(currentSSID, newSSID); strcpy(currentIP, newIP);
        scrollHN = scrollSSID = scrollIP = 0; 
        stateHN = stateSSID = stateIP = 0;
        timerHN = timerSSID = timerIP = now;
      }

      tft.setTextDatum(TL_DATUM);                                         // Alignment for the label (top left)
      viewW = Layout::INFO_VIEW_W;                                        // Width of the view area for scrolling
      // --- Hostname ---
      tft.setTextColor(Layout::COL_INFO_LABEL, TFT_BLACK);                // Set text color for the label
      tft.setFreeFont(&FreeSans9pt7b);                                    // Set a readable font for the label
      tft.drawString("Host:", 5, 2);                           // Draw Hostname label
      tft.setTextColor(Layout::COL_WHITE, Layout::COL_BLACK);             // Set text color for the hostname value
      tft.setTextFont(2);                                                 // Set a larger font for the hostname value
      hnW = tft.textWidth(currentHN, 2);                                  // Calculate the width of the hostname text in pixels for the current font and size

      if (hnW <= viewW) {                                                 // If the hostname text fits within the view area
        tft.drawString(currentHN, 5, 20);                                 // draw the hostname normally without scrolling
      } else {                                                            // If the hostname text exceeds the width of the view area, implement scrolling
        tft.setViewport(5, 20, viewW, 18);                                // Set a viewport for the hostname value
        tft.drawString(currentHN, -scrollHN, 0);                          // Draw the hostname text at a position offset by the scroll value
        tft.resetViewport();                                              // Reset the viewport to the full screen for subsequent drawing operations

        if (stateHN == 0 && now - timerHN > UI_SCROLL_DELAY) stateHN = 1; // Use config.h constants
                                                                          // change state to scrolling
        else if (stateHN == 1) {                                          // If in scrolling state
          scrollHN += UI_SCROLL_SPEED;                                    // increment the scroll position
          if (scrollHN >= (hnW - viewW)) { stateHN = 2; timerHN = now; }  // If the scroll position has reached the end of the text,
                                                                          // change state to waiting after scroll and reset timer
        } 
        // If in waiting after scroll state and 2 seconds have passed, reset scroll position and return to initial state
        else if (stateHN == 2 && now - timerHN > UI_SCROLL_DELAY) { scrollHN = 0; stateHN = 0; timerHN = now; }
      }
      // --- SSID ---
      tft.setTextColor(Layout::COL_INFO_LABEL, TFT_BLACK);
      tft.setFreeFont(&FreeSans9pt7b);
      tft.drawString("SSID:", 5, 45);
      tft.setTextColor(Layout::COL_WHITE, Layout::COL_BLACK);
      tft.setTextFont(2);
      ssidW = tft.textWidth(currentSSID, 2);

      if (ssidW <= viewW) {
        tft.drawString(currentSSID, 5, 63);
      } else {
        tft.setViewport(5, 63, viewW, 18);
        tft.drawString(currentSSID, -scrollSSID, 0);
        tft.resetViewport();

        if (stateSSID == 0 && now - timerSSID > UI_SCROLL_DELAY) stateSSID = 1;
        else if (stateSSID == 1) {
          scrollSSID += UI_SCROLL_SPEED;
          if (scrollSSID >= (ssidW - viewW)) { stateSSID = 2; timerSSID = now; }
        } 
        else if (stateSSID == 2 && now - timerSSID > UI_SCROLL_DELAY) { scrollSSID = 0; stateSSID = 0; timerSSID = now; }
        }

      // --- IP Address ---
      tft.setTextColor(Layout::COL_INFO_LABEL, TFT_BLACK);
      tft.setFreeFont(&FreeSans9pt7b);
      tft.drawString("IP:", 5, 88);
      tft.setTextColor(Layout::COL_WHITE, Layout::COL_BLACK);
      tft.setTextFont(2);
      ipW = tft.textWidth(currentIP, 2);

      if (ipW <= viewW) {
        tft.drawString(currentIP, 5, 106);
      } else {
        tft.setViewport(5, 106, viewW, 18);
        tft.drawString(currentIP, -scrollIP, 0);
        tft.resetViewport();

        if (stateIP == 0 && now - timerIP > UI_SCROLL_DELAY) stateIP = 1;
        else if (stateIP == 1) {
          scrollIP += UI_SCROLL_SPEED;
          if (scrollIP >= (ipW - viewW)) { stateIP = 2; timerIP = now; }
        }
        else if (stateIP == 2 && now - timerIP > UI_SCROLL_DELAY) { scrollIP = 0; stateIP = 0; timerIP = now; }
      }

      // --- Firmware ---
      tft.setTextDatum(BL_DATUM);                                         // Alignment for the label (bottom left)
      tft.setTextColor(Layout::COL_INFO_LABEL, TFT_BLACK);                // Set text color for the label
      tft.setTextFont(2);                                                 // Set a readable font for the label
      tft.drawString(currentFW, 5, Layout::SCREEN_H - 5);                 // Draw Firmware value
      break;
      }
      
      case 6: {                                                           // Page 6: System actions page with buttons for AP mode, Reboot, and Reset
        const __FlashStringHelper* desc1 = F("");                         // Variables to hold the description text for the selected action.
                                                                          // These will be set based on the selected action and displayed in
                                                                          // the information area when no button is pressed.
        const __FlashStringHelper* desc2 = F("");                         // The second line of the description text
        
        // Draw the three system action buttons: AP Mode, Reboot, and Reset. The button for the currently selected action is highlighted.
        drawSystemBtn("APmode", Layout::BTN_SYS_X, 4, Layout::BTN_SYS_W, Layout::BTN_SYS_H, TFT_WHITE, sysState.selectedAction == 0);
        drawSystemBtn("Reboot", Layout::BTN_SYS_X, 44, Layout::BTN_SYS_W, Layout::BTN_SYS_H, TFT_ORANGE, sysState.selectedAction == 1);
        drawSystemBtn("Reset", Layout::BTN_SYS_X, 84, Layout::BTN_SYS_W, Layout::BTN_SYS_H, TFT_RED, sysState.selectedAction == 2);

        if (sysState.showAPModeMessage) {                                 // If the flag to show the AP mode message is set
          if (now - sysState.apModeMessageStart < AP_MSG_DURATION) {      // Use config.h constant
            // Draw a blue rounded rectangle as the background for the AP mode message
            tft.fillRoundRect(Layout::BAR_X, Layout::BAR_Y, Layout::BAR_W, Layout::BAR_H, 4, TFT_BLUE);
            tft.setFreeFont(&FreeSans9pt7b);                              // Set a readable font for the message
            tft.setTextDatum(MC_DATUM);                                   // Set text alignment to center-middle for the message
            tft.setTextColor(TFT_WHITE, TFT_BLUE);                        // Set text color to white with blue background for the message
            char apBuf[32];
            strncpy_P(apBuf, (PGM_P)MSG_DISPLAY_AP_ON, sizeof(apBuf));    // Copy the AP mode message from program memory to a local buffer for display
            tft.drawString(apBuf, Layout::SCREEN_CX, Layout::BAR_Y + (Layout::BAR_H / 2));    // Draw the AP mode message
            break;                                                        // Exit the drawing for this page to keep the message displayed
                                                                          // without drawing the rest of the page elements
          } else {                                                        // If 3 seconds have passed, reset the flag to hide the AP mode message
            sysState.showAPModeMessage = false;                           // and continue with drawing the rest of the page elements
          }
       }

      // --- Progress bar for system actions ---
      // Draw a rounded rectangle as the border for the progress bar. The color is a medium gray to contrast with the filled portion of the bar.
      tft.drawRoundRect(Layout::BAR_X, Layout::BAR_Y, Layout::BAR_W, Layout::BAR_H, 4, 0x3186);

      if (digitalRead(PIN_BTN) == LOW && !sysState.actionTriggered) {     // If the button is currently pressed and an action has not yet been triggered
        unsigned long elapsed = now - sysState.btnPressedTime;            // Calculate the elapsed time since the button was pressed
        if (elapsed > DEBOUNCE_DELAY) {                                   // Debounce
          float progress = (float)elapsed / (float)SYS_ACTION_HOLD;       // Use config.h constant
          if (progress > 1.0f) progress = 1.0f;                           // Cap the progress at 1.0 (100%) to avoid overfilling the bar
          int fillW = (int)((Layout::BAR_W - 4) * progress);              // Calculate width of filled portion of the bar
          uint16_t barColor;                                              
          // Determine the color of the filled portion of the bar based on the selected action: white for AP mode, orange for Reboot, red for Reset
          if (sysState.selectedAction == 0) barColor = Layout::COL_WHITE;
          else if (sysState.selectedAction == 1) barColor = Layout::COL_ORANGE;
          else barColor = Layout::COL_RED;
          
          // Draw the filled portion of the progress bar
          tft.fillRoundRect(Layout::BAR_X + 2, Layout::BAR_Y + 2, fillW, Layout::BAR_H - 4, 2, barColor);
             
          // Draw the progress percentage text centered in the bar
          tft.setFreeFont(&FreeSans9pt7b);                                // Set a readable font for the progress text
          tft.setTextDatum(MC_DATUM);                                     // Set text alignment to center-middle for the progress text
          tft.setTextColor(Layout::COL_GRAY_TEXT);                        // Set text color to a light gray
          char pStr[8];                                                   // Buffer for the progress percentage string (e.g., "75%")
          snprintf(pStr, sizeof(pStr), "%d%%", (int)(progress * 100));    // Format the progress percentage string
          tft.drawString(pStr, Layout::SCREEN_CX, Layout::BAR_Y + (Layout::BAR_H / 2));  // Draw the progress percentage text centered in the bar
        }
      } else {                                                            // If the button is not pressed
        switch(sysState.selectedAction) {                                 // Set the description text based on the currently selected action
          case 0: desc1 = MSG_DESC_AP_1; desc2 = MSG_DESC_AP_2; break;
          case 1: desc1 = MSG_DESC_REBOOT_1; desc2 = MSG_DESC_REBOOT_2; break;
          case 2: desc1 = MSG_DESC_RESET_1; desc2 = MSG_DESC_RESET_2; break;
        }

        tft.setTextFont(1);                                               // Set a smaller font for the description text
        tft.setTextColor(Layout::COL_WHITE, Layout::COL_BLACK);           // Set text color for the description text
        tft.setTextDatum(TC_DATUM);                                       // Set text alignment to top-center for the description text

        // Calculate the vertical position to start drawing the description text so that it is centered within the area of the bar
        int text_area_y = Layout::BAR_Y + 2;                              // Y coordinate of the area for the description text
        int text_area_h = Layout::BAR_H - 4;                              // Height of the area for the description text
        int total_text_h = 8 + 2 + 8;                                     // Total height (8px font height + 2px spacing)
        int start_y = text_area_y + (text_area_h - total_text_h) / 2;     // Starting Y to vertically center the two lines of description text
        char d1[32], d2[32];
        strncpy_P(d1, (PGM_P)desc1, sizeof(d1)); d1[sizeof(d1)-1] = '\0';
        strncpy_P(d2, (PGM_P)desc2, sizeof(d2)); d2[sizeof(d2)-1] = '\0';
        tft.drawString(d1, Layout::SCREEN_CX, start_y);
        tft.drawString(d2, Layout::SCREEN_CX, start_y + 10);
      }
    break;
    }
  }

}
