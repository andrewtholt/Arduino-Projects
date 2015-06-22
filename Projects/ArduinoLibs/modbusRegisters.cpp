#include <stdio.h>
#include <stdint.h>
#define SIZE 32
// #define TEST

class modbusRegisters {

    uint16_t reg[SIZE];

    private:

    uint16_t mask[16] = { 
        0x0001,0x0002,0x0004,0x0008,
        0x0010,0x0020,0x0040,0x0080,
        0x0100,0x0200,0x0400,0x0800,
        0x1000,0x2000,0x4000,0x8000
    };

#ifndef TEST
    SEMAPHORE_DECL(modbusSem,1);
#endif

    public:
    modbusRegisters() {
        for(int i=0; i<SIZE; i++) {
            reg[i]=0;
        }
    }

    bool getRegisterBit(uint8_t address,uint8_t bit) {
        bool v;
#ifndef TEST
        nilSemWait(&modbusSem);
#endif
        v=((reg[address] & mask[bit]) != 0);
#ifndef TEST
        nilSemSignal(&modbusSem);
#endif
        return v;
    }

    void setRegisterBit(uint8_t address, uint8_t bit) {
#ifndef TEST
        nilSemWait(&modbusSem);
#endif
        reg[address] = reg[address] | mask[bit];
#ifndef TEST
        nilSemSignal(&modbusSem);
#endif
    }

    void clrRegisterBit(uint8_t address, uint8_t bit) {
#ifndef TEST
        nilSemWait(&modbusSem);
#endif
        reg[address] = reg[address] & ~mask[bit];
#ifndef TEST
        nilSemSignal(&modbusSem);
#endif
    }

    uint16_t getRegister(uint8_t address) {
        uint16_t r;

#ifndef TEST
        nilSemWait(&modbusSem);
#endif
        r=reg[address];
#ifndef TEST
        nilSemSignal(&modbusSem);
#endif
        return(r);
    }

    void setRegister(uint8_t address,uint16_t value) {
#ifndef TEST
        nilSemWait(&modbusSem);
#endif
        reg[address]=value;
#ifndef TEST
        nilSemSignal(&modbusSem);
#endif
    }

    // 
    // fills the array data with a copy of the registers,
    // starting from address for count.
    //
    //
    void getMultipleRegisters(uint16_t *data,uint8_t address,uint8_t count) {
        uint8_t limit;

        if ((address+count) > SIZE ) 
            limit = SIZE;
        else
            limit = address+count;


        // LOCK HERE
        for(uint8_t idx=0;idx < limit;idx++) {
            data[idx] = reg[address+idx];
        }
        // UNLOCK HERE
    }

    void setMultipleRegisters(uint16_t *data,uint8_t address,uint8_t count) {
        uint8_t limit;
        if ((address+count) > SIZE ) 
            limit = SIZE;
        else
            limit = address+count;

        // LOCK HERE
        for(uint8_t idx=0;idx < limit;idx++) {
            reg[address+idx] = data[idx] ;
        }
        // UNLOCK HERE
    }
#ifdef TEST
    void dump() {
        printf("\n\n");
        for (int i=0;i<SIZE;i++) {
            printf("%04x ",reg[i]);
            if ((i > 0) && (i % 8 )== 0) {
                printf("\n");
            }
        }
        printf("\n");
    }
#endif

};


#ifdef TEST

int main() {

    uint16_t mine[6];
    modbusRegisters r;
    int i;
    int b;
    uint16_t a;
    uint16_t v;

    r.dump();

    a=0;
    v=0x8000;

    printf("set %02x \n",a);
    r.setRegister(a,v);
    printf("get %02x 0x%04x\n",a, r.getRegister(a));
    printf("===============\n");

    b=0;
    if(r.getRegisterBit(0,b) ) {
        printf("Bit %d is set\n",b);
    } else {
        printf("Bit %d is clr\n",b);
    }

    printf("START\n");
    b=15;
    if(r.getRegisterBit(0,b) ) {
        printf("Bit %d is set\n",b);
    } else {
        printf("Bit %d is clr\n",b);
    }

    printf("Setting bit %d\n",b);
    r.setRegisterBit(0,b);

    if(r.getRegisterBit(0,b) ) {
        printf("Bit %d is set\n",b);
    } else {
        printf("Bit %d is clr\n",b);
    }

    printf("Clearing bit %d\n",b);
    r.clrRegisterBit(0,b);

    if(r.getRegisterBit(0,b) ) {
        printf("Bit %d is set\n",b);
    } else {
        printf("Bit %d is clr\n",b);
    }

    b=15;
    r.setRegisterBit(0,b);
    if(r.getRegisterBit(0,b) ) {
        printf("Bit %d is set\n",b);
    } else {
        printf("Bit %d is clr\n",b);
    }

    r.getMultipleRegisters(&mine[0],0,6);

    for(i=0;i<6;i++) {
        printf("%04x:%04x\n",i,mine[i]);
    }

    /*
    mine[0] = 6;
    mine[1] = 5;
    mine[2] = 4;
    mine[3] = 3;
    mine[4] = 2;
    mine[5] = 1;
    r.setMultipleRegisters(&mine[0],0,6);
    */
    r.dump();


    return(0);
}
#endif

