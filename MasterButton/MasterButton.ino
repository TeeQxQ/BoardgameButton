#include "ArduinoJson.h"
#include "colors.h"
#include "effects.h"
#include "ESP8266WiFi.h"
#include "events.h"
#include "messages.h"
#include "player.h"
#include <WiFiClientSecure.h>

#define ISR_PREFIX ICACHE_RAM_ATTR

//Hardware parameters:
const int LED_PIN = 0;
const int BTN_PIN = 14;
const Color BTN_COLOR = RED;

bool ledState = LOW;
bool blinkEnabled = false;
//bool blinkState = LOW;
const int blinkDelay_ms = 500;
long lastBlinkTime_ms = 0;

//Wifi parameters:
const char* ssid = "PikkuPingviini";
const char* password = "Pinquliini";
const int WIFI_PORT = 80;
const int MAX_NOF_CLIENTS = 3;

//Wifi related variables:
WiFiServer wifiServer(WIFI_PORT);
WiFiClient *clients[nofColors] = { NULL };

//Drive:
WiFiClientSecure driveClient;
const char* driveHost = "script.google.com";
const int drivePort = 443;
const String GAS_ID = "AKfycbyyMIrQdgzFpbFaOPDEEMih1gN-afdZAdnqPT-_egqosxMO9NBL";
const String url = "/macros/s/" + GAS_ID + "/exec?";
const String httpString = " HTTP/1.1\r\nHost: " + String(driveHost) + "\r\nUser-Agent: BuildFailureDetectorESP8266\r\nConnection: keep-alive\r\n\r\n";


//Arrays to buffer events to be send/received
//There is slot for every defined color
Event receivedEvents[nofColors] = { UNKNOWN };
Event outgoingEvents[nofColors] = { UNKNOWN };

//Button related parameters
volatile byte btn_pressed_counter = 0;
volatile byte btn_released_counter = 0;
int btn_state = HIGH;
volatile unsigned long btn_pressed_time_ms = 0;
volatile unsigned long btn_released_time_ms = 0;
const unsigned long btn_long_press_threshold_MIN_ms = 800;
const unsigned long btn_long_press_threshold_MAX_ms = 2500;
const unsigned long debounceDelay_ms = 120;
volatile unsigned long lastDebounceTime_ms = 0;

//For testing purposes
bool ledStates[nofColors] = { false };
int ledLevel = 0;

//--------------------Event sending/receiving--------------------

//Set new event to be sent later to the <color>
void setEvent(Color color, EventType type, int data = 0)
{
  Event e;
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
int sendEvent(const Color color, const Event event)
{
  //Don't send unknown events
  if (event.type == UNKNOWN) return -1;
  
  //Sends message to the master button itself
  if(color == BTN_COLOR)
  {
    //Store the event
    receivedEvents[color] = event;
    return 0;
  }
  else if(clients[color] != NULL && clients[color]->connected())
  {
    //Construct a message
    msg::msg["color"] = color;
    msg::msg["event"] = static_cast<int>(event.type);
    msg::msg["data"] = static_cast<int>(event.data);

    //Serialize the message and send it
    serializeJson(msg::msg, *clients[color]);

    Serial.println("Event sent:");
    Serial.print("Color: ");
    Serial.println(static_cast<int>(color));
    Serial.print("Event type: ");
    Serial.println(static_cast<int>(event.type));
    Serial.print("Event data: ");
    Serial.println(static_cast<int>(event.data));
    Serial.println("");
  }
  else
  {
    Serial.print("Client at index ");
    Serial.print(static_cast<int>(color));
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
    if(color == BTN_COLOR)
    {
      sendEvent(static_cast<Color>(color), outgoingEvents[color]);
    }
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

  if(color == BTN_COLOR)
  {
    e = receivedEvents[static_cast<int>(color)];
  }
  else if (clients[static_cast<int>(color)] != NULL && 
      clients[static_cast<int>(color)]->available())
  {
    msg::err = deserializeJson(msg::msg, *clients[static_cast<int>(color)]);
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

      Serial.println("Received event:");
      Serial.print("Color: ");
      Serial.println(static_cast<int>(color));
      Serial.print("Event: ");
      Serial.println(static_cast<int>(e.type));
      Serial.print("Data: ");
      Serial.println(static_cast<int>(e.data));
      Serial.println("");
    }

    return e;
  }
}

//Receive all new events from available clients
void receiveAllEvents()
{
  for(size_t color = 0; color < nofColors; color++)
  {
    //Don't store unknown events
    Event e = receiveEvent(static_cast<Color>(color));
    if (e.type != UNKNOWN)
    {
      receivedEvents[color] = e;
    }
  }
}

//Clear the outgoing buffer
void clearOutgoingEvents()
{
  for(size_t color = 0; color < nofColors; color++)
  {
    Event *e = &outgoingEvents[static_cast<Color>(color)];
    e->type = UNKNOWN;
    e->data = 0;
  }
}

//Clear the receiving buffer
void clearReceivedEvents()
{
  for(size_t color = 0; color < nofColors; color++)
  {
    Event *e = &receivedEvents[static_cast<Color>(color)];
    e->type = UNKNOWN;
    e->data = 0;
  }
}

//--------------------Event handling--------------------

void handleEvents(const Event e)
{
  switch (e.type)
  {
    case UNKNOWN:
      break;
    case LED:
      break;
    case LED_ON:
      digitalWrite(LED_PIN, HIGH);
      break;
    case LED_OFF:
      digitalWrite(LED_PIN, LOW);
      break;
    case BLINK:
      break;
    case BLINK_ON:
      blinkEnabled = true;
      break;
    case BLINK_OFF:
      blinkEnabled = false;
      digitalWrite(LED_PIN, LOW);
      break;
    case BTN_SHORT:
      //ledState = !ledState;
      //(ledState) ? outgoingEvents[BTN_COLOR].type = LED_ON : outgoingEvents[BTN_COLOR].type = LED_OFF;
      //Serial.println("BTN SHORT");
      break;
    case BTN_LONG:
      //(blinkEnabled) ? outgoingEvents[BTN_COLOR].type = BLINK_OFF : outgoingEvents[BTN_COLOR].type = BLINK_ON;
      //Serial.println("BTN LONG");
      break;
    case COLOR:
      break;
  }
}

//--------------------Blinking--------------------

//Blink the led <times> times
void blink(int times)
{
  for(int i = 0; i < times; i++)
  {
    delay(blinkDelay_ms);
    digitalWrite(LED_PIN, HIGH);
    delay(blinkDelay_ms);
    digitalWrite(LED_PIN, LOW);
  }
}

//Handles how blinking works on this device
void handleBlinking()
{
  if(blinkEnabled)
  {
    if(millis() - lastBlinkTime_ms > blinkDelay_ms)
    {
      ledState = !ledState;
      ledState ? digitalWrite(LED_PIN, HIGH) : digitalWrite(LED_PIN, LOW);
      lastBlinkTime_ms = millis();
    }
  }
}

//--------------------Interrupts--------------------

//Handles interrupt caused by the button press
ISR_PREFIX void handleInterrupt()
{
  unsigned long currentTime_ms = millis();
  
  if (currentTime_ms - lastDebounceTime_ms > debounceDelay_ms)
  {
    if (digitalRead(BTN_PIN) == HIGH)
    {
      btn_released_counter++;
      btn_released_time_ms = currentTime_ms;
    }
    else
    {
      btn_pressed_counter++;
      btn_pressed_time_ms = currentTime_ms;
    }
    lastDebounceTime_ms = currentTime_ms;
  }
  
}

//--------------------Buttons--------------------

//Handles button press
void handleButtonPress(bool debug = false)
{
  Event btnEvent;
  btnEvent.type = UNKNOWN;
  
  if (btn_released_counter > 0)
  {
    if (btn_released_time_ms - btn_pressed_time_ms > btn_long_press_threshold_MIN_ms &&
        btn_released_time_ms - btn_pressed_time_ms < btn_long_press_threshold_MAX_ms)
    {
      btnEvent.type = BTN_LONG;
      if (debug) Serial.println("Long btn press");
    }
    else if (btn_released_time_ms - btn_pressed_time_ms <= btn_long_press_threshold_MIN_ms)
    {
      btnEvent.type = BTN_SHORT;
      if (debug) Serial.println("Short btn press");
    }
    btn_released_counter = 0;
    btn_pressed_counter = 0;
  }
  else if (btn_pressed_counter > 0)
  {
    if (millis() - btn_pressed_time_ms > btn_long_press_threshold_MAX_ms)
    {
      btnEvent.type = BTN_SHORT;
      if (debug) Serial.println("Short btn press without release event");
      btn_pressed_counter = 0;
    }
  }

  if (btnEvent.type != UNKNOWN)
  {
    sendEvent(BTN_COLOR, btnEvent);
    if (debug) Serial.println(btnEvent.type);
  }

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
    msg::msg["color"] = BTN_COLOR;
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
        Serial.print(static_cast<int>(e.data));
        Serial.println(" added to the game");
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
      Serial.print("Client at index: ");
      Serial.print(i);
      Serial.println(" disconnected and removed");
    }
  }
}

//--------------------Google Drive connection--------------------

void sendToDrive (){

  //Reconnect if connection lost
  if (!driveClient.connected()) {
    Serial.println("No connection to google drive, reconnecting...");
    while (!driveClient.connect(driveHost, drivePort)) {
      Serial.println("connection failed");
    }
    Serial.println("Google drive reconnected!");
  }

  String colorTimes ="";
  colorTimes += "green=" + players[GREEN].turnLength;
  colorTimes += "&blue=" + players[BLUE].turnLength;
  colorTimes += "&red=" + players[RED].turnLength;
  colorTimes += "&white=" + players[WHITE].turnLength;
  colorTimes += "&yellow=" + players[YELLOW].turnLength;

  driveClient.print(String("GET ") + url + colorTimes + httpString);
  Serial.println("Request sent");
}

//This is only for testing purposes
void testLogic()
{
  for (size_t color = 0; color < nofColors; color++)
  {
    if(color != BTN_COLOR)
    {
      if (getEvent(static_cast<Color>(color)).type == BTN_SHORT)
      {
        ledStates[color] = !ledStates[color];
        (ledStates[color]) ? setEvent(static_cast<Color>(color), LED_ON) : setEvent(static_cast<Color>(color), LED_OFF);
        //setEvent(static_cast<Color>(color), LED, ledLevel);
        //ledLevel += 10;
        //if (ledLevel > 100) ledLevel = 0;
      }
    }
    
  }
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  digitalWrite(LED_PIN, LOW);  //turn off by LOW

  attachInterrupt(digitalPinToInterrupt(BTN_PIN), handleInterrupt, CHANGE);

  //Serial connection for debugging purposes
  Serial.begin(9600);
  delay(1000);

  //Connect to wifi network:
  WiFi.begin(ssid, password);

  Serial.println("Wifi parameters:");
  Serial.print("ssid: ");
  Serial.println(ssid);
  Serial.print("password: ");
  Serial.println(password);
    
  while(WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Trying to connect wifi...");
  }
  Serial.print("\nWifi connected! My ip: ");
  Serial.println(WiFi.localIP());

  //5 blinks indicates successfull wifi connection
  //blink(5);

  //Start the server
  wifiServer.begin();
  Serial.println("Wifi server started");

  //Connect to google drive
  //Serial.print("Connecting to ");
  //Serial.println(driveHost);

  driveClient.setInsecure();
  while (!driveClient.connect(driveHost, drivePort)) {
    Serial.println("Drive connection failed...");
  }
  Serial.println("Connected to Drive");

  //Initialize player list
  initializePlayers();
  //By default master button is playing
  players[static_cast<int>(BTN_COLOR)].isPlaying = true;
  nofPlayers++;
}

//--------------------Main program--------------------

void loop() {
  
  //Check if some existing clients disconnected:
  removeDisconnectedClients();

  //Check if new clients are available and store them
  if (checkNewClients() != 0) Serial.println("Error while adding client");

  receiveAllEvents();

  //Handle buttons
  handleButtonPress();

  //Handle blinking
  handleBlinking();
  
  //Handle received events
  handleEvents(receivedEvents[static_cast<int>(BTN_COLOR)]);

  //Handle gameplay here
  gameLogic();
  //testLogic();

  clearReceivedEvents();
  sendAllEvents();
  clearOutgoingEvents();
  
  delay(10);
}
