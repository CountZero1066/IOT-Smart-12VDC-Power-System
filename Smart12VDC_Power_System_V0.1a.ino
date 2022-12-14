/*
Smart 12VDC Power System V0.1
22-08-2022
Robert James Hastings
Github CountZero1066

   ###############################################################
   #              Smart 12VDC Power System V0.1                  #
   #                                                             #
   #                                                             #
   #                                                             #
   ###############################################################
   Version Detail:
            Refined SSD1306 display interface funtion with simple current calculating functionality. 
            Placeholder relay control included.
            Bylnk iot interface -absent
            ESPnow interface -absent
            Current limiting relay control -absent 

   Hardware:
            ESP32(NodeMCU)
            4 Channel relay
            4x ACS712 20A Range Current Sensor
            AZDelivery I2C 0.96-inch OLED Display SSD1306 128x64
            12VDC @ 10A PSU
*/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

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
int relay_2_pin = 13;
int relay_3_pin = 12;
int relay_4_pin = 14;

const int A_sensor1 = 34;
const int A_sensor2 = 35;
const int A_sensor3 = 32;
const int A_sensor4 = 33;

double current_draw[3];
int mVperAmp = 100;
double Voltage = 0;
double VRMS = 0;
double AmpsRMS = 0;

boolean cct_status_1,cct_status_2, cct_status_3, cct_status_4;

//following arrays define the drawn graphics on the SSD1306 display
   int rec1_x[] = {0,0,0,0};
   int rec1_y[] = {0,16,32,48};
   int rec1_h[] = {64,64,64,64};
   int rec1_w[] = {16,16,16,16};
   
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
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));   
  }
  display.clearDisplay();
  pinMode(relay_1_pin, OUTPUT);
  pinMode(relay_2_pin, OUTPUT);
  pinMode(relay_3_pin, OUTPUT);
  pinMode(relay_4_pin, OUTPUT);
  delay(1000);
  digitalWrite(relay_1_pin, LOW);
  digitalWrite(relay_2_pin, LOW);
  digitalWrite(relay_3_pin, LOW);
  digitalWrite(relay_4_pin, LOW);
  delay(1000);

show_splash_bitmap();
draw_github_details();
progress_bar();

}

//-------------------------------------------------------------------------------------------
//Main Loop
//-------------------------------------------------------------------------------------------
void loop() {
  
     current_draw[0] = read_A_sensor(analogRead(A_sensor1));
     current_draw[1] = read_A_sensor(analogRead(A_sensor2));
     current_draw[2] = read_A_sensor(analogRead(A_sensor3));
     current_draw[3] = read_A_sensor(analogRead(A_sensor4));
Visual_System_Interface(current_draw[0],current_draw[1],current_draw[2],current_draw[3]);

digitalWrite(relay_1_pin, HIGH);
digitalWrite(relay_1_pin, LOW);
delay(1000);
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
