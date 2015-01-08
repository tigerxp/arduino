#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "printf.h"
#define CE_PIN   9
#define CSN_PIN 10

const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

RF24 radio(CE_PIN, CSN_PIN); // Create a Radio
int myTemp[2];

void setup() {
  Serial.begin(57600);
//  delay(1000);
  Serial.println("Base Station starting");
  radio.begin();
  radio.setRetries(15, 15);
  radio.setPayloadSize(4);

  // Pipes config for receiver
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1, pipes[0]);

  radio.startListening();
  printf_begin();
  radio.printDetails();
}


void loop() {
  
    if (radio.available()) {
      // Dump the payloads until we've gotten everything
      bool done = false;
      unsigned int tempData;
      
      while (!done) {
        // Fetch the payload, and see if this was the last one.
        done = radio.read(&tempData, sizeof(tempData));

        char tempStr[10];
        ftoa(tempStr, realTemp(tempData), 2);        
        printf("Got temperature: %s (%i)... ", tempStr, tempData);

	delay(100);
      }

      radio.stopListening();
      radio.write(&tempData, sizeof(tempData));
      printf("Sent response.\n\r");

      // Resume listening so we catch the next packets.
      radio.startListening();
    }
    
  delay(1000);
} // loop


float realTemp(unsigned int data) {
    return (float) data / 16;
}

/*
 * Returns string representation of a float
 */
char *ftoa(char *a, double f, int precision)
{
  long p[] = {0,10,100,1000,10000,100000,1000000,10000000,100000000};
  
  char *ret = a;
  long heiltal = (long)f;
  itoa(heiltal, a, 10);
  while (*a != '\0') a++;
  *a++ = '.';
  long desimal = abs((long)((f - heiltal) * p[precision]));
  itoa(desimal, a, 10);
  return ret;
}

