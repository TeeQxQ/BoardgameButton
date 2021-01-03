#include "ESP8266WiFi.h"
#include "messages.h"

#define ISR_PREFIX ICACHE_RAM_ATTR

//Hardware parameters:
const int LED_PIN = 0;
const int BTN_PIN = 14;

//Wifi parameters:
const char* ssid = "Kalat_ja_Rapu_2G";
const char* password = "rutaQlli";

//Wifi related variables:
WiFiClient wifiClient;
const char* clientAddress = "192.168.1.115";
const int clientPort = 80;

//Blinking related parameters
const int blinkDelay_ms = 500;
bool blink_enabled = false;

//Button related parameters
const long debounceDelay_ms = 500;

//Global variables:
volatile byte interruptCounter = 0;
int btn_state = HIGH;
int blink_state = -1;
long lastDebounceTime = 0;
long lastBlinkTime = 0;

//Communication related:
String messageToSent = "";
String messageToRead = "";

//Game related:
const String myColor = msg_color_green; //green

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

//Handles interrupt caused by the button press
ISR_PREFIX void handleInterrupt()
{
  interruptCounter++;
}

//Handles button press
void handleButtonPress()
{
  if(interruptCounter > 0)
  {
    interruptCounter = 0;
    if((millis() - lastDebounceTime) > debounceDelay_ms)
    {
      Serial.println("BTN Pressed");
      messageToSent = msg_btn_pressed_short;
      lastDebounceTime = millis();
    }
  }
}

//Wait until main button is available and then connect
int connectToMainButton(const int waitTime_ms = 1000)
{
  if (!wifiClient.connect(clientAddress, clientPort))
  {
    delay(waitTime_ms);
    return -1;
  }
  Serial.print("Connected to ip: ");
  Serial.print(clientAddress);
  Serial.print(" port: ");
  Serial.println(clientPort);

  return 0;
}

//Send message to the main button if connected
int sendMessage()
{
  if (messageToSent != "")
  {
    if (wifiClient)
    {
      wifiClient.println(messageToSent);
    }
    else
    {
      Serial.println("Master button was lost");
      return 1;
    }
    messageToSent = "";
  }
  return 0;
}

//Read new messages from the main button if connected
int checkNewMessages()
{
  if (wifiClient)
  {
    while (wifiClient.available())
    {
      char c = wifiClient.read();
      if (!(c == '\n' || c == '\r')) messageToRead += String(c);
    }
  }
  else
  {
    return 1;
  }
  return 0;
}

//Handles messages received from the main button
int handleNewMessages(bool clear = true)
{
  if (messageToRead != "")
  {

    if (messageToRead == msg_led_on)
    {
      Serial.println("led on");
      digitalWrite(LED_PIN, HIGH);
    }
    else if (messageToRead == msg_led_off)
    {
      Serial.println("led off");
      digitalWrite(LED_PIN, LOW);
    }
    else if (messageToRead == msg_ask_color)
    {
      messageToSent = myColor;
    }
    else
    {
      Serial.print("Unknown message received: ");
      Serial.println(messageToRead);
    }
    
    //Clear message:
    if (clear) messageToRead = "";
  }

  delay(10);
  
  return 0;
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  digitalWrite(LED_PIN, LOW);  //turn off by LOW

  attachInterrupt(digitalPinToInterrupt(BTN_PIN), handleInterrupt, FALLING);

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
  Serial.print("\nWifi Connected! My ip: ");
  Serial.println(WiFi.localIP());
  Serial.println("");

  //5 blinks indicates successfull wifi connection
  blink(5);
}

//--------------------Main program--------------------

void loop() {
  interruptCounter = 0;

  //Connect to the main button
  while (connectToMainButton() != 0)
  {
    Serial.println("Trying to connect to the main button..."); 
  }

  if (wifiClient)
  {
    //While connection to the main button is alive:
    while(wifiClient.connected())
    {
      delay(10);

      //Handle buttons
      handleButtonPress();

      //Handle blinking
      handleBlinking();
      
      //Read if there are any new messages from the main button
      if (checkNewMessages() != 0) Serial.println("Error while reading new messages.");

      //Handle new messages
      if (handleNewMessages() != 0) Serial.println("Error while handling new messages");

      //Send new messages
      if (sendMessage() != 0) Serial.println("Error while sending message.");

    }
  }
  
  wifiClient.stop();
  Serial.print("Disconnected: ");
  Serial.print(clientAddress);
  Serial.print(" port: ");
  Serial.println(clientPort);
}
