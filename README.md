
# mobilephone

Project to build a simple mobile phone (GSM). The mobile phone will handle
voice calls.

I will use off-the-shelf electronic components.

The software development and electronic components is sponsored by
Evidente.

## Progress

### Step 1 : GSM chip

I bought a SM5100B GSM chip, a simple evaluation board for this chip, and a
GSM antenna. The chip understands AT commands.

I bought a normal prepaid comviq sim card.

![alt text](http://github.com/danmar/mobilephone/raw/master/images/1-mini.png "Chip")

### Step 2 : Setup

I connected everything and tried to setup the chip for the GSM card. It was
quite straight forward.

By using AT commands from my PC I can send/receive SMS and dial/answer voice
calls (however I have no audio yet).

### Step 3 : Audio

The plan in this "step" is to connect a microphone and speaker. The datasheet
for SM5100B has reference designs for connecting microphone and speaker that
I will try to use.

According to the datasheet, the speaker can be connected directly to output
pins on the chip.

Before I had any parts, I wanted to "dry run" the audio. When a voice call is
made, it should be possible to measure something on the speaker output. I made
this connection:
  GSM chip speaker output => voltage divider => microphone input on my PC

![alt text](http://github.com/danmar/mobilephone/raw/master/images/3a-mini.png "Connection")

Oscilloscope view when I whizzle in the calling telephone

![alt text](http://github.com/danmar/mobilephone/raw/master/images/3b.png "Oscilloscope view")

Audio recording when I say "test, test". It's very faint.

http://github.com/danmar/mobilephone/raw/master/images/3.wma

### Step 4 : Connecting the microcontroller to the GSM chip

Here I have connected a MBED microcontroller to the UART0 port on the GSM chip:

![alt text](http://github.com/danmar/mobilephone/raw/master/images/4-mini.png "MBED")

To start with the MBED just sends "AT" commands and then receive the response "OK".

### Step 5 : Prototype box

I bought a small box. The microphone and speaker I bought was connected. The
reference circuits from the datasheets worked fine!

Internal view:

![alt text](http://github.com/danmar/mobilephone/raw/master/images/5b-mini.png "Internal")

External view:

![alt text](http://github.com/danmar/mobilephone/raw/master/images/5a-mini.png "External")

The dialer is both used to dial numbers and to answer / hang up. By spinning the dialer
all the way twice you answer and hang up.

It works when using a AC/DC power adaptor. But unfortunately it doesn't work well from a
battery. I use a normal 9V battery right now. This may be too weak.
