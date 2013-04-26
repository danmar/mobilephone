#include "mbed.h"

Serial ser(p9,p10); // tx,rx
Serial pc(USBTX, USBRX); // tx, rx

DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);

DigitalIn button(p19);

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
    int answercounter = 0;

    int sind = 4;

    enum { WAIT, DIAL, TALK } status = WAIT;

    startPhone();

    while (1) {
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
        while (receivedMessage != NULL) {
            char buf[100];
            strcpy(buf, receivedMessage);
            receivedMessage = NULL;

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

static void initADC()
{
    NVIC_DisableIRQ(ADC_IRQn);

    // Power on the ADC
    LPC_SC->PCONP |= 1 << 12;

    // PCLK = 96MHz / 4 = 12MHz
    LPC_SC->PCLKSEL0 &= ~(3 << 24);

    // P15 = AD0.5
    LPC_PINCON->PINSEL3 |= (3UL << 30);

    LPC_ADC->ADCR = (1 << 8)     // CLKDIV=1
                    | (1 << 21);   // PDN
}

#define AD_CR_START_MASK   (7<<24)
#define AD_CR_START_NOW    (1<<24)
#define AD_CR_SELMASK      (0xff)
#define AD_DR_DONE         (1UL<<31)

static int readADC(int sel)
{
    LPC_GPIO1->FIOPIN = LED1;

    // Stop ADC
    LPC_ADC->ADCR &= ~(AD_CR_START_MASK | AD_CR_SELMASK);

    // Start conversion
    LPC_ADC->ADCR |= (AD_CR_START_NOW | (1 << sel));

    LPC_GPIO1->FIOPIN = LED2;

    // Wait for conversion done
    while ((LPC_ADC->ADGDR & AD_DR_DONE) == 0)
        ;

    LPC_GPIO1->FIOPIN = LED3;

    // Return result
    return ((LPC_ADC->ADGDR >> 4) & 0xfff);
}

void testaSnurrskivan()
{
    int count = 0;

    initADC();

    pc.printf("test");
    while (true) {
        wait_ms(500);

        if (byteReceived == 0) {
            int x = readADC(5);
            pc.printf("%i\n", x);
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

    phone();

    return 0;
}

