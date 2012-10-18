/**
 * CAN BUS
 *
 * Copyright (c) 2010 Sukkin Pang All rights reserved.
 */

#ifndef crystalfontz635__h
#define crystalfontz635__h

#include <stdint.h>
#include <Stream.h>

// set this to undef to save space.
#define CFA635_DEBUG

#ifdef CFA635_DEBUG
#define dumpPacket(x) dumpPacket2(x)
#else
#define dumpPacket(x)
#endif

// TODO: Is this the proper size?
#define CFA635_WRITEBUFFER_SIZE 32

/*
 * This what the packet structure would look like, but we're not using
 * a packet (we're just writing directly to the buffer to save space)
 * 
 *   typedef struct {
 *       uint8_t command;       // buffer[0]
 *       uint8_t data_length;   // buffer[1]
 *       uint8_t *data;         // buffer[2-(data_length+2)]
 *       uint16_t CRC;          // buffer[data_length+2] (LSB is first!)
 *   } Command_Packet;
 */

class CrystalFontz635 {
  public:

    CrystalFontz635();
    uint16_t get_crc ( uint8_t count, uint8_t *ptr );
    void init ( Stream *stream );
    void clearLCD();
    void getHardwareFirmwareVersion();
    void writeString ( uint8_t row, uint8_t column, char *string );
    void setCursorPosition ( int row, int column );

  private:
    void clearWriteBuffer();
    void sendPacket();
    #ifdef CFA635_DEBUG
        void dumpPacket2 ( char *str );
    #endif
    uint8_t buffer[CFA635_WRITEBUFFER_SIZE];
    Stream *stream;
	
};

#endif //crystalfontz635__h

