// ###########################################################################################################################
// Implements the embedded web server handlers.
// It generates HTML pages for status, configuration, and firmware updates, and processes form submissions.
//
// Author: CtrlAltJon
// Last Updated: January 2026
// Copyright (c) 2026 CtrlAltJon
// License: MIT
// The software is provided "as is", without warranty of any kind.
// This license notice must be included in any copy or portion of the software.
//
// ###########################################################################################################################

#include "webpages.h"                                                     // Include declaration of web page handler functions and embedded HTML/CSS/JS
#include "sys_globals.h"                                                  // Include global variables and structures
#include "languages.h"                                                    // Include language definitions for multi-language support
#include <Arduino.h>                                                      // Include Arduino core library for basic types and functions
#include <ESP8266WiFi.h>                                                  // Include WiFi library for ESP8266
#include <WiFiUdp.h>                                                      // Include WiFi UDP library for network communication
#include <PubSubClient.h>                                                 // Include MQTT client library for ESP8266
#include <EEPROM.h>                                                       // Include EEPROM library for reading/writing configuration data
#include <Updater.h>                                                      // Include OTA update library for handling firmware updates

// Embedded HTML, CSS, and JavaScript for the web interface are stored in PROGMEM to optimize RAM usage on the ESP8266.
// These constants contain the styles and scripts used across the web pages served by the device.
// The sendCommonHeader function sends the standard HTML header along with these styles and scripts, while the sendCommonFooter
// function sends the closing tags for the HTML document.
const char HTTP_STYLE[] PROGMEM = "<style>body{font-family:'Segoe UI',sans-serif;background:#121212;color:#e0e0e0;margin:0;padding:20px}div{background:#1e1e1e;padding:25px;margin:0 auto;max-width:400px;border-radius:12px;box-shadow:0 4px 10px rgba(0,0,0,0.5)}h1{color:#fff;text-align:center;margin-top:0}h2{color:#b0b0b0;font-size:1.2em;border-bottom:1px solid #333;padding-bottom:10px;margin-top:20px}p{font-size:1.1em;margin:15px 0;display:flex;justify-content:space-between}b{color:#4dabf7}label{display:block;margin-top:15px;color:#ccc;font-size:0.9em}input[type='text'],input[type='number'],input[type='password']{width:100%;padding:10px;margin-top:5px;border-radius:6px;border:1px solid #444;background:#2d2d2d;color:#fff;box-sizing:border-box}input[type='submit']{width:100%;background:#0069d9;color:white;border:none;padding:12px;border-radius:6px;cursor:pointer;margin-top:25px;font-size:1em;font-weight:bold;transition:background 0.3s}input[type='submit']:hover{background:#0056b3}input[type='submit'].btn-red{background:#d32f2f}input[type='submit'].btn-red:hover{background:#b71c1c}.small{font-size:0.8em;color:#777;text-align:center;margin-top:20px}.val{font-weight:bold;font-size:1.2em}.badge{font-size:0.6em;padding:3px 6px;border-radius:4px;background:#333;color:#fff;margin-left:5px;vertical-align:middle}.badge.on{background:#28a745}.badge.blue{background:#0069d9}a{color:#777;text-decoration:none}a:hover{color:#fff}#static-ip-fields{display:";

// display is controlled dynamically based on whether static IP is enabled or not in the configuration.
// If static IP is enabled, the fields for entering static IP configuration will be shown; otherwise, they will be hidden.
// This allows for a cleaner user interface that only shows relevant options based on the user's configuration choices.
const char HTTP_SCRIPT[] PROGMEM = ";}</style><script>function toggleStaticIP(){var c=document.getElementById('staticIP');var f=document.getElementById('static-ip-fields');if(c.checked){f.style.display='block'}else{f.style.display='none'}}</script>";

// Protect configuration and OTA endpoints with HTTP Basic Auth.
static bool requireWebAuth() {
  if (webServer.authenticate(sysConfig.web_user, sysConfig.web_pass)) return true;
  webServer.requestAuthentication();
  return false;
}

// Send text safely inside HTML contexts (attribute/body) to prevent HTML injection.
static void sendEscapedHtml(const char* raw) {
  if (!raw) return;
  for (const char* p = raw; *p != '\0'; ++p) {
    switch (*p) {
      case '&': webServer.sendContent(F("&amp;")); break;
      case '<': webServer.sendContent(F("&lt;")); break;
      case '>': webServer.sendContent(F("&gt;")); break;
      case '"': webServer.sendContent(F("&quot;")); break;
      case '\'': webServer.sendContent(F("&#39;")); break;
      default: {
        char c[2] = {*p, '\0'};
        webServer.sendContent(c);
        break;
      }
    }
  }
}

// --------------------------------------------------------------------------------------------- Web page - Send Common Header
// Helper function to send the standard HTML header, including CSS styles and JavaScript.
// It initializes the HTTP response, sets up the page title, and optionally adds a meta-refresh tag for auto-reloading.
void sendCommonHeader(const __FlashStringHelper* title, bool refresh = false) {
  // Start the HTTP response with chunked transfer encoding to allow for dynamic content generation without needing to know the total content length upfront
  webServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
  // Send an initial empty response to establish the connection and allow for subsequent content to be sent in chunks
  webServer.send(200, "text/html", "");
  // Set TCP_NODELAY to true for the client connection to ensure that the data is sent immediately without waiting for the buffer to fill up, which can improve responsiveness of the web interface on the ESP8266
  if (webServer.client()) {
    webServer.client().setNoDelay(true);
  }

  // Send the standard HTML header, including the page title, embedded CSS styles for the web interface, and JavaScript for dynamic behavior
  webServer.sendContent(F("<html><head><title>"));
  webServer.sendContent(title); // This is already a String copied from Flash
  webServer.sendContent(F("</title><meta name='viewport' content='width=device-width, initial-scale=1'>"));
  if (refresh) { char b[8]; snprintf(b,8,"%d",WEB_REFRESH_STATUS); webServer.sendContent(F("<meta http-equiv='refresh' content='")); webServer.sendContent(b); webServer.sendContent(F("'>")); }
  webServer.sendContent_P(HTTP_STYLE);
  webServer.sendContent(sysConfig.staticIP ? F("block") : F("none"));
  webServer.sendContent_P(HTTP_SCRIPT);
  webServer.sendContent(F("</head><body><div>"));
}

// --------------------------------------------------------------------------------------------- Web page - Send Common Footer
// Helper function to send the standard HTML footer
void sendCommonFooter() {
  webServer.sendContent(F("</div></body></html>"));
  webServer.sendContent("");
}

// ------------------------------------------------------------------------------------------------------ Web page - root page
// This is the handler for the root page ("/") of the web interface.
void handleRoot() {
  // --- Variables ---
  unsigned long sec = millis() / 1000;                                    // Uptime in seconds
  int min, hr, days;
  char uptime[24];
  const char* daySuffix;
  const char* apClass = "badge";
  const char* wifiClass = "badge";
  const char* mqttClass;
  char valBuf[12];

  // --- Calculate Uptime ---
  min = sec / 60;
  hr = min / 60;
  days = hr / 24;
  hr = hr % 24;                                                           // Remaining hours
  daySuffix = (currentLanguage == LANG_IT) ? "g" : "d";
  if (days > 0) snprintf(uptime, sizeof(uptime), "%d%s %02d:%02d:%02lu", days, daySuffix, hr, min % 60, sec % 60);
  else snprintf(uptime, sizeof(uptime), "%02d:%02d:%02lu", hr, min % 60, sec % 60);

  // --- Connectivity Status Badges ---
  mqttClass = (mqttClient.connected()) ? "badge on" : "badge";
  if (WiFi.getMode() == WIFI_AP) {
    apClass = "badge blue";
    if (WiFi.softAPgetStationNum() > 0) {
      wifiClass = "badge on";
    }
  } else {
    if (WiFi.status() == WL_CONNECTED) wifiClass = "badge on";
  }

  // --- Generate HTML Content ---
  sendCommonHeader(MSG_WEB_TITLE, true);                                  // Send header

  webServer.sendContent(F("<h1>"));
  sendEscapedHtml(sysConfig.device_name);
  webServer.sendContent(F("</h1><h2>"));
  webServer.sendContent(MSG_WEB_STATUS);
  webServer.sendContent(F(" <span style='float:right'><span class='"));
  webServer.sendContent(apClass);
  webServer.sendContent(F("'>AP</span> <span class='"));
  webServer.sendContent(wifiClass);
  webServer.sendContent(F("'>"));
  webServer.sendContent(MSG_DISPLAY_WIFI);
  webServer.sendContent(F("</span> <span class='"));
  webServer.sendContent(mqttClass);
  webServer.sendContent(F("'>"));
  webServer.sendContent(MSG_DISPLAY_MQTT);
  webServer.sendContent(F("</span></span></h2>"));

  webServer.sendContent(F("<p>")); 
  webServer.sendContent(MSG_WEB_POWER); 
  webServer.sendContent(F(": <span class='val' style='color: #ff6b6b;'>")); 
  dtostrf(sysState.powerW, 1, 0, valBuf);
  webServer.sendContent(valBuf); 
  webServer.sendContent(F(" W</span></p>"));
  
  // --- Voltage ---
  webServer.sendContent(F("<p>")); 
  // Use "Tensione" for IT and "Voltage" for EN based on current language
  webServer.sendContent(MSG_WEB_VOLTAGE); 
  const char* vColor = (measuredVoltageRMS == (float)DEFAULT_GRID_VOLTAGE) ? "#ff6b6b" : "#4dabf7";
  webServer.sendContent(F(": <span class='val' style='color: "));
  webServer.sendContent(vColor);
  webServer.sendContent(F(";'>"));
  dtostrf(measuredVoltageRMS, 1, 0, valBuf);                              // Real-time voltage, 0 decimals
  webServer.sendContent(valBuf); 
  webServer.sendContent(F(" V</span></p>"));

  // --- Current ---
  webServer.sendContent(F("<p>")); 
  webServer.sendContent(MSG_WEB_CURRENT); 
  webServer.sendContent(F(": <span class='val' style='color: #fcc419;'>")); 
  dtostrf(sysState.currentRMS, 1, 1, valBuf);                             // Current with 1 decimal place
  webServer.sendContent(valBuf); 
  webServer.sendContent(F(" A</span></p>"));
  
  // --- Temperature ---
  webServer.sendContent(F("<p>")); 
  webServer.sendContent(MSG_WEB_TEMPERATURE); 

  // Use centralized warning threshold
  const char* tempColor = (isnan(sysState.temperature) || sysState.temperature >= TEMP_WARNING_THRESHOLD) ? "#ff6b6b" : "#28a745";
  
  webServer.sendContent(F(": <span class='val' style='color: ")); 
  webServer.sendContent(tempColor);
  webServer.sendContent(F(";'>"));

  if (isnan(sysState.temperature)) {
    webServer.sendContent(F("---"));
  } else {
    dtostrf(sysState.temperature, 1, 1, valBuf);                          // Temperature with 1 decimal place
    webServer.sendContent(valBuf);
  }
  webServer.sendContent(F(" &deg;C</span></p>"));

  // --- Uptime ---
  webServer.sendContent(F("<p>")); 
  webServer.sendContent(MSG_WEB_UPTIME); 
  webServer.sendContent(F(": <span>")); webServer.sendContent(uptime); webServer.sendContent(F("</span></p>"));

  webServer.sendContent(F("<form action='/config' method='get'><input type='submit' value='")); webServer.sendContent(MSG_WEB_BTN_CONFIG); webServer.sendContent(F("'></form>"));
  webServer.sendContent(F("<form action='/' method='get'><input type='submit' value='")); webServer.sendContent(MSG_WEB_LINK_REFRESH); webServer.sendContent(F("'></form>"));
  webServer.sendContent(F("<p class='small'>Firmware ")); webServer.sendContent(FIRMWARE_VERSION); webServer.sendContent(F("</p>"));

  sendCommonFooter();                                                     // Send footer
}

// -------------------------------------------------------------------------------------------------- Web page - Configuration
// This is the handler for the configuration page ("/config") of the web interface
void handleConfig() {
  if (!requireWebAuth()) return;
  char valBuf[16];
  sendCommonHeader(MSG_CFG_TITLE, false);                                 // Send header

  webServer.sendContent(F("<h1>"));
  webServer.sendContent(MSG_CFG_HEADER);
  webServer.sendContent(F("</h1><form action='/save_config' method='post'>"));

  // --- General Settings ---
  webServer.sendContent(F("<h2>")); webServer.sendContent(MSG_CFG_GEN_SETTINGS); webServer.sendContent(F("</h2>"));

  // --- Device Name ---
  webServer.sendContent(F("<label for='name'>")); webServer.sendContent(MSG_CFG_DEV_NAME); webServer.sendContent(F("</label>"));
  webServer.sendContent(F("<input type='text' id='name' name='name' value='"));
  sendEscapedHtml(sysConfig.device_name);
  webServer.sendContent(F("'>")); // Removed redundant if for safety as device_name is usually not empty

  // --- Power Threshold ---
  webServer.sendContent(F("<label for='threshold'>")); webServer.sendContent(MSG_CFG_THRESHOLD); webServer.sendContent(F("</label>"));
  snprintf(valBuf, sizeof(valBuf), "%d", sysConfig.powerThreshold);
  webServer.sendContent(F("<input type='number' id='threshold' name='threshold' value='")); 
  webServer.sendContent(valBuf); webServer.sendContent(F("'>"));

  // --- Manufacturer ---
  webServer.sendContent(F("<label for='manufacturer'>")); webServer.sendContent(MSG_CFG_MANUFACTURER); webServer.sendContent(F("</label>"));
  webServer.sendContent(F("<input type='text' id='manufacturer' name='manufacturer' value='"));
  sendEscapedHtml(sysConfig.manufacturer);
  webServer.sendContent(F("'>"));

  // --- Language Selection ---
  webServer.sendContent(F("<label for='lang'>")); webServer.sendContent(MSG_CFG_LANGUAGE); webServer.sendContent(F("</label>"));
  webServer.sendContent(F("<select id='lang' name='lang' style='width:100%;padding:10px;margin-top:5px;border-radius:6px;border:1px solid #444;background:#2d2d2d;color:#fff'>"));
  webServer.sendContent(F("<option value='0'")); if (currentLanguage == LANG_IT) webServer.sendContent(F(" selected")); webServer.sendContent(F(">")); webServer.sendContent(MSG_LANG_IT); webServer.sendContent(F("</option>"));
  webServer.sendContent(F("<option value='1'")); if (currentLanguage == LANG_EN) webServer.sendContent(F(" selected")); webServer.sendContent(F(">")); webServer.sendContent(MSG_LANG_EN); webServer.sendContent(F("</option></select>"));

  // Buzzer Settings
  webServer.sendContent(F("<label for='buzzer_mode'>")); webServer.sendContent(MSG_CFG_BUZZER_MODE); webServer.sendContent(F("</label>"));
  webServer.sendContent(F("<select id='buzzer_mode' name='buzzer_mode' style='width:100%;padding:10px;margin-top:5px;border-radius:6px;border:1px solid #444;background:#2d2d2d;color:#fff'>"));
  webServer.sendContent(F("<option value='0'")); if (sysConfig.buzzerMode == BUZZER_MODE_CONTINUOUS) webServer.sendContent(F(" selected")); webServer.sendContent(F(">")); webServer.sendContent(MSG_BUZZER_MODE_CONTINUOUS); webServer.sendContent(F("</option>"));
  webServer.sendContent(F("<option value='1'")); if (sysConfig.buzzerMode == BUZZER_MODE_PULSE_SLOW) webServer.sendContent(F(" selected")); webServer.sendContent(F(">")); webServer.sendContent(MSG_BUZZER_MODE_PULSE_SLOW); webServer.sendContent(F("</option>"));
  webServer.sendContent(F("<option value='2'")); if (sysConfig.buzzerMode == BUZZER_MODE_PULSE_FAST) webServer.sendContent(F(" selected")); webServer.sendContent(F(">")); webServer.sendContent(MSG_BUZZER_MODE_PULSE_FAST); webServer.sendContent(F("</option></select>"));

  // --- Sensor Calibration (per-device, stored in EEPROM) ---
  webServer.sendContent(F("<h2>")); webServer.sendContent(MSG_CFG_CALIB_SETTINGS); webServer.sendContent(F("</h2>"));
  webServer.sendContent(F("<p class='small' style='margin-top:0'>")); webServer.sendContent(MSG_CFG_CALIB_HINT); webServer.sendContent(F("</p>"));
  webServer.sendContent(F("<label for='cal_offset_v'>")); webServer.sendContent(MSG_CFG_CAL_OFFSET_V); webServer.sendContent(F("</label><input type='number' step='0.01' id='cal_offset_v' name='cal_offset_v' value='"));
  dtostrf(sysConfig.calOffsetV, 1, 2, valBuf); webServer.sendContent(valBuf); webServer.sendContent(F("'>"));
  webServer.sendContent(F("<label for='cal_scale_v'>")); webServer.sendContent(MSG_CFG_CAL_SCALE_V); webServer.sendContent(F("</label><input type='number' step='0.001' id='cal_scale_v' name='cal_scale_v' value='"));
  dtostrf(sysConfig.calScaleV, 1, 3, valBuf); webServer.sendContent(valBuf); webServer.sendContent(F("'>"));
  webServer.sendContent(F("<label for='cal_offset_a'>")); webServer.sendContent(MSG_CFG_CAL_OFFSET_A); webServer.sendContent(F("</label><input type='number' step='0.01' id='cal_offset_a' name='cal_offset_a' value='"));
  dtostrf(sysConfig.calOffsetA, 1, 2, valBuf); webServer.sendContent(valBuf); webServer.sendContent(F("'>"));
  webServer.sendContent(F("<label for='cal_scale_a'>")); webServer.sendContent(MSG_CFG_CAL_SCALE_A); webServer.sendContent(F("</label><input type='number' step='0.001' id='cal_scale_a' name='cal_scale_a' value='"));
  dtostrf(sysConfig.calScaleA, 1, 3, valBuf); webServer.sendContent(valBuf); webServer.sendContent(F("'>"));
  webServer.sendContent(F("<label for='cal_offset_temp'>")); webServer.sendContent(MSG_CFG_CAL_OFFSET_TEMP); webServer.sendContent(F("</label><input type='number' step='0.1' id='cal_offset_temp' name='cal_offset_temp' value='"));
  dtostrf(sysConfig.calOffsetTemp, 1, 1, valBuf); webServer.sendContent(valBuf); webServer.sendContent(F("'>"));

  // --- WiFi Settings ---
  webServer.sendContent(F("<h2>")); webServer.sendContent(MSG_CFG_WIFI_SETTINGS); webServer.sendContent(F("</h2>"));
  webServer.sendContent(F("<label for='ssid'>")); webServer.sendContent(MSG_CFG_SSID); webServer.sendContent(F("</label><input type='text' id='ssid' name='ssid' value='")); 
  if (sysConfig.ssid[0] != '\0') sendEscapedHtml(sysConfig.ssid);
  webServer.sendContent(F("'>"));
  webServer.sendContent(F("<label for='password'>")); webServer.sendContent(MSG_CFG_PASSWORD); webServer.sendContent(F("</label><input type='password' id='password' name='password' value='' placeholder='"));
  webServer.sendContent(MSG_CFG_KEEP_CURRENT_PLACEHOLDER);
  webServer.sendContent(F("'>"));

  // --- Static IP Settings with dynamic display based on whether static IP is enabled or not in the configuration. If static IP is enabled, the fields for entering static IP configuration will be shown; otherwise, they will be hidden. This allows for a cleaner user interface that only shows relevant options based on the user's configuration choices.
  webServer.sendContent(F("<p style='margin-top:20px'><label for='staticIP' style='display:inline-block;margin-top:0'>")); webServer.sendContent(MSG_CFG_STATIC_IP_CHECK); webServer.sendContent(F("</label>"));
  webServer.sendContent(F("<input type='checkbox' id='staticIP' name='staticIP' ")); if (sysConfig.staticIP) webServer.sendContent(F("checked")); webServer.sendContent(F(" onclick='toggleStaticIP()'></p><div id='static-ip-fields'>"));
  webServer.sendContent(F("<label for='ip'>")); webServer.sendContent(MSG_CFG_IP_ADDR); webServer.sendContent(F("</label><input type='text' id='ip' name='ip' value='")); sendEscapedHtml(sysConfig.static_ip); webServer.sendContent(F("'>"));
  webServer.sendContent(F("<label for='subnet'>")); webServer.sendContent(MSG_CFG_SUBNET); webServer.sendContent(F("</label><input type='text' id='subnet' name='subnet' value='")); sendEscapedHtml(sysConfig.static_subnet); webServer.sendContent(F("'>"));
  webServer.sendContent(F("<label for='gateway'>")); webServer.sendContent(MSG_CFG_GATEWAY); webServer.sendContent(F("</label><input type='text' id='gateway' name='gateway' value='")); sendEscapedHtml(sysConfig.static_gateway); webServer.sendContent(F("'>"));
  webServer.sendContent(F("<label for='dns'>")); webServer.sendContent(MSG_CFG_DNS); webServer.sendContent(F("</label><input type='text' id='dns' name='dns' value='")); sendEscapedHtml(sysConfig.static_dns); webServer.sendContent(F("'></div>"));

  // --- MQTT Settings ---
  webServer.sendContent(F("<h2>")); webServer.sendContent(MSG_CFG_MQTT_SETTINGS); webServer.sendContent(F("</h2>"));
  webServer.sendContent(F("<label for='mqtt_server'>")); webServer.sendContent(MSG_CFG_BROKER_IP); webServer.sendContent(F("</label><input type='text' id='mqtt_server' name='mqtt_server' value='")); 
  if (sysConfig.mqtt_server[0] != '\0') sendEscapedHtml(sysConfig.mqtt_server);
  webServer.sendContent(F("'>"));
  webServer.sendContent(F("<label for='mqtt_port'>")); webServer.sendContent(MSG_CFG_PORT); webServer.sendContent(F("</label><input type='number' id='mqtt_port' name='mqtt_port' value='")); 
  snprintf(valBuf, sizeof(valBuf), "%d", sysConfig.mqtt_port);
  webServer.sendContent(valBuf); webServer.sendContent(F("'>"));
  webServer.sendContent(F("<label for='mqtt_user'>")); webServer.sendContent(MSG_CFG_USER); webServer.sendContent(F("</label><input type='text' id='mqtt_user' name='mqtt_user' value='")); 
  if (sysConfig.mqtt_user[0] != '\0') sendEscapedHtml(sysConfig.mqtt_user);
  webServer.sendContent(F("'>"));
  webServer.sendContent(F("<label for='mqtt_pass'>")); webServer.sendContent(MSG_CFG_PASS); webServer.sendContent(F("</label><input type='password' id='mqtt_pass' name='mqtt_pass' value='' placeholder='"));
  webServer.sendContent(MSG_CFG_KEEP_CURRENT_PLACEHOLDER);
  webServer.sendContent(F("'>"));

  // --- Home Assistant Settings ---
  webServer.sendContent(F("<h2>")); webServer.sendContent(MSG_CFG_HA_SETTINGS); webServer.sendContent(F("</h2>"));
  webServer.sendContent(F("<label for='ha_prefix'>")); webServer.sendContent(MSG_CFG_HA_PREFIX); webServer.sendContent(F("</label>"));
  webServer.sendContent(F("<input type='text' id='ha_prefix' name='ha_prefix' placeholder='" DEFAULT_HA_PREFIX "' value='"));
  sysConfig.ha_prefix[sizeof(sysConfig.ha_prefix) - 1] = '\0';             // Safety null-termination to prevent rendering crashes with dirty EEPROM
  sendEscapedHtml(sysConfig.ha_prefix);
  webServer.sendContent(F("'>"));

  // --- Web Access Settings ---
  webServer.sendContent(F("<h2>")); webServer.sendContent(MSG_WEB_ACCESS_SETTINGS); webServer.sendContent(F("</h2>"));
  webServer.sendContent(F("<label for='web_user'>")); webServer.sendContent(MSG_WEB_USERNAME);
  webServer.sendContent(F("</label><input type='text' id='web_user' name='web_user' maxlength='24' value='"));
  sendEscapedHtml(sysConfig.web_user);
  webServer.sendContent(F("'><label for='web_pass'>")); webServer.sendContent(MSG_WEB_PASSWORD);
  webServer.sendContent(F("</label><input type='password' id='web_pass' name='web_pass' maxlength='24' value='' placeholder='"));
  webServer.sendContent(MSG_CFG_KEEP_CURRENT_PLACEHOLDER);
  webServer.sendContent(F("'>"));

  // --- Action Buttons ---
  webServer.sendContent(F("<input type='submit' class='btn-red' value='")); webServer.sendContent(MSG_CFG_BTN_SAVE); webServer.sendContent(F("'/></form>"));
  webServer.sendContent(F("<form action='/reboot' method='post'><input type='submit' class='btn-red' value='")); webServer.sendContent(MSG_WEB_BTN_REBOOT); webServer.sendContent(F("' onclick=\"return confirm('Are you sure?');\"></form>"));
  webServer.sendContent(F("<form action='/update' method='get'><input type='submit' value='")); webServer.sendContent(MSG_WEB_LINK_UPDATE); webServer.sendContent(F("'></form>"));
  webServer.sendContent(F("<form action='/' method='get'><input type='submit' value='")); webServer.sendContent(MSG_CFG_BTN_BACK); webServer.sendContent(F("'></form>"));

  sendCommonFooter();                                                     // Send footer
}

// -------------------------------------------------------------------------------------------------- Web page - 404 Not Found
void handleNotFound(){
  webServer.send(404, "text/plain", "404: Not found");
}

// ------------------------------------------------------------------------- Configuration web page - Save settings and reboot
// This function is called when the user clicks the "Save" button on the configuration page and saves the configuration.
void handleSaveConfig() {
  if (!requireWebAuth()) return;
  // Lambda function to process string arguments safely and efficiently. Using const String& reference avoids unnecessary copies before c_str(). This ensures that the configuration values are correctly extracted from the web server's arguments and stored in the sysConfig structure, which is then saved to EEPROM for persistence across reboots.
  auto processArg = [](const char* key, char* dest, size_t size) {        // Helper lambda to process string arguments safely and efficiently
    if (webServer.hasArg(key)) {                                          // Check if the argument is present in the request
      const String& val = webServer.arg(key);                             // Get the argument value as a String reference to avoid unnecessary copies
      strncpy(dest, val.c_str(), size - 1);                               // Copy the string value to the destination buffer
      dest[size - 1] = '\0';                                              // Ensure null termination of the string to prevent buffer overflows
    }
  };
  auto parseIntInRange = [](const char* key, int minVal, int maxVal, int& outVal) -> bool {
    if (!webServer.hasArg(key)) return false;
    const String& raw = webServer.arg(key);
    if (raw.length() == 0) return false;

    char* endPtr = nullptr;
    long parsed = strtol(raw.c_str(), &endPtr, 10);
    if (endPtr == raw.c_str() || *endPtr != '\0' || parsed < minVal || parsed > maxVal) {
      Serial.printf("Rejected invalid %s: %s\n", key, raw.c_str());
      return false;
    }
    outVal = (int)parsed;
    return true;
  };
  auto parseIpArg = [](const char* key, IPAddress& outIp) -> bool {
    if (!webServer.hasArg(key)) return false;
    const String& raw = webServer.arg(key);
    if (!outIp.fromString(raw)) {
      Serial.printf("Rejected invalid %s: %s\n", key, raw.c_str());
      return false;
    }
    return true;
  };
  auto parseFloatInRange = [](const char* key, float minVal, float maxVal, float& outVal) -> bool {
    if (!webServer.hasArg(key)) return false;
    const String& raw = webServer.arg(key);
    if (raw.length() == 0) return false;

    char* endPtr = nullptr;
    float parsed = strtof(raw.c_str(), &endPtr);
    if (endPtr == raw.c_str() || *endPtr != '\0' || parsed < minVal || parsed > maxVal) {
      Serial.printf("Rejected invalid %s: %s\n", key, raw.c_str());
      return false;
    }
    outVal = parsed;
    return true;
  };

  // --- Update Configuration from Web Form ---
  processArg("name", sysConfig.device_name, sizeof(sysConfig.device_name));
  processArg("manufacturer", sysConfig.manufacturer, sizeof(sysConfig.manufacturer));
  int parsedValue;
  if (parseIntInRange("threshold", 0, MAX_POWER_THRESHOLD, parsedValue)) {
    sysConfig.powerThreshold = parsedValue;
  }
  processArg("ssid", sysConfig.ssid, sizeof(sysConfig.ssid));
  if (webServer.hasArg("password")) {
    const String& wifiPass = webServer.arg("password");
    if (wifiPass.length() > 0) {
      strncpy(sysConfig.password, wifiPass.c_str(), sizeof(sysConfig.password) - 1);
      sysConfig.password[sizeof(sysConfig.password) - 1] = '\0';
    }
  }

  // --- Static IP Configuration ---
  bool requestedStaticIP = webServer.hasArg("staticIP");
  if (requestedStaticIP) {
    IPAddress ip, gateway, subnet, dns;
    bool ipOk = parseIpArg("ip", ip);
    bool gatewayOk = parseIpArg("gateway", gateway);
    bool subnetOk = parseIpArg("subnet", subnet);
    bool dnsOk = parseIpArg("dns", dns);

    if (ipOk && gatewayOk && subnetOk && dnsOk) {
      sysConfig.staticIP = true;
      strncpy(sysConfig.static_ip, ip.toString().c_str(), sizeof(sysConfig.static_ip) - 1);
      sysConfig.static_ip[sizeof(sysConfig.static_ip) - 1] = '\0';
      strncpy(sysConfig.static_gateway, gateway.toString().c_str(), sizeof(sysConfig.static_gateway) - 1);
      sysConfig.static_gateway[sizeof(sysConfig.static_gateway) - 1] = '\0';
      strncpy(sysConfig.static_subnet, subnet.toString().c_str(), sizeof(sysConfig.static_subnet) - 1);
      sysConfig.static_subnet[sizeof(sysConfig.static_subnet) - 1] = '\0';
      strncpy(sysConfig.static_dns, dns.toString().c_str(), sizeof(sysConfig.static_dns) - 1);
      sysConfig.static_dns[sizeof(sysConfig.static_dns) - 1] = '\0';
    } else {
      sysConfig.staticIP = false;                                            // Fail safe: disable static mode if any field is invalid
      Serial.println(F("Static IP disabled due to invalid configuration."));
    }
  } else {
    sysConfig.staticIP = false;
  }

  // --- MQTT Configuration ---
  processArg("mqtt_server", sysConfig.mqtt_server, sizeof(sysConfig.mqtt_server));
  if (parseIntInRange("mqtt_port", 1, 65535, parsedValue)) {
    sysConfig.mqtt_port = parsedValue;
  }
  processArg("mqtt_user", sysConfig.mqtt_user, sizeof(sysConfig.mqtt_user));
  if (webServer.hasArg("mqtt_pass")) {
    const String& mqttPass = webServer.arg("mqtt_pass");
    if (mqttPass.length() > 0) {
      strncpy(sysConfig.mqtt_pass, mqttPass.c_str(), sizeof(sysConfig.mqtt_pass) - 1);
      sysConfig.mqtt_pass[sizeof(sysConfig.mqtt_pass) - 1] = '\0';
    }
  }
  processArg("ha_prefix", sysConfig.ha_prefix, sizeof(sysConfig.ha_prefix));
  processArg("web_user", sysConfig.web_user, sizeof(sysConfig.web_user));
  if (webServer.hasArg("web_pass")) {
    const String& wp = webServer.arg("web_pass");
    if (wp.length() > 0) {
      strncpy(sysConfig.web_pass, wp.c_str(), sizeof(sysConfig.web_pass) - 1);
      sysConfig.web_pass[sizeof(sysConfig.web_pass) - 1] = '\0';
    }
  }
  
  // --- Language and Buzzer Mode ---
  if (parseIntInRange("lang", 0, LANG_COUNT - 1, parsedValue)) {
    sysConfig.language = (Language)parsedValue;
    setLanguage(sysConfig.language);
  }
  if (parseIntInRange("buzzer_mode", 0, BUZZER_MODE_COUNT - 1, parsedValue)) {
    sysConfig.buzzerMode = (BuzzerMode)parsedValue;
  }

  // --- Sensor calibration ---
  float parsedFloat;
  if (parseFloatInRange("cal_offset_v", CAL_OFFSET_V_MIN, CAL_OFFSET_V_MAX, parsedFloat)) {
    sysConfig.calOffsetV = parsedFloat;
  }
  if (parseFloatInRange("cal_scale_v", CAL_SCALE_MIN, CAL_SCALE_MAX, parsedFloat)) {
    sysConfig.calScaleV = parsedFloat;
  }
  if (parseFloatInRange("cal_offset_a", CAL_OFFSET_A_MIN, CAL_OFFSET_A_MAX, parsedFloat)) {
    sysConfig.calOffsetA = parsedFloat;
  }
  if (parseFloatInRange("cal_scale_a", CAL_SCALE_MIN, CAL_SCALE_MAX, parsedFloat)) {
    sysConfig.calScaleA = parsedFloat;
  }
  if (parseFloatInRange("cal_offset_temp", CAL_OFFSET_TEMP_MIN, CAL_OFFSET_TEMP_MAX, parsedFloat)) {
    sysConfig.calOffsetTemp = parsedFloat;
  }
  configSanitizeCalibration(sysConfig);

  // --- Save Configuration to EEPROM ---
  EEPROM.write(0, EEPROM_MAGIC);
  EEPROM.put(1, sysConfig);
  bool success = EEPROM.commit();

  // Start streaming response directly to established connection.
  webServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
  webServer.send(200, "text/html", "");
  webServer.sendContent(F("<html><head><meta http-equiv='refresh' content='"));
  char b[4]; snprintf(b,4,"%d",WEB_REFRESH_SAVE); webServer.sendContent(b);
  webServer.sendContent(F("; url=/' /><style>body{font-family:'Segoe UI',sans-serif;background:#121212;color:#e0e0e0;text-align:center;padding-top:50px;}h1{color:"));
  webServer.sendContent(success ? F("#fff") : F("#ff6b6b"));
  webServer.sendContent(F(";}</style></head><body><h1>"));
  webServer.sendContent(success ? MSG_SAVE_OK_TITLE : MSG_SAVE_ERR_TITLE);
  webServer.sendContent(F("</h1><p>"));
  webServer.sendContent(MSG_SAVE_REBOOTING);
  webServer.sendContent(F("</p></body></html>"));
  webServer.sendContent(""); 

  shouldReboot = true;                                                    // Set flag to reboot safely in the main loop after sending response
}

// -------------------------------------------------------------------------------- Reboot web page after saving configuration
// This function is called after saving the configuration to display a rebooting message and then trigger a safe reboot of the device
void handleReboot() {
  if (!requireWebAuth()) return;
  webServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
  webServer.send(200, "text/html", "");
  webServer.sendContent(F("<html><head><meta http-equiv='refresh' content='"));
  char b[4]; snprintf(b,4,"%d",WEB_REFRESH_REBOOT); webServer.sendContent(b);
  webServer.sendContent(F("; url=/' />"));
  webServer.sendContent(F("<style>body{font-family:'Segoe UI',sans-serif;background:#121212;color:#e0e0e0;text-align:center;padding-top:50px;}h1{color:#fff;}</style></head><body><h1>"));
  webServer.sendContent(MSG_SAVE_REBOOTING);
  webServer.sendContent(F("</h1></body></html>"));
  webServer.sendContent("");
  shouldReboot = true;                                                    // Set flag to reboot safely in the main loop after sending response
}

// ------------------------------------------------------------------------------------------------ Web page - Firmware Update
// This is the handler for the firmware update page ("/update") of the web interface, which allows users to upload a new firmware binary file to update the device.
void handleUpdate() {
  if (!requireWebAuth()) return;
  sendCommonHeader(MSG_WEB_LINK_UPDATE, false);

  webServer.sendContent(F("<h1>"));
  webServer.sendContent(MSG_WEB_LINK_UPDATE);
  webServer.sendContent(F("</h1>"));
  webServer.sendContent(F("<form method='POST' action='/update' enctype='multipart/form-data'>"));
  
  uint32_t maxSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
  char fBuf[10];
  dtostrf(maxSpace / 1024.0 / 1024.0, 1, 2, fBuf);
  char sizeBuf[40];
  snprintf(sizeBuf, sizeof(sizeBuf), " (Max: %s MB)", fBuf);

  webServer.sendContent(F("<p>Firmware (.bin):"));
  webServer.sendContent(sizeBuf);
  webServer.sendContent(F("</p>"));
  webServer.sendContent(F("<input type='file' name='update' style='width: 100%; padding: 10px; margin-top: 5px; background: #2d2d2d; color: #fff; border: 1px solid #444; border-radius: 6px;'>"));
  webServer.sendContent(F("<input type='submit' class='btn-red' value='"));
  webServer.sendContent(MSG_WEB_LINK_UPDATE);
  webServer.sendContent(F("'>"));
  webServer.sendContent(F("</form>"));
  webServer.sendContent(F("<form action='/' method='get'><input type='submit' value='"));
  webServer.sendContent(MSG_CFG_BTN_BACK);
  webServer.sendContent(F("'></form>"));

  sendCommonFooter();
}

// ----------------------------------------------------------------------------------------------------------- Firmware Upload
// This function handles the file upload process for firmware updates
void handleUpdateUpload() {
  if (!requireWebAuth()) return;
  uint32_t maxSketchSpace;                                                // Get the upload object from the web server
  HTTPUpload& upload = webServer.upload();                                // Check the status of the upload and handle accordingly
  if (upload.status == UPLOAD_FILE_START) {                               // When the upload starts, prepare for the firmware update process
    Serial.setDebugOutput(true);
    WiFiUDP::stopAll();                                                   // Stop any ongoing UDP communication to free up resources for the update process
    Serial.printf("Update: %s\n", upload.filename.c_str());
    maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;    // Calculate the maximum available space for the new firmware
    if (!Update.begin(maxSketchSpace)) {                                  // Start the update process with the calculated maximum sketch space
      Update.printError(Serial);                                          // If any error, print the error to serial console
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {                        // During the file upload
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {  // Write the uploaded chunk to flash memory and check if the write was successful
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_END) {                          // If upload completed, finalize process and check if it was successful
    if (Update.end(true)) {
      Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
    } else {
      Update.printError(Serial);
    }
    Serial.setDebugOutput(false);
  }
  yield();
}

// ---------------------------------------------------------------------------------------------------- Firmware Update Result
// This function is called after the firmware upload process is completed to display the result of the update (success or failure) and then trigger a safe reboot of the device if the update was successful
void handleUpdateResult() {
  if (!requireWebAuth()) return;
  webServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
  webServer.send(200, "text/html", "");
  webServer.sendContent(F("<html><head><meta http-equiv='refresh' content='5; url=/' /><style>body{font-family:'Segoe UI',sans-serif;background:#121212;color:#e0e0e0;text-align:center;padding-top:50px;}h1{color:#fff;}</style></head><body>"));
  if (Update.hasError()) {
    webServer.sendContent(F("<h1>Update Failed</h1><p>Error: "));
    char errBuf[12]; snprintf(errBuf, sizeof(errBuf), "%d", Update.getError());
    webServer.sendContent(errBuf); webServer.sendContent(F("</p>"));
  } else {
    webServer.sendContent(F("<h1>Update Success</h1><p>Rebooting...</p>"));
  }
  webServer.sendContent(F("</body></html>"));
  webServer.sendContent("");
  shouldReboot = true;
}
