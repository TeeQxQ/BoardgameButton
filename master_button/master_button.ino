#include "ESP8266WiFi.h"

#define ISR_PREFIX ICACHE_RAM_ATTR

//Hardware parameters:
const int LED_PIN = 0;
const int BTN_PIN = 14;

//Wifi parameters:
const char* ssid = "Kalat_ja_Rapu_2G";
const char* password = "";
const int WIFI_PORT = 80;
const int MAX_NOF_CLIENTS = 3;  //TODO change to const

//Wifi related variables:
WiFiServer wifiServer(WIFI_PORT);
WiFiClient *clients[MAX_NOF_CLIENTS] = { NULL };
String input_msgs[4];     //Messages from myself are stored also

//Messages:
const String msg_led_on = "LED_ON";
const String msg_led_off = "LED_OFF";
const String msg_led_blink = "LED_BLINKING";
const String msg_led_blink_off = "LED_BLINKING_OFF";
const String msg_btn_pressed = "BTN_SHORT";
const String msg_btn_pressed_long = "BTN_LONG";

//Blinking related parameters
const int blinkDelay_ms = 1000;
bool blink_enabled = false;

//Button related parameters
const long debounceDelay_ms = 500;

//Global variables:
//volatile byte interruptCounter = 0;
int btn_state = HIGH;
int blink_state = -1;
long lastDebounceTime = 0;
long lastBlinkTime = 0;

//Enumerations for different button colors
enum BUTTON_TYPE
{
  RED = 0,
  BLUE = 1,
  GREEN = 2,
  MY_COLOR = 3
};

//Game logic related variables:
int led_states[4];


//--------------------Functions--------------------

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
  if(blink_enabled)
  {
    if(millis() - lastBlinkTime > blinkDelay_ms)
    {
      blink_state = -blink_state;
      if(blink_state > 0)
      {
        digitalWrite(LED_PIN, HIGH);
      }
      else
      {
        digitalWrite(LED_PIN, LOW);
      }
      lastBlinkTime = millis();
    }
  }
}

//Interrupt handler routine
//Handles button press on this device
ISR_PREFIX void handleInterrupt()
{
  if((millis() - lastDebounceTime) > debounceDelay_ms)
  {
    input_msgs[MY_COLOR] = msg_btn_pressed;
    lastDebounceTime = millis();
  }
}

//Check new clients 
//If new clients are available add them to the client list
int checkNewClients()
{
  WiFiClient newClient = wifiServer.available();
  if (newClient)
  {
    //New client found
    //Add it to the list if there is space left
    for (size_t i = 0; i < MAX_NOF_CLIENTS; i++)
    {
      if (clients[i] == NULL)
      {
        clients[i] = new WiFiClient(newClient);
        Serial.println("New client added successfully");
      }
    }
    Serial.println("New client available, but no space left on the client list.");
    return -1;
  }
  return 0;
}

//Check if some clients have disconnected
//Remove them from the list
void removeDisconnectedClients()
{
  for (size_t i = 0; i < MAX_NOF_CLIENTS; i++)
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

//Check whether clients have new messages
//If new messages are available add messages to the list
int checkNewMessages()
{
  for (size_t i = 0; i < MAX_NOF_CLIENTS; i++)
  {
    //Check if client exists and has any messages
    if (clients[i] != NULL && clients[i]->available())
    {
      //Read the message and store it
      input_msgs[i] = clients[i]->readStringUntil('\n');
      Serial.print("New message received from index: ");
      Serial.println(i);
    }
  }
  
  return 0;
}

//Get new message from memory
//TODO change parameter to button type
String getNewMessage(int index)
{
  String msg = input_msgs[index];
  //input_msg[index] = "";
  return msg;
}

//Clear new messages from memory
void clearNewMessages()
{
  for (size_t i = 0; i < MAX_NOF_CLIENTS; i++)
  {
    input_msgs[i] = "";
  }
}

//Send message to the client
int sendMessage(const String newMessage, size_t atIndex)
{
  if(clients[atIndex] != NULL && clients[atIndex]->available())
  {
    clients[atIndex]->println(newMessage);
  }
  else
  {
    Serial.print("Client at index ");
    Serial.print(atIndex);
    Serial.println("is not connected!");
    return -1;
  }

  return 0;
}

//This is only for testing purposes
void testLogic()
{
  //Read messages
  for (size_t i = 0; i < MAX_NOF_CLIENTS +1 ; i++)
  {
    //Handle button press
    if (input_msgs[i] == msg_btn_pressed)
    {
      //Send message to change led state
      led_states[i] = -led_states[i];
      (led_states[i] == HIGH) ? sendMessage(msg_led_on, i) : sendMessage(msg_led_off, i);
    }
  }

  //clear messages after handling
  clearNewMessages();
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  digitalWrite(LED_PIN, LOW);  //turn off by LOW

  attachInterrupt(digitalPinToInterrupt(BTN_PIN), handleInterrupt, FALLING);

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
    delay(3000);
    Serial.println("Trying to connect wifi...");
  }
  Serial.print("\nWifi connected! My ip: ");
  Serial.println(WiFi.localIP());
  Serial.println("");

  //5 blinks indicates successfull wifi connection
  blink(5);
}

//--------------------Main program--------------------

void loop() {
  
  //Check if some existing clients disconnected:
  removeDisconnectedClients();

  //Check if new clients are available and store them
  if (checkNewClients() != 0) Serial.println("Error while adding client");

  //Check whether clients have new messages
  checkNewMessages();

  //Interrupt handler adds messages from this device

  /*
   * 
   * Handle messages and game logic here
   * Test example testLogic() next:
   * 
   */

  testLogic();

  delay(10);
}
