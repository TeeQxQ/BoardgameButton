#include "ArduinoJson.h"
#include "colors.h"
#include "events.h"
#include "messages.h"
#include "player.h"
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>

//Wifi access point (AP) parameters:
const char* AP_ssid = "BoardGameBox";
const char* AP_password = "box12345";
const int CHANNEL = 1;
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

//--------------------Client connections--------------------

//Check new clients 
//If new clients are available add them to the client list
int checkNewClients()
{
  WiFiClient newClient = wifiServer.available();
  
  //New client found
  if (newClient)
  {
    //Ask its color:
    //Construct a message
    msg::msg["color"] = UNDEFINED;
    msg::msg["event"] = static_cast<int>(COLOR);
    msg::msg["data"] = 0;

    //Serialize the message and send it
    serializeJson(msg::msg, newClient);

    //Receive answer
    Event e;
    e.type = UNKNOWN;
    while (newClient.connected() && e.type != COLOR)
    {
      if (newClient.available())
      {
        msg::err = deserializeJson(msg::msg, newClient);
        if (msg::err)
        {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(msg::err.f_str());
        }
        else
        {
          //Construct an event from the message:
          e.type = msg::msg["event"];
          e.data = msg::msg["data"];
        }
      }
    }

    //Add new client to the list
    if (static_cast<int>(e.data) < nofColors)
    {
      clients[static_cast<int>(e.data)] = new WiFiClient(newClient);
      Serial.println("New client added successfully");

      int indexOfColor = 0;
      for (size_t i = 0; i< nofColors; i++)
      {
        if (players[i].color == static_cast<Color>(e.data)) indexOfColor = i;
      }

      //Check if player reconnected
      if (players[indexOfColor].isPlaying == false)
      {
        //Add new player to the game
        players[indexOfColor].isPlaying = true;
        nofPlayers++;

        Serial.print("New player ");
        Serial.print(colorToString(static_cast<Color>(e.data)));
        Serial.println(" added to the game");
      }

      while(newClient.available() > 0)
      {
        Serial.println("Cleaning receiving buffer");
        newClient.read();
      }
      return 0;
    }

    Serial.print("Color: \(");
    Serial.print(static_cast<int>(e.data));
    Serial.println("\) is not a valid color value.");
    Serial.println("Client rejected.");
    return 1;
  }
}

//Check if some clients have disconnected
//Remove them from the list
void removeDisconnectedClients()
{
  for (size_t i = 0; i < nofColors; i++)
  {
    if (clients[i] != NULL && !clients[i]->connected())
    {
      clients[i]->stop();
      delete clients[i];  //Removes memory allocation
      clients[i] = NULL;
      Serial.print("Client: ");
      Serial.print(colorToString(static_cast<Color>(i)));
      Serial.println(" disconnected and removed");
    }
  }
}

void createSoftAP()
{
  if (!WiFi.softAP(AP_ssid, AP_password, CHANNEL, HIDE_AP, MAX_NOF_CLIENTS))
  {
    Serial.println("Failed to create soft access point");
  }
  else
  {
    Serial.println("Soft access point created.");
    Serial.print("Soft access point ssid: ");
    Serial.println(AP_ssid);
    Serial.print("Soft access point password: ");
    Serial.println(AP_password);
    Serial.print("Soft access point IP address: ");
    Serial.println(WiFi.softAPIP());
  }
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

void OnWiFiEvent(WiFiEvent_t event)
{
  switch (event) {
 
    case SYSTEM_EVENT_STA_CONNECTED:
      Serial.println("ESP32 Connected to WiFi Network");
      break;
    case SYSTEM_EVENT_AP_START:
      Serial.println("ESP32 soft AP started");
      break;
    case SYSTEM_EVENT_AP_STACONNECTED:
      Serial.println("Station connected to ESP32 soft AP");
      break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
      Serial.println("Station disconnected from ESP32 soft AP");
      break;
    default: break;
  }
}

void setup()
{
  //Serial connection for debugging purposes
  Serial.begin(115200);

  WiFi.onEvent(OnWiFiEvent);

  //Set the ESP32 to work in soft access point and station modes simultaneously
  WiFi.mode(WIFI_MODE_APSTA);
  
  //Create wifi AP for the client buttons:
  createSoftAP();

  //Connect to local wifi network
  connectToWifi();

  //Start the server
  wifiServer.begin();
  Serial.println("Game server started");

  //Initialize player list
  initializePlayers();
}
 
void loop()
{
  //Check if some existing clients disconnected:
  removeDisconnectedClients();

  //Check if new clients are available and store them
  checkNewClients();

  /*if (clients[0] != NULL)
  {
    Serial.println(clients[0]->available());
  }*/

}
