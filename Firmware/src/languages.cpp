// ###########################################################################################################################
// Implementation of the multi-language support system.
// It stores localized strings in PROGMEM to save RAM and provides functions to retrieve text based on the selected language.
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
#include "languages.h"                                                    // Language management header

Language currentLanguage = LANG_EN;                                       // Set default language

// Set Language --------------------------------------------------------------------------------------------------------------
void setLanguage(Language lang) {                                         // Set language function
  if (lang < LANG_COUNT) {                                                // Check if language is valid
    currentLanguage = lang;                                               // Set language
  }
}

const char* const messages[MSG_COUNT][LANG_COUNT] PROGMEM = {             // Messages array in PROGMEM to save RAM
                                                                          // IT, EN
    // Display LCD
  { "WiFi", "WiFi" },                                                     // WiFi
  { "MQTT", "MQTT" },                                                     // MQTT
  { "Soglia", "Threshold" },                                              // Threshold
  { "Salvato", "Saved" },                                                 // Saved
  { "Errore", "Error" },                                                  // Error
  { "Salva", "Save" },                                                    // Save
  { "Impostaz.", "Settings" },                                            // Settings
  { "Modo", "Buzzer" },                                                   // Buzzer mode Line 1
  { "Cicalino", "Mode" },                                                 // Buzzer mode Line 2

    // Page 6 Descriptions
  { "AP ON", "AP ON" },                                                   // AP ON message
  { "Avvia", "Start" },                                                   // AP mode line 1
  { "AP Wi-Fi", "AP Mode" },                                              // AP mode line 2
  { "Riavvia il", "Reboot" },                                             // Reboot Line 1
  { "dispositivo", "device" },                                            // Reboot Line 2
  { "Cancella", "Erase" },                                                // Erase Line 1
  { "tutti i dati", "all data" },                                         // Erase Line 2

    // Web Root
  { "Misuratore di Potenza", "Power Meter" },                             // Web title
  { "Stato", "Status" },                                                  // Web status
  { "Potenza", "Power" },                                                 // Web power
  { "Tensione di Rete", "Grid Voltage" },                                 // Web grid voltage
  { "Corrente", "Current" },                                              // Web current
  { "Temperatura", "Temperature" },                                       // Web Temperature
  { "Tempo di Attivita", "Uptime" },                                      // Web uptime
  { "Configurazione", "Configuration" },                                  // Web button config
  { "Riavvia", "Reboot" },                                                // Web button reboot
  { "Aggiorna pagina", "Refresh page" },                                  // Web link refresh
  { "Aggiorna Firmware", "Update Firmware" },                             // Web link update

    // Web Config
  { "Configurazione Power Meter", "Power Meter Config" },                 // Web config title
  { "Configurazione", "Configuration" },                                  // Web config header
  { "Impostazioni Generali", "General Settings" },                        // Web config generale settings
  { "Nome Dispositivo", "Device Name" },                                  // Web config device name
  { "Produttore", "Manufacturer" },                                       // Web config manufacturer
  { "Lingua", "Language" },                                               // Web config language
  { "Modalita Cicalino", "Buzzer Mode" },                                 // Web config buzzer mode
  { "Soglia Allarme (W)", "Alarm Threshold (W)" },                        // Web config alarm threshold
  { "Impostazioni WiFi", "WiFi Settings" },                               // Web config WiFi settings
  { "SSID", "SSID" },                                                     // Web config SSID
  { "Password", "Password" },                                             // Web config password
  { "Usa IP Statico", "Use Static IP" },                                  // Web static IP use
  { "Indirizzo IP", "IP Address" },                                       // Web config static IP
  { "Subnet Mask", "Subnet Mask" },                                       // Web config static subnet
  { "Gateway", "Gateway" },                                               // Web config static gateway
  { "DNS", "DNS" },                                                       // Web config static DNS
  { "Impostazioni MQTT", "MQTT Settings" },                               // Web config MQTT settings
  { "IP Broker", "Broker IP" },                                           // Web config broker IP
  { "Porta", "Port" },                                                    // Web config broker port
  { "Utente (Opzionale)", "User (Optional)" },                            // Web config MQTT user
  { "Password (Opzionale)", "Password (Optional)" },                      // Web config MQTT password
  { "Impostazioni Home Assistant", "Home Assistant Settings" },           // Web config HA settings
  { "Prefisso Discovery", "Discovery Prefix" },                           // Web config HA prefix
  { "Calibrazione sensori", "Sensor calibration" },                         // Web config calibration section
  { "valore = misura x scala + offset (scala 1 = nessuna correzione)", "value = reading x scale + offset (scale 1 = no correction)" }, // Calibration hint
  { "Offset tensione (V)", "Voltage offset (V)" },                        // Voltage offset
  { "Scala tensione", "Voltage scale" },                                  // Voltage scale
  { "Offset corrente (A)", "Current offset (A)" },                          // Current offset
  { "Scala corrente", "Current scale" },                                  // Current scale
  { "Offset temperatura (C)", "Temperature offset (C)" },                 // Temperature offset
  { "lascia vuoto per mantenere attuale", "Leave empty to keep current" }, // Keep current password placeholder
  { "Salva e Riavvia", "Save and Reboot" },                               // Web config save and reboot
  { "Torna alla Home", "Back to Main Page" },                             // Web config back to home

    // Save/Error
  { "Configurazione salvata.", "Configuration saved." },                  // Save success message
  { "Errore salvataggio.", "Error saving configuration." },               // Save error message
  { "Riavvio in 2 secondi...", "Rebooting in 2 seconds..." },             // Reboot message

    // Language Names
  { "Italiano", "Italiano" },                                             // Italian
  { "English", "English" },                                               // English

  // Buzzer Modes
  { "Continuo", "Continuous" },                                           // Continuous
  { "Lento", "Slow" },                                                    // Slow pulse
  { "Veloce", "Fast" },                                                   // Fast pulse

    // Web Access Settings
  { "Accesso Configurazione", "Web Interface Access" },                   // Web Access Settings
  { "Utente", "Username" },                                               // Username for web access configuration page
  { "Password", "Password" }                                              // Password for web access configuration page
};

// Get text message by ID, returns a pointer to the localized string based on the current language selection.
// If the ID is invalid, it returns a placeholder "?".
const __FlashStringHelper* getText(MsgId id) {                             // Get text message by ID
  if (id >= MSG_COUNT) return F("?");                                               // Invalid ID
  const char* ptr = (const char*)pgm_read_ptr(&messages[id][currentLanguage]);      // Get message pointer from PROGMEM
  return (const __FlashStringHelper*)ptr;                                  // Cast to Flash String Helper
}
