#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN   9     // Configurable, see typical pin layout above
#define SS_PIN    10    // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

MFRC522::MIFARE_Key key;

void setup() {
  pinMode(8, OUTPUT);

  Serial.begin(9600);
  while (!Serial);
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println(F("Starting up..."));

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
}

void loop() {
  if (Serial.available()) {
    int state = Serial.parseInt();
    if (state == 1)
    {
      digitalWrite(8, HIGH);
    }
    if (state == 0)
    {
      digitalWrite(8, LOW);
    }
  }

  // Look for new cards, and select one if present
  if ( ! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial() ) {
    delay(50);
    return;
  }

  Serial.print(F("Card UID:"));
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println();

  // Dump PICC type
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.print(F("PICC type: "));
  Serial.print(mfrc522.PICC_GetTypeName(piccType));
  Serial.print(F(" (SAK "));
  Serial.print(mfrc522.uid.sak);
  Serial.print(")\r\n");
  if (  piccType != MFRC522::PICC_TYPE_MIFARE_MINI
        &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
        &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("This sample only works with MIFARE Classic cards."));
    return;
  }

  // In this sample we use the second sector,
  // that is: sector #1, covering block #4 up to and including block #7
  byte sector         = 1;
  byte blockAddr      = 4;
  byte dataBlock[]    = {
    0x00, 0x00, 0x00, 0x00, //  1,  2,   3,  4,
    0x00, 0x00, 0x00, 0x00, //  5,  6,   7,  8,
    0x00, 0x00, 0x00, 0x00, //  9, 10, 255, 12,
    0x00, 0x00, 0x00, 0x02  // 13, 14,  15, 16
  };
  byte trailerBlock   = 7;
  MFRC522::StatusCode status;
  byte buffer[18];
  byte size = sizeof(buffer);


  // Authenticate using key A
  Serial.println(F("Authenticating using key A..."));
  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Show the whole sector  as it currently is
  Serial.println(F("Current data in sector:"));
  mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
  Serial.println();

  // Read data from the block
  Serial.print(F("Reading data from block ")); Serial.print(blockAddr);
  Serial.println(F(" ..."));
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Read() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
  }
  Serial.print(F("Data in block ")); Serial.print(blockAddr); Serial.println(F(":"));
  dump_byte_array(buffer, 16); Serial.println();
  Serial.println();
//
//  // Authenticate using key B
//    Serial.println(F("Authenticating again using key B..."));
//    status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &key, &(mfrc522.uid));
//    if (status != MFRC522::STATUS_OK) {
//        Serial.print(F("PCD_Authenticate() failed: "));
//        Serial.println(mfrc522.GetStatusCodeName(status));
//        return;
//    }
//
//    // Write data to the block
//    Serial.print(F("Writing data into block ")); Serial.print(blockAddr);
//    Serial.println(F(" ..."));
//    dump_byte_array(dataBlock, 16); Serial.println();
//    status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(blockAddr, dataBlock, 16);
//    if (status != MFRC522::STATUS_OK) {
//        Serial.print(F("MIFARE_Write() failed: "));
//        Serial.println(mfrc522.GetStatusCodeName(status));
//    }
//    Serial.println();
//
//    // Read data from the block (again, should now be what we have written)
//    Serial.print(F("Reading data from block ")); Serial.print(blockAddr);
//    Serial.println(F(" ..."));
//    status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
//    if (status != MFRC522::STATUS_OK) {
//        Serial.print(F("MIFARE_Read() failed: "));
//        Serial.println(mfrc522.GetStatusCodeName(status));
//    }
//    Serial.print(F("Data in block ")); Serial.print(blockAddr); Serial.println(F(":"));
//    dump_byte_array(buffer, 16); Serial.println();
//        
//    // Check that data in block is what we have written
//    // by counting the number of bytes that are equal
//    Serial.println(F("Checking result..."));
//    byte count = 0;
//    for (byte i = 0; i < 16; i++) {
//        // Compare buffer (= what we've read) with dataBlock (= what we've written)
//        if (buffer[i] == dataBlock[i])
//            count++;
//    }
//    Serial.print(F("Number of bytes that match = ")); Serial.println(count);
//    if (count == 16) {
//        Serial.println(F("Success :-)"));
//    } else {
//        Serial.println(F("Failure, no match :-("));
//        Serial.println(F("  perhaps the write didn't work properly..."));
//    }
//    Serial.println();
//        
//    // Dump the sector data
//    Serial.println(F("Current data in sector:"));
//    mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
//    Serial.println();


  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  delay(2000);
}


/**
   Helper routine to dump a byte array as hex values to Serial.
*/
void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}
