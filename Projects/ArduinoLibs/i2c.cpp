/* 
 *  Use the I2C bus with EEPROM 24LC64 
 *  
 *  Original Author: hkhijhe
 *  Date: 01/10/2010
 * 
 * TODO:    Make this into a C++ class
 *          Add an i2c bus semaphore.
 *  
 */

#include <Wire.h> //I2C library

SEMAPHORE_DECL(modbusSem,1);

class eeprom {

    private:
        int deviceAddress;

    public:
        eeprom(int a) {
            deviceAddress = a;
        }

        void eepromWriteByte(byte *address, byte data) {
            nilSemWait(&i2cSem);
            // 
            // Write data.
            //
            Wire.beginTransmission(deviceAddress);
            Wire.send((int)(address >> 8)); // MSB
            Wire.send((int)(address & 0xFF)); // LSB
            Wire.send(data);
            Wire.endTransmission();

            nilSemSignal(&i2cSem);
        }

        byte eepromReadByte(byte *address) {
            byte data=0xff;
            nilSemWait(&i2cSem);
            // 
            // Read data.
            //
            Wire.beginTransmission(deviceAddress);
            Wire.send((int)(address >> 8)); // MSB
            Wire.send((int)(address & 0xFF)); // LSB
            Wire.endTransmission();

            Wire.requestFrom(deviceAddress,1);
            if (Wire.available()) 
                data = Wire.receive();

            nilSemSignal(&i2cSem);

            return data;
        }
};


void i2c_eeprom_write_byte( int deviceaddress, unsigned int eeaddress, byte data ) {
    int rdata = data;
    Wire.beginTransmission(deviceaddress);
    Wire.send((int)(eeaddress >> 8)); // MSB
    Wire.send((int)(eeaddress & 0xFF)); // LSB
    Wire.send(rdata);
    Wire.endTransmission();
}

/*
// WARNING: address is a page address, 6-bit end will wrap around
// also, data can be maximum of about 30 bytes, because the Wire library has a buffer of 32 bytes
void i2c_eeprom_write_page( int deviceaddress, unsigned int eeaddresspage, byte* data, byte length ) {
Wire.beginTransmission(deviceaddress);
Wire.send((int)(eeaddresspage >> 8)); // MSB
Wire.send((int)(eeaddresspage & 0xFF)); // LSB
byte c;
for ( c = 0; c < length; c++)
Wire.send(data[c]);
Wire.endTransmission();
}
*/

byte i2c_eeprom_read_byte( int deviceaddress, unsigned int eeaddress ) {
    byte rdata = 0xFF;
    Wire.beginTransmission(deviceaddress);
    Wire.send((int)(eeaddress >> 8)); // MSB
    Wire.send((int)(eeaddress & 0xFF)); // LSB
    Wire.endTransmission();
    Wire.requestFrom(deviceaddress,1);
    if (Wire.available()) rdata = Wire.receive();
    return rdata;
}

// maybe let's not read more than 30 or 32 bytes at a time!
void i2c_eeprom_read_buffer( int deviceaddress, unsigned int eeaddress, byte *buffer, int length ) {
    int c = 0;

    Wire.beginTransmission(deviceaddress);
    Wire.send((int)(eeaddress >> 8)); // MSB
    Wire.send((int)(eeaddress & 0xFF)); // LSB
    Wire.endTransmission();
    Wire.requestFrom(deviceaddress,length);

    for ( c = 0; c < length; c++ ) {
        if (Wire.available()) buffer[c] = Wire.receive();
    }
}
