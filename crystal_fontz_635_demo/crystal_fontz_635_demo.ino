#include <CrystalFontz635.h>

CrystalFontz635 crystalFontz635;
void setup() {
    Serial.begin(115200);
    crystalFontz635.init ( &Serial1 );
    uint8_t data[6] = "hello";
    uint8_t buffer[256];

    Serial.println ( "About to test clear screen" );
    crystalFontz635.clearLCD();
    Serial.println ( "About to set cursor position to row 2, column 15" );
    crystalFontz635.setCursorPosition ( 2, 15 );
    Serial.println ( "About to write string 'Hello' to 3,1 (row,col)" );
    crystalFontz635.writeString ( 3, 1, "Hello" );
}

void loop() {
    delay ( 50000000 );
}
