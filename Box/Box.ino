#include "ArduinoJson.h"
#include "colors.h"
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>

//Wifi access point (AP) parameters:
const char* AP_ssid = "BoardGameBox";
const char* AP_password = "box1234";
const bool HIDE_AP = false;
const int MAX_NOF_CLIENTS = 5;

//Wifi station mode (STA) parameters
const char* network_ssid = "Kalat_ja_Rapu_2G";
const char* network_password = "rutaQlli";

//Server related parameters:
const int WIFI_PORT = 80;
WiFiServer wifiServer(WIFI_PORT);
WiFiClient *clients[nofColors] = { NULL };

//Drive connection:
WiFiClientSecure driveClient;
const char* driveHost = "script.google.com";
const int drivePort = 443;
const String GAS_ID = "AKfycbzn_JvVTjFdTwfLjchA6kIjdkF8AiGZ-ODYY8G_f6nFaeMyzuP9torUzQ";
const String url = "/macros/s/" + GAS_ID + "/exec?";
const String httpString = " HTTP/1.1\r\nHost: " + String(driveHost) + "\r\nUser-Agent: BuildFailureDetectorESP8266\r\nConnection: keep-alive\r\n\r\n";


//const int nofKnownWifis = 3;
//const char* ssids[nofKnownWifis] = {"Kalat_ja_Rapu_2G", "TeeQNote9", "PikkuPingviini"};
//const char* passwords[nofKnownWifis] = {"rutaQlli", "rutaQlli", "Pinquliini"};

void createSoftAP()
{
  WiFi.softAP(AP_ssid, AP_password, HIDE_AP, MAX_NOF_CLIENTS);
  Serial.println("Soft access point created.");
  Serial.print("Soft access point ssid: ");
  Serial.println(AP_ssid);
  Serial.print("Soft access point password: ");
  Serial.println(AP_password);
  Serial.print("Soft access point IP address: ");
  Serial.println(WiFi.softAPIP());
}

void connectToWifi()
{
  WiFi.begin(network_ssid, network_password);
  while(WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Connecting to wifi...");
    delay(1000);
  }
  Serial.println("Wifi connected.");
  Serial.print("Wifi ssid: ");
  Serial.println(network_ssid);
  Serial.print("Wifi password: ");
  Serial.println(network_password);
  Serial.print("Local network IP address: ");
  Serial.println(WiFi.localIP());
}

void setup()
{
  //Serial connection for debugging purposes
  Serial.begin(9600);

  //Set the ESP32 to work in soft access point and station modes simultaneously
  WiFi.mode(WIFI_MODE_APSTA);
  
  //Create wifi AP for the client buttons:
  createSoftAP();

  //Connect to local wifi network
  connectToWifi();
}
 
void loop()
{

}
