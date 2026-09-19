#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

// RFID pins
#define SS_PIN 10
#define RST_PIN 9

// Other components
#define SERVO_PIN 6
#define BUZZER_PIN 7
#define GREEN_LED 4
#define RED_LED 5

MFRC522 rfid(SS_PIN, RST_PIN);
Servo doorServo;

// Change this UID to your authorized card UID
byte authorizedUID[] = {
  0xDE, 0xAD, 0xBE, 0xEF
};

const int LOCK_POSITION = 0;
const int UNLOCK_POSITION = 90;

const unsigned long UNLOCK_TIME = 5000;

bool isAuthorized(byte *uid, byte uidSize) {

  if (uidSize != sizeof(authorizedUID)) {
    return false;
  }

  for (byte i = 0; i < uidSize; i++) {
    if (uid[i] != authorizedUID[i]) {
      return false;
    }
  }

  return true;
}

void lockDoor() {

  doorServo.write(LOCK_POSITION);

  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, HIGH);

  Serial.println("Door LOCKED");
}

void unlockDoor() {

  doorServo.write(UNLOCK_POSITION);

  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);

  Serial.println("Door UNLOCKED");

  tone(BUZZER_PIN, 1500, 200);
}

void printUID() {

  Serial.print("Card UID: ");

  for (byte i = 0; i < rfid.uid.size; i++) {

    if (rfid.uid.uidByte[i] < 0x10) {
      Serial.print("0");
    }

    Serial.print(
      rfid.uid.uidByte[i],
      HEX
    );

    if (i < rfid.uid.size - 1) {
      Serial.print(" ");
    }
  }

  Serial.println();
}

void setup() {

  Serial.begin(9600);

  SPI.begin();
  rfid.PCD_Init();

  doorServo.attach(SERVO_PIN);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  lockDoor();

  Serial.println("============================");
  Serial.println("       RFID DOOR LOCK");
  Serial.println("============================");
  Serial.println("Scan your RFID card...");
}

void loop() {

  // Check whether a new RFID card is present
  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }

  // Read the RFID card
  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }

  printUID();

  if (isAuthorized(
        rfid.uid.uidByte,
        rfid.uid.size
      )) {

    Serial.println("ACCESS GRANTED");

    unlockDoor();

    delay(UNLOCK_TIME);

    lockDoor();

  } else {

    Serial.println("ACCESS DENIED");

    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);

    // Alarm sound
    for (int i = 0; i < 3; i++) {
      tone(BUZZER_PIN, 1000);
      delay(200);
      noTone(BUZZER_PIN);
      delay(150);
    }

  }

  // Stop communication with current card
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  delay(500);
}
