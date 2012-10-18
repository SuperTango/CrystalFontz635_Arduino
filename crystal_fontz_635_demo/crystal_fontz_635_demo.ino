#include <SoftwareSerial.h>
#include <CrystalFontz635.h>

CrystalFontz635 crystalFontz635;
// host tx/LCD Rx: Green, arduino 3
// host rx;LCD Tx: White, arduino 7
//                                       rx, tx
SoftwareSerial lcdSerial = SoftwareSerial(7, 3);
uint8_t buffer[256];
unsigned long lastTime = 0;

void setup() {
    Serial.begin(115200);
    Serial1.begin(115200);
    lcdSerial.begin(115200);
    crystalFontz635.init ( &Serial1 );
    crystalFontz635.clearLCD();
    /*
    buffer[0] = 0x1F;
    buffer[1] = 0x07;
    buffer[2] = 0x01;
    buffer[3] = 0x48;
    buffer[4] = 0x65;
    buffer[5] = 0x6C;
    buffer[6] = 0x6C;
    buffer[7] = 0x6C;
    buffer[8] = 0xA6;
    buffer[9] = 0x4C;
    Serial.println ( "About to send data3" );
    */
    //Serial1.write ( buffer, 10 );
    /*
    for ( int i = 0; i < 10; i ++ ) {
        Serial.println ( buffer[i], HEX );
        Serial1.write ( buffer[i] );
    }
    */

/*
    buffer[0] = 0x16;
    buffer[1] = 0x00;
    buffer[2] = 0x97;
    buffer[3] = 0x5B;
    Serial.println ( "About to send data2" );
    Serial1.write ( buffer, 4 );
*/
/*
    buffer[0] = 0x00;
    buffer[1] = 0x01;
    buffer[2] = 0x05;
    buffer[3] = 0x3A;
    buffer[4] = 0xB9;
    Serial.println ( "About to send data2" );
    Serial1.write ( buffer, 5 );
*/
/*
    Serial.println ( "About to test clear screen" );
    crystalFontz635.clearLCD();
*/
    /*
    Serial.println ( "About to set cursor position to row 2, column 15" );
    crystalFontz635.setCursorPosition ( 2, 15 );
    */
    //Serial.println ( "About to write string 'Hello' to 3,1 (row,col)" );
    //crystalFontz635.writeString ( 3, 1, "Hello" );
    /*
    */
}

void loop() {
    int data = 0;
    /*
    for ( int i = 0; i < 10; i ++ ) {
        Serial.println ( buffer[i], HEX );
        Serial1.write ( buffer[i] );
    }
    */

    char str[20];
    //Serial1.write ( 0xA0 );
    //Serial.println ( 'A' );
    //delay ( 1000 );
    //Serial.println ( "Waiting for data" );
    unsigned long now = millis();
    char buf[8];
    if ( ( now - lastTime ) > 1000 ) {
        unsigned long t1 = millis();
        //String ( "Millis: " + String(millis()) ).toCharArray(str, 20);
        //crystalFontz635.getHardwareFirmwareVersion();
        //crystalFontz635.clearLCD();
        //crystalFontz635.clearLCD();
        crystalFontz635.writeString (0, 0, "Millis:" );
        crystalFontz635.writeString (0, 8, ultoa ( millis(), buf, 10 ) );
        Serial.println ( millis() - t1 );
        lastTime = now;
    }
    if (Serial1.available() > 0) {
        data = Serial1.read();
        Serial.print ( "Got data: " );
        Serial.print ( data, DEC );
        Serial.print ( " (0x" );
        Serial.print ( data, HEX );
        Serial.print ( ")" );
        if ( ( data >= 32 ) && ( data <= 126 ) ) {
            Serial.print ( " '" );
            Serial.print ( (char)data );
            Serial.print ( "'" );
        }
        Serial.println();
    }
}

int incomingByte = 0;
void loop2() {
/*
        // send data only when you receive data:
        if (Serial1.available() > 0) {
                // read the incoming byte:
                incomingByte = Serial1.read();

                // say what you got:
                Serial.print("I received: ");
                Serial.println(incomingByte, HEX);
        }
        */
}

