/**
 * Copyright (c) 2012 Alex Tang
 */
//#include <SoftwareSerial.h>
#include <PString.h>
#include <CrystalFontz635.h>

CrystalFontz635 crystalFontz635;
// host tx/LCD Rx: Green, arduino 3
// host rx;LCD Tx: White, arduino 7
//                                       rx, tx
//SoftwareSerial lcdSerial = SoftwareSerial(7, 3);
uint8_t buffer[256];
unsigned long lastTime = 0;

void setup() {
    Serial.begin(115200);
    Serial1.begin(115200);
    //lcdSerial.begin(115200);
    crystalFontz635.init ( &Serial1 );
    crystalFontz635.clearLCD();
    crystalFontz635.getHardwareFirmwareVersion();
}

void loop() {
    int data = 0;
    char str[20];
    unsigned long now = millis();
    char buf[8];
    if ( ( now - lastTime ) > 1000 ) {
        unsigned long t1 = millis();
        // TEST1: 40ms, 10,646 bytes
        //String ( "Millis: " + String(millis()) ).toCharArray(str, 20);
        //crystalFontz635.printAt (0, 0, str );
        
        // TEST 2: 67ms, 8742 bytes.
        //crystalFontz635.printAt (0, 0, "Millis:" );
        //crystalFontz635.printAt (0, 9, ultoa ( millis(), buf, 10 ) );
        
        // TEST3: 
        crystalFontz635.printAt (0, 0, "Millis:" );
        crystalFontz635.printAt (0, 9, millis() );
        //crystalFontz635.printAt (1, 0, 43.23467, 8, 3 );
        
        Serial.println ( millis() - t1 );
        lastTime = now;
    }
    */
}
