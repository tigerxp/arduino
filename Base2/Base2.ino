#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "printf.h"
#define CE_PIN   9
#define CSN_PIN 10

const uint64_t pipe = 0xE8E8F0F0E1LL; // Define the transmit pipe
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

RF24 radio(CE_PIN, CSN_PIN); // Create a Radio
int myTemp[2];

void setup() {
  Serial.begin(57600);
  delay(1000);
  Serial.println("Nrf24L01 Receiver Starting");
  radio.begin();
  radio.setRetries(15,15);
  radio.setPayloadSize(8);
//  radio.openReadingPipe(1, pipe);

  // Receiving
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);

  radio.startListening();
  printf_begin();
  radio.printDetails();
}


void loop() {
  
    if ( radio.available() )
    {
      // Dump the payloads until we've gotten everything
      bool done = false;
      while (!done)
      {
        // Fetch the payload, and see if this was the last one.
        done = radio.read( &myTemp, sizeof(myTemp) );

        // Spew it
        float temp = tempFromBytes(myTemp[0], myTemp[1]);
        Serial.print("Got temp ");
        Serial.print(temp);
        Serial.print(" ");

	delay(100);
      }

      // First, stop listening so we can talk
      radio.stopListening();

      // Send the final one back.
      radio.write( &myTemp, sizeof(myTemp) );
      printf("Sent response.\n\r");

      // Now, resume listening so we catch the next packets.
      radio.startListening();
    }

  delay(1000);
  
/*  if ( radio.available() )
  {
    // Read the data payload until we've received everything
    bool done = false;
    while (!done)
    {
      // Fetch the data payload
      done = radio.read(myTemp, sizeof(myTemp));
      byte MSB = myTemp[1];
      byte LSB = myTemp[0];

      float tempRead = ((MSB << 8)   | LSB); //using two's compliment 
      float TemperatureSum = tempRead / 16;
      Serial.println(TemperatureSum);
    }
  }
  else
  {    
      Serial.println("No radio available");
      delay(1000);
  }
*/
} // loop

float tempFromBytes(byte lsb, byte msb) {
  return ((msb << 8) | lsb) / 16;
}

