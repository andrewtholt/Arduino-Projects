#include <stdio.h>
#include <stdint.h>


int main() {

    uint8_t br;
    uint8_t history[4] ;

    history[0] =  0xc0;
    history[1] =  0xc0;
    history[2] =  0xc0;
    history[3] =  0xc0;

    br = history[0] | history[1] | history[2] | history[3] ;

    printf("br=%02x\n",br);

    return 0;
}
