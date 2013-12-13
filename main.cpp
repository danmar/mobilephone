#include "mbed.h"

Serial ser(p9,p10); // tx,rx
Serial pc(USBTX, USBRX); // tx, rx

DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);

PwmOut buzzer(p23);

static int readADC(int pin);

static int getDialRawValue()
{
    int x = 1000 * readADC(19) / readADC(20);
    if (x > 1000)
        x = 1000;
    return 1000 - x;
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
        for (int counter = 0; sind != 11 && counter < 600; counter++) {
            wait_ms(100);
            led1 = ((counter & 3) == 0);
            led2 = ((counter & 3) == 1);
            led3 = ((counter & 3) == 2);
            led4 = ((counter & 3) == 3);

            if (receivedMessage) {
                char buf[100];
                strcpy(buf, receivedMessage);
                receivedMessage = NULL;

                pc.printf("%s\r\n", buf);

                if (strncmp(buf,"+SIND:",6) == 0)
                    pc.printf(" => SIND=%i\r\n", sind = atoi(buf+6));
            }
        }
        if (sind == 11)
            break;

        pc.printf("Failed to login");
        buzzer = 0.5;
        wait(1);
        buzzer = 0;
    }

    for (int i = 0; i < 10; i++) {
        buzzer = (i & 1) * 0.5;
        wait_ms(100);
    }

    led1 = 0;
    led2 = 0;
    led3 = 0;
    led4 = 0;

    buzzer = 0;
}

void phone()
{
    int counter = 0;
    int ringcounter = 0;
    int ringcounter2 = 0;
    int ringcode = 0;

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
            buzzer = (ringcounter & 1) * 0.5;
        }

        if (ringcounter2 > 0)
            --ringcounter2;
        else
            ringcode = 0;

        // Dial number..
        if (status == WAIT && ringcounter2 == 0) {
            static int maxDialRawValue = 0;
            const int dialRawValue = getDialRawValue();
            if (dialRawValue > maxDialRawValue)
                maxDialRawValue = dialRawValue;
            if (dialRawValue == 0 && maxDialRawValue > 20) {
                static const int rawValue[] = { 30, 155, 252, 390, 500, 620, 750, 860, 970, 1000 };
                int num = (maxDialRawValue == 1000) ? 9 : 0;
                while (maxDialRawValue > rawValue[num] + 40)
                    num++;
                if (maxDialRawValue < rawValue[num] - 40) {
                    pc.printf("nr: %i\r\n", maxDialRawValue);
                    maxDialRawValue = 0;
                    continue;
                }
                pc.printf("nr: %i %i ", num, maxDialRawValue);
                maxDialRawValue = 0;

                static char dialnumber[32];
                int i = 0;
                while (i < 30 && dialnumber[i] != '\0')
                    i++;
                dialnumber[i] = '0' + num;

                if (i >= 2 && strcmp(dialnumber+i-2, "999") == 0)
                    memset(dialnumber, 0, sizeof(dialnumber));

                if (i == 0 && num != 0)
                    memset(dialnumber, 0, sizeof(dialnumber));

                pc.printf("dialnr=%s\r\n", dialnumber);

                if (strcmp(dialnumber,"055")==0) {
                    const char cmd[] = "ATD0709124262\r\n";
                    ser.printf(cmd);
                    pc.printf(cmd);
                    memset(dialnumber, 0, sizeof(dialnumber));
                }

                if (dialnumber[9] != '\0') {
                    if (strcmp(dialnumber, "0709124262") == 0 || strcmp(dialnumber, "0736156826") == 0) {
                        status = DIAL;
                        char cmd[32];
                        strcpy(cmd, "ATD");
                        strcat(cmd, dialnumber);
                        strcat(cmd, "\r\n");
                        ser.printf(cmd);
                        pc.printf(cmd);
                    }
                    memset(dialnumber, 0, sizeof(dialnumber));
                }
            }
        }

        // Answer..
        if (status == WAIT && ringcounter > 0) {
            const int dialRawValue = getDialRawValue();
            if (dialRawValue < 50)
                ringcode |= 1;
            else if (dialRawValue > 950)
                ringcode |= 2;
            if (ringcode == 3) {
                answer();
                ringcounter = ringcounter2 = 0;
                status = TALK;

                led1 = true;
                led2 = true;
                led3 = true;
                led4 = true;

                buzzer = 0;
            }
        }

        // Hang up
        {
            static int hangUpCode;
            static int hangUpCounter;

            if (status == TALK || status == DIAL) {
                if (hangUpCounter > 0)
                    hangUpCounter--;
                else
                    hangUpCode = 0;

                const int dialRawValue = getDialRawValue();
                int num = 0;
                if (dialRawValue < 50)
                    num = 1;
                else if (dialRawValue > 950)
                    num = 2;
                if ((num != hangUpCode) && (num == 1 || num == 2)) {
                    if (hangUpCode != 0) {
                        led1 = false;
                        led2 = false;
                        led3 = false;
                        led4 = false;
                        status = WAIT;
                        ser.printf("ATH\r\n");
                        pc.printf("ATH\r\n");
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

            pc.printf("%s\r\n", buf);

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
    static const unsigned int AD_CR_START_MASK = 7 << 24;
    static const unsigned int AD_CR_START_NOW  = 1 << 24;
    static const unsigned int AD_CR_SELMASK    = 0xff;
    static const unsigned int AD_DR_DONE       = 1UL << 31;

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

    pc.printf("testa snurrskivan\r\n");
    while (true) {
        wait_ms(100);

        if (byteReceived == 0) {
            pc.printf("%i\n", getDialRawValue());
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

    pc.printf("START!\r\n");

    buzzer.period_us(250);
    buzzer = 0;

    ser.attach(&receivebyte);

    initADC((1<<19) | (1<<20));

    phone();

    return 0;
}
