#include <OneWire.h> 
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "printf.h"

/*-----( Declare Constants and Pin Numbers )-----*/
#define CE_PIN   9
#define CSN_PIN 10
#define RCV_TIMEOUT 1000

const uint64_t pipes[2] = {0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL};

int DS18S20_Pin = 2; //DS18S20 Signal pin on digital 2

// Temperature chip i/o
OneWire ds(DS18S20_Pin);  // on digital pin 2
// RF init
RF24 radio(CE_PIN, CSN_PIN); // Create a Radio

void setup(void) {
    Serial.begin(57600);
    delay(1000);
    
    // Set up radio
    radio.begin();
    radio.setRetries(15, 15);
    radio.setPayloadSize(4);

    // Sending
    radio.openWritingPipe(pipes[0]);
    radio.openReadingPipe(1, pipes[1]);

    printf_begin();

    radio.startListening();
    radio.printDetails();
}

void loop(void) {
    unsigned int tempWord = getTemp();
    float temperature = realTemp(tempWord);
    char tempStr[10];

    ftoa(tempStr, temperature, 2); 
    printf("Sending %s (%i) ", tempStr, tempWord);

    // Stop listening and write
    radio.stopListening();
    boolean res = radio.write(&tempWord, sizeof(tempWord));
    if (res)
        printf("ok...");
    else
        printf("failed.\n\r");

    // Now, continue listening
    radio.startListening();

    // Wait here until we get a response, or timeout (250ms)
    unsigned long started_waiting_at = millis();
    bool timeout = false;
    while (!radio.available() && !timeout)
        if (millis() - started_waiting_at > RCV_TIMEOUT)
            timeout = true;

    // Describe the results
    if (timeout) {
        printf("Failed, response timed out.\n\r");
    } else {
        // Grab the response, compare, and send to debugging spew
        unsigned int gotTemp;
        radio.read(&gotTemp, sizeof(gotTemp));

        // Show what we've got from base
        temperature = realTemp(gotTemp);
        ftoa(tempStr, temperature, 2);
        printf("Got response %s (%i) \n\r", tempStr, gotTemp);
    }

    delay(5000);
}

/*
 * Returns the temperature from one DS18S20 in DEG Celsius
 */
unsigned int getTemp() {
    //

    byte data[12];
    byte addr[8];

    if (!ds.search(addr)) {
        //no more sensors on chain, reset search
        ds.reset_search();
        return 0;
    }

    if (OneWire::crc8(addr, 7) != addr[7]) {
        Serial.println("CRC is not valid!");
        return 0;
    }

    if (addr[0] != 0x10 && addr[0] != 0x28) {
        Serial.print("Device is not recognized");
        return 0;
    }

    ds.reset();
    ds.select(addr);
    ds.write(0x44, 1); // start conversion, with parasite power on at the end

    byte present = ds.reset();
    ds.select(addr);
    ds.write(0xBE); // Read Scratchpad


    for (int i = 0; i < 9; i++) { // we need 9 bytes
        data[i] = ds.read();
    }

    ds.reset_search();

    return ((data[1] << 8) | data[0]);

}

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

