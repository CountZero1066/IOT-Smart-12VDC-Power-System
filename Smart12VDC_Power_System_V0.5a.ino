/*
Smart 12VDC Power System V0.5a
31-08-2022
Robert James Hastings
Github CountZero1066

   ###############################################################
   #              Smart 12VDC Power System V0.5a                  #
   #                                                             #
   #                                                             #
   #                                                             #
   ###############################################################
   Version Detail:
            -Current Measuring on outputs implmented
            -Transfers current measurements to Blynk
            -Wifi Credentials saved to flash memory
            -If the WiFi credentials neither work or are absent, the system can now request the WiFi
             credentials from the Central Node (connected to the C# app via serial com) via ESPnow  

   Hardware:
            ESP32(NodeMCU)
            4 Channel relay
            4x ACS712 20A Range Current Sensor
            AZDelivery I2C 0.96-inch OLED Display SSD1306 128x64
            12VDC @ 10A PSU
*/
//-------------------------------------------------------------------------------------------
//Libaries, Global Variables and Definitions
//-------------------------------------------------------------------------------------------
//#define BLYNK_TEMPLATE_ID           "###################"
//#define BLYNK_DEVICE_NAME           "###################"
#define BLYNK_AUTH_TOKEN              "###################"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <esp_now.h>
#include <Preferences.h>

String mem_ssid;
String mem_pass;
char auth[] = BLYNK_AUTH_TOKEN;

Preferences preferences;

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

TaskHandle_t Task_Blynk;
TaskHandle_t Task_Draw_Dispaly;

#define CHANNEL 1

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define splash_height   128
#define splash_width    64
#define LOGO_HEIGHT   51
#define LOGO_WIDTH    52

esp_now_peer_info_t slave;
esp_now_peer_info_t peerInfo;

uint8_t broadcastAddress[] = {0x08,0x3A,0xF2,0x6D,0x25,0x78};
uint8_t NewAddress[sizeof(broadcastAddress)];

int relay_1_pin = 13;
int relay_2_pin = 12;
int relay_3_pin = 14;
int relay_4_pin = 27;

int relay_status_1  = 1;
int relay_status_2  = 1;
int relay_status_3  = 1;
int relay_status_4  = 1;

const int A_sensor1 = 36;
const int A_sensor2 = 35;
const int A_sensor3 = 32;
const int A_sensor4 = 33;

double read_current_values[] = {0,0,0,0}; //initialized with zerod values as to avert Guru Meditation Error
boolean received_awaited_message = false;

//following arrays define the drawn graphics on the SSD1306 display
   int rec1_x[] = {0,0,0,0};
   int rec1_y[] = {0,16,32,48};
   int rec1_h[] = {16,16,16,16};
   int rec1_w[] = {64,64,64,64};
   
   int rec2_x[] = {64,64,64,64};
   int rec2_y[] = {0,16,32,48};
   int rec2_w[] = {96,96,96,96};
   int rec2_h[] = {16,16,16,16};

   int rec3_x[] = {96,96,96,96};
   int rec3_y[] = {0,16,32,48};
   int rec3_w[] = {32,32,32,32};
   int rec3_h[] = {16,16,16,16};
//the arrays below define the cursor position for the SSD1306 display
   int text_c1_x[] = {2,2,2,2};
   int text_c2_x[] = {70,70,70,70};
   int text_c3_x[] = {102,102,102,102};
   int text_c1_y[] = {5,21,37,53};
   int text_c2_y[] = {5,21,37,53};
   int text_c3_y[] = {5,21,37,53};
   

//-------------------------------------------------------------------------------------------
//Bitmap Images
//-------------------------------------------------------------------------------------------
static const unsigned char PROGMEM bitmap[] =
{
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x31, 0x80, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0xC0, 0x01, 0x80, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x70, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x47, 0x1C, 0x71, 0xC0, 0x00,
0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x47, 0x00, 0x71, 0xC0, 0x00,
0x07, 0xE0, 0x38, 0xC3, 0x87, 0xCF, 0xF6, 0x63, 0x9F, 0x8F, 0xC0, 0x07, 0x00, 0x01, 0xC0, 0x00,
0x07, 0xF0, 0x38, 0xC7, 0x8F, 0xEF, 0xF6, 0x73, 0x9F, 0xDF, 0xC0, 0x00, 0x1C, 0x00, 0x00, 0x00,
0x06, 0x38, 0x38, 0xC7, 0x8C, 0x61, 0x86, 0x73, 0xB9, 0xD8, 0xE0, 0x00, 0x3C, 0x00, 0x00, 0x00,
0x06, 0x38, 0x38, 0xC7, 0xCC, 0x61, 0x86, 0x7B, 0xB8, 0xD8, 0xE0, 0x38, 0xFC, 0x38, 0xE0, 0x00,
0x06, 0x38, 0x38, 0xC7, 0xCC, 0x61, 0x86, 0x7B, 0xB8, 0x1C, 0xE0, 0x38, 0xFC, 0x38, 0xE0, 0x00,
0x07, 0xF8, 0x3F, 0xC6, 0xCF, 0x01, 0x86, 0x7D, 0xB8, 0x0E, 0x00, 0x3F, 0xFC, 0x38, 0xE0, 0x00,
0x07, 0xF0, 0x3F, 0xC6, 0xC7, 0xC1, 0x86, 0x7F, 0xBB, 0xCF, 0x80, 0x3F, 0xFF, 0xFF, 0xE0, 0x00,
0x06, 0x70, 0x38, 0xCE, 0xC1, 0xE1, 0x86, 0x6F, 0xBB, 0xC3, 0xC0, 0x3F, 0xFF, 0xFF, 0xE0, 0x00,
0x06, 0x30, 0x38, 0xCC, 0xC0, 0xE1, 0x86, 0x6F, 0xBB, 0xC0, 0xE0, 0x3F, 0xFF, 0xFF, 0xE0, 0x00,
0x06, 0x38, 0x38, 0xCC, 0xEC, 0x61, 0x86, 0x67, 0xB8, 0xD8, 0xE0, 0x3F, 0xFF, 0xFF, 0xE0, 0x00,
0x06, 0x38, 0x38, 0xCF, 0xEC, 0x61, 0x86, 0x67, 0xB9, 0xDC, 0xE0, 0x3E, 0x07, 0x03, 0xE0, 0x00,
0x06, 0x38, 0x38, 0xCF, 0xEF, 0xE1, 0x86, 0x63, 0x9F, 0xDF, 0xC0, 0x3C, 0x07, 0x01, 0xE0, 0x00,
0x06, 0x38, 0x38, 0xDC, 0x67, 0xC1, 0x86, 0x63, 0x9F, 0x8F, 0xC0, 0x3C, 0x07, 0x01, 0xE0, 0x00,
0x06, 0x38, 0x38, 0xDC, 0x63, 0x81, 0x86, 0x63, 0x86, 0x03, 0x00, 0x3C, 0x07, 0x01, 0xE0, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x07, 0x01, 0xE0, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x07, 0x01, 0xE0, 0x00,
0x00, 0x07, 0x87, 0x83, 0xC0, 0x03, 0xC3, 0xC7, 0xC7, 0x80, 0x00, 0x3C, 0x07, 0x01, 0xE0, 0x00,
0x00, 0x04, 0x44, 0x46, 0x60, 0x24, 0x06, 0x21, 0x0C, 0xC0, 0x00, 0x3F, 0xFF, 0xFF, 0xE0, 0x00,
0x00, 0x04, 0x24, 0x24, 0x20, 0x24, 0x04, 0x21, 0x08, 0x40, 0x00, 0x3F, 0xFF, 0xFF, 0xE0, 0x00,
0x00, 0x04, 0x24, 0x24, 0x20, 0x24, 0x04, 0x21, 0x08, 0x40, 0x00, 0x3F, 0xFF, 0xFF, 0xE0, 0x00,
0x00, 0x04, 0x24, 0x24, 0x20, 0x24, 0x04, 0x01, 0x08, 0x00, 0x00, 0x0F, 0xFF, 0xFF, 0x80, 0x00,
0x00, 0x04, 0x24, 0x24, 0x20, 0x24, 0x04, 0x01, 0x08, 0x00, 0x00, 0x0F, 0xFF, 0xFF, 0x80, 0x00,
0x00, 0x04, 0x24, 0x44, 0x20, 0x23, 0xC4, 0x01, 0x07, 0x00, 0x00, 0x01, 0xF8, 0xFC, 0x00, 0x00,
0x00, 0x07, 0xC7, 0x84, 0x20, 0x24, 0x04, 0x01, 0x00, 0xC0, 0x00, 0x01, 0xF0, 0x7C, 0x00, 0x00,
0x00, 0x04, 0x04, 0x84, 0x20, 0x24, 0x04, 0x01, 0x00, 0x40, 0x00, 0x01, 0xF0, 0x7C, 0x00, 0x00,
0x00, 0x04, 0x04, 0x44, 0x24, 0x24, 0x04, 0x21, 0x08, 0x40, 0x00, 0x01, 0xFA, 0xFC, 0x00, 0x00,
0x00, 0x04, 0x04, 0x44, 0x24, 0x24, 0x04, 0x21, 0x08, 0x40, 0x00, 0x01, 0xFF, 0xFC, 0x00, 0x00,
0x00, 0x04, 0x04, 0x24, 0x24, 0x64, 0x04, 0x21, 0x08, 0x40, 0x00, 0x00, 0x7F, 0xF0, 0x00, 0x00,
0x00, 0x04, 0x04, 0x23, 0xC3, 0xC3, 0xE3, 0xC1, 0x07, 0x80, 0x00, 0x00, 0x7F, 0xF0, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x80, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x30, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4D, 0x90, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char github_bmp_logo [357] PROGMEM = {
  0x00, 0x00, 0x07, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x01, 
  0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 
  0xff, 0x80, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xe0, 
  0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x01, 0xf1, 0xff, 0xff, 0xf8, 0xf8, 0x00, 0x03, 
  0xe0, 0x7f, 0xff, 0xe0, 0x7c, 0x00, 0x07, 0xe0, 0x1f, 0xff, 0x80, 0x7e, 0x00, 0x0f, 0xe0, 0x00, 
  0x00, 0x00, 0x7f, 0x00, 0x0f, 0xe0, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x1f, 0xe0, 0x00, 0x00, 0x00, 
  0x7f, 0x80, 0x1f, 0xe0, 0x00, 0x00, 0x00, 0x7f, 0x80, 0x3f, 0xe0, 0x00, 0x00, 0x00, 0x7f, 0xc0, 
  0x3f, 0xe0, 0x00, 0x00, 0x00, 0x7f, 0xc0, 0x7f, 0xc0, 0x00, 0x00, 0x00, 0x3f, 0xe0, 0x7f, 0x80, 
  0x00, 0x00, 0x00, 0x1f, 0xe0, 0x7f, 0x80, 0x00, 0x00, 0x00, 0x1f, 0xe0, 0x7f, 0x80, 0x00, 0x00, 
  0x00, 0x1f, 0xe0, 0xff, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xf0, 0xff, 0x00, 0x00, 0x00, 0x00, 0x0f, 
  0xf0, 0xff, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xf0, 0xff, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xf0, 0xff, 
  0x00, 0x00, 0x00, 0x00, 0x0f, 0xf0, 0xff, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xf0, 0xff, 0x00, 0x00, 
  0x00, 0x00, 0x0f, 0xf0, 0xff, 0x80, 0x00, 0x00, 0x00, 0x1f, 0xf0, 0xff, 0x80, 0x00, 0x00, 0x00, 
  0x1f, 0xf0, 0xff, 0x80, 0x00, 0x00, 0x00, 0x1f, 0xf0, 0x7f, 0xc0, 0x00, 0x00, 0x00, 0x3f, 0xe0, 
  0x7f, 0xe0, 0x00, 0x00, 0x00, 0x7f, 0xe0, 0x7f, 0xf0, 0x00, 0x00, 0x00, 0xff, 0xe0, 0x7f, 0xf8, 
  0x00, 0x00, 0x01, 0xff, 0xe0, 0x3f, 0xfe, 0x00, 0x00, 0x07, 0xff, 0xc0, 0x3e, 0x3f, 0x80, 0x00, 
  0x1f, 0xff, 0xc0, 0x1f, 0x1f, 0xf0, 0x00, 0xff, 0xff, 0x80, 0x1f, 0x8f, 0xf0, 0x00, 0xff, 0xff, 
  0x80, 0x0f, 0xc7, 0xf0, 0x00, 0xff, 0xff, 0x00, 0x0f, 0xc3, 0xf0, 0x00, 0xff, 0xff, 0x00, 0x07, 
  0xe0, 0x00, 0x00, 0xff, 0xfe, 0x00, 0x03, 0xe0, 0x00, 0x00, 0xff, 0xfc, 0x00, 0x01, 0xf0, 0x00, 
  0x00, 0xff, 0xf8, 0x00, 0x00, 0xfc, 0x00, 0x00, 0xff, 0xf0, 0x00, 0x00, 0x7f, 0xf0, 0x00, 0xff, 
  0xe0, 0x00, 0x00, 0x3f, 0xf0, 0x00, 0xff, 0xc0, 0x00, 0x00, 0x1f, 0xf0, 0x00, 0xff, 0x80, 0x00, 
  0x00, 0x07, 0xf0, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x01, 0xf0, 0x00, 0xf8, 0x00, 0x00, 0x00, 0x00, 
  0x60, 0x00, 0x60, 0x00, 0x00
};

//-------------------------------------------------------------------------------------------
//Setup
//Set GPIO pin modes, start ESPnow, Check for WiFi credentials, Connect to Blynk, establish tasks, display splash screens
//-------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
   pinMode(relay_1_pin, OUTPUT);
   pinMode(relay_2_pin, OUTPUT);
   pinMode(relay_3_pin, OUTPUT);
   pinMode(relay_4_pin, OUTPUT);
   digitalWrite(relay_1_pin, HIGH);
   digitalWrite(relay_2_pin, HIGH);
   digitalWrite(relay_3_pin, HIGH);
   digitalWrite(relay_4_pin, HIGH);
   
   preferences.begin("wifi_cred", false);
   Serial.println(preferences.getString("flash_ssid", ""));
   Serial.println(preferences.getString("flash_password", ""));
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));       
  }   
      WiFi.mode(WIFI_STA);// Set device as a Wi-Fi Station     
    if (esp_now_init() == ESP_OK) {
         Serial.println("ESPNow Init Success");
      }
        else {
          Serial.println("err,ESPNow Init Failed");   
      ESP.restart();
        }
        
        esp_now_register_recv_cb(OnDataRecv);
        esp_now_register_send_cb(OnDataSent);
        esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 1;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  else if (esp_now_add_peer(&peerInfo) == ESP_OK){
    Serial.println("Peer added");
    return;
  }  
display.clearDisplay();
  show_splash_bitmap();
  draw_github_details();
  progress_bar();
  int n = (preferences.getString("flash_ssid", "")).length();
  char pref_ssid[n+1];
  (preferences.getString("flash_ssid", "")).toCharArray(pref_ssid, n+1);

  n = (preferences.getString("flash_password", "")).length();
  char pref_pass[n+1];
  (preferences.getString("flash_password", "")).toCharArray(pref_pass, n+1);
  
  Serial.println("Starting blynk");
  Blynk.begin(auth, pref_ssid, pref_pass); 
  //if the device cannot connect to the AP, the system will check the stored in memory AP credentials, test them and if 
  //unsuccessful, will request new AP credentials from the Central Node/C# application 
  if (Blynk.connected()!= true){
    check_for_WiFi_cred();
  }
  else{
    Serial.println("Blynk connection successful");
  }
  
  Draw_Visual_System_Interface(); 
  delay(1000);
          //create a task that will be executed in the Task_Blynk_code_code() function, with priority 1 and executed on core 0
          xTaskCreatePinnedToCore(
                            Task_Blynk_code,   /* Task function. */
                            "Task_Blynk",     /* name of task. */
                            10000,       /* Stack size of task */
                            NULL,        /* parameter of the task */
                            6,           /* priority of the task */
                            &Task_Blynk,      /* Task handle to keep track of created task */
                            0);          /* pin task to core 0 */                  
          delay(500); 
        
          //create a task that will be executed in the Task_Draw_Dispaly_code() function, with priority 1 and executed on core 1
          xTaskCreatePinnedToCore(
                            Task_Draw_Dispaly_code,   /* Task function. */
                            "Task_Draw_Dispaly",     /* name of task. */
                            10000,       /* Stack size of task */
                            NULL,        /* parameter of the task */
                            7,           /* priority of the task */
                            &Task_Draw_Dispaly,      /* Task handle to keep track of created task */
                            1);          /* pin task to core 1 */
              
}
//-------------------------------------------------------------------------------------------
//Check Flash for Existing Credentials
//Credentials stored in memory as name/value pairs via preferences.h
//-------------------------------------------------------------------------------------------
void check_for_WiFi_cred() {
  String mem_ssid = preferences.getString("flash_ssid", "");
  String mem_pass = preferences.getString("flash_password", "");
  delay(2000);
   //is anything saved to the memory addresses associated with the credentials
  if (mem_ssid.length() <= 1 || mem_pass.length() <= 1) {
    Serial.println("No credentials found");
    request_credentials();
  }
  else {
    Serial.println("Found Saved Credentials");
    Serial.println("Testing discovered Credentials");
    wifi_test_network(mem_ssid, mem_pass);
  }
}
//-------------------------------------------------------------------------------------------
//Test WiFi Credentials
//check if received AP credentials work when tested against the WiFi.begin function. If yes, then save credentials to memory as name/value pair via preferences.h
//-------------------------------------------------------------------------------------------

void wifi_test_network(String ssid, String wifipassword) {
  String mem_ssid = ssid;
  String mem_pass = wifipassword;
  mem_ssid.trim();
  mem_pass.trim();
  WiFi.begin(mem_ssid.c_str(), mem_pass.c_str());
  int attempt_loop = 0;
        while ((WiFi.status() != WL_CONNECTED) && (attempt_loop < 40)) { //test credentials, but provide method for failed attempt to escape loop
          Serial.println("Testing Wi-Fi conn");
          delay(10);
          attempt_loop++;
        }
            if (WiFi.status() == WL_CONNECTED) {
              Serial.println("Succeeded in connecting to the AP");
              preferences.putString("flash_ssid", mem_ssid.c_str());
              preferences.putString("flash_password", mem_pass.c_str());
              delay(100);
              WiFi.mode(WIFI_OFF);
            } else {
              Serial.println("Failed to connect to the AP");
              request_credentials();
            }
}
//-------------------------------------------------------------------------------------------
//Request WiFi AP credentials from Central Node
//Send request for WiFi SSID and password to C# application via ESPnow to the Central Node device which relays the message to the C# app through serial coms
//-------------------------------------------------------------------------------------------
void request_credentials(){
       String unit_mac = WiFi.macAddress();
       Send_message("spw,WiFi,", unit_mac);
       Serial.println("waiting for response from central node");
            while(!received_awaited_message){
              Serial.print(".");
              delay(50);
             }
}
//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
void loop() {
  vTaskDelete(NULL);
}
//-------------------------------------------------------------------------------------------
//First Task Loop
//Task running on core 0, handles Blynk interactions
//-------------------------------------------------------------------------------------------
void Task_Blynk_code( void * pvParameters ){
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());
      for(;;){
            vTaskDelay( pdMS_TO_TICKS( 10 ) );
            Blynk.run(); 
            delay(100);
            Blynk.virtualWrite(V4,read_current_values[0]);
            Blynk.virtualWrite(V5,read_current_values[1]);
            Blynk.virtualWrite(V6,read_current_values[2]);
            Blynk.virtualWrite(V7,read_current_values[3]);     
      } 
}
//-------------------------------------------------------------------------------------------
//Second Task Loop
//Task running on core 1, handles retrieving and displaying data to the SSD1306 OLED display
//-------------------------------------------------------------------------------------------
void Task_Draw_Dispaly_code( void * Parameters ){
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());
  unsigned long time_now = 0;
            for(;;){  
              vTaskDelay( pdMS_TO_TICKS( 10 ) );
              time_now = millis();
              Read_Current();    
              Update_Display();
              while(millis() < time_now + 800){
                //non blocking wait for 800ms
                //alows proper firing of ESPnow callback     
              }
            }
}
//-------------------------------------------------------------------------------------------
//Inbound ESPnow callback
//-------------------------------------------------------------------------------------------
void OnDataRecv(const uint8_t * mac,const uint8_t *incomingData, int len) 
{ 
     char* buff = (char*) incomingData;
     String buffStr = String(buff);
     buffStr.trim();
     Process_ESPnow_message(buffStr);
}
//-------------------------------------------------------------------------------------------
//Process inbound ESPnow Messages Logic Tree
//-------------------------------------------------------------------------------------------
void Process_ESPnow_message(String message_in){
  Serial.println("message in=" + message_in);
  String sub_strings[20];
  int sub_string_count = 0;
            while (message_in.length() > 0)
            {
              message_in.trim();
              int index = message_in.indexOf(',');
              if (index == -1) // No comma found
              {
                sub_strings[sub_string_count++] = message_in;
                break;
              }
              else
              {
                sub_strings[sub_string_count++] = message_in.substring(0, index);
                message_in = message_in.substring(index+1);
              }
            }
         for (int i = 0; i < sub_string_count; i++)
        {
          Serial.print(i);
          Serial.print(": \"");
          Serial.print(sub_strings[i]);
          Serial.println("\"");
        }
//request from C# application to determine if Networked Smart 12V Power System is connected to ESPnow network 
              if(sub_strings[0] == "tst"){
                String unit_mac = WiFi.macAddress();
                   Send_message("tst,spw ok,", unit_mac);
                     }
//Messaage contains Credentials for WiFi network, usedfor connecting to Blynk in setup
               else if(sub_strings[0] == "WiFi"){
                   received_awaited_message = true;
                   wifi_test_network(sub_strings[1], sub_strings[2]);
                     }  
}
//-------------------------------------------------------------------------------------------
//ESPnow outbound Callback
//-------------------------------------------------------------------------------------------
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) 
{   
     Serial.print("\r\nLast Packet Send Status:\t");
     Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
//-------------------------------------------------------------------------------------------
//Send Message via ESPnow
//-------------------------------------------------------------------------------------------
void Send_message(String outbound_message,String mac_ad)
{
   Serial.println("sending ESPnow message");
   String outgoing = outbound_message + mac_ad;
   uint8_t *buff = (uint8_t*) outgoing.c_str();
   size_t sizeBuff = sizeof(buff) * outgoing.length();
   esp_err_t result = esp_now_send(broadcastAddress, buff, sizeBuff);  
     Serial.print("Send Status: ");
        if (result == ESP_OK) {             
              Serial.println("Success");
        }        
        else if (result == ESP_ERR_ESPNOW_ARG) {
              Serial.println("err,Invalid Argument");
        }
        else if (result == ESP_ERR_ESPNOW_INTERNAL) {
              Serial.println("err,Internal Error");
        }
        else if (result == ESP_ERR_ESPNOW_NO_MEM) {
              Serial.println("err,No Memory");
        }
        else if (result == ESP_ERR_ESPNOW_NOT_FOUND) {
              Serial.println("err,Peer not found.");
        }
        else {
              Serial.println("err,Unknown Error");
        }         
}
//-------------------------------------------------------------------------------------------
//Inbound Blynk Instructions
//-------------------------------------------------------------------------------------------
BLYNK_WRITE(V0)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V0 to a variable 
  if(pinValue > 0){
    Serial.println("High " + pinValue);
    digitalWrite(relay_1_pin, HIGH);
    relay_status_1 = 1;
  }
  else{
     Serial.println("Low " + pinValue);
    digitalWrite(relay_1_pin, LOW);
     relay_status_1 = 0;
    // Blynk.virtualWrite(V0,0);
  }
}
BLYNK_WRITE(V1)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V0 to a variable  
  if(pinValue > 0){
    Serial.println("High " + pinValue);
    digitalWrite(relay_2_pin, HIGH);
     relay_status_2 = 1;
  }
  else{
     Serial.println("Low " + pinValue);
    digitalWrite(relay_2_pin, LOW);
     relay_status_2 = 0;
    // Blynk.virtualWrite(V1,0);
  }
}
BLYNK_WRITE(V2)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V0 to a variable  
  if(pinValue > 0){
    Serial.println("High " + pinValue);
    digitalWrite(relay_3_pin, HIGH);
     relay_status_3 = 1;   
  }
  else{
     Serial.println("Low " + pinValue);
    digitalWrite(relay_3_pin, LOW);
     relay_status_3 = 0;
    // Blynk.virtualWrite(V2,0);
  }
}
BLYNK_WRITE(V3)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V0 to a variable  
  if(pinValue > 0){
    Serial.println("High " + pinValue);
    digitalWrite(relay_4_pin, HIGH);
     relay_status_4 = 1;
  }
  else{
     Serial.println("Low " + pinValue);
    digitalWrite(relay_4_pin, LOW);
     relay_status_4 = 0;
    // Blynk.virtualWrite(V3,0);
  }
}
//-------------------------------------------------------------------------------------------
//Draw Bitmap Splashscreen
//-------------------------------------------------------------------------------------------
void show_splash_bitmap(void) {
  display.clearDisplay();
  display.drawBitmap(0, 0, bitmap, splash_height, splash_width, WHITE);
  display.display();
  delay(3000);
}
//-------------------------------------------------------------------------------------------
//Draw Bitmap image of Github Logo
//-------------------------------------------------------------------------------------------
void draw_github_details(void) {
  display.clearDisplay();
  display.drawBitmap(38, 0, github_bmp_logo,52,52,WHITE);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(26, 53);
  display.println("CountZero1066");
  display.display();
  delay(1500); //main program tasks have yet to be invoked, thus a blocking delay has no negative impact 
}
//-------------------------------------------------------------------------------------------
//Animated Loading Screen
//-------------------------------------------------------------------------------------------
void progress_bar(){  
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(22, 20);
  display.println("LOADING");
  display.display();
  display.drawRect(0, 44, 128, 20, WHITE);
  display.display();
  delay(1000);  
      for(int i = 0; i <= 125; i = i + 5){
        display.fillRect(2, 46, i, 16, WHITE); 
        display.display();
        delay(50);
      }
  delay(1000);
}
//-------------------------------------------------------------------------------------------
//Draw Interface for SSD1306 OLED Display
//-------------------------------------------------------------------------------------------
void Draw_Visual_System_Interface(){  
  display.clearDisplay();
  display.setTextSize(1);
  //Definition for the SSD1306 drawn graphics
  //Fout rows of rectangles of 16 pixel height,each with two other embedded rectangles.
  //The final rectangle, unlike the preceding two rectangles,is drawn filled. As such, the text occupying this-
  //-final rectangle is drawn in BLACK,as to contrast with the WHITE background
       for(int i = 0; i <=3; i++){
          display.drawRect(rec1_x[i], rec1_y[i], rec1_w[i], rec1_h[i], WHITE); //x, y, width, height (x & y determine the position of the top left corner of the box)
          display.drawRect(rec2_x[i], rec2_y[i], rec2_w[i], rec2_h[i], WHITE);
          display.fillRect(rec3_x[i], rec3_y[i], rec3_w[i], rec3_h[i], WHITE);           
          display.setTextColor(WHITE);
          display.setCursor(text_c1_x[i],text_c1_y[i]);
          display.println("Circuit " + String(i + 1 ));        
       }   
  display.display();
}
//-------------------------------------------------------------------------------------------
//Update Display Data
//-------------------------------------------------------------------------------------------
void Update_Display(){
  display.setTextSize(1);
  int cct_stat[] =  {relay_status_1,relay_status_2,relay_status_3,relay_status_4,};
  String cct_status_array[4];   
     for (int n = 0; n<=3;n++){
      if (cct_stat[n] == 0){
          cct_status_array[n] = "ON ";    
      }
      else{
        cct_status_array[n] = "OFF";
      }
     }  
    for(int i = 0; i <=3; i++){
        display.setTextColor(WHITE,BLACK);        
        display.setCursor(text_c2_x[i], text_c2_y[i]);
        display.println(cct_status_array[i]);
        display.setTextColor(BLACK,WHITE);
        display.setCursor(text_c3_x[i], text_c3_y[i]);
        display.println(String(read_current_values[i],1));
  }
  display.display();
}
//-------------------------------------------------------------------------------------------
// Calculate Current Draw from each Relay Output
//-------------------------------------------------------------------------------------------
void Read_Current(){  
  int mA[3];
  mA[0] = analogRead(A_sensor1);
  mA[1] = analogRead(A_sensor2);
  mA[2] = analogRead(A_sensor3);
  mA[3] = analogRead(A_sensor4);
      for (int i =0; i<=3; i++){
        read_current_values[i] = ((mA[i] * 3.3/4095) -2.5)/0.185;
        if(read_current_values[i] < 0){
          read_current_values[i] = 0;
        }
        else if(read_current_values[i] >9){
          read_current_values[i] = 9;
        }
      } 
}
//-------------------------------------------------------------------------------------------
