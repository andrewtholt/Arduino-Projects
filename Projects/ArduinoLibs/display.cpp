#include <LedControl.h>

class display {
    private:
        bool lz;
        LedControl* lc;
    public:
        display(uint8_t din, uint8_t clk, uint8_t cs,uint8_t displayCount) {
            lc=new LedControl(din,clk,cs,displayCount);
            lz=true;
        }

        // 
        // digit is the position of the least significant
        // i.e. unit, digit.
        // 0 is the extreme left,
        // 4,is the right hand display
        //
        void writeDecNumber(uint8_t v,uint8_t digit) {
            uint8_t units;
            uint8_t tens;
            uint8_t hundreds;

            units=v%10;
            v=v/10;
            tens=v%10;
            v=v/10;
            hundreds=v;

            lc->setDigit(0,digit,units,false);
            lc->setDigit(0,digit+1,tens,false);
            lc->setDigit(0,digit+2,hundreds,false);

        }

        void writeHexNumber(uint8_t v,uint8_t digit) {
            uint8_t units;
            uint8_t teens;
            uint8_t more;

            units=v%0x10;
            v=v/0x10;
            teens=v%0x10;
            v=v/0x10;
            more=v;

            lc->setDigit(0,digit,units,false);
            lc->setDigit(0,digit+1,teens,false);
            lc->setDigit(0,digit+2,more,false);
        }

        void shutdown() {
            lc->shutdown(0,true);
        }

        void startup() {
            lc->shutdown(0,false);
        }

        void clear() {
            lc->clearDisplay(0);
        }

        void brightness(uint8_t level) {
            uint8_t l;

            l=map(level,0,100,0,15);
            lc->setIntensity(0,level);
        }

        void leadingZeros(bool f) {
            lz = f;
        }

};
