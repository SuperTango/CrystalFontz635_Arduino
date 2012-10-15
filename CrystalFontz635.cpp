/**
 * Copyright (c) 2012 Alex Tang
 */
#if ARDUINO>=100
#include <Arduino.h> // Arduino 1.0
#else
#include <Wprogram.h> // Arduino 0022
#endif
#include <stdint.h>

#include <stdio.h>
#include <avr/io.h>
#include <inttypes.h>
#include "CrystalFontz635.h"

CrystalFontz635::CrystalFontz635() {
}

void CrystalFontz635::init ( Stream *stream2 ) {
    stream = stream2;
}

void CrystalFontz635::clearWriteBuffer() {
    memset ( buffer, 0, CFA635_WRITEBUFFER_SIZE );
}

void CrystalFontz635::clearLCD() {
    clearWriteBuffer();
    buffer[0] = 0x06;
    buffer[1] = 0;
    sendPacket();
}

void CrystalFontz635::setCursorPosition ( int row, int column ) {
    clearWriteBuffer();
    buffer[0] = 0x0B;
    buffer[1] = 2;
    buffer[2] = column;
    buffer[3] = row;
    sendPacket();
}

void CrystalFontz635::writeString ( uint8_t row, uint8_t column, char *string ) {
    clearWriteBuffer();
    buffer[0] = 0x1F;
    if ( string == NULL ) {
        buffer[1] = 0;
    } else {
        buffer[1] = strlen ( string ) + 2;
        memcpy ( &buffer[4], string, strlen ( string ) );
    }
    buffer[2] = column;
    buffer[3] = row;
    sendPacket();
}

#ifdef CFA635_DEBUG
void CrystalFontz635::dumpPacket2 ( char *str ) {
    Serial.print ( "Dumping Packet: " );
    Serial.println ( str );
    Serial.print ( "  Command: " );
    Serial.println ( (uint8_t)buffer[0], HEX );
    Serial.print ( "  length:  " );
    Serial.println ( (uint8_t)buffer[1], HEX );
    for ( int i = 2; i < ( buffer[1] + 2 ); i++ ) {
        Serial.print ( "  data " );
        Serial.print ( i - 2, DEC );
        Serial.print ( ":  " );
        Serial.println ( buffer[i], HEX );
    }
    uint16_t crc = ( ( buffer[buffer[1] + 3] << 8 ) & 0xFF00 ) | buffer[buffer[1] + 2] & 0xFF;

    Serial.print ( "  CRC:     " );
    Serial.print ( crc, HEX );
    Serial.print ( " (" );
    Serial.print ( (uint8_t)buffer[buffer[1] + 2], HEX );
    Serial.print ( ", " );
    Serial.print ( (uint8_t)buffer[buffer[1] + 3], HEX );
    Serial.println ( ")" );
    Serial.println ( "Done" );
}
#endif


void CrystalFontz635::sendPacket() {
    uint16_t crc;
    crc = get_crc ( buffer[1] + 2, buffer );
    buffer[buffer[1] + 2] = crc & 0XFF;
    buffer[buffer[1] + 3] = ( crc >> 8 ) & 0XFF;
    stream->write ( buffer, buffer[1] + 4 );
    dumpPacket ( "packet that was sent" );
}

/*
 * CRC code taken from the CFA735 documentation, data types updated to 
 * stdint types.
 */
uint16_t CrystalFontz635::get_crc ( uint8_t count, uint8_t *ptr ) {
    uint16_t crc;   //Calculated CRC
    uint8_t i;     //Loop count, bits in byte
    uint8_t data;  //Current byte being shifted
    crc = 0xFFFF; // Preset to all 1's, prevent loss of leading zeros
    while ( count-- ) {
        data = *ptr++;
        i = 8;
        do {
            if ( ( crc ^ data ) & 0x01 ) {
                crc >>= 1;
                crc ^= 0x8408;
            } else {
                crc >>= 1;
            }
            data >>= 1;
        } while (--i != 0 );
    }
    return ( ~crc );
}
