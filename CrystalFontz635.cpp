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
#include <SoftwareSerial.h>
#include "CrystalFontz635.h"
#include <PString.h>

uint8_t tmpBuffer[CFA635_LINEBUFFER_SIZE];
PString tmpString((char*)tmpBuffer, sizeof(tmpBuffer));
CrystalFontz635::CrystalFontz635() {
    currentReadBuffer = 0;
    currentReadBufferSize = 0;
    currentReadState = CFA635_STATE_READING_COMMAND;
    nextReturnBuffer = 0;
    memset ( readBuffers, 0, CFA635_READBUFFER_SIZE * CFA635_READBUFFER_COUNT );
}

void CrystalFontz635::init ( Stream *stream2 ) {
    stream = stream2;
}

void CrystalFontz635::clearWriteBuffer() {
    memset ( writeBuffer, 0, CFA635_WRITEBUFFER_SIZE );
}

    clearWriteBuffer();
    writeBuffer[0] = 0x01;
    writeBuffer[1] = 0;
    sendPacket();
}


void CrystalFontz635::clearLCD() {
    clearWriteBuffer();
    writeBuffer[0] = 0x06;
    writeBuffer[1] = 0;
}

void CrystalFontz635::setLED ( uint8_t led, uint8_t redVal, uint8_t greenVal ) {
    clearWriteBuffer();
    writeBuffer[0] = 0x22;
    writeBuffer[1] = 2;
    writeBuffer[2] = 12 - (led * 2 );
    writeBuffer[3] = redVal;
    sendPacket();

    writeBuffer[2] = 11 - (led * 2 );
    writeBuffer[3] = greenVal;
    sendPacket();
}

void CrystalFontz635::setCursorPosition ( int row, int column ) {
    clearWriteBuffer();
    writeBuffer[0] = 0x0B;
    writeBuffer[1] = 2;
    writeBuffer[2] = column;
    writeBuffer[3] = row;
    sendPacket();
}

void CrystalFontz635::printAt ( uint8_t row, uint8_t column, char *string ) {
    clearWriteBuffer();
    writeBuffer[0] = 0x1F;
    if ( string == NULL ) {
        writeBuffer[1] = 0;
    } else {
        writeBuffer[1] = strlen ( string ) + 2;
        memcpy ( &writeBuffer[4], string, strlen ( string ) );
    }
    writeBuffer[2] = column;
    writeBuffer[3] = row;
    sendPacket();
}

void CrystalFontz635::printAt ( uint8_t row, uint8_t column, uint8_t val, int type ) {
    tmpString.begin();
    tmpString.print ( val, type );
    printAt ( row, column, (char *)tmpBuffer );
}

void CrystalFontz635::printAt ( uint8_t row, uint8_t column, uint32_t val, int type ) {
    tmpString.begin();
    tmpString.print ( val, type );
    printAt ( row, column, (char *)tmpBuffer );
}

void CrystalFontz635::printAt ( uint8_t row, uint8_t column, double val, int8_t width, uint8_t precision ) {
    //dtostrf ( val, width, precision, (char *)tmpBuffer );
    sprintf ( (char *)tmpBuffer, "%8.3f", val );
    printAt ( row, column, (char *)tmpBuffer );
}

#ifdef CFA635_DEBUG
void CrystalFontz635::_dumpPacket ( char *str, uint8_t buffer[] ) {
    Serial.print ( "  Dumping Packet: " );
    Serial.print ( str );
    Serial.print ( ", C: " );
    Serial.print ( (uint8_t)buffer[0], HEX );
    Serial.print ( ", L:" );
    Serial.print ( (uint8_t)buffer[1], HEX );
    for ( int i = 2; i < ( buffer[1] + 2 ); i++ ) {
        if ( i == 2 ) {
            Serial.print ( ", D: " );
        } else {
            Serial.print ( " " );
        }
        Serial.print ( buffer[i], HEX );
    }
    Serial.print ( ", CRC: " );
    uint16_t crc = ( ( buffer[buffer[1] + 3] << 8 ) & 0xFF00 ) | buffer[buffer[1] + 2] & 0xFF;
    Serial.println ( crc, HEX );
}
#endif

void CrystalFontz635::updateBufferCRC ( uint8_t buffer[] ) {
    uint16_t crc = get_crc ( buffer[1] + 2, buffer );
    buffer[buffer[1] + 2] = crc & 0XFF;
    buffer[buffer[1] + 3] = ( crc >> 8 ) & 0XFF;
}

void CrystalFontz635::sendPacket() {
    uint16_t crc;
    updateBufferCRC ( writeBuffer );
    stream->write ( writeBuffer, writeBuffer[1] + 4 );
    //dumpPacket ( "packet that was sent", writeBuffer );
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

uint8_t CrystalFontz635::processInput() {
    uint8_t byteRead;
    uint8_t numValidPackets = 0;
    while ( int i = stream->available() > 0 ) {
        byteRead = (uint8_t)stream->read() & 0xFF;
        if ( currentReadState == CFA635_STATE_READING_COMMAND ) {
            if ( byteRead && 0xC0 ) {
                    // set state of buffer to unread.
                currentReadBufferSize = 0;
                readBuffers[currentReadBuffer][currentReadBufferSize++] = CFA635_READ;
                readBuffers[currentReadBuffer][currentReadBufferSize++] = byteRead;
                currentReadState = CFA635_STATE_READING_SIZE;
            }
        } else if ( currentReadState == CFA635_STATE_READING_SIZE ) {
                // bad size, reset state to beginning.
            if ( byteRead >= 20 ) {
                currentReadState = CFA635_STATE_READING_COMMAND;
            } else {
                readBuffers[currentReadBuffer][currentReadBufferSize++] = byteRead;
                currentReadState = CFA635_STATE_READING_GENERAL;
            }
        } else {
                // currentReadState = CFA635_STATE_READING_GENERAL
            readBuffers[currentReadBuffer][currentReadBufferSize++] = byteRead;
                // packet data length is readBuffers[currentReadBuffer][2] )
            if ( currentReadBufferSize >= readBuffers[currentReadBuffer][2] + 5 ) {
                uint16_t expectedCRC = get_crc ( readBuffers[currentReadBuffer][2] + 2, &(readBuffers[currentReadBuffer][1]) );
                    // validate CRC
                if ( ( readBuffers[currentReadBuffer][currentReadBufferSize - 1] == ( ( expectedCRC >> 8 ) & 0xFF ) ) &&
                     ( readBuffers[currentReadBuffer][currentReadBufferSize - 2] == ( expectedCRC & 0xFF ) ) ) {
                    readBuffers[currentReadBuffer][0] = CFA635_UNREAD;
                    currentReadState = CFA635_STATE_READING_COMMAND;
                    tmpString.begin();
                    tmpString.print ( "input, buf: " );
                    tmpString.print ( currentReadBuffer, DEC );
                    //dumpPacket ( (char *)tmpBuffer, &(readBuffers[currentReadBuffer][1]) );
                    incrementBufferIndex ( &currentReadBuffer );
                        // if the currentReadBuffer index == nextReturnBuffer index, we've wrapped 
                        // around, and we're going to lose the oldest Buffer entry, so increment
                        // nextReturnBuffer
                    if ( currentReadBuffer == nextReturnBuffer ) {
                        incrementBufferIndex ( &nextReturnBuffer );
                    }
                    numValidPackets++;
                } else {
                        // CRC validation failed
                    currentReadState = CFA635_STATE_READING_COMMAND;
                }
            }
        }
    }
    return numValidPackets;
}

Packet* CrystalFontz635::getNextPacket() {
    if ( readBuffers[nextReturnBuffer][0] == CFA635_UNREAD ) {
        packet = (Packet *)&(readBuffers[nextReturnBuffer][1]);
        readBuffers[nextReturnBuffer][0] = CFA635_READ;
        incrementBufferIndex ( &nextReturnBuffer );
        return packet;
    } else {
        return NULL;
    }
}

void CrystalFontz635::incrementBufferIndex ( uint8_t *index ) {
    (*index)++;
    if ( *index >= CFA635_READBUFFER_COUNT ) {
        *index = 0;
    }
}
