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

Packet* CrystalFontz635::getHardwareFirmwareVersion ( Packet *returnPacket ) {
    clearWriteBuffer();
    writeBuffer[0] = 0x01;
    writeBuffer[1] = 0;
    return sendPacket ( writeBuffer, 0x41, returnPacket );
}

bool CrystalFontz635::clearLCD ( bool async ) {
    clearWriteBuffer();
    writeBuffer[0] = 0x06;
    writeBuffer[1] = 0;
    Serial.println ( (uint16_t)&dataPacket, HEX );
    if ( ! async ) {
        Packet *packet = sendPacket ( writeBuffer, 0x46, &dataPacket );
        return ( packet ) ? true : false;
    } else {
        sendPacket ( writeBuffer, 0x46, NULL );
        return true; 
    }
}

bool CrystalFontz635::setLED ( uint8_t led, uint8_t redVal, uint8_t greenVal, bool async ) {
    clearWriteBuffer();
    writeBuffer[0] = 0x22;
    writeBuffer[1] = 2;
    writeBuffer[2] = 12 - (led * 2 );
    writeBuffer[3] = redVal;
    if ( ! async ) {
        Packet *packet = sendPacket ( writeBuffer, 0x62, &dataPacket );
        if ( packet == NULL ) {
            return false;
        }
    } else {
        sendPacket ( writeBuffer, 0x62, NULL );
    }

    writeBuffer[2] = 11 - (led * 2 );
    writeBuffer[3] = greenVal;
    if ( ! async ) {
        Packet *packet = sendPacket ( writeBuffer, 0x62, &dataPacket );
        return ( packet ) ? true : false;
    } else {
        sendPacket ( writeBuffer, 0x62, NULL );
        return true; 
    }
}

bool CrystalFontz635::setCursorPosition ( int row, int column, bool async ) {
    clearWriteBuffer();
    writeBuffer[0] = 0x0B;
    writeBuffer[1] = 2;
    writeBuffer[2] = column;
    writeBuffer[3] = row;
    Packet *packet = sendPacket ( writeBuffer, 0x4B, &dataPacket );
    if ( async ) {
        return true;
    } else {
        return ( packet ) ? true : false;
    }
}

bool CrystalFontz635::printAt ( uint8_t row, uint8_t column, char *string, bool async ) {
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
    Packet *packet = sendPacket ( writeBuffer, 0x5F, &dataPacket );
    if ( async ) {
        return true;
    } else {
        return ( packet ) ? true : false;
    }
}

bool CrystalFontz635::printAt ( uint8_t row, uint8_t column, uint8_t val, int type, bool async ) {
    tmpString.begin();
    tmpString.print ( val, type );
    return printAt ( row, column, (char *)tmpBuffer, async );
}

bool CrystalFontz635::printAt ( uint8_t row, uint8_t column, uint32_t val, int type, bool async ) {
    tmpString.begin();
    tmpString.print ( val, type );
    return printAt ( row, column, (char *)tmpBuffer, async );
}

bool CrystalFontz635::printAt ( uint8_t row, uint8_t column, double val, int8_t width, uint8_t precision, bool async ) {
    //dtostrf ( val, width, precision, (char *)tmpBuffer );
    sprintf ( (char *)tmpBuffer, "%8.3f", val );
    return printAt ( row, column, (char *)tmpBuffer, async );
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
    buffer[buffer[1] + 2] = (uint8_t)(crc & 0XFF);
    buffer[buffer[1] + 3] = (uint8_t)(( crc >> 8 ) & 0XFF);
}

Packet* CrystalFontz635::sendPacket ( uint8_t writeBuffer[], uint8_t type, Packet *returnPacket ) {
    updateBufferCRC ( writeBuffer );
    stream->write ( writeBuffer, writeBuffer[1] + 4 );

    if ( returnPacket ) {
        unsigned long tStart = millis();
        bool done = false;
        int count = 0;
        while ( ( ! done ) && ( ( millis() - tStart ) <= CFA635_MAX_RESPONSE_TIME ) ) {
            processInput();
            if ( getNextPacketOfType ( type, returnPacket ) ) {
                return returnPacket;
            }
        }
    }
    Serial.println ( "sendPacket didn't get a valid response" );
    return NULL;
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
                    currentReadBuffer = nextBufferIndex ( currentReadBuffer );
                        // if the currentReadBuffer index == nextReturnBuffer index, we've wrapped 
                        // around, and we're going to lose the oldest Buffer entry, so increment
                        // nextReturnBuffer
                    if ( currentReadBuffer == nextReturnBuffer ) {
                        nextReturnBuffer = nextBufferIndex ( nextReturnBuffer );
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

Packet* CrystalFontz635::getNextPacket ( Packet *packet ) {
    if ( readBuffers[nextReturnBuffer][0] == CFA635_UNREAD ) {
        packet = (Packet *)&(readBuffers[nextReturnBuffer][1]);
        readBuffers[nextReturnBuffer][0] = CFA635_READ;
        nextReturnBuffer = nextBufferIndex ( nextReturnBuffer );
        return packet;
    } else {
        return NULL;
    }
}

Packet* CrystalFontz635::getNextPacketOfType ( uint8_t type, Packet *packet ) {
    uint8_t index = nextReturnBuffer;
    while ( index != currentReadBuffer ) {
        if ( ( readBuffers[index][0] == CFA635_UNREAD ) && ( readBuffers[index][1] == type ) ) {
            readBuffers[index][0] = CFA635_READ;
            memcpy ( packet, &(readBuffers[index][1]), sizeof ( Packet )   );
            compactReadBuffers ( index );
            return packet;
        } else {
            index = nextBufferIndex ( index );
        }
    }
    return NULL;
}

void CrystalFontz635::compactReadBuffers ( uint8_t index ) {
    if ( index == nextReturnBuffer ) {
        nextReturnBuffer = nextBufferIndex ( nextReturnBuffer );
    } else if ( index == previousBufferIndex ( currentReadBuffer ) ) {
        currentReadBuffer = previousBufferIndex ( currentReadBuffer );
    } else if ( currentReadBuffer > index ) {
        memmove ( readBuffers[index], readBuffers[index + 1], ( currentReadBuffer - index ) * CFA635_READBUFFER_SIZE );
        currentReadBuffer = previousBufferIndex ( currentReadBuffer );
    } else if ( currentReadBuffer < index ) {
            // the length is actually ( ( CFA635_READBUFFER_COUNT - 1 ) - ( index + 1 ) )
        memmove ( readBuffers[index], readBuffers[index + 1], ( CFA635_READBUFFER_COUNT - index ) * CFA635_READBUFFER_SIZE );
        if ( currentReadBuffer > 0 ) {
            memmove ( readBuffers[CFA635_READBUFFER_COUNT - 1], readBuffers[0],  CFA635_READBUFFER_SIZE );
            memmove ( readBuffers[0], readBuffers[1], currentReadBuffer * CFA635_READBUFFER_SIZE );
        }
        currentReadBuffer = previousBufferIndex ( currentReadBuffer );
    }
}

uint8_t CrystalFontz635::nextBufferIndex ( uint8_t index ) {
    if ( index >= ( CFA635_READBUFFER_COUNT - 1 ) ) {
        index = 0;
    } else {
        index++;
    }
    return index;
}

uint8_t CrystalFontz635::previousBufferIndex ( uint8_t index ) {
    if ( index == 0 ) {
        index = ( CFA635_READBUFFER_COUNT - 1 );
    } else {
        index--;
    }
    return index;
}

/*
 * These functions are used for testing.
 */
void CrystalFontz635::setPacket ( Packet* inputPacket ) {
    Serial.print ( "Setting Packet.  CurrentReadBuffer is: " );
    Serial.print ( currentReadBuffer );
    memcpy ( &(readBuffers[currentReadBuffer][1]), inputPacket, CFA635_PACKETDATA_MAX );
    readBuffers[currentReadBuffer][0] = CFA635_UNREAD;
    updateBufferCRC ( &(readBuffers[currentReadBuffer][1]) );
    currentReadBuffer = nextBufferIndex ( currentReadBuffer );
    Serial.print ( ". CurrentReadBuffer is now: " );
    Serial.println ( currentReadBuffer );
    dumpPacket ( "setPacket", (uint8_t *)inputPacket );
}

void CrystalFontz635::setPacketPositions ( uint8_t readPacketPosition, uint8_t writePacketPosition ) {
    uint8_t index = readPacketPosition;
    uint8_t status = CFA635_UNREAD;
    currentReadBuffer = writePacketPosition;
    nextReturnBuffer = readPacketPosition;;
    do {
        if ( index == writePacketPosition ) {
            status = CFA635_READ;
        }
        readBuffers[index][0] = status;
        index = nextBufferIndex ( index );
    } while ( index != readPacketPosition );
}

void CrystalFontz635::dumpReadBuffers() {
    Serial.print  ( "Dumping Buffers. currentReadBuffer: " );
    Serial.print ( currentReadBuffer );
    Serial.print  ( ", nextReturnBuffer is: " );
    Serial.println ( nextReturnBuffer );
    for ( uint8_t i = 0; i < CFA635_READBUFFER_COUNT; i++ ) {
        tmpString.begin();
        if ( i == currentReadBuffer ) {
            tmpString.print ( "W" );
        } else {
            tmpString.print ( " " );
        }
        if ( i == nextReturnBuffer ) {
            tmpString.print ( "R" );
        } else {
            tmpString.print ( " " );
        }
        tmpString.print ( i, DEC );
        tmpString.print ( ", S:" );
        if ( readBuffers[i][0] == CFA635_UNREAD ) {
            tmpString.print ( "U" );
        } else if ( readBuffers[i][0] == CFA635_READ ) {
            tmpString.print ( "R" );
        } else {
            tmpString.print ( "ERROR" );
        }
        tmpString.print ( readBuffers[i][0], DEC );
        dumpPacket ( (char *)tmpBuffer, &(readBuffers[i][1]) );
    }
}
