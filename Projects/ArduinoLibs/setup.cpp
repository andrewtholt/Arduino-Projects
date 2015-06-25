/*
 *
 * Screen control sequences
 */
void home() {
    setupSerial.print("\033[H");
}

void ces() { // clear to end of screen
    setupSerial.print("\033[J");
}

void cls() {
    home();
    ces();
}

// x=line
// y=column
//
void move(int x, int y) {
    setupSerial.print("\033[");
    setupSerial.print(x);
    setupSerial.print(";");
    setupSerial.print(y);
    setupSerial.print("H");
}

void drawModbusMenu() {
    int line;
    int col;

    uint8_t eedata[4];
    uint8_t rtu;

    line = 3;
    col = 20; 

    cls();
    move(line++, col);
    setupSerial.print("Modbus Menu");
    move(line++, col);
    setupSerial.print("===========");

    line = 6;
    col = 10; 

    move(line++, col);
    setupSerial.print("1:    RTU Address");
    setupSerial.print(" (");
    rtu=r.getRegister(0x11);

    setupSerial.print(rtu);
    setupSerial.print(")");

    move(line++, col);
    setupSerial.print("2:    Baud Rate");

    line++;
    move(line++, col);
    setupSerial.print("x:    Exit menu");

    move(20, 20);
    setupSerial.print("Option> ");
}

void modbusMenu() {
    uint8_t flag = 0;
    uint8_t redraw = 1;
    uint8_t eedata[4];
    uint8_t rtu;

    char rep;

    eep.read(0,eedata,1);
    rtu = eedata[0];
    r.setRegister(0x11,rtu);

    while ( 0 == flag ) { 
        rtu=r.getRegister(0x11);
        if (redraw) {
            drawModbusMenu();
            redraw = 0;
        }   
        rep = setupSerial.read();

        if (rep > 0) {
            switch (rep) {
                case '1':
                    move(6,35);
                    setupSerial.setTimeout(1000);
                    rtu=getNumber(3);
                    r.setRegister(0x11,rtu);
                    LED.writeHexNumber(rtu,4);
                    redraw = 1;
                    break;
                case '2':
                    setupSerial.print("Baud Rate");
                    delay(1000);
                    redraw = 1;
                    break;
                case 'x':
                    flag++;
                    break;
                default:
                    redraw = 1;
                    break;
            }

            if ( 'x' == rep ) {
                eedata[0] = rtu;
                eep.write(0,eedata,1);
                flag++;
            }
        }
    }
    cls();
    setupSerial.println("Setup exited.");
}

void drawSetupMenu() {
    int line;
    int col;

    line = 3;
    col = 20;

    cls();
    move(line++, col);
    setupSerial.print("Setup Menu");
    move(line++, col);
    setupSerial.print("==========");

    line = 6;
    col = 10;

    move(line++, col);
    setupSerial.print("1:    ModBus Settings");

    move(line++, col);
    setupSerial.print("2:    Power Settings");

    line++;
    move(line++, col);
    setupSerial.print("s:    Show Settings");

    line++;
    move(line++, col);
    setupSerial.print("6:    Exit Settings");

    move(20, 20);
    setupSerial.print("Option> ");
}

void setupMenu() {
    uint8_t flag = 0;
    uint8_t redraw = 1;

    char r;

    while ( 0 == flag ) { 
        if (redraw) {
            drawSetupMenu();
            redraw = 0;
        }   
        r = setupSerial.read();

        if (r > 0) {
            switch (r) {
                case '1':
                    setupSerial.print("Modbus");
                    modbusMenu();
                    redraw = 1;
                    break;
                case '2':
                    setupSerial.print("Power");
                    delay(1000);
                    redraw = 1;
                    break;
                case 's':
                    setupSerial.print("Show Settings");
                    //                    showSettings();
                    redraw = 1;
                    break;
                case 'q':
                    flag++;
                    break;
                default:
                    redraw = 1;
                    break;
            }   

            if ( 'q' == r) {
                flag++;
            }   
        }   
    }   
    cls();
    setupSerial.println("Setup exited.");
}
