/*
Infrastructure Independent Smart 12VDC Power System V0.1.01
Robert James Hastings
Github CountZero1066

   #######################################################################
   #     Infrastructure Independent Smart 12VDC Power System V0.1.01     #
   #    -------------------------------------------------------------    #
   #                                                                     #
   #                                                                     #
   #######################################################################

   Hardware:
            ESP32(NodeMCU)
            4 Channel relay
            4x ACS712 20A Range Current Sensor
            AZDelivery I2C 0.96-inch OLED Display SSD1306 128x64
            12VDC @ 10A PSU
*/
//-------------------------------------------------------------------------------------------
//Preprocessor directives,Libaries,Headers,Global Variables and Definitions
//-------------------------------------------------------------------------------------------
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Preferences.h>
#include "bitmap_images.h"
#include "graphics_coordinates.h"

#define CHANNEL 1
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define splash_height   128
#define splash_width    64
#define LOGO_HEIGHT   51
#define LOGO_WIDTH    52

TaskHandle_t Task_0;
TaskHandle_t Task_1;

Preferences preferences;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

esp_now_peer_info_t slave;
esp_now_peer_info_t peerInfo;

uint8_t NewAddress[6];
uint8_t Universal_Project_Interface_Address[] = {0x08,0x3A,0xF2,0x6D,0x25,0x78};

int relay_1_pin = 14;
int relay_2_pin = 27;
int relay_3_pin = 26;
int relay_4_pin = 25;

double current_correction[3];

int relay_status_1  = 1;
int relay_status_2  = 1;
int relay_status_3  = 1;
int relay_status_4  = 1;

const int A_sensor1 = 36;
const int A_sensor2 = 35;
const int A_sensor3 = 32;
const int A_sensor4 = 33;
double read_current_values[] = {0,0,0,0};
//-------------------------------------------------------------------------------------------
//Setup
//-------------------------------------------------------------------------------------------
void setup() {
  pinMode(relay_1_pin, OUTPUT);
   pinMode(relay_2_pin, OUTPUT);
   pinMode(relay_3_pin, OUTPUT);
   pinMode(relay_4_pin, OUTPUT);
   digitalWrite(relay_1_pin, HIGH);
   digitalWrite(relay_2_pin, HIGH);
   digitalWrite(relay_3_pin, HIGH);
   digitalWrite(relay_4_pin, HIGH);

 WiFi.mode(WIFI_STA);// Set device as a Wi-Fi Station
  WiFi.disconnect();

  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent); 

  memcpy(peerInfo.peer_addr, Universal_Project_Interface_Address, 6);
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
   
   if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
                  Serial.println(F("SSD1306 allocation failed"));       
                }
  display.clearDisplay();
  show_splash_bitmap();
  draw_github_details();
  progress_bar();
  for(int i=0; i<=3;i++){
    double zero_current = Read_Current(i);
    current_correction[i] = zero_current * -1;
  }
  Draw_Visual_System_Interface();
  delay(100);
  xTaskCreatePinnedToCore(
                            Task_Core_0,   /* Task function. */
                            "Task_0",     /* name of task. */
                            16000,       /* Stack size of task */
                            NULL,        /* parameter of the task */
                            6,           /* priority of the task */
                            &Task_0,      /* Task handle to keep track of created task */
                            0);          /* pin task to core 0 */                  
        
          //create a task that will be executed in the Task_Draw_Dispaly_code() function, with priority 1 and executed on core 1
          xTaskCreatePinnedToCore(
                            Task_Core_1,   /* Task function. */
                            "Task_1",     /* name of task. */
                            10000,       /* Stack size of task */
                            NULL,        /* parameter of the task */
                            7,           /* priority of the task */
                            &Task_1,      /* Task handle to keep track of created task */
                            1);          /* pin task to core 1 */
}
//-------------------------------------------------------------------------------------------
//Main Loop
//-------------------------------------------------------------------------------------------
//Core0
//-------------------------------------------------------------------------------------------
void Task_Core_0( void * pvParameters )
{
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());
  for(;;){ 

      delay(100);             
  }
  vTaskDelay(10);
}
//-------------------------------------------------------------------------------------------
//Core1 Loop
//-------------------------------------------------------------------------------------------
void Task_Core_1( void * Parameters )
{
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());
  
  for(;;){ 
    Update_Display();
    delay(500);
     }
     vTaskDelay( pdMS_TO_TICKS( 150 ) );
  }
void loop() {
  vTaskDelete(NULL);

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
        display.println(String(Read_Current(i)));
  }
  display.display();
}
//-------------------------------------------------------------------------------------------
// Calculate Current Draw from each Relay Output
//-------------------------------------------------------------------------------------------
double Read_Current(int cct_to_check){  
  double cur_val = 0;
  float average_current = 0;
  for(int i = 0; i<<1000; i++){
  switch(cct_to_check){
    case 0:
      cur_val = analogRead(A_sensor1);
    break;
    case 1:
      cur_val = analogRead(A_sensor2);
    break;
    case 2:
      cur_val = analogRead(A_sensor3);
    break;
    case 3:
      cur_val = analogRead(A_sensor4);
    break;
  }
  average_current = average_current + (0.19 * cur_val -25)/1000;
  delay(1);
  }

      return average_current;
}
/*___________________________________________________________________________________________
 *   ______  _____ _____    _   _               
    |  ____|/ ____|  __ \  | \ | |              
    | |__  | (___ | |__) | |  \| | _____      __
    |  __|  \___ \|  ___/  | . ` |/ _ \ \ /\ / /
    | |____ ____) | |      | |\  | (_) \ V  V / 
    |______|_____/|_|      |_| \_|\___/ \_/\_/  
 */
//-------------------------------------------------------------------------------------------
//ESP Now 'data-sent' Callback
//-------------------------------------------------------------------------------------------
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) 
{ 
  Serial.print("Last Packet Send Status: ");
  if ((status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail") == "Delivery Success" || (status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail") == "Success"){
    Serial.println("ESP Now message sent");
  }
  else{
    Serial.println("ESP Now send Error");
  }
}
//-------------------------------------------------------------------------------------------
//ESP Now 'data-received' Callback
//-------------------------------------------------------------------------------------------
void OnDataRecv(const uint8_t * mac,const uint8_t *incomingData, int len) 
{ 
     String sub_strings[20];
     int sub_string_count = 0;
     char* buff = (char*) incomingData;
     String buffStr = String(buff);
     buffStr.trim();
     Serial.print(buffStr);
     Serial.println("ESP Now data received");
     read_ESP_now_inbound(buffStr);
}
//-------------------------------------------------------------------------------------------
void read_ESP_now_inbound(String message){
  String sub_strings[20];
  int sub_string_count = 0;
            while (message.length() > 0)
            {
              message.trim();
              int index = message.indexOf(',');
              if (index == -1) // No comma found
              {
                sub_strings[sub_string_count++] = message;
                break;
              }
              else
              {
                sub_strings[sub_string_count++] = message.substring(0, index);
                message = message.substring(index+1);
              }
            }
                   //UPI,reqdata,MAC_address
              if(sub_strings[0] == "UPI"){
                   if(sub_strings[1] == "reqdata"){
                    initBroadcastSlave(sub_strings[2]);
                    int relay[] = {digitalRead(relay_1_pin),digitalRead(relay_2_pin),digitalRead(relay_3_pin),digitalRead(relay_4_pin)};
                    String payload = String(relay[0]) + "," + String(relay[1]) + "," + String(relay[2])+ "," + String(relay[3]);
                    Send_message("status," + payload + "," + WiFi.macAddress());
                   }
                   //UPI,toggle,int,MAC_address
                   else if(sub_strings[1] == "toggle"){
                      switch((sub_strings[2]).toInt()){
                        case 1:
                          if(relay_status_1 == 0){
                            digitalWrite(relay_1_pin, HIGH);
                            relay_status_1 = 1;
                          }
                          else{
                            digitalWrite(relay_1_pin, LOW);
                            relay_status_1 = 0;
                          }
                        break;
                        case 2:
                          if(relay_status_2 == 0){
                            digitalWrite(relay_2_pin, HIGH);
                            relay_status_2 = 1;
                          }
                          else{
                            digitalWrite(relay_2_pin, LOW);
                            relay_status_2 = 0;
                          }
                        break;
                        case 3:
                          if(relay_status_3 == 0){
                            digitalWrite(relay_3_pin, HIGH);
                            relay_status_3 = 1;
                          }
                          else{
                            digitalWrite(relay_3_pin, LOW);
                            relay_status_3 = 0;
                          }
                        break;
                        case 4:
                          if(relay_status_4 == 0){
                            digitalWrite(relay_4_pin, HIGH);
                            relay_status_4 = 1;
                          }
                          else{
                            digitalWrite(relay_4_pin, LOW);
                            relay_status_4 = 0;
                          }
                        break;
                      }
                      initBroadcastSlave(sub_strings[3]);
                    int relay[] = {digitalRead(relay_1_pin),digitalRead(relay_2_pin),digitalRead(relay_3_pin),digitalRead(relay_4_pin)};
                    String payload = String(relay[0]) + "," + String(relay[1]) + "," + String(relay[2])+ "," + String(relay[3]);
                    delay(125);
                    Send_message("12V," + payload + "," + WiFi.macAddress());
                   }
           }
}
//-------------------------------------------------------------------------------------------
//Add ESP Now broadcast slave
//-------------------------------------------------------------------------------------------
void initBroadcastSlave(String temp_mac) 
{    
char mac_received[32]; 
temp_mac.toCharArray(mac_received,32);
byte byteMac[6];
byte index = 0;
char *token;
char *ptr;
const char colon[2] = ":";
//Split string into tokens
token = strtok(mac_received, colon);

            while ( token != NULL ){
              //Convert string to unsigned long integer
              byte tokenbyte = strtoul(token, &ptr, 16);
              byteMac[index] = tokenbyte;
              index++;
              token = strtok(NULL, colon);
            }
  //zero slave memory for rewrite          
  memset(&slave, 0, sizeof(slave));
  //add new address to slave
  for (int n = 0; n < 6; ++n) {
       slave.peer_addr[n] = (uint8_t)byteMac[n];
  }
  
  slave.channel = 1;
  slave.encrypt = 0;
  //check if slave added successfully
  manageSlave();
}
//-------------------------------------------------------------------------------------------
//Check if slave device paired
//-------------------------------------------------------------------------------------------
bool manageSlave() 
{
    const esp_now_peer_info_t *peer = &slave;
    const uint8_t *peer_addr = slave.peer_addr;
    // check if the peer exists
    bool exists = esp_now_is_peer_exist(peer_addr);
    if (exists) {
      // Slave already paired.
      return true;
    }
    else {
      // Slave not paired, attempt pair
      esp_err_t addStatus = esp_now_add_peer(peer);
      if (addStatus == ESP_OK) {
        Serial.println("pair success");
        // Pair success
        return true;
      }
      else if (addStatus == ESP_ERR_ESPNOW_NOT_INIT) {
        Serial.println("ESPNOW Not Init");
        return false;
      }
      else if (addStatus == ESP_ERR_ESPNOW_ARG) {
        Serial.println("Invalid Argument");
        return false;
      }
      else if (addStatus == ESP_ERR_ESPNOW_FULL) {
        Serial.println("Peer list full");
        return false;
      }
      else if (addStatus == ESP_ERR_ESPNOW_NO_MEM) {
        Serial.println("Out of memory");
        return false;
      }
      else if (addStatus == ESP_ERR_ESPNOW_EXIST) {
       Serial.println("Peer Exists");
        return true;
      }
      else {
        Serial.println("Not sure what happened");
        return false;
      }
    }
  }
//-------------------------------------------------------------------------------------------
//Send ESP Now message
//-------------------------------------------------------------------------------------------
void Send_message(String instruction)
{
  Serial.println("data to be sent = " + instruction);
   const uint8_t *peer_addr = slave.peer_addr;
   uint8_t *buff = (uint8_t*) instruction.c_str();
   size_t sizeBuff = sizeof(buff) * instruction.length();
   esp_err_t result = esp_now_send(peer_addr, buff, sizeBuff);  
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
/*__________________________________________________________________________________________
 *    ____  _   _                 ______                _   _                 
     / __ \| | | |               |  ____|              | | (_)                
    | |  | | |_| |__   ___ _ __  | |__ _   _ _ __   ___| |_ _  ___  _ __  ___ 
    | |  | | __| '_ \ / _ \ '__| |  __| | | | '_ \ / __| __| |/ _ \| '_ \/ __|
    | |__| | |_| | | |  __/ |    | |  | |_| | | | | (__| |_| | (_) | | | \__ \
     \____/ \__|_| |_|\___|_|    |_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|___/
 */
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
//Draw Bitmap Splashscreen
//-------------------------------------------------------------------------------------------
void show_splash_bitmap(void) 
{
  display.clearDisplay();
  display.display();
  display.drawBitmap(0, 0, splash_bmp, splash_height, splash_width, WHITE);
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
