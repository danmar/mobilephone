#include "mbed.h"
#include "MODSERIAL.h"

MODSERIAL ser(p9,p10); // tx,rx
Serial pc(USBTX, USBRX); // tx, rx

DigitalOut led1(LED1);

int main() {
    ser.baud(115200);
    pc.baud(115200);

    int counter = 0;

    pc.printf("start\n");

    while(1) {
        counter++;

        led1 = ((counter >> 2) & 1);

        if ((counter & 63) == 0) {
            ser.printf("AT\r\n");
        }

        wait_ms(100);
        while (ser.readable())
            pc.printf("%c", ser.getc());
    }
}
