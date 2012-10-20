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
uint8_t buffer[20];
PString bufferString((char*)buffer, sizeof(buffer));

unsigned long lastTime = 0;

uint8_t ledValues[] = { 0, 5, 15, 25, 100 };
int8_t redValue = 0;
int8_t greenValue = 0;

unsigned long now;
Packet *packet;
bool updateLED = false;

void setup() {
    Serial.begin(115200);
    Serial1.begin(115200);
    //lcdSerial.begin(115200);
    crystalFontz635.init ( &Serial1 );
    crystalFontz635.clearLCD();
    crystalFontz635.getHardwareFirmwareVersion();
}

void loop() {
    now = millis();
    updateLED = false;
    crystalFontz635.processInput();
    while ( packet = crystalFontz635.getNextPacket() ) {
        if ( packet->type == CFA635_PACKET_TYPE_KEY_ACTIVITY ) {
            if ( CFA635_KEY_UP_PRESS == packet->data[0] ) {
                redValue++;
                if ( redValue > 4 ) {
                    redValue = 4;
                }
                updateLED = true;
            } else if ( CFA635_KEY_DOWN_PRESS == packet->data[0] ) {
                redValue--;
                if ( redValue < 0  ) {
                    redValue = 0;
                }
                updateLED = true;
            } else if ( CFA635_KEY_RIGHT_PRESS == packet->data[0] ) {
                greenValue++;
                if ( greenValue > 4 ) {
                    greenValue = 4;
                }
                updateLED = true;
            } else if ( CFA635_KEY_LEFT_PRESS == packet->data[0] ) {
                greenValue--;
                if ( greenValue < 0  ) {
                    greenValue = 0;
                }
                updateLED = true;
            } else if ( CFA635_KEY_ENTER_PRESS == packet->data[0] ) {
                redValue = 4;
                greenValue = 4;
                updateLED = true;
            } else if ( CFA635_KEY_EXIT_PRESS == packet->data[0] ) {
                redValue = 0;
                greenValue = 0;
                updateLED = true;
            } else {
                //crystalFontz635.dumpPacket ( "Got a valid packet: ", (uint8_t *)packet );
            }
        } else {
            //Serial.println ( "Got a packet that we don't care about" );
        }
    }
    if ( updateLED ) {
        bufferString.begin();
        bufferString.print ( "Red LED:   " );
        if ( ledValues[redValue] < 10 ) {
            bufferString.print ( "  " );
        } else if ( ledValues[redValue] < 100 ) {
            bufferString.print ( " " );
        }
        bufferString.print ( ledValues[redValue] );
        crystalFontz635.printAt ( 1, 0, (char *)buffer );
        bufferString.begin();
        bufferString.print ( "Green LED: " );
        if ( ledValues[greenValue] < 10 ) {
            bufferString.print ( "  " );
        } else if ( ledValues[greenValue] < 100 ) {
            bufferString.print ( " " );
        }
        bufferString.print ( ledValues[greenValue] );
        crystalFontz635.printAt ( 2, 0, (char *)buffer );
        crystalFontz635.setLED ( 0, ledValues[redValue], ledValues[greenValue] );
    }
    /*
    if ( ( now - lastTime ) > 1000 ) {
        crystalFontz635.setLED ( random(4) , random(100), random(100) );

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
