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
Packet packet;
Packet *tmpPacket;
bool updateLED = false;

void setup() {
    Serial.begin(115200);
    Serial1.begin(115200);
    //lcdSerial.begin(115200);
    crystalFontz635.init ( &Serial1 );
    //crystalFontz635.clearLCD();
    //crystalFontz635.getHardwareFirmwareVersion();
    memset ( &packet, 0, sizeof ( Packet ) );
    for ( int i = 0; i < CFA635_READBUFFER_COUNT; i++ ) {
        Serial.print ( "setting Packet: " );
        Serial.println ( i, DEC );
        packet.type = i + 0x10;
        packet.length = 0x00;
        crystalFontz635.setPacket ( &packet );
        //crystalFontz635.dumpReadBuffers();
    }
    Serial.println ( "After setting all" );
    crystalFontz635.dumpReadBuffers();
    crystalFontz635.setPacketPositions( 2, 5 );
    Serial.println ( "After setting packet positions 2,5" );
    crystalFontz635.dumpReadBuffers();

    packet.type = 0x46;
    packet.length = 0;
    crystalFontz635.setPacket ( &packet );
    Serial.println ( "After setting packet 5 to 0x46" );
    crystalFontz635.dumpReadBuffers();

    crystalFontz635.setPacketPositions( 3, 1 );
    Serial.println ( "After setting packet positions 3,1" );
    crystalFontz635.dumpReadBuffers();

    Serial.println ( "Fetching 0x46" );
    packet.type = 0x99;
    packet.length = 19;
    tmpPacket = crystalFontz635.getNextPacketOfType ( 0x46, &packet );
    crystalFontz635.dumpPacket ( "packet returned", (uint8_t *)tmpPacket );
    Serial.println ( "After calling getNextPacketOfType ( 0x46 )" );
    crystalFontz635.dumpReadBuffers();

    Serial.println ( "Fetching 0x12" );
    tmpPacket = crystalFontz635.getNextPacketOfType ( 0x12, &packet );
    crystalFontz635.dumpPacket ( "packet returned", (uint8_t *)tmpPacket );
    Serial.println ( "After calling getNextPacketOfType ( 0x12 )" );
    crystalFontz635.dumpReadBuffers();

    Serial.println ( "Fetching 0x17" );
    tmpPacket = crystalFontz635.getNextPacketOfType ( 0x17, &packet );
    if ( tmpPacket == NULL ) {
        Serial.println ( "test: returned Packet is null" );
    } else {
        crystalFontz635.dumpPacket ( "packet returned", (uint8_t *)tmpPacket );
    }
    Serial.println ( "After calling getNextPacketOfType ( 0x17 )" );
    crystalFontz635.dumpReadBuffers();

    packet.type = 0x99;
    packet.length = 19;
    Serial.println ( "Fetching 0x88 (should be null)" );
    tmpPacket = crystalFontz635.getNextPacketOfType ( 0x88, &packet );
    if ( tmpPacket == NULL ) {
        Serial.println ( "test: returned Packet is null" );
    } else {
        crystalFontz635.dumpPacket ( "packet returned", (uint8_t *)tmpPacket );
    }
    crystalFontz635.dumpReadBuffers();
}

void loop() {
    delay ( 500000000 );

}
