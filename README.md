# Infrastructure Independent Smart 12VDC-Power-System

## Overview:

The Infrastructure Independent Smart 12VDC Power System is designed to intelligently manage power delivery to up to four separate 12V devices or circuits. The heart of the system is an ESP32 microcontroller that controls a 4-channel relay with built-in current monitoring capabilities. The system utilizes an SSD1306 128x64 I2C OLED display to communicate real-time current draw from each connected device and serves as an overcurrent protection mechanism.

![IMG_20231213_184454603](https://github.com/CountZero1066/IOT-Smart-12VDC-Power-System/assets/32957102/0bd76f8b-f14f-4e42-a651-32813cece7c2)

## Key Features:
- Device Control: The ESP32-controlled 4-channel relay allows precise control over power delivery to individual 12V devices or circuits.
- Current Monitoring: The system provides real-time current monitoring for each connected device, offering insights into power consumption.
- OLED Display Interface: The SSD1306 OLED display acts as a visual interface, showcasing current draw information and serving as a vital component of overcurrent protection.
- Communication Protocol: In its current iteration, communication is facilitated by ESPnow, eliminating dependence on third-party infrastructure and ensuring robust and reliable communication.

## Previous Blynk Support:

Previous versions of the system supported Blynk IoT communication, providing an additional layer of connectivity and remote control. The evolution to ESPnow demonstrates a commitment to infrastructure independence.

## Hardware
![12V SCH](https://github.com/CountZero1066/IOT-Smart-12VDC-Power-System/assets/32957102/bfc9c4f2-52b3-44ee-a35f-a1a0388e5bc7)

- Screw Terminal Block
- ESP32 Dev kit
- 4x ACS712 current sensor Module
- 4 Channel Relay
- 128x64 SSD1306

## Software Structure
### Header Files:
  - bitmap_images.h: Defines graphics for the SSD1306 display.
  - graphics_coordinates.h: Specifies coordinates for drawn graphics.
### Main Code (main.cpp):
  - Utilizes the ESP32, relay, and OLED display to control power and display real-time information.
  ![12v menue ani](https://github.com/CountZero1066/IOT-Smart-12VDC-Power-System/assets/32957102/9f8b9da7-5e3a-489f-a634-f6dea34f078a)

![relay2](https://github.com/CountZero1066/IOT-Smart-12VDC-Power-System/assets/32957102/7175b496-60cd-41c4-952c-d33762d0c830)





