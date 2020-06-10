#include <Timer.h>
#include <Event.h>

/*********************************************************************
 This is an example for our nRF51822 based Bluefruit LE modules

 Pick one up today in the adafruit shop!

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/

#include <Arduino.h>
#include <SPI.h>
#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined (_VARIANT_ARDUINO_ZERO_)
  #include <SoftwareSerial.h>
#endif

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"

/*=========================================================================
    APPLICATION SETTINGS

  FACTORYRESET_ENABLE     Perform a factory reset when running this sketch
 
                            Enabling this will put your Bluefruit LE module
                              in a 'known good' state and clear any config
                              data set in previous sketches or projects, so
                            running this at least once is a good idea.
 
                            When deploying your project, however, you will
                              want to disable factory reset by setting this
                              value to 0. If you are making changes to your
                            Bluefruit LE device via AT commands, and those
                              changes aren't persisting across resets, this
                              is the reason why. Factory reset will erase
                              the non-volatile memory where config data is
                              stored, setting it back to factory default
                              values.
   
                            Some sketches that require you to bond to a
                              central device (HID mouse, keyboard, etc.)
                              won't work at all with this feature enabled
                              since the factory reset will clear all of the
                              bonding data stored on the chip, meaning the
                              central device won't be able to reconnect.
    MINIMUM_FIRMWARE_VERSION  Minimum firmware version to have some new features
    MODE_LED_BEHAVIOUR        LED activity, valid options are
                              "DISABLE" or "MODE" or "BLEUART" or
                              "HWUART"  or "SPI"  or "MANUAL"
    -----------------------------------------------------------------------*/
    #define FACTORYRESET_ENABLE         1
    #define MINIMUM_FIRMWARE_VERSION    "0.6.6"
    #define MODE_LED_BEHAVIOUR          "MODE"
/*=========================================================================*/

// Create the bluefruit object, either software serial...uncomment these lines

SoftwareSerial bluefruitSS = SoftwareSerial(BLUEFRUIT_SWUART_TXD_PIN, BLUEFRUIT_SWUART_RXD_PIN);

Adafruit_BluefruitLE_UART ble(bluefruitSS, BLUEFRUIT_UART_MODE_PIN,
                      BLUEFRUIT_UART_CTS_PIN, BLUEFRUIT_UART_RTS_PIN);


/* ...or hardware serial, which does not need the RTS/CTS pins. Uncomment this line */
// Adafruit_BluefruitLE_UART ble(Serial1, BLUEFRUIT_UART_MODE_PIN);

/* ...hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
//Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

/* ...software SPI, using SCK/MOSI/MISO user-defined SPI pins and then user selected CS/IRQ/RST */
//Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_SCK, BLUEFRUIT_SPI_MISO,
//                             BLUEFRUIT_SPI_MOSI, BLUEFRUIT_SPI_CS,
//                             BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);


// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

Timer t;
int regularFeedEvent;
/**************************************************************************/
/*!
    @brief  Sets up the HW an the BLE module (this function is called
            automatically on startup)
*/
/**************************************************************************/
void setup(void)
{
  while (!Serial);  // required for Flora & Micro
  delay(500);

  // initialize digital pin 13 as an output.
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  Serial.begin(115200);
  Serial.println(F("Adafruit Bluefruit Command Mode Example"));
  Serial.println(F("---------------------------------------"));

  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
      error(F("Couldn't factory reset"));
    }
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  Serial.println(F("Please use Adafruit Bluefruit LE app to connect in UART mode"));
  Serial.println(F("Then Enter characters to send to Bluefruit"));
  Serial.println();

  ble.verbose(false);  // debug info is a little annoying after this point!

  /* Wait for connection */
  while (! ble.isConnected()) {
      delay(500);
  }

  // LED Activity command is only supported from 0.6.6
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    // Change Mode LED Activity
    Serial.println(F("******************************"));
    Serial.println(F("Change LED activity to " MODE_LED_BEHAVIOUR));
    ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
    Serial.println(F("******************************"));
  }
}

/**************************************************************************/
/*!
    @brief  Constantly poll for new command or response data
*/
/**************************************************************************/
String releaseLatch, bleBuffer, feedInterval, releaseInterval;
long feedInt;
int releaseInt = 1000;
void loop(void)
{
  
  //timer lib
  t.update();
  
  // Check for user input
  char inputs[BUFSIZE+1];

  if ( getUserInput(inputs, BUFSIZE) )
  {
    // Send characters to Bluefruit
    Serial.print("[Send] ");
    Serial.println(inputs);

    ble.print("AT+BLEUARTTX=");
    ble.println(inputs);

    // check response stastus
    if (! ble.waitForOK() ) {
      Serial.println(F("Failed to send?"));
    }
  }

  // Check for incoming characters from Bluefruit
  ble.println("AT+BLEUARTRX");
  ble.readline();
  if (strcmp(ble.buffer, "OK") == 0) {
    // no data
    return;
  }
  releaseLatch = String("release_latch");
  // Some data was found, its in the buffer
  bleBuffer = ble.buffer;
  Serial.print(F("[Recv] ")); Serial.println(bleBuffer);
  if(bleBuffer == releaseLatch) {

    feedToto();

    //send ACK back to Android    
    // Check for user input
    char inputs[BUFSIZE+1];
    String ack = String("ac");
    
    ack.toCharArray(inputs, BUFSIZE);

    // Send characters to Bluefruit
    Serial.print("[Send] ");
    Serial.println(inputs);

    ble.print("AT+BLEUARTTX=");
    ble.println(inputs);

    // check response stastus
    if (! ble.waitForOK() ) {
      Serial.println(F("Failed to send?"));
    }

  } else if(bleBuffer.indexOf("feedinterval") > -1 ) {
      //expects -> feedinterval:seconds
      feedInterval = getValue(bleBuffer, ':', 1);
      feedInt = feedInterval.toInt();
      Serial.print("setting feedInterval to (seconds): ");
      Serial.println(feedInt);
      
      setRegularFeeding(feedInt);
//      t.stop(regularFeedEvent);
//      if(feedInt != 0) {
//        Serial.println("regular feeding set.");
////        int tickEvent = t.every(2000, feedTotoWrapper, NULL);
//        regularFeedEvent = t.every(feedInt * 1000, feedToto, NULL);
//      } else {
//          Serial.println("regular feeding canceled."); 
//          digitalWrite(13, LOW);
//          delay(1000); 
//      }
      
  } else if(bleBuffer.indexOf("releaseinterval") > -1 ) {
    //expects -> releaseinterval:milliseconds
    //expects -> String[] tickMarkLabels = {"1", "3", "5", "10", "20"};
      releaseInterval = getValue(bleBuffer, ':', 1);
      releaseInt = releaseInterval.toInt();
      if(releaseInt == 1) {
        releaseInt = 1 * 1000;
        } else if(releaseInt == 2) {
          releaseInt = 3 * 1000;     
        } else if(releaseInt == 3) {
          releaseInt = 5 * 1000; 
        } else if (releaseInt == 4) {
          releaseInt = 10 * 1000;  
        } else if(releaseInt == 5) {
          releaseInt = 20 * 1000;  
        } else {
          releaseInt = 1 * 1000;
        }
      Serial.print("RI:setting releaseInterval to (milliseconds): ");
      Serial.println(releaseInt);
      
  } else {
     //any other input put things back to LOW
     Serial.println("Other input: solenoid -> LOW");
     // releases solenoid - push position 
     t.stop(regularFeedEvent);
     digitalWrite(13, LOW);
     delay(1000);  
  }
  ble.waitForOK();


}

/**************************************************************************/
/*!
    @brief  Checks for user input (via the Serial Monitor)
*/
/**************************************************************************/
bool getUserInput(char buffer[], uint8_t maxSize)
{
  // timeout in 100 milliseconds
  TimeoutTimer timeout(100);

  memset(buffer, 0, maxSize);
  while( (!Serial.available()) && !timeout.expired() ) { delay(1); }

  if ( timeout.expired() ) return false;

  delay(2);
  uint8_t count=0;
  do
  {
    count += Serial.readBytes(buffer+count, maxSize);
    delay(2);
  } while( (count < maxSize) && (Serial.available()) );

  return true;
}

String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void setRegularFeeding(long feedInt_) {
  t.stop(regularFeedEvent);
  if(feedInt_ != 0) {
    Serial.println("regular feeding set.");
    regularFeedEvent = t.every(feedInt_ * 1000, feedToto, NULL);
  } else {
      Serial.println("regular feeding canceled."); 
      digitalWrite(13, LOW);
      delay(1000); 
  }
  
}  

void feedTotoLoop(int x) {
  for(int i = 0; i < x; i++) {
      digitalWrite(13, HIGH);
      delay(100);
      digitalWrite(13, LOW);
      delay(100);
      digitalWrite(13, HIGH);
      delay(100);
      digitalWrite(13, LOW);
      delay(100);
    }
  }

//one time feed
void feedToto() {
  //shake things
  feedTotoLoop(releaseInt/1000);
  
  //open up
//  // pulls solenoid
//  digitalWrite(13, HIGH); 
//  
//  // releases solenoid - push position
//  delay(20000);  
//  digitalWrite(13, LOW);

  //OR
//  Serial.println("feedtoto1");
//  t.pulse(13, 1000, HIGH);
//  Serial.println("feedtoto2");
  
  }

static void feedTotoWrapper(void* context) {
 feedToto();
}
