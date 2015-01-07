#include <OneWire.h> 
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "printf.h"

/*-----( Declare Constants and Pin Numbers )-----*/
#define CE_PIN   9
#define CSN_PIN 10

const uint64_t pipe = 0xF0F0F0F0E1LL; // Define the transmit pipe
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

int DS18S20_Pin = 2; //DS18S20 Signal pin on digital 2

// Temperature chip i/o
OneWire ds(DS18S20_Pin);  // on digital pin 2
// RF init
RF24 radio(CE_PIN, CSN_PIN); // Create a Radio
int myTemp[2];

void setup(void) {
  Serial.begin(57600);
  delay(1000);
  radio.begin();
  radio.setRetries(15,15);
  radio.setPayloadSize(8);
  
  // Sending
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);

  printf_begin();

  radio.startListening();
  radio.printDetails();
//  radio.openWritingPipe(pipe);
}

void loop(void) {
  unsigned int tempWord = getTemp();
  float temperature = tempFromWord(tempWord);
  
//  printf("Sending %d ", temperature);
  Serial.print("Sending ");
  Serial.print(temperature);
  Serial.print(" ");
  boolean res;
  
  radio.stopListening();
  
  res = radio.write( &myTemp, sizeof(myTemp));
  
  if (res)
    printf("ok...");
  else
    printf("failed.\n\r");
      
  // Now, continue listening
  radio.startListening();

  // Wait here until we get a response, or timeout (250ms)
  unsigned long started_waiting_at = millis();
  bool timeout = false;
  while ( ! radio.available() && ! timeout )
    if (millis() - started_waiting_at > 350 )
      timeout = true;

    // Describe the results
    if ( timeout )
    {
      printf("Failed, response timed out.\n\r");
    }
    else
    {
      // Grab the response, compare, and send to debugging spew
      int gotTemp[2];
      radio.read( &gotTemp, sizeof(gotTemp) );

      // Spew it
      printf("Got response %i %i \n\r", gotTemp[0], gotTemp[1]);
    }

  // Serial.println(res);
  delay(2000);
}


unsigned int getTemp(){
  //returns the temperature from one DS18S20 in DEG Celsius

  byte data[12];
  byte addr[8];

  if ( !ds.search(addr)) {
      //no more sensors on chain, reset search
      ds.reset_search();
      return -1000;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return -1000;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
      Serial.print("Device is not recognized");
      return -1000;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); // start conversion, with parasite power on at the end

  byte present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE); // Read Scratchpad

  
  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = ds.read();
  }
  
  ds.reset_search();

  return ((data[1] << 8) | data[0]);

/*
  
  byte MSB = data[1];
  byte LSB = data[0];
  // TODO: make it better
  myTemp[0] = data[0];
  myTemp[1] = data[1];

  float tempRead = ((MSB << 8)   | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;
  
  Serial.println(TemperatureSum);
*/
}


float tempFromWord(unsigned int data) {
  return (float)data / 16;
}
