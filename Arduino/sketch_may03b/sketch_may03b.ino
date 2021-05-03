#include <CD74HC4067.h>

CD74HC4067 my_mux(4, 5, 6, 7);
const int writeCommonPin = 3;
const int readCommonPin = A0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial);

  pinMode(writeCommonPin, OUTPUT);
  pinMode(readCommonPin, INPUT);
}

void loop() {
//  if (!Serial.available()) {
//    delay(50);
//    return;
//  }
  // put your main code here, to run repeatedly:

 // digitalWrite(3, HIGH);

//
//  for (int i = 0; i < 16; i++) {
//    my_mux.channel(i);
//    
//    float value = digitalRead(readCommonPin);
//    Serial.print(i);
//    Serial.print(F(":"));
//    Serial.print(value);
//    Serial.println(F("----"));
//    delay(250);
//  }

 



}
