#include "mbed.h"
#include "MODSERIAL.h"

MODSERIAL ser(p9,p10); // tx,rx
Serial pc(USBTX, USBRX); // tx, rx

DigitalOut led1(LED1);

DigitalIn button(p20);

int main() {
    ser.baud(115200);
    pc.baud(115200);

    int counter = 0;

    pc.printf("start\n");

    char buf[100] = {0};
    int bufindex = 0;

    int sind = 0;
    bool call = false;

    while(1) {
        counter++;

        led1 = ((counter >> 2) & 1);

        if ((counter & 63) == 0) {
            //ser.printf("AT\r\n");
            call = false;
        }

        if (!call && button && (sind==4 || sind==6)) {
            call = true;
            counter = 0;
            const char cmd[] = "ATD0709124262\r\n";
            ser.printf(cmd);
            pc.printf(cmd);
        }

        wait_ms(100);
        while (ser.readable()) {
            char c = ser.getc();
            if (c == '\r' || c == '\n') {
                if (buf[0] != '\0')
                    pc.printf("%s\n", buf);
            
                if (strncmp(buf,"+SIND:",6) == 0)
                    pc.printf(" => SIND=%i\n", sind = atoi(buf+6));
                    
                memset(buf, 0, sizeof(buf));
                bufindex = 0;
            } else {
                buf[bufindex++] = c;
            }
        }

//        pc.printf("> %s\r\n", sendAndReceive(ser,"AT",1000));
    }
}
