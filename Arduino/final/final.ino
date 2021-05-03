#include <SPI.h>
#include <MFRC522.h>
#include <CD74HC4067.h>

#define RST_PIN   9     // Configurable, see typical pin layout above
#define SS_PIN    10    // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance
MFRC522::MIFARE_Key key;
CD74HC4067 my_mux(4, 5, 6, 7); //Create CD74HC4067 instance

const int writeRFIDPin = 8;
const int writeMUXPin = 3;
const int readMUXPin = 2;

int currentDoorId = -1;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  SPI.begin();
  mfrc522.PCD_Init();

  pinMode(writeRFIDPin, OUTPUT);
  pinMode(writeMUXPin, OUTPUT);
  pinMode(readMUXPin, INPUT);

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
}

void loop() {

  handle_commands();
  read_from_rfid();
  read_from_door_sensor_mux();

  delay(1000);
}


void read_from_rfid() {

  // Look for new cards, and select one if present
  if ( ! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial() ) {
    delay(50);
    return;
  }
  //
  //  Serial.print(F("RFID"));
  //  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  //  Serial.println();

  // In this sample we use the second sector,
  // that is: sector #1, covering block #4 up to and including block #7
  byte sector         = 1;
  byte blockAddr      = 4;
  byte trailerBlock   = 7;
  MFRC522::StatusCode status;
  byte buffer[18];
  byte size = sizeof(buffer);


  // Authenticate using key A
  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.println(F("[RFIDFAIL]"));
    return;
  }

  // Read data from the block
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Read() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
  }

  Serial.print(F("[RFIDOK:"));
  dump_byte_array(buffer, 16);
  Serial.println(']');

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

void handle_commands() {
  if (Serial.available() > 0) {
    char command = Serial.read();
    switch (command) {
      case 'o':
        currentDoorId = Serial.parseInt();
        Serial.print(F("OPEN DOOR REQUEST:"));
        Serial.println(currentDoorId);
                
        digitalWrite(writeMUXPin, HIGH);
        my_mux.channel(currentDoorId);
        delay(5000);
        digitalWrite(writeMUXPin, LOW);
        my_mux.channel(currentDoorId);
        currentDoorId = -1;
        break;
    }
  }

}

void read_from_door_sensor_mux() {
  for (int i = 0; i < 16; i++) {

    
    
    my_mux.channel(i);
    float value = digitalRead(readMUXPin);
    //delay(50);
    //Serial.print(i);
    //Serial.print(F(":"));
    //Serial.println(value);
    if (value > 0) {
      Serial.print(F("[DOOR:"));
      Serial.print(i);
      Serial.println(']');
    }
    //Serial.println("");
  }
}

/**
   Helper routine to dump a byte array as hex values to Serial.
*/
void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? "0" : "");
    Serial.print(buffer[i], HEX);
  }
}
