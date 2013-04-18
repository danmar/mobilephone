#include "mbed.h"
#include "MODSERIAL.h"

MODSERIAL ser(p9,p10); // tx,rx
Serial pc(USBTX, USBRX); // tx, rx

DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);

DigitalIn button(p20);

void restart()
{
    const char cmd[] = "AT+CFUN=1,1\r\n";
    ser.printf(cmd);
    pc.printf(cmd);
}

void answer()
{
    const char cmd[] = "ATA\r\n";
    ser.printf(cmd);
    pc.printf(cmd);    
}

int main() {
    ser.baud(115200);
    pc.baud(115200);

    int counter = 0;
    int ringcounter = 0;
    int ringcounter2 = 0;
    int answercounter = 0;

    char buf[100] = {0};
    int bufindex = 0;

    int sind = 0;

    // Initialize
    while (true) {
        restart();
        for (int counter = 0; sind != 4 && counter < 600; counter++) {
            wait_ms(100);
            led1 = ((counter & 3) == 0);
            led2 = ((counter & 3) == 1);
            led3 = ((counter & 3) == 2);
            led4 = ((counter & 3) == 3);

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
        }
        if (sind == 4)
            break;
    }

    enum { WAIT, DIAL, TALK } status = WAIT;

    while(1) {
        wait_ms(100);
        counter++;

        if (ringcounter > 0) {
            --ringcounter;
            led1 = ringcounter & 1;
            led2 = ringcounter & 1;
            led3 = ringcounter & 1;
            led4 = ringcounter & 1;
        }

        if (ringcounter2 > 0)
            --ringcounter2;

        led1 = ((counter >> 2) & 1);

        // Dial number..
        if (status == WAIT && button && (ringcounter2 == 0) && (sind==4 || sind==6)) {
            status = DIAL;
            counter = 0;
            const char cmd[] = "ATD0709124262\r\n";
            ser.printf(cmd);
            pc.printf(cmd);
        }

        // Answer..
        if (answercounter == 0 && ringcounter > 0 && button) {
            answer();
            ringcounter = ringcounter2 = 0;
            status = TALK;
            
            led1 = true;
            led2 = true;
            led3 = true;
            led4 = true;
        }

        // Read from GSM
        while (ser.readable()) {
            char c = ser.getc();
            if (c == '\r' || c == '\n') {
                if (buf[0] != '\0')
                    pc.printf("%s\n", buf);
            
                if (strncmp(buf,"+SIND:",6) == 0)
                    pc.printf(" => SIND=%i\n", sind = atoi(buf+6));

                if (strcmp(buf, "RING") == 0) {
                    ringcounter = 40;
                    ringcounter2 = 60;
                }

                if (strncmp(buf, "NO CARRIER", 10) == 0) {
                    led1 = false;
                    led2 = false;
                    led3 = false;
                    led4 = false;
                    status = WAIT;
                    pc.printf("HANGUP!\r\n");
                }

                memset(buf, 0, sizeof(buf));
                bufindex = 0;
            } else {
                buf[bufindex++] = c;
            }
        }
    }
}
