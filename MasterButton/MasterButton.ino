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
const int LED_PIN = 15;
const int BTN_PIN = 13;
const int DIP_PIN_1 = 2;
const int DIP_PIN_2 = 0;
const int DIP_PIN_3 = 4;
const int DIP_PIN_4 = 5;
Color BTN_COLOR = UNDEFINED;

bool ledState = LOW;
bool blinkEnabled = false;
const int defaultBlinkDelay_ms = 500;
int blinkDelay_ms;
int blinkCount = 0;
long lastBlinkTime_ms = 0;

//Wifi parameters:
const char* ssid = "Kalat_ja_Rapu_2G";
const char* password = "rutaQlli";
const int WIFI_PORT = 80;
const int MAX_NOF_CLIENTS = 3;
const int nofKnownWifis = 3;
const char* ssids[nofKnownWifis] = {"Kalat_ja_Rapu_2G", "TeeQNote9", "PikkuPingviini"};
const char* passwords[nofKnownWifis] = {"rutaQlli", "rutaQlli", "Pinquliini"};

//Wifi related variables:
WiFiServer wifiServer(WIFI_PORT);
WiFiClient *clients[nofColors] = { NULL };

//Drive:
WiFiClientSecure driveClient;
const char* driveHost = "script.google.com";
const int drivePort = 443;
const String GAS_ID = "AKfycbzn_JvVTjFdTwfLjchA6kIjdkF8AiGZ-ODYY8G_f6nFaeMyzuP9torUzQ";
const String url = "/macros/s/" + GAS_ID + "/exec?";
const String httpString = " HTTP/1.1\r\nHost: " + String(driveHost) + "\r\nUser-Agent: BuildFailureDetectorESP8266\r\nConnection: keep-alive\r\n\r\n";

//Arrays to buffer events to be send/received
//There is slot for every defined color
Event receivedEvents[nofColors] = { UNKNOWN };
Event outgoingEvents[nofColors] = { UNKNOWN };

//Button related parameters
volatile byte btn_pressed_counter = 0;
volatile byte btn_released_counter = 0;
volatile int btn_state = HIGH;
volatile unsigned long btn_pressed_time_ms = 0;
volatile unsigned long btn_released_time_ms = 0;
const unsigned long btn_long_press_threshold_ms = 750;
const unsigned long debounceDelay_ms = 50;
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
  //Sends message to the master button itself
  if(color == BTN_COLOR)
  {
    //Store the event
    receivedEvents[color] = event;
    return 0;
  }
  
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
  }
  return e;
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
  //Serial.println(outgoingEvents[static_cast<Color>(2)].type);
  for(size_t color = 0; color < nofColors; color++)
  {
    Event *e = &outgoingEvents[static_cast<Color>(color)];
    e->type = UNKNOWN;
    e->data = 0;
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
  }
  //Serial.println(receivedEvents[static_cast<Color>(2)].type);
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
      Serial.println("LED ON");
      digitalWrite(LED_PIN, HIGH);
      break;
    case LED_OFF:
      Serial.println("LED OFF");
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
      ledState = !ledState;
      (ledState) ? outgoingEvents[BTN_COLOR].type = LED_ON : outgoingEvents[BTN_COLOR].type = LED_OFF;
      Serial.println("BTN SHORT");
      //getColor(true);
      break;
    case BTN_LONG:
      //(blinkEnabled) ? outgoingEvents[BTN_COLOR].type = BLINK_OFF : outgoingEvents[BTN_COLOR].type = BLINK_ON;
      //Serial.println("BTN LONG");
      sendToDrive();
      break;
    case COLOR:
      break;
  }
}

//--------------------Blinking--------------------

//Blink the led <times> times
void blink(int times, int delay_ms = defaultBlinkDelay_ms)
{
  blinkEnabled = true;
  blinkCount = times;
  blinkDelay_ms = delay_ms;
  lastBlinkTime_ms = millis();
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

      if (ledState)
      {
        blinkCount--;
      }
      else
      {
        if(blinkCount == 0) blinkEnabled = false;
      }

    }
  }
}

//--------------------Interrupts--------------------

//Handles interrupt caused by the button press
ISR_PREFIX void handleInterrupt()
{
  const int btnState = digitalRead(BTN_PIN);
  unsigned long currentTime_ms = millis();
  if (currentTime_ms - lastDebounceTime_ms > debounceDelay_ms)
  {
    if (btnState == HIGH)
    {
      btn_pressed_counter++;
      btn_pressed_time_ms = currentTime_ms;
      //Serial.print("HIGH: ");
    }
    else
    {
      btn_released_counter++;
      btn_released_time_ms = currentTime_ms;
      //Serial.print("LOW: ");
    }
    lastDebounceTime_ms = currentTime_ms;
    //Serial.println(currentTime_ms);
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
    if (btn_released_time_ms - btn_pressed_time_ms > btn_long_press_threshold_ms)
    {
      btnEvent.type = BTN_LONG;
      if (debug) Serial.println("Long btn press");
    }
    else
    {
      btnEvent.type = BTN_SHORT;
      if (debug) Serial.println("Short btn press");
    }
    if (debug) Serial.print("Time: ");
    if (debug) Serial.println(btn_released_time_ms - btn_pressed_time_ms);

    btn_released_counter = 0;
    btn_pressed_counter = 0;
    btn_released_time_ms = millis();
    btn_pressed_time_ms = btn_released_time_ms;
    
  }

  if (btnEvent.type != UNKNOWN)
  {
    sendEvent(BTN_COLOR, btnEvent);
    //if (debug) Serial.println(btnEvent.type);
  }
}

//--------------------Button color--------------------

Color getColor(bool debug)
{
  if (debug)
  {
    Serial.println(digitalRead(DIP_PIN_1));
    Serial.println(digitalRead(DIP_PIN_2));
    Serial.println(digitalRead(DIP_PIN_3));
    Serial.println(digitalRead(DIP_PIN_4));
  }
  
  int bit0 = !digitalRead(DIP_PIN_1) << 3;
  int bit1 = !digitalRead(DIP_PIN_2) << 2;
  int bit2 = !digitalRead(DIP_PIN_3) << 1;
  int bit3 = !digitalRead(DIP_PIN_4);

  int dipValue = bit0 + bit1 + bit2 + bit3;
  if (debug)
  {
    Serial.print("DIP value: ");
    Serial.println(dipValue);
  }

  if (dipValue >= UNDEFINED)
  {
    return UNDEFINED;
  }

  return static_cast<Color>(dipValue);
}

//---------------------Wifi connections---------------------


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

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(DIP_PIN_1, INPUT);
  pinMode(DIP_PIN_2, INPUT);
  pinMode(DIP_PIN_3, INPUT);
  pinMode(DIP_PIN_4, INPUT);

  digitalWrite(LED_PIN, LOW);  //turn off by LOW

  //Serial connection for debugging purposes
  Serial.begin(9600);
  delay(1000);

  //Determine own color:
  BTN_COLOR = getColor(false);
  Serial.print("Color of the button: ");
  Serial.println(colorToString(BTN_COLOR));

  attachInterrupt(digitalPinToInterrupt(BTN_PIN), handleInterrupt, CHANGE);

  int tryWifiIndex = 0;
  while(WiFi.status() != WL_CONNECTED)
  {
    tryWifiIndex++;
    if (tryWifiIndex >= nofKnownWifis) tryWifiIndex = 0;
    WiFi.begin(ssids[tryWifiIndex], passwords[tryWifiIndex]);
    Serial.println("Trying to connect wifi...");
    delay(10000);
  }
  Serial.print("\nWifi connected! My ip: ");
  Serial.println(WiFi.localIP());
  Serial.println("Wifi parameters:");
  Serial.print("ssid: ");
  Serial.println(ssids[tryWifiIndex]);
  Serial.print("password: ");
  Serial.println(passwords[tryWifiIndex]);


  //5 blinks indicates successfull wifi connection
  blink(5);

  //Start the server
  wifiServer.begin();
  Serial.println("Wifi server started");

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

  //Handle buttons
  handleButtonPress(true);

  //Handle blinking
  handleBlinking();

  //Handle received events
  handleEvents(getEvent(BTN_COLOR));

  //Handle gameplay here
  //gameLogic();
  //testLogic();

  clearReceivedEvents();
  sendAllEvents();
  clearOutgoingEvents();
  receiveAllEvents();
}
