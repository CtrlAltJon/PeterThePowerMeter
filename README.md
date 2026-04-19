# POWER METER 
### with ESP8266 and ADS1115

## Disclaimer

> [!CAUTION]
> ### ⚠️ IMPORTANT SAFETY WARNING & DISCLAIMER
> **THIS PROJECT INVOLVES WORKING WITH HIGH VOLTAGE (230VAC). HIGH VOLTAGE CAN CAUSE SEVERE INJURY, ELECTROCUTION, OR DEATH. FIRE HAZARD IS ALSO PRESENT.**
>
> This project is provided "as is", for educational and hobbyist purposes only. By using any part of this project (including hardware designs, PCB layouts, and software), you acknowledge and agree to the following:
>
> * **Assumption of Risk:** You are solely responsible for your own safety and for any damage caused by the assembly, testing, or use of this device.
> * **No Liability:** The author(s) shall not be held liable for any direct or indirect damages, including but not limited to: hardware failure, data loss, property damage (fire, short circuits), personal injury, or death, resulting from the use or misuse of this project.
> * **Assembly & Modification:** Any errors in assembly, soldering (e.g., solder bridges), component selection, firmware modification or others actions are the sole responsibility of the user.
> * **No Warranty:** There is no guarantee regarding the accuracy of measurements, the stability of the software, or the safety of the hardware design for long-term use.
> * **Local Standards:** It is the user's responsibility to ensure that the device complies with local electrical codes and safety regulations.
>
> **IF YOU ARE NOT EXPERIENCED IN WORKING WITH MAINS VOLTAGE, DO NOT ATTEMPT TO BUILD THIS PROJECT.**

---

## Assembly Layout: TOP Layer
<p align="center">
  <a href="/Images/Guide/Guide_Assembly-Top.png">
    <img src="/Images/Guide/Guide_Assembly-Top.png" width="300" alt="Assembly Layout Top">
  </a>
</p>

## Assembly Layout: BOTTOM Layer
<p align="center">
  <a href="/Images/Guide/Guide_Assembly-Bottom.png">
    <img src="/Images/Guide/Guide_Assembly-Bottom.png" width="300" alt="Assembly Layout Bottom">
  </a>
</p>
---

## Revision History
* **00:** First release (04/2026)

---

## Design Rationale & Project Overview

### 1.1 System Architecture
This project is a Digital Power Meter designed for real-time energy monitoring with acoustic alarm when a threshold is exceeded. The system architecture is based on the ESP8266 (ESP-12F) microcontroller, leveraging its Wi-Fi capabilities for IoT integration and an ADS1115 to read voltage and current. For system health, an onboard TFPT1206L1002DV temperature sensor is included to monitor the internal thermal conditions of the enclosure.

For local interaction, the device features a 0.96" color LCD display that provides real-time data visualization and configuration menus. Users can navigate through multiple data pages and perform basic setup tasks using a multi-function physical button. Additionally, the system hosts a built-in Web Server, allowing for advanced remote configuration, firmware updates (OTA), and detailed system management via an intuitive Web UI.

The measurement stage utilizes two primary sensors:
* **Voltage Sensing:** Handled via a dedicated voltage transformer ZMPT107-1 (TR1) for galvanic isolation and safety.
* **Current Sensing:** Implemented using the SCT-013 split-core current transformer (clamp meter).

Both sensors provide a 1VAC sinusoidal output. These signals are conditioned via a voltage divider network and sampled by an ADS1115 16-bit ADC, which communicates with the MCU via the I2C protocol.

### 1.2 Hardware Features & User Interface
To optimize GPIO usage, a PCF8574 I/O expander is employed for digital signal management and future use. The user interface consists of:
* **Buttons:** A multi-function navigation button (Short press: Page toggle | Long press: Confirm/Action) and a dedicated Pin-hole Hardware Reset button connected directly to Pin 1 (RST) of the ESP8266.
* **Acoustic Feedback:** An active piezoelectric buzzer triggers an audible alarm when a software-defined power threshold is exceeded.
* **Thermal Monitoring:** An onboard TFPT1206L1002DV temperature sensor (PTC thermistor) is integrated into the circuit. This component allows the system to monitor the internal temperature of the enclosure.
* **Display:** A 0.96" Color LCD for local monitoring and basic interaction.
* **Web Server:** A Web User Interface for remote monitoring and full system configuration.

### 1.3 Power Supply & Protection
The device supports 2 power input options and 1 input 230VAC for Voltage reading:

* **Terminal Block (J1):** Accepts a +5VDC regulated supply.
    * *Warning - No reverse polarity protection:* This input currently lacks reverse polarity protection; correct orientation is mandatory.
    <p align="center">
      <a href="/Images/Guide/Guide_J1.png">
        <img src="/Images/Guide/Guide_J1.png" width="200" alt="J1 No reverse polarity protection">
      </a>
    </p>
* **USB Connector (J2):** For secondary power or debugging.
    * *Communication & Debugging:* This interface utilizes a CH340G USB-to-Serial bridge and a UMH3NFHATN transistor pair to communicate with the ESP8266. This setup enables full debugging, serial communication, and firmware programming (automatic reset/bootloader entry).
    * *Safety Feature:* The board implements a dual Schottky diode protection circuit on the positive rails of both J1 and J2. This prevents back-feeding, though it is recommended to disconnect J1 when using USB power.
    * *Overcurrent Protection:* The +5VDC main rail is protected by a MF-NSMF050-2 PTC (Resettable Fuse) rated at 500mA.
* **AC Voltage Input (J3):** High-voltage input for the ZMPT107-1 transformer.
    * *Polarity Recommendation:* Although the transformer operates on AC, it is strictly recommended to respect the Phase (L) and Neutral (N) markings.
    <p align="center">
      <a href="/Images/Guide/Guide_J3.png">
        <img src="/Images/Guide/Guide_J3.png" width="200" alt="J3 High-voltage input">
      </a>
    </p>
    * *Safety Rationale:* The protection fuse is located exclusively on the Phase line. Inverting the polarity would mean that, in the event of a fault or a blown fuse, the Neutral line would be disconnected while the entire internal circuit remains energized (live) through the Phase line. This creates a severe electrocution hazard even when the device appears to be non-functional.
    * *Safety Design:* The circuit features a 250mA T (Slow-blow) fuse placed exclusively on the Phase line. Additionally, the line is equipped with three resistors in series designed to limit the current to 1mA RMS at 230V, providing an essential layer of protection for the sensing stage.

### 1.4 Operational Limits & Grid Compatibility
This device is specifically designed and calibrated for 230VAC/50Hz Phase-Neutral grids (Standard European Grid).

* **Voltage Specificity:** The internal components, including the protection resistors and the ZMPT107-1 transformer ratio, are optimized for 230V. Use with different mains voltages (e.g., 110-120V) may result in inaccurate readings or insufficient signal-to-noise ratio.
* **Frequency Specificity:** The system is tuned for 50Hz operation. While the magnetic components (TR1 and CT) can physically handle 60Hz, the firmware's sampling window and RMS calculation algorithms are optimized for a 20ms period.
* **Warning:** Operation on different grid standards, high-voltage industrial lines, or Phase-to-Phase configurations (without a neutral line) may exceed the safety ratings of the components and the fuse, leading to hardware failure or fire hazard.

---

## 2.0 Connectivity & Software Integration
The firmware architecture features a built-in Web Server for real-time monitoring, local configuration, and connections management (Wi-Fi and MQTT). The device natively supports the MQTT protocol for a smart home integration with platforms such as Home Assistant.

### 2.0.1 LCD User Interface (Vertical Orientation)
Navigation is handled via the multi-function button (Short press: Page toggle | Long press: Confirm/Action).

* **Page 1: Real-time Monitor**
    * Power: Current power consumption displayed in kW.
    * Temperature: PCB temperature in °C.
    * Graph: A rolling history graph showing power trends (approx. 1-minute window).
    * Status Badges: Dynamic icons for Wi-Fi connection status and MQTT connection status.
* **Page 2: Power Threshold Configuration**
    * Displays the active alarm threshold (Watts).
    * Allows cyclic adjustment of the new threshold (100W increments) via long-press.
* **Page 3: Voltage Monitoring**
    * Displays the real-time mains voltage (V).
* **Page 4: Acoustic Alarm Settings**
    * Displays and cycles through buzzer modes: Continuous, Slow Intermittent, or Fast Intermittent.
* **Page 5: Save Settings**
    * Summarizes settings. Action: A long-press commits settings to non-volatile memory.
* **Page 6: System Information**
    * Displays network details: Hostname, SSID, IP Address, and Firmware Version.
* **Page 7: System Tools**
    * AP Mode, Reboot device, and Reset (Factory Reset). Requires a long-press with a visual timer-bar.

### 2.0.2 Web Interface & Configuration
The device features a responsive Web UI. Users can connect via browser to the device's IP (visible on LCD page 6).

#### Home Page (Real-Time Dashboard)
* **Connectivity Status:** Dynamic badges for AP Mode, Wi-Fi, and MQTT.
* **Live Telemetry:** Power (W), Voltage (V), Current (A).
* **System Uptime:** Tracks operational time in "Days hh:mm:ss" format.
* **Data Refresh:** Auto-refresh every 10 seconds or manual button.

#### Configuration Page
* **General Settings:** Hostname, Manufacturer, Language (IT/EN), Alarm Threshold, Buzzer Mode.
* **Wi-Fi Setup:** Credentials and IP Assignment (DHCP or Static IP).
* **MQTT Setup (Optional):** Broker IP, Port, and Authentication.
* **Buttons:** Save & Reboot, Reboot, Firmware Update (OTA), Home.

---

## Manufacturing & Assembly Notes

### 3.0 Hardware Configuration
**ADS1115 I2C Address Selection:** Hardware-defined through two "0-Ohm bridge" resistors: R16 and R17. Only one of these resistors must be populated at a time:
* **R16:** Connects the ADDR pin to +3.3V, ADS1115 Address `0x49`
* **R17:** Connects the ADDR pin to GND, ADS1115 Address `0x48` (Default).

**Safety Design:** To prevent assembly errors, the PCB layout utilizes a shared common pad for both R16 and R17. This physical constraint makes it nearly impossible to populate both resistors simultaneously, effectively protecting the +3.3V rail from a direct short circuit to Ground. However, be careful when assembling.

---

## Dimensions
<p align="center">
  <a href="/Images/Guide/Guide_Dimensions.png">
    <img src="/Images/Guide/Guide_Dimensions.png" width="300" alt="Dimensions">
  </a>
</p>
