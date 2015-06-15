#include <stdio.h>
#include <stdint.h>
#define SIZE 32
// #define TEST

class modbusRegisters {

    uint16_t reg[SIZE];

    private:
#ifndef TEST
    SEMAPHORE_DECL(modbusSem,1);
#endif

    public:
    modbusRegisters() {
        for(int i=0; i<SIZE; i++) {
            reg[i]=i;
        }

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

    r.dump();

    printf("set 0 \n");
    r.setRegister(0,0xffff);

    printf("get 0 0x%04x\n", r.getRegister(1));

    r.getMultipleRegisters(&mine[0],0,6);

    for(i=0;i<6;i++) {
        printf("%04x:%04x\n",i,mine[i]);
    }

    mine[0] = 6;
    mine[1] = 5;
    mine[2] = 4;
    mine[3] = 3;
    mine[4] = 2;
    mine[5] = 1;
    r.setMultipleRegisters(&mine[0],0,6);
    r.dump();


    return(0);
}
#endif

