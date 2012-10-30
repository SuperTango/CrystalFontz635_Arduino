/**
 * Copyright (c) 2012 Alex Tang
 */
#ifndef crystalfontz635__h
#define crystalfontz635__h

#include <stdint.h>
#include <Stream.h>
#include <PString.h>

// set this to undef to save space.
#define CFA635_DEBUG

#ifdef CFA635_DEBUG
#define dumpPacket(x,y) _dumpPacket(x,y)
#else
#define dumpPacket(x,y)
#endif

// TODO: Is this the proper size?
#define CFA635_WRITEBUFFER_SIZE 32
#define CFA635_READBUFFER_SIZE 25
#define CFA635_LINEBUFFER_SIZE 20
#define CFA635_READBUFFER_COUNT 10

#define CFA635_UNREAD 1
#define CFA635_READ 0

#define CFA635_STATE_READING_COMMAND 0
#define CFA635_STATE_READING_SIZE 1
#define CFA635_STATE_READING_GENERAL 2

#define CFA635_PACKET_TYPE_PING 0x00
#define CFA635_PACKET_TYPE_PING_RESPONSE 0x40 | CFA_PACKET_TYPE_PING
#define CFA635_PACKET_TYPE_GET_HW_FW_INFO 0x01
#define CFA635_PACKET_TYPE_GET_HW_FW_INFO_RESPONSE 0x40 | CFA_PACKET_TYPE_GET_HW_FW_INFO

#define CFA635_PACKET_TYPE_KEY_ACTIVITY 0x80


#define CFA635_KEY_UP_PRESS              1
#define CFA635_KEY_DOWN_PRESS            2
#define CFA635_KEY_LEFT_PRESS            3
#define CFA635_KEY_RIGHT_PRESS           4
#define CFA635_KEY_ENTER_PRESS           5
#define CFA635_KEY_EXIT_PRESS            6
#define CFA635_KEY_UP_RELEASE            7
#define CFA635_KEY_DOWN_RELEASE          8
#define CFA635_KEY_LEFT_RELEASE          9
#define CFA635_KEY_RIGHT_RELEASE        10
#define CFA635_KEY_ENTER_RELEASE        11
#define CFA635_KEY_EXIT_RELEASE         12


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
 *
 * Another data structure we're using is the readBuffers.  This is an array 
 * of array of packets (with added meta info).  the first byte of the packet
 * is a read/unread flag, then the packet starts at byte 1.
 */

typedef struct {
    uint8_t type;
    uint8_t length;
    uint8_t data[];
} Packet;

class CrystalFontz635 {
  public:
    CrystalFontz635();
    uint16_t get_crc ( uint8_t count, uint8_t *ptr );
    void updateBufferCRC ( uint8_t buffer[] );
    void init ( Stream *stream );
    void clearLCD();
    void getHardwareFirmwareVersion();
    void printAt ( uint8_t row, uint8_t column, char *string );
    void printAt ( uint8_t row, uint8_t column, uint8_t val, int type = DEC );
    void printAt ( uint8_t row, uint8_t column, uint32_t val, int type = DEC);
    void printAt ( uint8_t row, uint8_t column, double val, int8_t width, uint8_t precision );
    void setCursorPosition ( int row, int column );
    void setLED ( uint8_t led, uint8_t redVal, uint8_t greenVal );
    uint8_t processInput();
    Packet* getNextPacket();
    void _dumpPacket ( char *str, uint8_t buffer[] );

  private:
    void clearWriteBuffer();
    void sendPacket();
    void receivePacket ( uint8_t expectedBuffer[] );
    #ifdef CFA635_DEBUG
    #endif
    uint8_t writeBuffer[CFA635_WRITEBUFFER_SIZE];
    uint8_t readBuffer[CFA635_READBUFFER_SIZE];
    uint8_t expectedBuffer[CFA635_READBUFFER_SIZE];
    Stream *stream;
    uint8_t readBuffers[CFA635_READBUFFER_COUNT][CFA635_READBUFFER_SIZE];
    uint8_t currentReadBuffer;
    uint8_t currentReadBufferSize;
    uint8_t currentReadState;
    uint8_t nextReturnBuffer;
    void incrementBufferIndex ( uint8_t *index );
    Packet *packet;
	
};

#endif //crystalfontz635__h

