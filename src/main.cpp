#include <Arduino.h>
#include <CAN.h>
#include "ICM_20948.h"
#include <Wire.h>
// #include "helper.h"
ICM_20948_I2C myICM;

#define TX_INTR_PIN 5   //from UNO to MEGA
#define CAN_MSG_ID 0x13
#define SEND_MSG_INTR_PIN 2 //from MEGA to UNO 
unsigned char bytes[4];
float accx=0;

void sendMsg() {
  CAN.beginPacket(CAN_MSG_ID);
  CAN.write(bytes[0]);
  CAN.write(bytes[1]);
  CAN.write(bytes[2]);
  CAN.write(bytes[3]);
  CAN.write(0);
  CAN.write(0);
  CAN.write(0);
  CAN.write(0);
  CAN.endPacket();

  //Indicate to Rx that transmission done
  digitalWrite(TX_INTR_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TX_INTR_PIN, LOW);
  Serial.print("ACCx: ");
  Serial.print(accx);
  Serial.println();
}

void setup()
{
  Serial.begin(9600);
  while (!Serial);

  pinMode(TX_INTR_PIN, OUTPUT);

  // Setup ICM20948
  Wire.begin();
  Wire.setClock(400000);
  myICM.enableDebugging();
  bool initialized = false;
  while (!initialized)
  {
    myICM.begin(Wire, 1); // Address is set to 0x69
    Serial.print(F("Initialization of the sensor returned: "));
    Serial.println(myICM.statusString());
    if (myICM.status != ICM_20948_Stat_Ok)
    {
      Serial.println("Trying again...");
      delay(500);
    }
    else
    {
      initialized = true;
      Serial.println("ICM20948 init success");
    }
  }

  Serial.println("CAN Sender");
  CAN.setPins(10, 2); // CS, INT (INT unused)

  // start the CAN bus at 500 kbps
  if (!CAN.begin(500E3))
  {
    Serial.println("Starting CAN failed!");
    while (1)
      ;
  }

  //Start the interrupt
  attachInterrupt(digitalPinToInterrupt(SEND_MSG_INTR_PIN), sendMsg, FALLING);
}

// Function to break a floating point number into 4 bytes
void breakFloat(float num, unsigned char *bytes) {
    // Assuming little endian architecture
    unsigned char *ptr = (unsigned char *)&num;
    
    // Copy each byte of the floating point number
    for (int i = 0; i < sizeof(float); ++i) {
        bytes[i] = *(ptr + i);
    }
}

void loop()
{
  if (myICM.dataReady())
  {
    myICM.getAGMT();         
    delay(30);
  }
  accx = myICM.accX();
  breakFloat(accx, bytes);
}
