#ifndef WEBPAGES_H
#define WEBPAGES_H

// ###########################################################################################################################
// Header for web server handlers.
// Declares functions for serving HTML pages and processing API requests.
//
// Author: CtrlAltJon
// Last Updated: January 2026
// Copyright (c) 2026 CtrlAltJon
// License: MIT
// The software is provided "as is", without warranty of any kind.
// This license notice must be included in any copy or portion of the software.
//
// ###########################################################################################################################

#include <ESP8266WebServer.h>                                             // Web Server Library for ESP8266

const char* getBuzzerModeName(int mode);                                  // Get Buzzer mode name

void handleRoot();                                                        // Manage root web page
void handleConfig();                                                      // Manage configuration web page
void handleNotFound();                                                    // Manage 404 Not Found web page
void handleSaveConfig();                                                  // Manage Save configuration and reboot web page
void handleReboot();                                                      // Manage Reboot web page
void handleUpdate();                                                      // Manage Update web page
void handleUpdateUpload();                                                // Manage Update upload web page
void handleUpdateResult();                                                // Manage Update result web page

#endif
