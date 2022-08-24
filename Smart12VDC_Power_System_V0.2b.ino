/*
Smart 12VDC Power System V0.2b
24-08-2022
Robert James Hastings
Github CountZero1066

   ###############################################################
   #              Smart 12VDC Power System V0.2b                 #
   #                                                             #
   #                                                             #
   #                                                             #
   ###############################################################
   Version Detail:
            -Implemented basic BLYNK interface for remote relay control. 
            -Split the Visual_System_Interface() and Blynk.run() functions into seperate tasks that can operate
             parallel to each other, thus increasing responsivness of Blynk commands
            
             ESPnow interface -absent
             Current limiting relay control -absent 

   Hardware:
            ESP32(NodeMCU)
            4 Channel relay
            4x ACS712 20A Range Current Sensor
            AZDelivery I2C 0.96-inch OLED Display SSD1306 128x64
            12VDC @ 10A PSU
*/

#define BLYNK_TEMPLATE_ID           "TMPLUI7H-NqS"
#define BLYNK_DEVICE_NAME           "Quickstart Device"
#define BLYNK_AUTH_TOKEN            "Iyj8j1nEQprFHVfhnZfO-V7hoyU_9csf"


// Comment this out to disable prints and save space
#define BLYNK_PRINT Serial


#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char auth[] = BLYNK_AUTH_TOKEN;

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Vietnamese Sweatshop";
char pass[] = "Pass1066T23balls";

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

TaskHandle_t Task_Blynk;
TaskHandle_t Task_Draw_Dispaly;


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define splash_height   128
#define splash_width    64
#define LOGO_HEIGHT   51
#define LOGO_WIDTH    52

int relay_1_pin = 15;
int relay_2_pin = 22;
int relay_3_pin = 23;
int relay_4_pin = 5;

const int A_sensor1 = 34;
//const int A_sensor2 = 35;
//const int A_sensor3 = 32;
//const int A_sensor4 = 33;

double current_draw[3];
int mVperAmp = 100;
double Voltage = 0;
double VRMS = 0;
double AmpsRMS = 0;

boolean cct_status_1,cct_status_2, cct_status_3, cct_status_4;

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
//-------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println("Starting blynk");
  Blynk.begin(auth, ssid, pass);
  Serial.println("success");
   pinMode(relay_1_pin, OUTPUT);
   pinMode(relay_2_pin, OUTPUT);
   pinMode(relay_3_pin, OUTPUT);
   pinMode(relay_4_pin, OUTPUT);
  
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));   
  }
  
display.clearDisplay();
  show_splash_bitmap();
  draw_github_details();
  progress_bar();

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
    delay(500);
  

  
}
//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
void loop() {
  vTaskDelete(NULL);
}
//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
void Task_Blynk_code( void * pvParameters ){
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  for(;;){
        vTaskDelay( pdMS_TO_TICKS( 10 ) );
        Blynk.run();
    delay(10);       
  } 
}
//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
void Task_Draw_Dispaly_code( void * Parameters ){
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());

  for(;;){    
        vTaskDelay( pdMS_TO_TICKS( 10 ) );

   Visual_System_Interface(1,1,1,1);
    delay(100);      
  }
}
//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
BLYNK_CONNECTED()
{
  // Change Web Link Button message to "Congratulations!"
  Blynk.setProperty(V3, "offImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations.png");
  Blynk.setProperty(V3, "onImageUrl",  "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations_pressed.png");
  Blynk.setProperty(V3, "url", "https://docs.blynk.io/en/getting-started/what-do-i-need-to-blynk/how-quickstart-device-was-made");
}
//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
BLYNK_WRITE(V0)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V0 to a variable

  
  if(pinValue > 0){
    Serial.println("High " + pinValue);
    digitalWrite(relay_1_pin, HIGH);
  }
  else{
     Serial.println("Low " + pinValue);
    digitalWrite(relay_1_pin, LOW);
  }
}

BLYNK_WRITE(V1)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V0 to a variable

  
  if(pinValue > 0){
    Serial.println("High " + pinValue);
    digitalWrite(relay_2_pin, HIGH);
  }
  else{
     Serial.println("Low " + pinValue);
    digitalWrite(relay_2_pin, LOW);
  }
}

BLYNK_WRITE(V2)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V0 to a variable

  
  if(pinValue > 0){
    Serial.println("High " + pinValue);
    digitalWrite(relay_3_pin, HIGH);
  }
  else{
     Serial.println("Low " + pinValue);
    digitalWrite(relay_3_pin, LOW);
  }
}

BLYNK_WRITE(V3)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V0 to a variable

  
  if(pinValue > 0){
    Serial.println("High " + pinValue);
    digitalWrite(relay_4_pin, HIGH);
  }
  else{
     Serial.println("Low " + pinValue);
    digitalWrite(relay_4_pin, LOW);
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
  delay(3000);
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
//Interface for SSD1306 OLED Display
//-------------------------------------------------------------------------------------------

//Function takes four double datatype parameters
//Parameters represent measured current draw via the read_A_sensor() function
void Visual_System_Interface(double cct1_A,double cct2_A,double cct3_A, double cct4_A){
  
  display.clearDisplay();
  display.setTextSize(1);
  
  double cct_A[] = {cct1_A, cct2_A, cct3_A, cct4_A}; 
  String cct_status_array[3];

 for (int n = 0; n<=3;n++){
  if (cct_A[n] > 0){
    cct_status_array[0] = "ON";
    cct_status_array[0] = String(cct_A[n],2) + "A";
  }
  else{
    cct_status_array[0] = "NULL";
  }
 } 
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
        display.setCursor(text_c2_x[i], text_c2_y[i]);
        display.println(cct_status_array[i]);
      display.setTextColor(BLACK);
        display.setCursor(text_c3_x[i], text_c3_y[i]);
        display.println(String(cct_A[i]));    
   }
   
  display.display();
  delay(500);
}

//-------------------------------------------------------------------------------------------
// Calculate Current Draw from each Relay
//-------------------------------------------------------------------------------------------

int read_A_sensor(int sensor_read){

  int current_reading_out;
  Voltage = getVPP(sensor_read);
  VRMS = (Voltage/2.0) *0.707;
  AmpsRMS = ((VRMS * 1000)/mVperAmp)-17.67;
  current_reading_out = AmpsRMS;
  
  Serial.print(AmpsRMS);
  Serial.print(" Amps RMS  ---  ");

  return current_reading_out;
 
}

float getVPP(int sensor)
{
  float result;
  int readValue;                // value read from the sensor
  int maxValue = 0;             // store max value here
  int minValue = 4096;          // store min value here ESP32 ADC resolution
  
   uint32_t start_time = millis();
   while((millis()-start_time) < 250) //sample for 250ms
   {
       readValue = sensor;
       // see if you have a new maxValue
       if (readValue > maxValue) 
       {
           /*record the maximum sensor value*/
           maxValue = readValue;
       }
       if (readValue < minValue) 
       {
           /*record the minimum sensor value*/
           minValue = readValue;
       }
   }
   
   // Subtract min from max
   result = ((maxValue - minValue) * 5)/4096.0; //ESP32 ADC resolution 4096
      
   return result;
 }
