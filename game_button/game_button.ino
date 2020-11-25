#include "ESP8266WiFi.h"

#define ISR_PREFIX ICACHE_RAM_ATTR

const char* ssid = "Kalat_ja_Rapu_2G";
const char* password = "";

const char* btn_pressed = "BTN_PRESSED";
const char* led_on = "LED_ON";
const char* led_off = "LED_OFF";
const char* led_blink = "LED_BLINKING";
const char* led_blink_off = "LED_BLINKING_OFF";
const char* timeout = "TIMEOUT";

volatile byte interruptCounter = 0;
const int LED_PIN = 0;
const int BTN_PIN = 14;
const int BZR_PIN = 2;  //builtin led represents buzzer
int btn_state = HIGH;
int blink_state = -1;
bool blink_enabled = false;
long lastDebounceTime = 0;
long debounceDelay = 500;
long lastBlinkTime = 0;
long blinkDelay = 1000;
WiFiServer wifiServer(80);

int handleRequest()
{
  return 0;
}

void blink(int times)
{
  int blinkDelay_ms = 200;
  for(int i = 0; i < times; i++)
  {
    delay(blinkDelay_ms);
    digitalWrite(LED_PIN, HIGH);
    delay(blinkDelay_ms);
    digitalWrite(LED_PIN, LOW);
  }
}

ISR_PREFIX void handleInterrupt()
{
  interruptCounter++;
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(BZR_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);  //turn off by LOW
  digitalWrite(BZR_PIN, HIGH); //turn off by HIGH

  attachInterrupt(digitalPinToInterrupt(BTN_PIN), handleInterrupt, FALLING);

  Serial.begin(9600);
  delay(1000);

  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED)
  {
    delay(3000);
    Serial.println("Trying to connect wifi...");
    Serial.print("ssid: ");
    Serial.println(ssid);
    Serial.print("password: ");
    Serial.println(password);
  }
  Serial.print("\nConnected! My ip: ");
  Serial.println(WiFi.localIP());
  Serial.println("");
  blink(5);
  wifiServer.begin();
}

void loop() {

  interruptCounter = 0;
  WiFiClient client = wifiServer.available();

  if(client)
  {
    Serial.println("Client connected");
    while(client.connected())
    {
      //Handle button press
      if(interruptCounter > 0)
      {
        interruptCounter = 0;
        if((millis() - lastDebounceTime) > debounceDelay)
        {
          Serial.println("BTN Pressed");
          client.println(btn_pressed);
          lastDebounceTime = millis();
        }
      }

      if(blink_enabled)
      {
        if(millis() - lastBlinkTime > blinkDelay)
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
      
      //Check if there are commands from the app
      while(client.available())
      {
        char c = client.read();
        switch(c)
        {
          case 'o':
            client.println(led_on);
            Serial.println("led on");
            digitalWrite(LED_PIN, HIGH);
            break;
          case 'f':
            client.println(led_off);
            Serial.println("led off");
            digitalWrite(LED_PIN, LOW);
            break;
          case 'b':
            client.println(led_blink);
            Serial.println("led blink");
            blink_enabled = true;
            break;
          case 'd':
            client.println(led_blink_off);
            Serial.println("led blink off");
            digitalWrite(LED_PIN, LOW);
            blink_enabled = false;
            break;
          case '\n':
            break;
          default:
            break;
        }
      }
      delay(10);
      
    }
    client.stop();
    Serial.println("Client disconnected");
  }

}
