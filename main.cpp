#include "mbed.h"

Serial ser(p9,p10); // tx,rx
Serial pc(USBTX, USBRX); // tx, rx

DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);

static int readADC(int pin);

static int getnumber()
{
    int x = 1000 * readADC(19) / readADC(20);
    if (x < 10)
        return -1;
    if (x > 990)
        return -2;
    return x / 100;
}

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

const char *receivedMessage;
int byteReceived;

void startPhone()
{
    int sind = 0;

    while (true) {
        restart();
        for (int counter = 0; sind != 4 && counter < 600; counter++) {
            wait_ms(100);
            led1 = ((counter & 3) == 0);
            led2 = ((counter & 3) == 1);
            led3 = ((counter & 3) == 2);
            led4 = ((counter & 3) == 3);

            if (receivedMessage) {
                char buf[100];
                strcpy(buf, receivedMessage);
                receivedMessage = NULL;

                pc.printf("%s\n", buf);

                if (strncmp(buf,"+SIND:",6) == 0)
                    pc.printf(" => SIND=%i\n", sind = atoi(buf+6));
            }
        }
        if (sind == 4)
            break;
    }

    led1 = 0;
    led2 = 0;
    led3 = 0;
    led4 = 0;
}

void phone()
{
    int counter = 0;
    int ringcounter = 0;
    int ringcounter2 = 0;
    int ringcode = 0;

    char dialnumber[32] = {0};

    enum { WAIT, DIAL, TALK } status = WAIT;

    startPhone();

    while (1) {
        wait_ms(100);

        counter++;
        led1 = ((counter >> 2) & 1);

        if (ringcounter > 0) {
            --ringcounter;
            led1 = ringcounter & 1;
            led2 = ringcounter & 1;
            led3 = ringcounter & 1;
            led4 = ringcounter & 1;
        }

        if (ringcounter2 > 0)
            --ringcounter2;
        else
            ringcode = 0;

        // Dial number..
        if (status == WAIT && ringcounter2 == 0) {
            static int numcount = 0;
            static int oldnum = -1;
            const int num = getnumber();
            if (num > 0 && num == oldnum)
                numcount++;
            else
                numcount = 0;
            oldnum = num;

            if (num==-1 || num==-2)
                memset(dialnumber, 0, sizeof(dialnumber));

            if (numcount == 1)
                pc.printf("%i\n", num);
            if (numcount == 40) {
                char *p = dialnumber;
                while (*p != '\0')
                    p++;
                *p = '0' + num;
                pc.printf("%s\n", dialnumber);

                if (dialnumber[0] != '\0') {
                    if (strcmp(dialnumber, "5") == 0) {
                        status = DIAL;
                        const char cmd[] = "ATD0709124262\r\n";
                        ser.printf(cmd);
                        pc.printf(cmd);
                    }
                    memset(dialnumber, 0, sizeof(dialnumber));
                }
            }
        }

        // Answer..
        if (status == WAIT && ringcounter > 0) {
            const int num = getnumber();
            if (num == -1)
                ringcode |= 1;
            if (num == -2)
                ringcode |= 2;
            if (ringcode == 3) {
                answer();
                ringcounter = ringcounter2 = 0;
                status = TALK;

                led1 = true;
                led2 = true;
                led3 = true;
                led4 = true;
            }
        }

        // Hang up
        {
            static int hangUpCode;
            static int hangUpCounter;

            if (status == TALK || status == DIAL) {
                if (hangUpCounter > 0)
                    hangUpCounter--;
                else {
                    hangUpCode = 0;
                }

                const int num = getnumber();
                if ((num != hangUpCode) && (num == -1 || num == -2)) {
                    if (hangUpCode != 0) {
                        led1 = false;
                        led2 = false;
                        led3 = false;
                        led4 = false;
                        status = WAIT;
                        ser.printf("ATH\r");
                        pc.printf("ATH\r");
                    } else {
                        hangUpCode = num;
                        hangUpCounter = 20;
                    }
                }
            }

            if (status == WAIT)
                hangUpCode = hangUpCounter = 0;
        }

        // Read from GSM
        while (receivedMessage != NULL) {
            char buf[100];
            strcpy(buf, receivedMessage);
            receivedMessage = NULL;

            pc.printf("%s\n", buf);

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
        }
    }
}

void receivebyte()
{
    static char buffer[100];
    static int bufindex = 0;

    char c = ser.getc();

    if (receivedMessage == NULL) {
        if (c == '\r' || c == '\n') {
            if (bufindex > 0)
                receivedMessage = buffer;
            buffer[bufindex] = 0;
            bufindex = 0;
        }

        else if (bufindex < sizeof(buffer)-1)
            buffer[bufindex++] = c;
    }

    byteReceived = 2;
}

static void initADC(int pins)
{
    NVIC_DisableIRQ(ADC_IRQn);

    // Power on the ADC
    LPC_SC->PCONP |= 1 << 12;

    // PCLK = 96MHz / 4 = 12MHz
    LPC_SC->PCLKSEL0 &= ~(3 << 24);

    unsigned int mask = 0;
    if (pins & (1<<19)) // P19 = AD0.4
        mask |= 3UL << 28;
    if (pins & (1<<20)) // P20 = AD0.5
        mask |= 3UL << 30;
    LPC_PINCON->PINSEL3 |= mask;

    LPC_ADC->ADCR = (1 << 8)     // CLKDIV=1
                    | (1 << 21);   // PDN
}

static int readADC(int pin)
{
    static const int AD_CR_START_MASK = 7 << 24;
    static const int AD_CR_START_NOW  = 1 << 24;
    static const int AD_CR_SELMASK    = 0xff;
    static const int AD_DR_DONE       = 1UL << 31;

    if (pin < 15 || pin > 20)
        return 0;

    // Stop ADC
    LPC_ADC->ADCR &= ~(AD_CR_START_MASK | AD_CR_SELMASK);

    // Start conversion
    LPC_ADC->ADCR |= (AD_CR_START_NOW | (1 << (pin-15)));

    // Wait for conversion done
    while ((LPC_ADC->ADGDR & AD_DR_DONE) == 0)
        ;

    // Return result
    return ((LPC_ADC->ADGDR >> 4) & 0xfff);
}

void testaSnurrskivan()
{
    int count = 0;

    pc.printf("testa snurrskivan");
    while (true) {
        wait_ms(100);

        if (byteReceived == 0) {
            pc.printf("%i\n", getnumber());
        } else {
            byteReceived--;
        }

        led1 = (byteReceived > 0);
        if (byteReceived)
            byteReceived--;

        count = (count + 1) & 31;
        if (count == 0)
            ser.printf("AT\r");
    }
}

int main()
{
    ser.baud(115200);
    pc.baud(115200);

    ser.attach(&receivebyte);

    initADC((1<<19) | (1<<20));

    phone();

    return 0;
}
