#include "ArduinoJson.h"
#include "colors.h"
#include "common.h"
#include "game.h"
#include "events.h"
#include "messages.h"
#include "player.h"
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>

//Wifi access point (AP) parameters:
const char* AP_ssid = "BoardGameBox";
const char* AP_password = "box12345"; //must be >= 8 chars
const int CHANNEL = 3;
const bool HIDE_AP = true;
const int MAX_NOF_CLIENTS = 5;

//Wifi station mode (STA) parameters
const char* network_ssid = "Kalat_ja_Rapu_2G"; //"TeeQNote9"; //"Kalat_ja_Rapu_2G";
const char* network_password = "rutaQlli";

//Server related parameters:
const int WIFI_PORT = 80;
WiFiServer wifiServer(WIFI_PORT);
WiFiClient *clients[nofColors] = { NULL };

//Drive connection:
WiFiClientSecure driveClient;
WiFiClientSecure driveUserContentClient;
const char* driveHost = "script.google.com";
const char* driveUserContentHost = "script.googleusercontent.com";
const int drivePort = 443;
const String GAS_ID = "AKfycbwVeVEbpmNkz3V9mcwKXbPTIrE4oZhMlXaQfCgDhRZ2OXs_sLsTR4JN0SvYxowkkRY5RQ";
const String url = "/macros/s/" + GAS_ID + "/exec?";
const String httpString = " HTTP/1.1\r\nHost: " + String(driveHost) + "\r\nConnection: close\r\n\r\n";
//Buffer to store data to be sent to drive
const int maxNofBufferedTurns = 100;
bool newLogEntriesAvailable = false;
const unsigned long driveLogInterval_ms = 30000;
unsigned long lastDriveLog_ms = 0;
int nofBufferedLogEntries = 0;
unsigned long turnBuffer[maxNofCachedTurns][nofColors] = { 0 };
const unsigned long driveConnectionTimeout_ms = 5000;


//const int nofKnownWifis = 3;
//const char* ssids[nofKnownWifis] = {"Kalat_ja_Rapu_2G", "TeeQNote9", "PikkuPingviini"};
//const char* passwords[nofKnownWifis] = {"rutaQlli", "rutaQlli", "Pinquliini"};

//Arrays to buffer events to be send/received
//There is slot for every defined color
Event receivedEvents[nofColors] = { UNKNOWN };
Event outgoingEvents[nofColors] = { UNKNOWN };
bool newEventsReceived = false;

//Button related
const unsigned int BTN_LONG_THRESHOLD_MS = 750;
const unsigned int BTN_LONG_LONG_THRESHOLD_MS = 3000; 

//Game related
Game game;

//--------------------Debug helpers--------------------

String colorToString(Color c)
{
  String s = "";
  switch(static_cast<int>(c))
  {
    case RED:
      s = "RED";
      break;
    case BLUE:
      s = "BLUE";
      break;
    case GREEN:
      s = "GREEN";
      break;
    case YELLOW:
      s = "YELLOW";
      break;
    case WHITE:
      s = "WHITE";
      break;
    case ORDER_SELECTION:
      s = "ORDER_SELECTION";
      break;
    default:
      s = "UNDEFINED";
      break;
  }

  return s;
}

void eventToString(Event e, char* arr)
{
  switch(static_cast<int>(e.type))
  {
    case UNKNOWN:
      strncpy(arr, "UNKNOWN", 7);
      arr[7] = '\0';
      break;
    case LED:
      strncpy(arr, "LED", 3);
      arr[4] = '\0';
      break;
    case LED_ON:
      strncpy(arr, "LED_ON", 6);
      arr[7] = '\0';
      break;
    case LED_OFF:
      strncpy(arr, "LED_OFF", 7);
      arr[8] = '\0';
      break;
    case BLINK:
      strncpy(arr, "BLINK", 5);
      arr[6] = '\0';
      break;
    case BLINK_ON:
      strncpy(arr, "BLINK_ON", 8);
      arr[9] = '\0';
      break;
    case BLINK_OFF:
      strncpy(arr, "BLINK_OFF", 9);
      arr[10] = '\0';
      break;
    case BTN_SHORT:
      strncpy(arr, "BTN_SHORT", 9);
      arr[9] = '\0';
      break;
    case BTN_LONG:
      strncpy(arr, "BTN_LONG", 8);
      arr[8] = '\0';
      break;
    case BTN:
      strncpy(arr, "BTN", 3);
      arr[3] = '\0';
      break;
    case BTN_LONG_LONG:
      strncpy(arr, "BTN_LONG_LONG", 13);
      arr[13] = '\0';
      break;
    case COLOR:
      strncpy(arr, "COLOR", 5);
      arr[5] = '\0';
      break;
    default:
      strncpy(arr, "DEFAULT", 7);
      arr[7] = '\0';
      break;
  }
}

//--------------------Event sending/receiving--------------------

//Set new event to be sent later to the <color>
void setEvent(Color color, EventType type, int data = 0)
{
  Event e;

  if (type == ALL_OFF_EXCEPT_ONE)
  {
    e.type = LED_OFF;
    e.data = 0;
    for (size_t i = 0; i < nofColors; ++i)
    {
      outgoingEvents[i] = e;
    }
    e.type = LED_ON;
    outgoingEvents[color] = e;
    return;
  }

  if (type == BLINK_ALL)
  {
    e.type = BLINK;
    e.data = 5;
    for (size_t i = 0; i < nofColors; ++i)
    {
      outgoingEvents[i] = e;
    }
    return;
  }
  
  e.type = type;
  e.data = data;
  outgoingEvents[color] = e;
}

//Get the latest available event from the <color>
Event getEvent(Color color)
{
  return receivedEvents[static_cast<int>(color)];
}

//Sends a single event to a single client
int sendEvent(const Color color, Event event)
{ 
  //Don't send unknown events
  if (event.type == UNKNOWN)
  {
    return -1;
  }
  else if(clients[color] != NULL && clients[color]->connected())
  {
    //Construct a message
    msg::msg["color"] = color;
    msg::msg["event"] = static_cast<int>(event.type);
    msg::msg["data"] = static_cast<int>(event.data);

    //Serialize the message and send it
    serializeJson(msg::msg, *clients[color]);
    
    Serial.print("Event sent (");
    Serial.print(colorToString(color));
    Serial.print("): ");
    char charArray[25];
    eventToString(event, charArray);
    Serial.println(charArray);
    //Serial.println(eventToString(event));
    /*Serial.print("Event data: ");
    Serial.println(static_cast<int>(event.data));
    Serial.println("");*/
  }
  else
  {
    Serial.print("Sending event failed. Color ");
    Serial.print(colorToString(color));
    Serial.println(" is not connected!");
    return -1;
  }

  return 0;
}

//Send all buffered events to all available clients
void sendAllEvents()
{
  for (size_t color = 0; color < nofColors; color++)
  {
    if(clients[color] != NULL && clients[color]->connected())
    {
      //Don't spam unknown (empty) events
      if (outgoingEvents[static_cast<int>(color)].type != UNKNOWN)
      {
        sendEvent(static_cast<Color>(color), outgoingEvents[color]);
      }
    }
  }
}


//Receive new event (if available) from the <color> client
Event receiveEvent(Color color)
{
  Event e;
  e.type = UNKNOWN;
  e.data = 0;
  e.ts = 0;

  if (clients[static_cast<int>(color)] != NULL && 
      clients[static_cast<int>(color)]->available())
  {
    msg::err = deserializeJson(msg::msg, *clients[static_cast<int>(color)]);
    if (msg::err)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(msg::err.f_str());
      return e;
    }

    //Construct an event from the message:
    e.type = msg::msg["event"];
    e.data = msg::msg["data"];
    e.ts = msg::msg["ts"];

    Serial.print("Received event (");
    Serial.print(colorToString(color));
    Serial.print("): ");

    char charArray[25];
    eventToString(e, charArray);
    Serial.print(charArray);

    if (e.type == BTN_SHORT || e.type == BTN_LONG || e.type == BTN)
    {
      Serial.print(" data: ");
      Serial.print(static_cast<int>(e.data));
      Serial.print(" ts: ");
      Serial.print(static_cast<int>(e.ts));
      

      //Convert general BTN event to SHORT or LONG or LONG LONG
      if(e.type == BTN)
      {
        e.type = BTN_SHORT;
        const int press_duration_ms = static_cast<unsigned int>(e.data);
        if (press_duration_ms > BTN_LONG_THRESHOLD_MS)
        {
          e.type = BTN_LONG;
        }

        if (press_duration_ms > BTN_LONG_LONG_THRESHOLD_MS)
        {
          e.type = BTN_LONG_LONG;
        }

        Serial.print(" - Converted to ");
        eventToString(e, charArray);
        Serial.print(charArray);
        
      }
      
    }
    else
    {
      //intentionally empty
    }
    
    Serial.println("");
  }
  
  return e;
}

//Receive all new events from available clients
bool receiveAllEvents()
{
  for(size_t color = 0; color < nofColors; color++)
  {
    //Don't store unknown events
    Event e = receiveEvent(static_cast<Color>(color));
    if (e.type != UNKNOWN)
    {
      receivedEvents[color] = e;
      return true;
    }
  }

  return false;
}

//Clear the outgoing buffer
void clearOutgoingEvents()
{
  //Serial.println(outgoingEvents[static_cast<Color>(2)].type);
  for(size_t color = 0; color < nofColors; color++)
  {
    Event *e = &outgoingEvents[static_cast<Color>(color)];
    e->type = UNKNOWN;
    e->data = 0;
    e->ts = 0;
  }
  //Serial.println(outgoingEvents[static_cast<Color>(2)].type);
}

//Clear the receiving buffer
void clearReceivedEvents()
{
  //Serial.println(receivedEvents[static_cast<Color>(2)].type);
  for(size_t color = 0; color < nofColors; color++)
  {
    Event *e = &receivedEvents[static_cast<Color>(color)];
    e->type = UNKNOWN;
    e->data = 0;
    e->ts = 0;
  }
  //Serial.println(receivedEvents[static_cast<Color>(2)].type);
}


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

      const Game::Action returnAction = game.addPlayer(static_cast<Color>(e.data));
      if (returnAction.type == ADDED)
      {
        Serial.print("New player ");
        Serial.print(colorToString(static_cast<Color>(e.data)));
        Serial.println(" added to the game");
      }
      else
      {
        sendEvent(returnAction.color, Event(returnAction.type));
      }

      /*
      int indexOfColor = 0;
      for (size_t i = 0; i< nofColors; i++)
      {
        //if (players[i].color == static_cast<Color>(e.data)) indexOfColor = i;
      }*/

      //Check if player reconnected
      /*
      if (players[indexOfColor].isPlaying == false)
      {
        //Add new player to the game
        players[indexOfColor].isPlaying = true;
        //nofPlayers++; 

        Serial.print("New player ");
        Serial.print(colorToString(static_cast<Color>(e.data)));
        Serial.println(" added to the game");
      }*/

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
      //Serial.println("ESP32 Connected to WiFi Network");
      break;
    case SYSTEM_EVENT_AP_START:
      //Serial.println("ESP32 soft AP started");
      break;
    case SYSTEM_EVENT_AP_STACONNECTED:
      //Serial.println("Station connected to ESP32 soft AP");
      break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
      //Serial.println("Station disconnected from ESP32 soft AP");
      break;
    default: break;
  }
}

//--------------------Google Drive connection--------------------

void addLogEntry(unsigned long turnLengths_s[nofColors])
{
  newLogEntriesAvailable = true;

  if (nofBufferedLogEntries >= maxNofBufferedTurns)
  {
    Serial.println("Log buffer full!");
    return;
  }

  Serial.println("----------");
  for (size_t i = 0; i < nofColors; ++i)
  {
    turnBuffer[nofBufferedLogEntries][i] = turnLengths_s[i];
    
    Serial.print(colorToString(static_cast<Color>(i)));
    Serial.print(": ");
    Serial.print(turnLengths_s[i]);
    Serial.println("s");
  }
  Serial.println("----------");
  
  ++nofBufferedLogEntries;
}

void clearLogBuffer()
{
  newLogEntriesAvailable = false;
  nofBufferedLogEntries = 0;

  for (size_t i = 0; i < maxNofBufferedTurns; ++i)
  {
    for (size_t j = 0; j < nofColors; ++j)
    {
      turnBuffer[i][j] = 0;
    }
  }

  Serial.println("Log buffer cleared");
}

//Fetch settings from Google Drive
void getSettings()
{
  Serial.println("Start fetching settings from Google Drive...");

  //Check if drive connection was left open, this should not happen
  if (driveClient.connected())
  {
    Serial.println("Drive connection was left open!");
  }

  //Connect to the Drive
  unsigned long connectionStartTime = millis();
  while(!driveClient.connect(driveHost, drivePort))
  {
    //Serial.println("No connection to google Drive...");
    if (millis() - connectionStartTime > driveConnectionTimeout_ms)
    {
      Serial.print("No connection to the Google Drive within timeout (");
      Serial.print(driveConnectionTimeout_ms);
      Serial.println("ms)");
      return;
    }
  }
  
  Serial.println("Google Drive connection successfull");

  Serial.println("Trying to get settings data...");
  String request = "method=get&sheet=gameSettings";
  driveClient.print(String("GET ") + url + request + httpString);

  //Read response headers
  String userContentUrl = "";
  while(driveClient.connected())
  {
    String line = driveClient.readStringUntil('\n');
    //Serial.println(line);
    if (line == "\r")
    {
      Serial.println("Info: Headers received");
      break;
    }

    int indexOfColon = line.indexOf(':');
    if (indexOfColon > 0)
    {
      String key = line.substring(0, indexOfColon);
      if (key == "Location")
      {
        userContentUrl = line.substring(indexOfColon+1, line.length()-1);
        userContentUrl.trim();
      }
    }
  }

  if(userContentUrl == "")
  {
    Serial.println("Failed to parse address for the relocated response");
    Serial.println("Failed to fetch settings");
    //Close connection to Google Drive as it is already open
    driveClient.stop();
    return;
  }

  //Read response content, which in this case is relocated in userContentUrl
  Serial.print("Address for the relocated response message: ");
  Serial.println(userContentUrl);

  if (driveUserContentClient.connected())
  {
    Serial.println("Drive user content connection was left open!");
  }

  connectionStartTime = millis();
  while (!driveUserContentClient.connect(driveUserContentHost, drivePort))
  {
    if (millis() - connectionStartTime > driveConnectionTimeout_ms)
    {
      Serial.print("No connection to the Google Drive user content within timeout (");
      Serial.print(driveConnectionTimeout_ms);
      Serial.println("ms)");
      Serial.println("Failed to get settings");
      //Close connection to Google Drive as it is already open
      driveClient.stop();
      return;
    }
  }
  Serial.println("Google Drive User Content connection succesfull");

  //Get response
  driveUserContentClient.print(String("GET ") + userContentUrl + httpString);

  while(driveUserContentClient.connected())
  {
    String line = driveUserContentClient.readStringUntil('\n');
    //Serial.println(line);
    if (line == "\r")
    {
      Serial.println("Info: Headers received");
      break;
    }
  }

  String dataLength = driveUserContentClient.readStringUntil('\n');
  String data = driveUserContentClient.readStringUntil('\n');

  Serial.println(data);

  driveUserContentClient.stop();
  driveClient.stop();
  
  //Close everything gracefully
  Serial.println("Google Drive user content disconnected gracefully");
  Serial.println("Google Drive disconnected gracefully");

  //Parse received data:

  //Remove whitespaces
  data.trim();

  //Remove brackets and separate individual settings
  //2D array [number of settings][setting, value]
  String settings[game.getNofSettings()][2];

  unsigned int currentSetting = 0;
  unsigned int currentChar = 1; //The first (0) char is '{' - skip
  unsigned int keyValueIndex = 0; //0 for key, 1 for value in 2D array
  char c;
  while (c != '}')
  {
    c = data[currentChar];
    if (c == '}')
    {
      break;
    }
    else if (c == ',')
    {
      currentSetting++;
      keyValueIndex = 0;
    }
    else if (c == ':')
    {
      keyValueIndex = 1;
    }
    else
    {
      settings[currentSetting][keyValueIndex] += c;
    }
    currentChar++;
  }

  int turnsPerRound;
  bool roundOverWhenPassed;
  bool orderMayChange;
  unsigned int changeOrderBySteps;
  bool predictablePlayerOrder;
  unsigned int logAfterRounds;

  for (int i = 0; i < game.getNofSettings(); i++)
  {
    if (settings[i][0] == "turnsPerRound")
    {
      turnsPerRound = settings[i][1].toInt();
    }
    else if (settings[i][0] == "roundOverWhenPassed")
    {
      if (settings[i][1] == "true")
      {
        roundOverWhenPassed = true;  
      }
      else
      {
        roundOverWhenPassed = false;
      }
    }
    else if (settings[i][0] == "orderMayChange")
    {
      if (settings[i][1] == "true")
      {
        orderMayChange = true;  
      }
      else
      {
        orderMayChange = false;
      }
    }
    else if (settings[i][0] == "changeOrderBySteps")
    {
      changeOrderBySteps = settings[i][1].toInt();
    }
    else if (settings[i][0] == "predictablePlayerOrder")
    {
      if (settings[i][1] == "true")
      {
        predictablePlayerOrder = true;  
      }
      else
      {
        predictablePlayerOrder = false;
      }
    }
    else if (settings[i][0] == "logAfterRounds")
    {
      logAfterRounds = settings[i][1].toInt();
    }
    else
    {
      Serial.print("Unknown setting ");
      Serial.print(settings[i][0]);
      Serial.print(" with a value of ");
      Serial.println(settings[i][1]);
    }
  }
  
  
  //Save new settings
  game.changeSettings(turnsPerRound,
                      roundOverWhenPassed,
                      orderMayChange,
                      changeOrderBySteps,
                      predictablePlayerOrder,
                      logAfterRounds);

  Serial.println("Settings updated successfully!");
  game.printSettings();
  
}

void logToDrive()
{
  Serial.println("Start logging to Google Drive...");

  //Check if drive connection was left open, this should not happen
  if (driveClient.connected())
  {
    Serial.println("Drive connection was left open!");
  }

  //Connect to the Drive
  unsigned long connectionStartTime = millis();
  while(!driveClient.connect(driveHost, drivePort))
  {
    //Serial.println("No connection to google Drive...");
    if (millis() - connectionStartTime > driveConnectionTimeout_ms)
    {
      Serial.print("No connection to the Google Drive within timeout (");
      Serial.print(driveConnectionTimeout_ms);
      Serial.println("ms)");
      return;
    }
  }
  Serial.println("Google Drive connection successfull");

  //Construct new data entry
  String colorTimes ="";
  colorTimes += "method=log&entries=" + String(nofBufferedLogEntries);
  for (size_t i = 0; i < nofBufferedLogEntries; ++i)
  {
    colorTimes += "&green=" + String(turnBuffer[i][static_cast<int>(GREEN)]);
    colorTimes += "&blue=" + String(turnBuffer[i][static_cast<int>(BLUE)]);
    colorTimes += "&red=" + String(turnBuffer[i][static_cast<int>(RED)]);
    colorTimes += "&white=" + String(turnBuffer[i][static_cast<int>(WHITE)]);
    colorTimes += "&yellow=" + String(turnBuffer[i][static_cast<int>(YELLOW)]);
    colorTimes += "&order_selection=" + String(turnBuffer[i][static_cast<int>(ORDER_SELECTION)]);
  }

  //String colorTimes = "method=get&sheet=Settings";

  /*
  colorTimes += "&green=99&green=88"; //  + String(turnLengths_s[static_cast<int>(GREEN)]);
  colorTimes += "&blue=6"; // + String(turnLengths_s[static_cast<int>(BLUE)]);
  colorTimes += "&red=8"; // + String(turnLengths_s[static_cast<int>(RED)]);
  colorTimes += "&white=8"; // + String(turnLengths_s[static_cast<int>(WHITE)]);
  colorTimes += "&yellow=3"; // + String(turnLengths_s[static_cast<int>(YELLOW)]);*/

  //Log new entry to the Drive
  Serial.println("Trying to log new data...");
  driveClient.print(String("GET ") + url + colorTimes + httpString);

  //Read response headers
  String userContentUrl = "";
  while(driveClient.connected())
  {
    String line = driveClient.readStringUntil('\n');
    //Serial.println(line);
    if (line == "\r")
    {
      Serial.println("Info: Headers received");
      break;
    }

    int indexOfColon = line.indexOf(':');
    if (indexOfColon > 0)
    {
      String key = line.substring(0, indexOfColon);
      if (key == "Location")
      {
        userContentUrl = line.substring(indexOfColon+1, line.length()-1);
        userContentUrl.trim();
      }
    }
  }

  if(userContentUrl == "")
  {
    Serial.println("Failed to parse address for the relocated response");
    Serial.println("Logging failed");
    //Close connection to Google Drive as it is already open
    driveClient.stop();
    return;
  }

  //Read response content, which in this case is relocated in userContentUrl
  Serial.print("Address for the relocated response message: ");
  Serial.println(userContentUrl);

  if (driveUserContentClient.connected())
  {
    Serial.println("Drive user content connection was left open!");
  }

  connectionStartTime = millis();
  while (!driveUserContentClient.connect(driveUserContentHost, drivePort))
  {
    if (millis() - connectionStartTime > driveConnectionTimeout_ms)
    {
      Serial.print("No connection to the Google Drive user content within timeout (");
      Serial.print(driveConnectionTimeout_ms);
      Serial.println("ms)");
      Serial.println("Logging failed");
      //Close connection to Google Drive as it is already open
      driveClient.stop();
      return;
    }
  }
  Serial.println("Google Drive User Content connection succesfull");

  //Get response
  driveUserContentClient.print(String("GET ") + userContentUrl + httpString);

  while(driveUserContentClient.connected())
  {
    String line = driveUserContentClient.readStringUntil('\n');
    //Serial.println(line);
    if (line == "\r")
    {
      Serial.println("Info: Headers received");
      break;
    }
  }

  String dataLength = driveUserContentClient.readStringUntil('\n');
  String data = driveUserContentClient.readStringUntil('\n');

  if (data != "OK\r")
  {
    Serial.print("Logging Failed. Result: ");
    Serial.print(data);
    Serial.println("");
  }
  else
  {
    Serial.println("Logging OK");
    clearLogBuffer();
  }
  
  driveUserContentClient.stop();
  driveClient.stop();
  
  //Close everything gracefully
  Serial.println("Google Drive user content disconnected gracefully");
  Serial.println("Google Drive disconnected gracefully");
  
}

void setup()
{
  //Serial connection for debugging purposes
  Serial.begin(115200);

  //WiFi.onEvent(OnWiFiEvent);

  //Set the ESP32 to work in soft access point and station modes simultaneously
  WiFi.mode(WIFI_MODE_APSTA);
  
  //Create wifi AP for the client buttons:
  createSoftAP();

  //Connect to local wifi network
  connectToWifi();

  //Start the server
  wifiServer.begin();
  Serial.println("Game server started");

  //Initialize game
  game.init(addLogEntry);

  //Prepare buffer for storing drive log entries
  clearLogBuffer();

  //Initialize player list
  //initializePlayers();

  //Only for testing purposes
  //unsigned long arr[5] = {1, 2, 3, 4, 5};
  //sendToDrive(arr);
  //logToDrive();
  getSettings();
}
 
void loop()
{
  //Check if some existing clients disconnected:
  removeDisconnectedClients();

  //Check if new clients are available and store them
  checkNewClients();

  //Check wheter any client send a message
  //If so, store them for later processing
  newEventsReceived = receiveAllEvents();
  
  if (newEventsReceived)
  {
    for (size_t i = 0; i < nofColors; i++)
    {
      const Event e = getEvent(static_cast<Color>(i));
      if (e.type != UNKNOWN)
      {
        /*Serial.print("e: ");
        Serial.print(e.type);
        Serial.print(" data: ");
        Serial.print(e.data);
        Serial.print(" ts: ");
        Serial.println(e.ts);*/
        
        const Game::Action returnAction = game.play(Game::Action(static_cast<Color>(i), e.type));
        setEvent(returnAction.color, returnAction.type);
      }
    }
    clearReceivedEvents();
    sendAllEvents();
    clearOutgoingEvents();
  }

  if (newLogEntriesAvailable)
  {
    if (millis() - lastDriveLog_ms > driveLogInterval_ms)
    {
      logToDrive();
      lastDriveLog_ms = millis();
    }
  }
  
}
