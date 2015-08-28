#include <stdint.h>
#include <stdio.h>
#include <string.h>
uint16_t dataRegister[16];

static uint16_t bitMask[] = { 
    0x0001,
    0x0002,
    0x0004,
    0x0008,
    0x0010,
    0x0020,
    0x0040,
    0x0080,
    0x0100,
    0x0200,
    0x0400,
    0x0800,
    0x1000,
    0x2000,
    0x4000,
    0x8000
};

enum inst {
    END=0,
    LD,
    LD_NOT,
    AND,
    AND_NOT,
    OR,
    OR_NOT,
    OUT,
    NOP=0xff
};

enum Boolean {
    false,
    true
};

typedef enum Boolean Boolean_t;

struct {
    Boolean_t verbose;
    Boolean_t STATUS;
    uint8_t IP;
    Boolean_t T;
} reg ;

Boolean_t invertBoolean( Boolean_t n ) {
    Boolean_t ret;
    if( n == true ) {
        ret=false;
    } else {
        ret=true;
    }
    return(ret);
}
Boolean_t getRegisterBit( uint8_t addr ) {
    uint8_t regOffset;
    uint8_t bitNo;
    uint16_t mask;
    Boolean_t state=false;

    regOffset = addr / 16;
    bitNo = addr % 16;

    if ( 0 == (dataRegister[ regOffset ] & bitMask[ bitNo ]) )
        state = false;
    else
        state = true;

    return( state );
}

void setRegisterBit( uint8_t addr ) {
    uint16_t data;
    uint8_t regOffset;
    uint8_t bitNo;
    uint16_t mask;

    regOffset = addr / 16;
    bitNo = addr % 16;

    data = dataRegister[regOffset];

    mask = bitMask[ bitNo ];

    data = data | mask;

    dataRegister[ regOffset ] = data;
}

void clrRegisterBit( uint8_t addr ) {
    uint16_t data;
    uint8_t regOffset;
    uint8_t bitNo;
    uint16_t mask;

    regOffset = addr / 16;
    bitNo = addr % 16;

    data = dataRegister[regOffset];

    mask = ~bitMask[ bitNo ];

    data = data & mask;

    dataRegister[ regOffset ] = data;
}

void dumpRegisters() {
    printf("\tStatus:");
    if(reg.STATUS)
        printf("TRUE\n");
    else
        printf("FALSE\n");

    printf("\tIP    :%02x\n", reg.IP);
}

void dumpDataRegisters() {
    int i;

    for(i=0;i<16;i++) {
        printf("%04x:",dataRegister[i]);
    }
    printf("\n");
}

void runPlc(uint8_t *m) {
    uint8_t W;
    uint8_t inst;

    uint8_t data;
    uint8_t tmp;

    Boolean_t verbose;
    Boolean_t runFlag=true;
    verbose=reg.verbose;

    while( runFlag ) {
        inst=m[reg.IP++];

        switch(inst) {
            case LD:
                tmp=reg.IP;
                data = m[reg.IP++];

                if (verbose ) {
                    printf("%04x:",tmp);
                    printf("LD %04x\n",data);
                }

                reg.STATUS=getRegisterBit( data );
                break;
            case LD_NOT:
                tmp=reg.IP;
                data = m[reg.IP++];

                if (verbose ) {
                    printf("%04x:",tmp);
                    printf("LD_NOT %04x\n",data);
                }

                reg.STATUS=invertBoolean(getRegisterBit( data ));
                break;
            case OR:
                tmp=reg.IP;
                data = m[reg.IP++];

                if (verbose ) {
                    printf("%04x:",tmp);
                    printf("OR  %04x\n",data);
                }

                W = getRegisterBit( data );
                reg.STATUS = reg.STATUS | W;

                break;
            case OR_NOT:
                tmp=reg.IP;
                data = m[reg.IP++];

                if (verbose ) {
                    printf("%04x:",tmp);
                    printf("OR_NOT %04x\n",data);
                }

                W = getRegisterBit( data );
                reg.STATUS = reg.STATUS | invertBoolean(W);

                break;
            case AND:
                tmp=reg.IP;
                data = m[reg.IP++];

                if (verbose ) {
                    printf("%04x:",tmp);
                    printf("AND %04x\n",data);
                }

                W = getRegisterBit( data );
                reg.STATUS = reg.STATUS & W;

                break;
            case AND_NOT:
                tmp=reg.IP;
                data = m[reg.IP++];

                if (verbose ) {
                    printf("%04x:",tmp);
                    printf("AND_NOT %04x\n",data);
                }

                W = getRegisterBit( data );
                reg.STATUS = reg.STATUS & invertBoolean(W);
                break;
            case OUT:
                tmp=reg.IP;
                data = m[reg.IP++];

                if (verbose ) {
                    printf("%04x:",tmp);
                    printf("OUT %04x\n",data);
                }

                if(reg.STATUS) {
                    setRegisterBit( data );
                } else {
                    clrRegisterBit( data );
                }
                break;
            case NOP:
                printf("%04x:",reg.IP);
                printf("NOP\n");
                reg.IP++;
                break;
            case END:
                printf("%04x:",reg.IP);
                printf("END\n");
                runFlag=false;
                reg.IP=0;
                break;
            default:
                printf("%02x\n", m[reg.IP]);
                reg.IP++;
                break;
        }
        if(verbose) {
            dumpRegisters();
            dumpDataRegisters();
        }
    }

}

int main() {
    uint8_t machine[255];
    uint8_t ldIp=0;

    Boolean_t runFlag=true;

    memset( (void *)&machine,0, sizeof(machine));
    memset( (void *)&reg,0, sizeof(reg));
    memset( (void *)&dataRegister,0, sizeof(dataRegister));

    reg.verbose=true;

    machine[ldIp++] = LD;
    machine[ldIp++] = 0x0;
    machine[ldIp++] = OR;
    machine[ldIp++] = 0x10;

    machine[ldIp++] = AND_NOT;
    machine[ldIp++] = 0x01;

    machine[ldIp++] = OUT;
    machine[ldIp++] = 0x10;
    machine[ldIp++] = END;

    while( runFlag ) {
        // Read input & do any calculations.
        runPlc(&machine[0]);
        usleep(100);
        // Update o/ps
        // check comms
        // Delay here ?
    }
}

