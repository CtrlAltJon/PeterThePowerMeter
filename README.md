# PeterThePowerMeter

A professional-grade Digital Power Meter based on ESP8266 and ADS1115 for real-time energy monitoring, featuring a color LCD, Web UI, and MQTT integration.

> [!WARNING]
> ### ⚠️ IMPORTANT SAFETY WARNING & DISCLAIMER
> **THIS PROJECT INVOLVES WORKING WITH HIGH VOLTAGE (230VAC).** High voltage can cause severe injury, electrocution, or death. Fire hazard is also present.
>
> This project is provided "as is", for educational and hobbyist purposes only. By using any part of this project, you acknowledge:
> * **Assumption of Risk:** You are solely responsible for your own safety.
> * **No Liability:** The author shall not be held liable for any damages (hardware failure, property damage, injury).
> * **Assembly:** Any errors in assembly or soldering are the user's responsibility.
> * **IF YOU ARE NOT EXPERIENCED WITH MAINS VOLTAGE, DO NOT ATTEMPT THIS PROJECT.**

---

## 1. Project Overview
PeterThePowerMeter is designed for real-time energy monitoring with an integrated acoustic alarm.
* **Core:** ESP8266 (ESP-12F) with Wi-Fi for IoT integration.
* **Sensing:** ADS1115 (16-bit ADC) for high-precision voltage and current readings.
* **Safety:** Galvanic isolation via ZMPT107-1 transformer and current sensing via SCT-013 clamp.
* **Monitoring:** Internal thermal tracking via TFPT1206L1002DV sensor.

## 2. Hardware Features
### 2.1 User Interface
* **Display:** 0.96" Color LCD (Vertical) for local monitoring.
* **Buttons:** * **Multi-function:** Short press (Page toggle) | Long press (Action).
    * **Reset:** Recessed pin-hole button connected to RST pin.
* **Buzzer:** Active piezo for software-defined power threshold alarms.

### 2.2 Power & Protection
| Input | Type | Protection / Notes |
| :--- | :--- | :--- |
| **J1 Terminal** | +5VDC | **No reverse polarity protection.** |
| **J2 USB** | +5VDC | CH340G for debug/flash. Dual Schottky protection. |
| **J3 AC** | 230VAC | Protected by 250mA T-Fuse & 3-resistor current limiter (1mA). |

**Note on AC Polarity:** It is mandatory to respect **Phase (L)** and **Neutral (N)**. The fuse is on the Phase line; inverting it keeps the circuit "live" even if the fuse blows.

### 2.3 Operational Limits
* **Grid:** 230VAC / 50Hz (Standard European Grid).
* **Compatibility:** Calibrated for 50Hz (20ms sampling). Use on 110-120V or 60Hz may require firmware/hardware adjustments.

---

## 3. Software & Connectivity
The device includes a built-in Web Server and natively supports **MQTT** (e.g., for Home Assistant).

### 3.1 LCD Page Map
1. **Monitor:** Power (kW), Temp (°C), Rolling Graph, Wi-Fi/MQTT status.
2. **Threshold:** Set alarm limit (cyclic 100W increments).
3. **Voltage:** Real-time V line monitoring.
4. **Buzzer Mode:** Toggle between Continuous, Slow, or Fast intermittent.
5. **Save:** Commit settings to non-volatile memory.
6. **System Info:** Hostname, SSID, IP, and Firmware version.
7. **Tools:** AP Mode, Reboot, and Factory Reset (with safety timer).

### 3.2 Web Interface
Access the device IP via browser (found on Page 6).
* **Home:** Real-time dashboard with Power (W), Voltage (V), Current (A), and Uptime.
* **Config:** Edit Hostname, MQTT settings, Static IP, and Language (IT/EN).
* **OTA Update:** Dedicated page for Over-The-Air firmware (.bin) flashing.

> [!NOTE]
> **Security:** The Web UI is password-less. It is highly recommended to use this device within an isolated IoT VLAN.

---

## 4. Manufacturing & Assembly
### 4.1 I2C Addressing (ADS1115)
The I2C address is set via **R16** and **R17** (0-Ohm bridges). **Only one should be populated.**
* **R17 (GND):** Address `0x48` (Default).
* **R16 (+3.3V):** Address `0x49`.

*Safety Design:* The PCB uses a shared common pad to prevent short-circuiting 3.3V to GND, but careful soldering is still required.

---

## 5. License
This project is licensed under the **MIT License**.

**Note on Attribution:** While the MIT license allows for free use, I kindly request that any clones, forks, or derivative works maintain a clear attribution to the original author (**CtrlAltJon**) and a link back to this repository.

---

## Author
**CtrlAltJon**
Contributions, issues, and feature requests are welcome!
