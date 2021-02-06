int ledPin = 2;
 
void setup()
{
    // Set LED as output
    pinMode(ledPin, OUTPUT);
    
    // Serial monitor setup
    Serial.begin(9600);
}
 
void loop()
{
    Serial.print("Hello");
    digitalWrite(ledPin, HIGH);
    
    delay(5000);
    
    Serial.println(" world!");
    digitalWrite(ledPin, LOW);
    
    delay(5000);
}
